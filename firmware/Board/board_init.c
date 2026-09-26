#include "board_init.h"

#include "bsp_debug_usart.h"
#include "bsp_key.h"
#include "bsp_SysTick.h"

RVM_BoardStatus RVM_Board_Init(void)
{
    Debug_USART_Config();
    SysTick_Init();
    Key_GPIO_Config();

    return RVM_BOARD_STATUS_OK;
}
