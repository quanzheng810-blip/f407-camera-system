#include "bsp_SysTick.h"

#include "FreeRTOS.h"
#include "task.h"

static __IO uint32_t s_timing_delay;

void SysTick_Init(void)
{
    /* FreeRTOS configures SysTick when the scheduler starts. */
}

void Delay_ms(__IO uint32_t milliseconds)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        vTaskDelay(pdMS_TO_TICKS(milliseconds));
        return;
    }

    while (milliseconds-- != 0U)
    {
        __IO uint32_t cycles = SystemCoreClock / 8000U;
        while (cycles-- != 0U)
        {
            __NOP();
        }
    }
}

void TimingDelay_Decrement(void)
{
    if (s_timing_delay != 0U)
    {
        --s_timing_delay;
    }
}
