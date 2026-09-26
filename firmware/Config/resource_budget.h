#ifndef RVM_RESOURCE_BUDGET_H
#define RVM_RESOURCE_BUDGET_H

#define RVM_MAX_FRAME_BUFFERS              3U
#define RVM_PREVIEW_RGB565_BYTES_PER_PIXEL 2U
#define RVM_PREVIEW_RGB565_FRAME_BYTES     (320UL * 240UL * 2UL)

/* DMA cannot access the STM32F407 CCM RAM. Frame storage must be assigned to
 * DMA-accessible internal SRAM or verified external memory. The Phase 1
 * baseline currently streams DCMI DMA directly to the LCD data register. */

#endif /* RVM_RESOURCE_BUDGET_H */
