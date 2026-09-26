#ifndef RVM_VIDEO_FRAME_H
#define RVM_VIDEO_FRAME_H

#include <stddef.h>
#include <stdint.h>

typedef enum
{
    VIDEO_FORMAT_RGB565 = 0,
    VIDEO_FORMAT_JPEG = 1
} VideoFormat;

typedef struct
{
    uint32_t frame_id;
    uint32_t timestamp_ms;
    uint16_t width;
    uint16_t height;
    VideoFormat format;
    size_t payload_length;
} VideoFrameMeta;

typedef struct
{
    uint16_t index;
    uint16_t generation;
} VideoBufferHandle;

#endif /* RVM_VIDEO_FRAME_H */
