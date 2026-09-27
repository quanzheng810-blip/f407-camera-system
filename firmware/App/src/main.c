#include "app_tasks.h"
#include "board_init.h"

#include "FreeRTOS.h"
#include "task.h"

int main(void)
{
    if (RVM_Board_Init() != RVM_BOARD_STATUS_OK)
    {
        for (;;)
        {
        }
    }

    if (!RVM_AppTasks_Start())
    {
        for (;;)
        {
        }
    }

    vTaskStartScheduler();

    for (;;)
    {
    }
}
