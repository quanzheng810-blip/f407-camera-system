#include "app_main.h"

#include "camera_service.h"
#include "display_service.h"
#include "feature_config.h"
#include "product_config.h"
#include "rvm_log.h"
#include "rvm_metrics.h"
#include "rvm_timebase.h"
#include "system_service.h"

static const RVM_CameraConfig s_camera_config = {
#if RVM_FEATURE_SERIAL_IMAGE
    RVM_SERIAL_IMAGE_WIDTH,
    RVM_SERIAL_IMAGE_HEIGHT,
#else
    RVM_CAMERA_PREVIEW_WIDTH,
    RVM_CAMERA_PREVIEW_HEIGHT,
#endif
    RVM_CAMERA_PREVIEW_LCD_X,
    RVM_CAMERA_PREVIEW_LCD_Y,
    RVM_LCD_SCAN_MODE,
    RVM_FEATURE_AUTO_FOCUS != 0
};

void RVM_App_Init(void)
{
    RVM_CameraServiceStatus camera_status;
    RVM_CameraStats camera_stats;

    RVM_SystemService_Init();
#if RVM_FEATURE_DISPLAY
    RVM_DisplayService_Init(RVM_LCD_SCAN_MODE);
#endif
    RVM_LOG_INFO("System", "%s boot", RVM_PRODUCT_NAME);

    camera_status = RVM_CameraService_Init(&s_camera_config);
    if (camera_status != RVM_CAMERA_SERVICE_OK)
    {
#if RVM_FEATURE_DISPLAY
        RVM_DisplayService_ShowCameraError();
#endif
        RVM_SystemService_SetState(RVM_SYSTEM_STATE_ERROR);
        RVM_LOG_ERROR("Camera", "OV5640 initialization failed: %d",
                      camera_status);
        return;
    }

    camera_stats = RVM_CameraService_GetStats();
#if RVM_FEATURE_DISPLAY
    RVM_DisplayService_ShowCameraDetected(camera_stats.product_id);
#endif
    RVM_LOG_INFO("Camera", "OV5640 detected: 0x%04X",
                 camera_stats.product_id);

#if RVM_FEATURE_DISPLAY
    RVM_DisplayService_PreparePreview(s_camera_config.lcd_x,
                                      s_camera_config.lcd_y,
                                      s_camera_config.width,
                                      s_camera_config.height,
                                      s_camera_config.lcd_scan_mode);
#endif

#if RVM_FEATURE_DISPLAY
    if (RVM_CameraService_Start() != RVM_CAMERA_SERVICE_OK)
    {
        RVM_SystemService_SetState(RVM_SYSTEM_STATE_ERROR);
        RVM_LOG_ERROR("Camera", "capture start failed");
        return;
    }

    camera_stats = RVM_CameraService_GetStats();
    RVM_Metrics_Reset(RVM_Timebase_GetMilliseconds(),
                      camera_stats.captured_frames);
    RVM_SystemService_SetState(RVM_SYSTEM_STATE_STREAMING);
    RVM_LOG_INFO("System", "streaming %ux%u RGB565",
                 s_camera_config.width,
                 s_camera_config.height);
#else
    RVM_SystemService_SetState(RVM_SYSTEM_STATE_READY);
    RVM_LOG_INFO("System", "headless snapshot mode %ux%u RGB565",
                 s_camera_config.width,
                 s_camera_config.height);
#endif
}

void RVM_App_Run(void)
{
#if RVM_FEATURE_DISPLAY
    RVM_SystemCommand command;
    RVM_CameraStats camera_stats;
    RVM_VideoMetrics metrics;
    uint32_t now_ms;

    if (RVM_SystemService_GetState() != RVM_SYSTEM_STATE_STREAMING)
    {
        return;
    }

    command = RVM_SystemService_PollCommand();
    if (command == RVM_SYSTEM_COMMAND_RESTART_STREAM)
    {
        RVM_DisplayService_PreparePreview(s_camera_config.lcd_x,
                                          s_camera_config.lcd_y,
                                          s_camera_config.width,
                                          s_camera_config.height,
                                          s_camera_config.lcd_scan_mode);
        if (RVM_CameraService_Restart(&s_camera_config) !=
            RVM_CAMERA_SERVICE_OK)
        {
            RVM_SystemService_SetState(RVM_SYSTEM_STATE_ERROR);
            RVM_LOG_ERROR("Camera", "stream restart failed");
        }
    }
    else if (command == RVM_SYSTEM_COMMAND_TOGGLE_FOCUS)
    {
        (void)RVM_CameraService_ToggleFocus();
    }

    camera_stats = RVM_CameraService_GetStats();
    now_ms = RVM_Timebase_GetMilliseconds();
    if (RVM_Metrics_Sample(now_ms,
                           camera_stats.captured_frames,
                           RVM_CAMERA_STATS_PERIOD_MS,
                           &metrics))
    {
        RVM_LOG_INFO("Video", "fps=%u.%u total=%lu errors=%lu",
                     metrics.fps_x10 / 10U,
                     metrics.fps_x10 % 10U,
                     metrics.total_frames,
                     camera_stats.capture_errors);
    }
#endif
}
