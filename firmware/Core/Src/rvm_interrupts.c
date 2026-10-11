#include "stm32f4xx_it.h"

#include "camera_driver.h"
#include "bsp_debug_usart.h"

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

void DCMI_IRQHandler(void)
{
    RVM_CameraDriver_OnFrameIrq();
}

void USART1_IRQHandler(void)
{
    Debug_USART_OnIrq();
}

void DMA2_Stream7_IRQHandler(void)
{
    Debug_USART_OnTxDmaIrq();
}
