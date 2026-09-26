#include "camera_driver.h"

#include "bsp_ov5640.h"
#include "ov5640_AF.h"

static bool s_initialized;
static bool s_running;
static bool s_focus_running;
static volatile uint32_t s_frame_count;

RVM_CameraDriverStatus RVM_CameraDriver_Init(uint16_t *product_id)
{
    OV5640_IDTypeDef id;

    if (product_id == 0)
    {
        return RVM_CAMERA_DRIVER_INVALID_ARGUMENT;
    }

    OV5640_HW_Init();
    OV5640_ReadID(&id);
    *product_id = (uint16_t)(((uint16_t)id.PIDH << 8U) | id.PIDL);

    if (id.PIDH != 0x56U)
    {
        return RVM_CAMERA_DRIVER_NOT_FOUND;
    }

    OV5640_Init();
    OV5640_RGB565_Default_Config();
    (void)OV5640_FOCUS_AD5820_Init();

    s_initialized = true;
    s_running = false;
    s_focus_running = false;
    s_frame_count = 0U;

    return RVM_CAMERA_DRIVER_OK;
}

RVM_CameraDriverStatus RVM_CameraDriver_Configure(
    const RVM_CameraDriverConfig *config)
{
    if (config == 0)
    {
        return RVM_CAMERA_DRIVER_INVALID_ARGUMENT;
    }

    if (!s_initialized)
    {
        return RVM_CAMERA_DRIVER_NOT_READY;
    }

    cam_mode.frame_rate = FRAME_RATE_15FPS;
    cam_mode.cam_isp_sx = 0U;
    cam_mode.cam_isp_sy = 0U;
    cam_mode.cam_isp_width = 1920U;
    cam_mode.cam_isp_height = 1080U;
    cam_mode.scaling = 1U;
    cam_mode.cam_out_sx = 16U;
    cam_mode.cam_out_sy = 4U;
    cam_mode.cam_out_width = config->width;
    cam_mode.cam_out_height = config->height;
    cam_mode.lcd_sx = config->lcd_x;
    cam_mode.lcd_sy = config->lcd_y;
    cam_mode.lcd_scan = config->lcd_scan_mode;
    cam_mode.light_mode = 0U;
    cam_mode.saturation = 0;
    cam_mode.brightness = 0;
    cam_mode.contrast = 0;
    cam_mode.effect = 0U;
    cam_mode.exposure = 0;
    cam_mode.auto_focus = config->auto_focus ? 1U : 0U;

    OV5640_USER_Config();

    if (config->auto_focus)
    {
        (void)OV5640_FOCUS_AD5820_Constant_Focus();
        s_focus_running = true;
    }

    return RVM_CAMERA_DRIVER_OK;
}

RVM_CameraDriverStatus RVM_CameraDriver_Start(void)
{
    if (!s_initialized)
    {
        return RVM_CAMERA_DRIVER_NOT_READY;
    }

    OV5640_Capture_Control(ENABLE);
    s_running = true;
    return RVM_CAMERA_DRIVER_OK;
}

RVM_CameraDriverStatus RVM_CameraDriver_Stop(void)
{
    if (!s_initialized)
    {
        return RVM_CAMERA_DRIVER_NOT_READY;
    }

    OV5640_Capture_Control(DISABLE);
    s_running = false;
    return RVM_CAMERA_DRIVER_OK;
}

RVM_CameraDriverStatus RVM_CameraDriver_ToggleFocus(void)
{
    if (!s_initialized)
    {
        return RVM_CAMERA_DRIVER_NOT_READY;
    }

    if (s_focus_running)
    {
        (void)OV5640_FOCUS_AD5820_Pause_Focus();
        s_focus_running = false;
    }
    else
    {
        (void)OV5640_FOCUS_AD5820_Constant_Focus();
        s_focus_running = true;
    }

    return RVM_CAMERA_DRIVER_OK;
}

bool RVM_CameraDriver_IsRunning(void)
{
    return s_running;
}

uint32_t RVM_CameraDriver_GetFrameCount(void)
{
    return s_frame_count;
}

void RVM_CameraDriver_OnFrameIrq(void)
{
    if (DCMI_GetITStatus(DCMI_IT_FRAME) == SET)
    {
        DCMI_ClearITPendingBit(DCMI_IT_FRAME);
        ++s_frame_count;
    }
}

