#include "video_buffer.h"

#include "resource_budget.h"

typedef struct
{
    uint8_t *data;
    size_t capacity;
    VideoFrameMeta meta;
    VideoBufferState state;
    uint16_t generation;
} VideoBufferEntry;

static VideoBufferEntry s_buffers[RVM_MAX_FRAME_BUFFERS] = {
    {(uint8_t *)RVM_FRAME_BUFFER_0_BASE, RVM_FRAME_SLOT_SIZE_BYTES,
     {0U, 0U, 0U, 0U, VIDEO_FORMAT_RGB565, 0U}, VIDEO_BUFFER_FREE, 0U},
    {(uint8_t *)RVM_FRAME_BUFFER_1_BASE, RVM_FRAME_SLOT_SIZE_BYTES,
     {0U, 0U, 0U, 0U, VIDEO_FORMAT_RGB565, 0U}, VIDEO_BUFFER_FREE, 0U}
};

static VideoBufferEntry *VideoBuffer_Get(VideoBufferHandle handle)
{
    VideoBufferEntry *entry;

    if (handle.index >= RVM_MAX_FRAME_BUFFERS)
    {
        return 0;
    }

    entry = &s_buffers[handle.index];
    if ((handle.generation == 0U) ||
        (entry->generation != handle.generation))
    {
        return 0;
    }
    return entry;
}

static void VideoBuffer_FillView(const VideoBufferEntry *entry,
                                 VideoBufferView *view)
{
    view->data = entry->data;
    view->capacity = entry->capacity;
    view->meta = entry->meta;
    view->state = entry->state;
}

VideoBufferStatus VideoBuffer_AcquireForCapture(VideoBufferHandle *handle,
                                                VideoBufferView *view)
{
    uint16_t index;
    VideoBufferEntry *entry;

    if ((handle == 0) || (view == 0))
    {
        return VIDEO_BUFFER_INVALID_ARGUMENT;
    }

    for (index = 0U; index < RVM_MAX_FRAME_BUFFERS; ++index)
    {
        entry = &s_buffers[index];
        if (entry->state == VIDEO_BUFFER_FREE)
        {
            ++entry->generation;
            if (entry->generation == 0U)
            {
                entry->generation = 1U;
            }
            entry->state = VIDEO_BUFFER_CAPTURING;
            handle->index = index;
            handle->generation = entry->generation;
            VideoBuffer_FillView(entry, view);
            return VIDEO_BUFFER_OK;
        }
    }

    return VIDEO_BUFFER_UNAVAILABLE;
}

VideoBufferStatus VideoBuffer_MarkReady(VideoBufferHandle handle,
                                        const VideoFrameMeta *meta)
{
    VideoBufferEntry *entry = VideoBuffer_Get(handle);

    if (meta == 0)
    {
        return VIDEO_BUFFER_INVALID_ARGUMENT;
    }
    if (entry == 0)
    {
        return VIDEO_BUFFER_INVALID_HANDLE;
    }
    if ((entry->state != VIDEO_BUFFER_CAPTURING) ||
        (meta->payload_length > entry->capacity))
    {
        return VIDEO_BUFFER_INVALID_TRANSITION;
    }

    entry->meta = *meta;
    entry->state = VIDEO_BUFFER_READY;
    return VIDEO_BUFFER_OK;
}

VideoBufferStatus VideoBuffer_AcquireReady(VideoBufferHandle *handle,
                                           VideoBufferView *view)
{
    uint16_t index;
    VideoBufferEntry *entry;

    if ((handle == 0) || (view == 0))
    {
        return VIDEO_BUFFER_INVALID_ARGUMENT;
    }

    for (index = 0U; index < RVM_MAX_FRAME_BUFFERS; ++index)
    {
        entry = &s_buffers[index];
        if (entry->state == VIDEO_BUFFER_READY)
        {
            handle->index = index;
            handle->generation = entry->generation;
            VideoBuffer_FillView(entry, view);
            return VIDEO_BUFFER_OK;
        }
    }
    return VIDEO_BUFFER_UNAVAILABLE;
}

VideoBufferStatus VideoBuffer_Transition(VideoBufferHandle handle,
                                         VideoBufferState expected,
                                         VideoBufferState next)
{
    VideoBufferEntry *entry = VideoBuffer_Get(handle);

    if (entry == 0)
    {
        return VIDEO_BUFFER_INVALID_HANDLE;
    }
    if (entry->state != expected)
    {
        return VIDEO_BUFFER_INVALID_TRANSITION;
    }

    entry->state = next;
    return VIDEO_BUFFER_OK;
}

VideoBufferStatus VideoBuffer_Release(VideoBufferHandle handle)
{
    VideoBufferEntry *entry = VideoBuffer_Get(handle);

    if (entry == 0)
    {
        return VIDEO_BUFFER_INVALID_HANDLE;
    }

    entry->state = VIDEO_BUFFER_FREE;
    entry->meta.payload_length = 0U;
    return VIDEO_BUFFER_OK;
}

bool VideoBuffer_IsValid(VideoBufferHandle handle)
{
    return VideoBuffer_Get(handle) != 0;
}
