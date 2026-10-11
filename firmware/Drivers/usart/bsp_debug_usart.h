#ifndef RVM_BSP_DEBUG_USART_H
#define RVM_BSP_DEBUG_USART_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "feature_config.h"
#include "product_config.h"
#include "stm32f4xx.h"

#define DEBUG_USART                     USART1
#define DEBUG_USART_CLK                 RCC_APB2Periph_USART1
#if RVM_FEATURE_SERIAL_IMAGE
#define DEBUG_USART_BAUDRATE            RVM_SERIAL_IMAGE_BAUD_RATE
#else
#define DEBUG_USART_BAUDRATE            RVM_SERIAL_LOG_BAUD_RATE
#endif

#define DEBUG_USART_RX_GPIO_PORT        GPIOA
#define DEBUG_USART_RX_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define DEBUG_USART_RX_PIN              GPIO_Pin_10
#define DEBUG_USART_RX_AF               GPIO_AF_USART1
#define DEBUG_USART_RX_SOURCE           GPIO_PinSource10

#define DEBUG_USART_TX_GPIO_PORT        GPIOA
#define DEBUG_USART_TX_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define DEBUG_USART_TX_PIN              GPIO_Pin_9
#define DEBUG_USART_TX_AF               GPIO_AF_USART1
#define DEBUG_USART_TX_SOURCE           GPIO_PinSource9

void Debug_USART_Config(void);
bool Debug_USART_ReadByte(uint8_t *byte);

bool Debug_USART_BeginBinary(void);
void Debug_USART_EndBinary(void);
bool Debug_USART_StartTxDma(const uint8_t *data, uint16_t length);
bool Debug_USART_TxDmaBusy(void);
bool Debug_USART_TakeTxDmaComplete(void);
bool Debug_USART_TakeTxDmaError(void);

void Debug_USART_OnIrq(void);
void Debug_USART_OnTxDmaIrq(void);

#endif /* RVM_BSP_DEBUG_USART_H */
