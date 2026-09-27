#include "lwip/opt.h"
#include "lwip/sys.h"

int errno;

uint32_t RVM_LwipRand(void)
{
    static uint32_t seed = 0x40787200U;

    seed = (seed * 1664525U) + 1013904223U + (uint32_t)xTaskGetTickCount();
    return seed;
}

static TickType_t RVM_SysArch_MillisecondsToTicks(u32_t milliseconds)
{
    TickType_t ticks;

    if (milliseconds == 0U)
    {
        return portMAX_DELAY;
    }

    ticks = pdMS_TO_TICKS(milliseconds);
    return (ticks == 0U) ? 1U : ticks;
}

void sys_init(void)
{
}

u32_t sys_now(void)
{
    return (u32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

u32_t sys_jiffies(void)
{
    return (u32_t)xTaskGetTickCount();
}

sys_prot_t sys_arch_protect(void)
{
    taskENTER_CRITICAL();
    return 0U;
}

void sys_arch_unprotect(sys_prot_t protection)
{
    (void)protection;
    taskEXIT_CRITICAL();
}

err_t sys_sem_new(sys_sem_t *semaphore, u8_t count)
{
    *semaphore = xSemaphoreCreateBinary();
    if (*semaphore == NULL)
    {
        return ERR_MEM;
    }
    if (count != 0U)
    {
        (void)xSemaphoreGive(*semaphore);
    }
    return ERR_OK;
}

void sys_sem_free(sys_sem_t *semaphore)
{
    vSemaphoreDelete(*semaphore);
    *semaphore = SYS_SEM_NULL;
}

void sys_sem_signal(sys_sem_t *semaphore)
{
    (void)xSemaphoreGive(*semaphore);
}

u32_t sys_arch_sem_wait(sys_sem_t *semaphore, u32_t timeout)
{
    TickType_t start = xTaskGetTickCount();

    if (xSemaphoreTake(*semaphore,
                       RVM_SysArch_MillisecondsToTicks(timeout)) != pdTRUE)
    {
        return SYS_ARCH_TIMEOUT;
    }
    return (u32_t)((xTaskGetTickCount() - start) * portTICK_PERIOD_MS);
}

int sys_sem_valid(sys_sem_t *semaphore)
{
    return *semaphore != SYS_SEM_NULL;
}

void sys_sem_set_invalid(sys_sem_t *semaphore)
{
    *semaphore = SYS_SEM_NULL;
}

err_t sys_mutex_new(sys_mutex_t *mutex)
{
    *mutex = xSemaphoreCreateMutex();
    return (*mutex == NULL) ? ERR_MEM : ERR_OK;
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    vSemaphoreDelete(*mutex);
    *mutex = NULL;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    (void)xSemaphoreTake(*mutex, portMAX_DELAY);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    (void)xSemaphoreGive(*mutex);
}

int sys_mutex_valid(sys_mutex_t *mutex)
{
    return *mutex != NULL;
}

void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    *mutex = NULL;
}

err_t sys_mbox_new(sys_mbox_t *mailbox, int size)
{
    *mailbox = xQueueCreate((UBaseType_t)size, sizeof(void *));
    return (*mailbox == NULL) ? ERR_MEM : ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mailbox)
{
    vQueueDelete(*mailbox);
    *mailbox = SYS_MBOX_NULL;
}

void sys_mbox_post(sys_mbox_t *mailbox, void *message)
{
    (void)xQueueSend(*mailbox, &message, portMAX_DELAY);
}

err_t sys_mbox_trypost(sys_mbox_t *mailbox, void *message)
{
    return (xQueueSend(*mailbox, &message, 0U) == pdPASS) ? ERR_OK : ERR_MEM;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mailbox, void *message)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    BaseType_t result;

    result = xQueueSendFromISR(*mailbox, &message, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
    return (result == pdPASS) ? ERR_OK : ERR_MEM;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mailbox, void **message, u32_t timeout)
{
    void *discarded_message;
    TickType_t start = xTaskGetTickCount();

    if (message == NULL)
    {
        message = &discarded_message;
    }
    if (xQueueReceive(*mailbox,
                      message,
                      RVM_SysArch_MillisecondsToTicks(timeout)) != pdTRUE)
    {
        *message = NULL;
        return SYS_ARCH_TIMEOUT;
    }
    return (u32_t)((xTaskGetTickCount() - start) * portTICK_PERIOD_MS);
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mailbox, void **message)
{
    void *discarded_message;

    if (message == NULL)
    {
        message = &discarded_message;
    }
    return (xQueueReceive(*mailbox, message, 0U) == pdTRUE) ?
           0U : SYS_MBOX_EMPTY;
}

int sys_mbox_valid(sys_mbox_t *mailbox)
{
    return *mailbox != SYS_MBOX_NULL;
}

void sys_mbox_set_invalid(sys_mbox_t *mailbox)
{
    *mailbox = SYS_MBOX_NULL;
}

sys_thread_t sys_thread_new(const char *name,
                            lwip_thread_fn thread,
                            void *argument,
                            int stack_size,
                            int priority)
{
    TaskHandle_t handle = NULL;

    if (xTaskCreate((TaskFunction_t)thread,
                    name,
                    (uint16_t)stack_size,
                    argument,
                    (UBaseType_t)priority,
                    &handle) != pdPASS)
    {
        return NULL;
    }
    return handle;
}
