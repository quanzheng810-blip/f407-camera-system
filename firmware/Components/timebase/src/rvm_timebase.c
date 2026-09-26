#include "rvm_timebase.h"

static volatile uint32_t s_milliseconds;

void RVM_Timebase_OnTickIrq(void)
{
    ++s_milliseconds;
}

uint32_t RVM_Timebase_GetMilliseconds(void)
{
    return s_milliseconds;
}

bool RVM_Timebase_HasElapsed(uint32_t start_ms, uint32_t interval_ms)
{
    return (uint32_t)(RVM_Timebase_GetMilliseconds() - start_ms) >= interval_ms;
}
