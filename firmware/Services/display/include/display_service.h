#ifndef RVM_DISPLAY_SERVICE_H
#define RVM_DISPLAY_SERVICE_H

#include <stdint.h>

void RVM_DisplayService_Init(uint8_t scan_mode);
void RVM_DisplayService_ShowCameraDetected(uint16_t product_id);
void RVM_DisplayService_ShowCameraError(void);
void RVM_DisplayService_PreparePreview(uint16_t x,
                                       uint16_t y,
                                       uint16_t width,
                                       uint16_t height,
                                       uint8_t scan_mode);

#endif /* RVM_DISPLAY_SERVICE_H */
