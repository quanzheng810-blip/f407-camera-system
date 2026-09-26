#include "stm32f4xx_it.h"

#include "bsp_SysTick.h"
#include "camera_driver.h"
#include "rvm_timebase.h"

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    for (;;)
    {
    }
}

void MemManage_Handler(void)
{
    for (;;)
    {
    }
}

void BusFault_Handler(void)
{
    for (;;)
    {
    }
}

void UsageFault_Handler(void)
{
    for (;;)
    {
    }
}

void DebugMon_Handler(void)
{
}

void SVC_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    TimingDelay_Decrement();
    RVM_Timebase_OnTickIrq();
}

void DCMI_IRQHandler(void)
{
    RVM_CameraDriver_OnFrameIrq();
}
