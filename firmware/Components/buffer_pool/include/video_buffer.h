#ifndef RVM_VIDEO_BUFFER_H
#define RVM_VIDEO_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "video_frame.h"

typedef enum
{
    VIDEO_BUFFER_FREE = 0,
    VIDEO_BUFFER_CAPTURING,
    VIDEO_BUFFER_READY,
    VIDEO_BUFFER_DISPLAYING,
    VIDEO_BUFFER_SENDING,
    VIDEO_BUFFER_ERROR
} VideoBufferState;

typedef enum
{
    VIDEO_BUFFER_OK = 0,
    VIDEO_BUFFER_INVALID_ARGUMENT,
    VIDEO_BUFFER_UNAVAILABLE,
    VIDEO_BUFFER_INVALID_HANDLE,
    VIDEO_BUFFER_INVALID_TRANSITION
} VideoBufferStatus;

typedef struct
{
    uint8_t *data;
    size_t capacity;
    VideoFrameMeta meta;
    VideoBufferState state;
} VideoBufferView;

/* Fixed-pool ownership API. Implementation must not allocate frame memory. */
VideoBufferStatus VideoBuffer_AcquireForCapture(VideoBufferHandle *handle,
                                                VideoBufferView *view);
VideoBufferStatus VideoBuffer_MarkReady(VideoBufferHandle handle,
                                        const VideoFrameMeta *meta);
VideoBufferStatus VideoBuffer_AcquireReady(VideoBufferHandle *handle,
                                           VideoBufferView *view);
VideoBufferStatus VideoBuffer_Transition(VideoBufferHandle handle,
                                         VideoBufferState expected,
                                         VideoBufferState next);
VideoBufferStatus VideoBuffer_Release(VideoBufferHandle handle);
bool VideoBuffer_IsValid(VideoBufferHandle handle);

#endif /* RVM_VIDEO_BUFFER_H */
