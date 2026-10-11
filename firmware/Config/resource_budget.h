#ifndef RVM_RESOURCE_BUDGET_H
#define RVM_RESOURCE_BUDGET_H

#define RVM_EXT_SRAM_BASE                    0x6C000000UL
#define RVM_EXT_SRAM_SIZE_BYTES              0x00100000UL
#define RVM_EXT_SRAM_END                     (RVM_EXT_SRAM_BASE + RVM_EXT_SRAM_SIZE_BYTES)

#define RVM_LVGL_HEAP_BASE                   RVM_EXT_SRAM_BASE
#define RVM_LVGL_HEAP_SIZE_BYTES             (32UL * 1024UL)
#define RVM_LVGL_DRAW_BUFFER_BASE            0x6C010000UL
#define RVM_LVGL_DRAW_BUFFER_SIZE_BYTES      (800UL * 8UL * 2UL)

#define RVM_RGB565_BYTES_PER_PIXEL           2UL
#define RVM_FRAME_MAX_WIDTH                  320UL
#define RVM_FRAME_MAX_HEIGHT                 240UL
#define RVM_FRAME_SLOT_SIZE_BYTES            (RVM_FRAME_MAX_WIDTH * \
                                               RVM_FRAME_MAX_HEIGHT * \
                                               RVM_RGB565_BYTES_PER_PIXEL)
#define RVM_FRAME_BUFFER_BASE                0x6C020000UL
#define RVM_MAX_FRAME_BUFFERS                2U
#define RVM_FRAME_BUFFER_0_BASE              RVM_FRAME_BUFFER_BASE
#define RVM_FRAME_BUFFER_1_BASE              (RVM_FRAME_BUFFER_0_BASE + \
                                               RVM_FRAME_SLOT_SIZE_BYTES)
#define RVM_FRAME_BUFFER_END                 (RVM_FRAME_BUFFER_BASE + \
                                               (RVM_FRAME_SLOT_SIZE_BYTES * \
                                                RVM_MAX_FRAME_BUFFERS))

/* DMA cannot access STM32F407 CCM RAM. All DCMI buffers above are in the
 * verified external SRAM window and are word aligned for 32-bit DMA. */
#if (RVM_FRAME_BUFFER_END > RVM_EXT_SRAM_END)
#error "Camera frame slots exceed external SRAM"
#endif

#if (RVM_FRAME_BUFFER_BASE < (RVM_LVGL_DRAW_BUFFER_BASE + RVM_LVGL_DRAW_BUFFER_SIZE_BYTES))
#error "Camera frame slots overlap the LVGL draw buffer"
#endif

#endif /* RVM_RESOURCE_BUDGET_H */
