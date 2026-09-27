#include "lv_port_disp.h"

#include "bsp_ili9806g_lcd.h"
#include "camera_driver.h"
#include "lvgl.h"
#include "product_config.h"

#define RVM_LVGL_BUFFER_LINES  8U
#define RVM_LVGL_DRAW_BUFFER   ((lv_color_t *)0x6C010000U)

static lv_disp_draw_buf_t s_draw_buffer;
static lv_disp_drv_t s_display_driver;

static void RVM_LVPort_Flush(lv_disp_drv_t *driver,
                             const lv_area_t *area,
                             lv_color_t *colors)
{
    uint32_t pixel_count;
    volatile uint16_t *const lcd_data =
        (volatile uint16_t *)FSMC_Addr_ILI9806G_DATA;
    bool resume_camera;

    resume_camera = RVM_CameraDriver_IsRunning();
    if (resume_camera)
    {
        (void)RVM_CameraDriver_Stop();
    }

    ILI9806G_OpenWindow((uint16_t)area->x1,
                        (uint16_t)area->y1,
                        (uint16_t)(area->x2 - area->x1 + 1),
                        (uint16_t)(area->y2 - area->y1 + 1));

    pixel_count = (uint32_t)(area->x2 - area->x1 + 1) *
                  (uint32_t)(area->y2 - area->y1 + 1);
    while (pixel_count-- != 0U)
    {
        *lcd_data = colors->full;
        ++colors;
    }

    if (resume_camera)
    {
        ILI9806G_OpenWindow(RVM_CAMERA_PREVIEW_LCD_X,
                            RVM_CAMERA_PREVIEW_LCD_Y,
                            RVM_CAMERA_PREVIEW_WIDTH,
                            RVM_CAMERA_PREVIEW_HEIGHT);
        (void)RVM_CameraDriver_Start();
    }

    lv_disp_flush_ready(driver);
}

void RVM_LVPort_DisplayInit(void)
{
    lv_disp_draw_buf_init(&s_draw_buffer,
                          RVM_LVGL_DRAW_BUFFER,
                          0,
                          RVM_DISPLAY_WIDTH * RVM_LVGL_BUFFER_LINES);

    lv_disp_drv_init(&s_display_driver);
    s_display_driver.hor_res = RVM_DISPLAY_WIDTH;
    s_display_driver.ver_res = RVM_DISPLAY_HEIGHT;
    s_display_driver.flush_cb = RVM_LVPort_Flush;
    s_display_driver.draw_buf = &s_draw_buffer;
    (void)lv_disp_drv_register(&s_display_driver);
}
