#ifndef RVM_DISPLAY_DRIVER_H
#define RVM_DISPLAY_DRIVER_H

#include <stdint.h>

void RVM_DisplayDriver_Init(uint8_t scan_mode);
void RVM_DisplayDriver_Clear(void);
void RVM_DisplayDriver_ShowLine(uint16_t line, const char *text);
void RVM_DisplayDriver_ShowText(uint16_t x, uint16_t y, const char *text);
void RVM_DisplayDriver_OpenPreview(uint16_t x,
                                   uint16_t y,
                                   uint16_t width,
                                   uint16_t height,
                                   uint8_t scan_mode);

#endif /* RVM_DISPLAY_DRIVER_H */

