#include "camera_service.h"

#include "camera_driver.h"

static RVM_CameraState s_state = RVM_CAMERA_STATE_UNINITIALIZED;
static uint16_t s_product_id;
static uint32_t s_capture_errors;

static RVM_CameraServiceStatus RVM_CameraService_MapDriverStatus(
    RVM_CameraDriverStatus status)
{
    if (status == RVM_CAMERA_DRIVER_OK)
    {
        return RVM_CAMERA_SERVICE_OK;
    }
    if (status == RVM_CAMERA_DRIVER_NOT_FOUND)
    {
        return RVM_CAMERA_SERVICE_NOT_FOUND;
    }
    if (status == RVM_CAMERA_DRIVER_INVALID_ARGUMENT)
    {
        return RVM_CAMERA_SERVICE_INVALID_ARGUMENT;
    }

    return RVM_CAMERA_SERVICE_DRIVER_ERROR;
}

RVM_CameraServiceStatus RVM_CameraService_Init(
    const RVM_CameraConfig *config)
{
    RVM_CameraDriverConfig driver_config;
    RVM_CameraDriverStatus driver_status;

    if (config == 0)
    {
        return RVM_CAMERA_SERVICE_INVALID_ARGUMENT;
    }

    driver_status = RVM_CameraDriver_Init(&s_product_id);
    if (driver_status != RVM_CAMERA_DRIVER_OK)
    {
        s_state = RVM_CAMERA_STATE_ERROR;
        ++s_capture_errors;
        return RVM_CameraService_MapDriverStatus(driver_status);
    }

    driver_config.width = config->width;
    driver_config.height = config->height;
    driver_config.lcd_x = config->lcd_x;
    driver_config.lcd_y = config->lcd_y;
    driver_config.lcd_scan_mode = config->lcd_scan_mode;
    driver_config.auto_focus = config->auto_focus;

    driver_status = RVM_CameraDriver_Configure(&driver_config);
    if (driver_status != RVM_CAMERA_DRIVER_OK)
    {
        s_state = RVM_CAMERA_STATE_ERROR;
        ++s_capture_errors;
        return RVM_CameraService_MapDriverStatus(driver_status);
    }

    s_state = RVM_CAMERA_STATE_IDLE;
    return RVM_CAMERA_SERVICE_OK;
}

RVM_CameraServiceStatus RVM_CameraService_Start(void)
{
    RVM_CameraDriverStatus status;

    if (s_state != RVM_CAMERA_STATE_IDLE)
    {
        return RVM_CAMERA_SERVICE_INVALID_STATE;
    }

    status = RVM_CameraDriver_Start();
    if (status != RVM_CAMERA_DRIVER_OK)
    {
        s_state = RVM_CAMERA_STATE_ERROR;
        ++s_capture_errors;
        return RVM_CameraService_MapDriverStatus(status);
    }

    s_state = RVM_CAMERA_STATE_STREAMING;
    return RVM_CAMERA_SERVICE_OK;
}

RVM_CameraServiceStatus RVM_CameraService_Stop(void)
{
    RVM_CameraDriverStatus status;

    if (s_state != RVM_CAMERA_STATE_STREAMING)
    {
        return RVM_CAMERA_SERVICE_INVALID_STATE;
    }

    status = RVM_CameraDriver_Stop();
    if (status != RVM_CAMERA_DRIVER_OK)
    {
        s_state = RVM_CAMERA_STATE_ERROR;
        ++s_capture_errors;
        return RVM_CameraService_MapDriverStatus(status);
    }

    s_state = RVM_CAMERA_STATE_IDLE;
    return RVM_CAMERA_SERVICE_OK;
}

RVM_CameraServiceStatus RVM_CameraService_Restart(
    const RVM_CameraConfig *config)
{
    RVM_CameraDriverConfig driver_config;
    RVM_CameraDriverStatus status;

    if ((config == 0) || (s_state != RVM_CAMERA_STATE_STREAMING))
    {
        return RVM_CAMERA_SERVICE_INVALID_STATE;
    }

    (void)RVM_CameraDriver_Stop();
    s_state = RVM_CAMERA_STATE_IDLE;

    driver_config.width = config->width;
    driver_config.height = config->height;
    driver_config.lcd_x = config->lcd_x;
    driver_config.lcd_y = config->lcd_y;
    driver_config.lcd_scan_mode = config->lcd_scan_mode;
    driver_config.auto_focus = config->auto_focus;
    status = RVM_CameraDriver_Configure(&driver_config);
    if (status != RVM_CAMERA_DRIVER_OK)
    {
        s_state = RVM_CAMERA_STATE_ERROR;
        ++s_capture_errors;
        return RVM_CameraService_MapDriverStatus(status);
    }

    return RVM_CameraService_Start();
}

RVM_CameraServiceStatus RVM_CameraService_ToggleFocus(void)
{
    return RVM_CameraService_MapDriverStatus(
        RVM_CameraDriver_ToggleFocus());
}

RVM_CameraState RVM_CameraService_GetState(void)
{
    return s_state;
}

RVM_CameraStats RVM_CameraService_GetStats(void)
{
    RVM_CameraStats stats;

    stats.state = s_state;
    stats.product_id = s_product_id;
    stats.captured_frames = RVM_CameraDriver_GetFrameCount();
    stats.dropped_frames = 0U;
    stats.capture_errors = s_capture_errors;
    return stats;
}
