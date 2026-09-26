#ifndef RVM_BOARD_CONFIG_H
#define RVM_BOARD_CONFIG_H

/* Single public entry point for board-level facts. Pin mappings used by the
 * imported Embedfire drivers are being migrated here without changing their
 * electrical configuration. */

#define RVM_BOARD_NAME              "Embedfire STM32F407 Batianhu V2"
#define RVM_BOARD_MCU_FAMILY        "STM32F407"

/* Phase 1 hardware baseline derived from the official Embedfire example.
 * Physical board verification is still required before acceptance. */
#define RVM_BOARD_CAMERA_ENABLED    1
#define RVM_BOARD_LCD_ENABLED       1
#define RVM_BOARD_ETHERNET_ENABLED  0
#define RVM_BOARD_TOUCH_ENABLED     0
#define RVM_BOARD_EXT_RAM_ENABLED   0

#define RVM_LCD_CONTROLLER_ILI9806G 1
#define RVM_LCD_NATIVE_WIDTH        480U
#define RVM_LCD_NATIVE_HEIGHT       800U

/* Hardware mappings verified from the official Embedfire Phase 1 example:
 * - OV5640 SCCB: PB8/PB9
 * - OV5640 PWDN/RESET: PC0/PF10
 * - DCMI sync: PB7/PA4/PA6
 * - DCMI data: PC6..PC9, PE4..PE6, PB6
 * - Debug UART: USART1 PA9/PA10 at 115200 baud
 * - Buttons: KEY1 PA0, KEY2 PC13
 * - LCD: FSMC Bank1 NOR/SRAM3, 16-bit bus, command 0x68000000,
 *   data 0x68000002
 *
 * Low-level StdPeriph symbols remain private to the driver headers during the
 * behavior-preserving refactor. New application code must not use them.
 */

#endif /* RVM_BOARD_CONFIG_H */
