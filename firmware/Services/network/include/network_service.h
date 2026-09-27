#ifndef RVM_NETWORK_SERVICE_H
#define RVM_NETWORK_SERVICE_H

#include <stdbool.h>

#include "network_state.h"

bool RVM_NetworkService_Init(void);
void RVM_NetworkService_Process(void);
NetworkState RVM_NetworkService_GetState(void);

#endif /* RVM_NETWORK_SERVICE_H */
