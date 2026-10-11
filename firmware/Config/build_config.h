#ifndef RVM_BUILD_CONFIG_H
#define RVM_BUILD_CONFIG_H

#include "feature_config.h"

#if !defined(RVM_BOARD_BATIANHU_V2)
#error "RVM_BOARD_BATIANHU_V2 must be defined by the Keil target"
#endif

#if RVM_FEATURE_DISPLAY && !defined(RVM_LCD_ILI9806G)
#error "The RVM F407 target requires RVM_LCD_ILI9806G"
#endif

#endif /* RVM_BUILD_CONFIG_H */
