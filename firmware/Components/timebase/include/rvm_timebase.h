#ifndef RVM_TIMEBASE_H
#define RVM_TIMEBASE_H

#include <stdbool.h>
#include <stdint.h>

void RVM_Timebase_OnTickIrq(void);
uint32_t RVM_Timebase_GetMilliseconds(void);
bool RVM_Timebase_HasElapsed(uint32_t start_ms, uint32_t interval_ms);

#endif /* RVM_TIMEBASE_H */
