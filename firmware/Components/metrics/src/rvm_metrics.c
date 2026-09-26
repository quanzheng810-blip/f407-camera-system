#include "rvm_metrics.h"

static uint32_t s_last_time_ms;
static uint32_t s_last_frame_count;

void RVM_Metrics_Reset(uint32_t now_ms, uint32_t total_frames)
{
    s_last_time_ms = now_ms;
    s_last_frame_count = total_frames;
}

bool RVM_Metrics_Sample(uint32_t now_ms,
                        uint32_t total_frames,
                        uint32_t period_ms,
                        RVM_VideoMetrics *metrics)
{
    uint32_t elapsed_ms;
    uint32_t frame_delta;

    if ((metrics == 0) || (period_ms == 0U))
    {
        return false;
    }

    elapsed_ms = (uint32_t)(now_ms - s_last_time_ms);
    if (elapsed_ms < period_ms)
    {
        return false;
    }

    frame_delta = (uint32_t)(total_frames - s_last_frame_count);
    metrics->total_frames = total_frames;
    metrics->window_frames = frame_delta;
    metrics->window_ms = elapsed_ms;
    metrics->fps_x10 = (frame_delta * 10000UL) / elapsed_ms;

    s_last_time_ms = now_ms;
    s_last_frame_count = total_frames;
    return true;
}
