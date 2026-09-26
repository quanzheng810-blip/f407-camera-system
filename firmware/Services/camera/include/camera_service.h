#ifndef RVM_CAMERA_SERVICE_H
#define RVM_CAMERA_SERVICE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    RVM_CAMERA_STATE_UNINITIALIZED = 0,
    RVM_CAMERA_STATE_IDLE,
    RVM_CAMERA_STATE_STREAMING,
    RVM_CAMERA_STATE_ERROR
} RVM_CameraState;

typedef enum
{
    RVM_CAMERA_SERVICE_OK = 0,
    RVM_CAMERA_SERVICE_INVALID_ARGUMENT,
    RVM_CAMERA_SERVICE_NOT_FOUND,
    RVM_CAMERA_SERVICE_INVALID_STATE,
    RVM_CAMERA_SERVICE_DRIVER_ERROR
} RVM_CameraServiceStatus;

typedef struct
{
    uint16_t width;
    uint16_t height;
    uint16_t lcd_x;
    uint16_t lcd_y;
    uint8_t lcd_scan_mode;
    bool auto_focus;
} RVM_CameraConfig;

typedef struct
{
    RVM_CameraState state;
    uint16_t product_id;
    uint32_t captured_frames;
    uint32_t dropped_frames;
    uint32_t capture_errors;
} RVM_CameraStats;

RVM_CameraServiceStatus RVM_CameraService_Init(
    const RVM_CameraConfig *config);
RVM_CameraServiceStatus RVM_CameraService_Start(void);
RVM_CameraServiceStatus RVM_CameraService_Stop(void);
RVM_CameraServiceStatus RVM_CameraService_Restart(
    const RVM_CameraConfig *config);
RVM_CameraServiceStatus RVM_CameraService_ToggleFocus(void);
RVM_CameraState RVM_CameraService_GetState(void);
RVM_CameraStats RVM_CameraService_GetStats(void);

#endif /* RVM_CAMERA_SERVICE_H */
