#ifndef RVM_METRICS_H
#define RVM_METRICS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t total_frames;
    uint32_t window_frames;
    uint32_t fps_x10;
    uint32_t window_ms;
} RVM_VideoMetrics;

void RVM_Metrics_Reset(uint32_t now_ms, uint32_t total_frames);
bool RVM_Metrics_Sample(uint32_t now_ms,
                        uint32_t total_frames,
                        uint32_t period_ms,
                        RVM_VideoMetrics *metrics);

#endif /* RVM_METRICS_H */
