#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

typedef SemaphoreHandle_t sys_sem_t;
typedef SemaphoreHandle_t sys_mutex_t;
typedef QueueHandle_t sys_mbox_t;
typedef TaskHandle_t sys_thread_t;
typedef UBaseType_t sys_prot_t;

#define SYS_SEM_NULL                 ((sys_sem_t)0)
#define SYS_MBOX_NULL                ((sys_mbox_t)0)

#endif /* LWIP_ARCH_SYS_ARCH_H */
