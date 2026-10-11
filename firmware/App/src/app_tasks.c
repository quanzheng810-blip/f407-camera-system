#include "app_tasks.h"

#include "app_main.h"
#include "camera_service.h"
#include "network_service.h"
#include "feature_config.h"
#include "rvm_log.h"
#include "serial_image_service.h"
#include "ui_service.h"

#include "FreeRTOS.h"
#include "task.h"

#define RVM_APP_TASK_STACK_WORDS  1024U
#define RVM_APP_TASK_PRIORITY     4U

static void RVM_AppTask(void *argument)
{
    (void)argument;

    RVM_App_Init();
#if RVM_FEATURE_LVGL
    RVM_UIService_Init();
#endif
#if RVM_FEATURE_NETWORK
    (void)RVM_NetworkService_Init();
#endif
#if RVM_FEATURE_SERIAL_IMAGE
    RVM_SerialImageService_Init();
#endif

    for (;;)
    {
        RVM_App_Run();
#if RVM_FEATURE_NETWORK
        RVM_NetworkService_Process();
#endif
#if RVM_FEATURE_LVGL
        RVM_UIService_Process();
#endif
#if RVM_FEATURE_SERIAL_IMAGE
        RVM_SerialImageService_Process();
#endif
        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}

bool RVM_AppTasks_Start(void)
{
    BaseType_t result;

    result = xTaskCreate(RVM_AppTask,
                         "RVM-App",
                         RVM_APP_TASK_STACK_WORDS,
                         0,
                         RVM_APP_TASK_PRIORITY,
                         0);
    return result == pdPASS;
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task,
                                   signed char *task_name)
{
    (void)task;
    (void)task_name;
    taskDISABLE_INTERRUPTS();
    for (;;)
    {
    }
}
