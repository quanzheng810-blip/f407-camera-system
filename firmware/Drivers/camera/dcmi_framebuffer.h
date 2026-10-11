#ifndef RVM_DCMI_FRAMEBUFFER_H
#define RVM_DCMI_FRAMEBUFFER_H

#include <stdbool.h>
#include <stdint.h>

void RVM_DcmiFrame_Init(void);
bool RVM_DcmiFrame_Start(uint8_t *destination, uint32_t byte_count);
void RVM_DcmiFrame_Stop(void);
bool RVM_DcmiFrame_IsComplete(void);
bool RVM_DcmiFrame_HasError(void);
bool RVM_DcmiFrame_OnIrq(void);

#endif /* RVM_DCMI_FRAMEBUFFER_H */
