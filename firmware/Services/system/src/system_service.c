#include "system_service.h"

#include "bsp_key.h"

static RVM_SystemState s_state = RVM_SYSTEM_STATE_BOOT;

void RVM_SystemService_Init(void)
{
    s_state = RVM_SYSTEM_STATE_INIT;
}

void RVM_SystemService_SetState(RVM_SystemState state)
{
    s_state = state;
}

RVM_SystemState RVM_SystemService_GetState(void)
{
    return s_state;
}

RVM_SystemCommand RVM_SystemService_PollCommand(void)
{
    if (Key_Scan(KEY1_GPIO_PORT, KEY1_PIN) == KEY_ON)
    {
        return RVM_SYSTEM_COMMAND_RESTART_STREAM;
    }

    if (Key_Scan(KEY2_GPIO_PORT, KEY2_PIN) == KEY_ON)
    {
        return RVM_SYSTEM_COMMAND_TOGGLE_FOCUS;
    }

    return RVM_SYSTEM_COMMAND_NONE;
}
