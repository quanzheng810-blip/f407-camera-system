#ifndef RVM_CAMERA_DRIVER_H
#define RVM_CAMERA_DRIVER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RVM_CAMERA_DRIVER_OK = 0,
    RVM_CAMERA_DRIVER_INVALID_ARGUMENT,
    RVM_CAMERA_DRIVER_NOT_FOUND,
    RVM_CAMERA_DRIVER_NOT_READY,
    RVM_CAMERA_DRIVER_ERROR
} RVM_CameraDriverStatus;

typedef struct
{
    uint16_t width;
    uint16_t height;
    uint16_t lcd_x;
    uint16_t lcd_y;
    uint8_t lcd_scan_mode;
    bool auto_focus;
} RVM_CameraDriverConfig;

RVM_CameraDriverStatus RVM_CameraDriver_Init(uint16_t *product_id);
RVM_CameraDriverStatus RVM_CameraDriver_Configure(
    const RVM_CameraDriverConfig *config);
RVM_CameraDriverStatus RVM_CameraDriver_Start(void);
RVM_CameraDriverStatus RVM_CameraDriver_Stop(void);
RVM_CameraDriverStatus RVM_CameraDriver_ToggleFocus(void);
bool RVM_CameraDriver_IsRunning(void);
uint32_t RVM_CameraDriver_GetFrameCount(void);
void RVM_CameraDriver_OnFrameIrq(void);

#endif /* RVM_CAMERA_DRIVER_H */

