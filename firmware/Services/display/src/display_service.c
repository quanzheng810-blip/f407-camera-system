#include "display_service.h"

#include <stdio.h>

#include "display_driver.h"

void RVM_DisplayService_Init(uint8_t scan_mode)
{
    RVM_DisplayDriver_Init(scan_mode);
}

void RVM_DisplayService_ShowCameraDetected(uint16_t product_id)
{
    char text[64];

    (void)sprintf(text, "OV5640 ID: 0x%04X", product_id);
    RVM_DisplayDriver_ShowLine(0U, text);
}

void RVM_DisplayService_ShowCameraError(void)
{
    RVM_DisplayDriver_ShowText(10U, 10U, "OV5640 not detected. Check cable.");
}

void RVM_DisplayService_PreparePreview(uint16_t x,
                                       uint16_t y,
                                       uint16_t width,
                                       uint16_t height,
                                       uint8_t scan_mode)
{
    RVM_DisplayDriver_OpenPreview(x, y, width, height, scan_mode);
}
