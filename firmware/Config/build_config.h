#ifndef RVM_BUILD_CONFIG_H
#define RVM_BUILD_CONFIG_H

#if !defined(RVM_BOARD_BATIANHU_V2)
#error "RVM_BOARD_BATIANHU_V2 must be defined by the Keil target"
#endif

#if !defined(RVM_LCD_ILI9806G)
#error "The Phase 1 target requires RVM_LCD_ILI9806G"
#endif

#endif /* RVM_BUILD_CONFIG_H */
