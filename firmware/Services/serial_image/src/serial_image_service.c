#include "serial_image_service.h"

#include <stdint.h>
#include <string.h>

#include "bsp_debug_usart.h"
#include "camera_service.h"
#include "product_config.h"
#include "resource_budget.h"
#include "rvm_log.h"
#include "rvm_timebase.h"
#include "serial_protocol.h"
#include "video_buffer.h"

#define RVM_SNAPSHOT_TIMEOUT_MS  2000UL
#define RVM_COMMAND_LENGTH       16U

typedef enum
{
    RVM_SERIAL_IMAGE_IDLE = 0,
    RVM_SERIAL_IMAGE_CAPTURING,
    RVM_SERIAL_IMAGE_TX_HEADER,
    RVM_SERIAL_IMAGE_TX_PAYLOAD
} RVM_SerialImageState;

static RVM_SerialImageState s_state;
static char s_command[RVM_COMMAND_LENGTH];
static uint8_t s_command_length;
static VideoBufferHandle s_buffer_handle;
static VideoBufferView s_buffer_view;
static uint8_t s_header[RVM_SERIAL_FRAME_HEADER_SIZE];
static uint32_t s_capture_started_ms;
static uint32_t s_payload_offset;
static uint16_t s_active_chunk;
static uint32_t s_frame_id;

static void RVM_SerialImage_Release(void)
{
    if (VideoBuffer_IsValid(s_buffer_handle))
    {
        (void)VideoBuffer_Release(s_buffer_handle);
    }
    s_buffer_handle.index = 0xFFFFU;
    s_buffer_handle.generation = 0U;
    s_state = RVM_SERIAL_IMAGE_IDLE;
}

static void RVM_SerialImage_Fail(const char *reason)
{
    if ((s_state == RVM_SERIAL_IMAGE_TX_HEADER) ||
        (s_state == RVM_SERIAL_IMAGE_TX_PAYLOAD))
    {
        Debug_USART_EndBinary();
    }
    RVM_SerialImage_Release();
    RVM_LOG_ERROR("SerialImage", "%s", reason);
}

static void RVM_SerialImage_StartSnapshot(void)
{
    uint32_t payload_length = (uint32_t)RVM_SERIAL_IMAGE_WIDTH *
                              (uint32_t)RVM_SERIAL_IMAGE_HEIGHT *
                              RVM_RGB565_BYTES_PER_PIXEL;

    if (s_state != RVM_SERIAL_IMAGE_IDLE)
    {
        RVM_LOG_WARN("SerialImage", "busy");
        return;
    }

    if (VideoBuffer_AcquireForCapture(&s_buffer_handle,
                                      &s_buffer_view) != VIDEO_BUFFER_OK)
    {
        RVM_LOG_WARN("SerialImage", "busy");
        return;
    }
    if (payload_length > s_buffer_view.capacity)
    {
        RVM_SerialImage_Release();
        RVM_LOG_ERROR("SerialImage", "frame exceeds buffer capacity");
        return;
    }

    if (RVM_CameraService_CaptureFrame(s_buffer_view.data,
                                       payload_length) !=
        RVM_CAMERA_SERVICE_OK)
    {
        RVM_SerialImage_Release();
        RVM_LOG_ERROR("SerialImage", "camera capture start failed");
        return;
    }

    s_capture_started_ms = RVM_Timebase_GetMilliseconds();
    s_state = RVM_SERIAL_IMAGE_CAPTURING;
    RVM_LOG_INFO("SerialImage", "capturing %ux%u RGB565",
                 RVM_SERIAL_IMAGE_WIDTH, RVM_SERIAL_IMAGE_HEIGHT);
}

static void RVM_SerialImage_HandleCommand(void)
{
    s_command[s_command_length] = '\0';
    if (strcmp(s_command, "SNAP") == 0)
    {
        RVM_SerialImage_StartSnapshot();
    }
    else if (s_command_length != 0U)
    {
        RVM_LOG_WARN("SerialImage", "unknown command: %s", s_command);
    }
    s_command_length = 0U;
}

static void RVM_SerialImage_ProcessRx(void)
{
    uint8_t byte;

    while (Debug_USART_ReadByte(&byte))
    {
        if (byte == '\n')
        {
            RVM_SerialImage_HandleCommand();
        }
        else if (byte != '\r')
        {
            if (s_command_length < (RVM_COMMAND_LENGTH - 1U))
            {
                s_command[s_command_length++] = (char)byte;
            }
            else
            {
                s_command_length = 0U;
                RVM_LOG_WARN("SerialImage", "command too long");
            }
        }
    }
}

static void RVM_SerialImage_SwapRgb565Bytes(uint8_t *data,
                                            uint32_t length)
{
#if RVM_CAMERA_RGB565_SWAP_BYTES
    uint32_t index;
    uint8_t high;

    for (index = 0U; index < length; index += 2U)
    {
        high = data[index];
        data[index] = data[index + 1U];
        data[index + 1U] = high;
    }
#else
    (void)data;
    (void)length;
#endif
}

static bool RVM_SerialImage_StartPayloadChunk(void)
{
    uint32_t remaining = (uint32_t)s_buffer_view.meta.payload_length -
                         s_payload_offset;
    uint32_t chunk = remaining;

    if (chunk > RVM_SERIAL_TX_DMA_CHUNK_SIZE)
    {
        chunk = RVM_SERIAL_TX_DMA_CHUNK_SIZE;
    }
    if (chunk > 0xFFFFU)
    {
        chunk = 0xFFFFU;
    }

    s_active_chunk = (uint16_t)chunk;
    return Debug_USART_StartTxDma(s_buffer_view.data + s_payload_offset,
                                  s_active_chunk);
}

