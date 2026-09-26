#ifndef RVM_SYSTEM_SERVICE_H
#define RVM_SYSTEM_SERVICE_H

#include "system_state.h"

typedef enum
{
    RVM_SYSTEM_COMMAND_NONE = 0,
    RVM_SYSTEM_COMMAND_RESTART_STREAM,
    RVM_SYSTEM_COMMAND_TOGGLE_FOCUS
} RVM_SystemCommand;

void RVM_SystemService_Init(void);
void RVM_SystemService_SetState(RVM_SystemState state);
RVM_SystemState RVM_SystemService_GetState(void);
RVM_SystemCommand RVM_SystemService_PollCommand(void);

#endif /* RVM_SYSTEM_SERVICE_H */
