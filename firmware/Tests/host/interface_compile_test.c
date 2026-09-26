#include "camera_service.h"
#include "network_state.h"
#include "system_state.h"
#include "video_buffer.h"
#include "video_frame.h"

typedef char RVM_VideoBufferHandleMustBe4Bytes[
    (sizeof(VideoBufferHandle) == 4U) ? 1 : -1];

void RVM_InterfaceCompileTest(void)
{
    const RVM_CameraConfig camera_config = {
        .width = 320U,
        .height = 240U,
        .lcd_x = 270U,
        .lcd_y = 120U,
        .lcd_scan_mode = 5U,
        .auto_focus = true,
    };

    (void)camera_config;
}
