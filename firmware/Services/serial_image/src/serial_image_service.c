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

#if RVM_SERIAL_IMAGE_USE_JPEG
#define RVM_SERIAL_ACTIVE_FORMAT    VIDEO_FORMAT_JPEG
#define RVM_SERIAL_PROTOCOL_FORMAT  RVM_SERIAL_FORMAT_JPEG
#define RVM_SERIAL_FORMAT_NAME      "JPEG"
#else
#define RVM_SERIAL_ACTIVE_FORMAT    VIDEO_FORMAT_RGB565
#define RVM_SERIAL_PROTOCOL_FORMAT  RVM_SERIAL_FORMAT_RGB565_LE
#define RVM_SERIAL_FORMAT_NAME      "RGB565"
#endif

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
static bool s_stream_enabled;

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
    s_stream_enabled = false;
    RVM_SerialImage_Release();
    RVM_LOG_ERROR("SerialImage", "%s", reason);
}

static bool RVM_SerialImage_StartSnapshot(void)
{
    uint32_t payload_length = (uint32_t)RVM_SERIAL_IMAGE_WIDTH *
                              (uint32_t)RVM_SERIAL_IMAGE_HEIGHT *
                              RVM_RGB565_BYTES_PER_PIXEL;
    uint32_t capture_capacity = payload_length;

#if RVM_SERIAL_IMAGE_USE_JPEG
    capture_capacity = RVM_JPEG_CAPTURE_CAPACITY_BYTES;
#endif

    if (s_state != RVM_SERIAL_IMAGE_IDLE)
    {
        RVM_LOG_WARN("SerialImage", "busy");
        return false;
    }

    if (VideoBuffer_AcquireForCapture(&s_buffer_handle,
                                      &s_buffer_view) != VIDEO_BUFFER_OK)
    {
        RVM_LOG_WARN("SerialImage", "busy");
        return false;
    }
    if (capture_capacity > s_buffer_view.capacity)
    {
        RVM_SerialImage_Release();
        RVM_LOG_ERROR("SerialImage", "frame exceeds buffer capacity");
        return false;
    }

    if (RVM_CameraService_CaptureFrame(s_buffer_view.data,
                                       capture_capacity) !=
        RVM_CAMERA_SERVICE_OK)
    {
        RVM_SerialImage_Release();
        RVM_LOG_ERROR("SerialImage", "camera capture start failed");
        return false;
    }

    s_capture_started_ms = RVM_Timebase_GetMilliseconds();
    s_state = RVM_SERIAL_IMAGE_CAPTURING;
    RVM_LOG_INFO("SerialImage", "capturing %ux%u %s",
                 RVM_SERIAL_IMAGE_WIDTH,
                 RVM_SERIAL_IMAGE_HEIGHT,
                 RVM_SERIAL_FORMAT_NAME);
    return true;
}

static void RVM_SerialImage_HandleCommand(void)
{
    s_command[s_command_length] = '\0';
    if (strcmp(s_command, "SNAP") == 0)
    {
        if (s_stream_enabled)
        {
            RVM_LOG_WARN("SerialImage", "busy: stream active");
        }
        else
        {
            (void)RVM_SerialImage_StartSnapshot();
        }
    }
    else if (strcmp(s_command, "STREAM ON") == 0)
    {
        if (!s_stream_enabled)
        {
            s_stream_enabled = true;
            RVM_LOG_INFO("SerialImage", "stream enabled");
            if ((s_state == RVM_SERIAL_IMAGE_IDLE) &&
                (!RVM_SerialImage_StartSnapshot()))
            {
                s_stream_enabled = false;
            }
        }
    }
    else if (strcmp(s_command, "STREAM OFF") == 0)
    {
        if (s_stream_enabled)
        {
            s_stream_enabled = false;
            RVM_LOG_INFO("SerialImage", "stream stopping");
        }
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

#if !RVM_SERIAL_IMAGE_USE_JPEG
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
#endif

#if RVM_SERIAL_IMAGE_USE_JPEG
static bool RVM_SerialImage_NormalizeJpeg(uint8_t *data,
                                          uint32_t captured_length,
                                          uint32_t *payload_length)
{
    uint32_t start;
    uint32_t end;

    if ((data == 0) || (payload_length == 0) || (captured_length < 4U))
    {
        return false;
    }

    start = 0U;
    while ((start + 1U) < captured_length)
    {
        if ((data[start] == 0xFFU) && (data[start + 1U] == 0xD8U))
        {
            break;
        }
        ++start;
    }
    if ((start + 1U) >= captured_length)
    {
        return false;
    }

    end = start + 2U;
    while ((end + 1U) < captured_length)
    {
        if ((data[end] == 0xFFU) && (data[end + 1U] == 0xD9U))
        {
            end += 2U;
            if (start != 0U)
            {
                memmove(data, data + start, end - start);
            }
            *payload_length = end - start;
            return true;
        }
        ++end;
    }
    return false;
}
#endif

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
    uint32_t captured_length;
    uint32_t payload_length;

    meta.frame_id = ++s_frame_id;
    meta.timestamp_ms = RVM_Timebase_GetMilliseconds();
    meta.width = RVM_SERIAL_IMAGE_WIDTH;
    meta.height = RVM_SERIAL_IMAGE_HEIGHT;
    meta.format = RVM_SERIAL_ACTIVE_FORMAT;

#if RVM_SERIAL_IMAGE_USE_JPEG
    captured_length = RVM_CameraService_GetCapturedBytes();
    if (!RVM_SerialImage_NormalizeJpeg(s_buffer_view.data,
                                       captured_length,
                                       &payload_length))
    {
        RVM_SerialImage_Fail("invalid JPEG markers");
        return;
    }
    meta.payload_length = payload_length;
#else
    captured_length = RVM_CameraService_GetCapturedBytes();
    payload_length = (uint32_t)meta.width * (uint32_t)meta.height *
                     RVM_RGB565_BYTES_PER_PIXEL;
    if (captured_length < payload_length)
    {
        RVM_SerialImage_Fail("incomplete RGB565 frame");
        return;
    }
    meta.payload_length = payload_length;
    RVM_SerialImage_SwapRgb565Bytes(s_buffer_view.data, payload_length);
#endif
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
                                   RVM_SERIAL_PROTOCOL_FORMAT,
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
    s_stream_enabled = false;
    s_buffer_handle.index = 0xFFFFU;
    s_buffer_handle.generation = 0U;
    RVM_LOG_INFO("SerialImage", "ready, SNAP/STREAM %s at %lu baud",
                 RVM_SERIAL_FORMAT_NAME,
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
            uint32_t payload_length = s_buffer_view.meta.payload_length;

            Debug_USART_EndBinary();
            RVM_SerialImage_Release();
            RVM_LOG_INFO("SerialImage", "frame sent: %ux%u %s, %lu bytes",
                         RVM_SERIAL_IMAGE_WIDTH,
                         RVM_SERIAL_IMAGE_HEIGHT,
                         RVM_SERIAL_FORMAT_NAME,
                         payload_length);
            if (s_stream_enabled && (!RVM_SerialImage_StartSnapshot()))
            {
                s_stream_enabled = false;
                RVM_LOG_ERROR("SerialImage", "stream restart failed");
            }
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
