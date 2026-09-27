#include "board_init.h"

#include "bsp_debug_usart.h"
#include "bsp_key.h"
#include "bsp_sram.h"

RVM_BoardStatus RVM_Board_Init(void)
{
    Debug_USART_Config();
    Key_GPIO_Config();
    FSMC_SRAM_Init();

    return RVM_BOARD_STATUS_OK;
}
