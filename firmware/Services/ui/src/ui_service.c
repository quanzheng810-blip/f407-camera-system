#include "ui_service.h"

#include <stdio.h>

#include "camera_service.h"
#include "lv_port_disp.h"
#include "lvgl.h"
#include "rvm_timebase.h"

static lv_obj_t *s_camera_label;
static lv_obj_t *s_network_label;
static uint32_t s_last_update_ms;

void RVM_UIService_Init(void)
{
    lv_obj_t *title;
    lv_obj_t *stack_label;

    lv_init();
    RVM_LVPort_DisplayInit();

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x101820), 0);

    title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "RVM Phase 2");
    lv_obj_set_style_text_color(title, lv_color_hex(0x35C2FF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    stack_label = lv_label_create(lv_scr_act());
    lv_label_set_text(stack_label, "FreeRTOS + lwIP + LVGL");
    lv_obj_set_style_text_color(stack_label, lv_color_hex(0xD7E3EA), 0);
    lv_obj_align(stack_label, LV_ALIGN_TOP_MID, 0, 42);

    s_camera_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(s_camera_label, lv_color_hex(0x80FFB0), 0);
    lv_obj_align(s_camera_label, LV_ALIGN_LEFT_MID, 18, 0);

    s_network_label = lv_label_create(lv_scr_act());
    lv_label_set_text(s_network_label, "Network\ninitializing");
    lv_obj_set_style_text_color(s_network_label, lv_color_hex(0xFFD166), 0);
    lv_obj_align(s_network_label, LV_ALIGN_RIGHT_MID, -18, 0);

    s_last_update_ms = 0U;
}

void RVM_UIService_Process(void)
{
    uint32_t now_ms;

    (void)lv_timer_handler();
    now_ms = RVM_Timebase_GetMilliseconds();
    if ((uint32_t)(now_ms - s_last_update_ms) >= 500U)
    {
        char text[64];
        RVM_CameraStats stats = RVM_CameraService_GetStats();

        (void)sprintf(text,
                      "Camera\nframes: %lu\nerrors: %lu",
                      stats.captured_frames,
                      stats.capture_errors);
        lv_label_set_text(s_camera_label, text);
        s_last_update_ms = now_ms;
    }
}

void RVM_UIService_SetNetworkStatus(const char *status)
{
    if ((s_network_label != 0) && (status != 0))
    {
        lv_label_set_text(s_network_label, status);
    }
}
