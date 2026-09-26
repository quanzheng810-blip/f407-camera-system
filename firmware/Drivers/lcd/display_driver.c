#include "display_driver.h"

#include "bsp_ili9806g_lcd.h"
#include "fonts.h"

void RVM_DisplayDriver_Init(uint8_t scan_mode)
{
    ILI9806G_Init();
    ILI9806G_GramScan(scan_mode);
    LCD_SetFont(&Font16x32);
    LCD_SetColors(RED, BLACK);
    RVM_DisplayDriver_Clear();
}

void RVM_DisplayDriver_Clear(void)
{
    ILI9806G_Clear(0U, 0U, LCD_X_LENGTH, LCD_Y_LENGTH);
}

void RVM_DisplayDriver_ShowLine(uint16_t line, const char *text)
{
    ILI9806G_DispStringLine_EN(line, (char *)text);
}

void RVM_DisplayDriver_ShowText(uint16_t x, uint16_t y, const char *text)
{
    ILI9806G_DispString_EN(x, y, (char *)text);
}

void RVM_DisplayDriver_OpenPreview(uint16_t x,
                                   uint16_t y,
                                   uint16_t width,
                                   uint16_t height,
                                   uint8_t scan_mode)
{
    ILI9806G_GramScan(scan_mode);
    RVM_DisplayDriver_Clear();
    ILI9806G_OpenWindow(x, y, width, height);
}