static void RVM_SerialImage_PrepareFrame(void)
{
    VideoFrameMeta meta;
    uint32_t crc;

    meta.frame_id = ++s_frame_id;
    meta.timestamp_ms = RVM_Timebase_GetMilliseconds();
    meta.width = RVM_SERIAL_IMAGE_WIDTH;
    meta.height = RVM_SERIAL_IMAGE_HEIGHT;
    meta.format = VIDEO_FORMAT_RGB565;
    meta.payload_length = (size_t)meta.width * (size_t)meta.height *
                          RVM_RGB565_BYTES_PER_PIXEL;

    RVM_SerialImage_SwapRgb565Bytes(s_buffer_view.data,
                                    (uint32_t)meta.payload_length);
    if (VideoBuffer_MarkReady(s_buffer_handle, &meta) != VIDEO_BUFFER_OK ||
        VideoBuffer_Transition(s_buffer_handle, VIDEO_BUFFER_READY,
                               VIDEO_BUFFER_SENDING) != VIDEO_BUFFER_OK)
    {
        RVM_SerialImage_Fail("buffer state error");
        return;
    }
    s_buffer_view.meta = meta;

    crc = RVM_SerialProtocol_Crc32(s_buffer_view.data,
                                   s_buffer_view.meta.payload_length);
    RVM_SerialProtocol_BuildHeader(s_header,
                                   RVM_SERIAL_FORMAT_RGB565_LE,
                                   meta.width,
                                   meta.height,
                                   (uint32_t)meta.payload_length,
                                   crc);

    if (!Debug_USART_BeginBinary())
    {
        RVM_SerialImage_Fail("USART binary channel busy");
        return;
    }
    if (!Debug_USART_StartTxDma(s_header, RVM_SERIAL_FRAME_HEADER_SIZE))
    {
        Debug_USART_EndBinary();
        RVM_SerialImage_Release();
        RVM_LOG_ERROR("SerialImage", "header DMA start failed");
        return;
    }
    s_payload_offset = 0U;
    s_active_chunk = 0U;
    s_state = RVM_SERIAL_IMAGE_TX_HEADER;
}

void RVM_SerialImageService_Init(void)
{
    s_state = RVM_SERIAL_IMAGE_IDLE;
    s_command_length = 0U;
    s_buffer_handle.index = 0xFFFFU;
    s_buffer_handle.generation = 0U;
    RVM_LOG_INFO("SerialImage", "ready, SNAP at %lu baud",
                 (uint32_t)DEBUG_USART_BAUDRATE);
}

void RVM_SerialImageService_Process(void)
{
    RVM_SerialImage_ProcessRx();

    if (((s_state == RVM_SERIAL_IMAGE_TX_HEADER) ||
         (s_state == RVM_SERIAL_IMAGE_TX_PAYLOAD)) &&
        Debug_USART_TakeTxDmaError())
    {
        RVM_SerialImage_Fail("USART TX DMA error");
        return;
    }

    if (s_state == RVM_SERIAL_IMAGE_CAPTURING)
    {
        if (RVM_CameraService_IsFrameReady())
        {
            RVM_SerialImage_PrepareFrame();
        }
        else if (RVM_CameraService_GetState() == RVM_CAMERA_STATE_ERROR)
        {
            RVM_SerialImage_Fail("camera DMA error");
        }
        else if (RVM_Timebase_HasElapsed(s_capture_started_ms,
                                         RVM_SNAPSHOT_TIMEOUT_MS))
        {
            (void)RVM_CameraService_AbortCapture();
            RVM_SerialImage_Fail("camera capture timeout");
        }
    }
    else if ((s_state == RVM_SERIAL_IMAGE_TX_HEADER) &&
             Debug_USART_TakeTxDmaComplete())
    {
        if (!RVM_SerialImage_StartPayloadChunk())
        {
            RVM_SerialImage_Fail("payload DMA start failed");
        }
        else
        {
            s_state = RVM_SERIAL_IMAGE_TX_PAYLOAD;
        }
    }
    else if ((s_state == RVM_SERIAL_IMAGE_TX_PAYLOAD) &&
             Debug_USART_TakeTxDmaComplete())
    {
        s_payload_offset += s_active_chunk;
        if (s_payload_offset >= s_buffer_view.meta.payload_length)
        {
            Debug_USART_EndBinary();
            RVM_SerialImage_Release();
            RVM_LOG_INFO("SerialImage", "frame sent: %ux%u, %lu bytes",
                         RVM_SERIAL_IMAGE_WIDTH,
                         RVM_SERIAL_IMAGE_HEIGHT,
                         (uint32_t)(RVM_SERIAL_IMAGE_WIDTH *
                                    RVM_SERIAL_IMAGE_HEIGHT * 2UL));
        }
        else if (!RVM_SerialImage_StartPayloadChunk())
        {
            RVM_SerialImage_Fail("payload DMA continuation failed");
        }
    }
}

bool RVM_SerialImageService_IsBusy(void)
{
    return s_state != RVM_SERIAL_IMAGE_IDLE;
}
