#ifndef STM32F4X7_ETH_CONF_H
#define STM32F4X7_ETH_CONF_H

#include "stm32f4xx.h"

#define CUSTOM_DRIVER_BUFFERS_CONFIG
#define ETH_RX_BUF_SIZE                       ETH_MAX_PACKET_SIZE
#define ETH_TX_BUF_SIZE                       ETH_MAX_PACKET_SIZE
#define ETH_RXBUFNB                           6U
#define ETH_TXBUFNB                           4U

#define _eth_delay_                           ETH_Delay

#define PHY_RESET_DELAY                       ((uint32_t)0x000FFFFFU)
#define PHY_CONFIG_DELAY                      ((uint32_t)0x00FFFFFFU)

/* LAN8720 Special Control/Status Register. */
#define PHY_SR                                ((uint16_t)0x001FU)
#define PHY_DUPLEX_SPEED_STATUS_MASK          ((uint16_t)0x0014U)
#define PHY_100BTX_FULL                       ((uint16_t)0x0010U)
#define PHY_100BTX_HALF                       ((uint16_t)0x0000U)
#define PHY_10M_FULL                          ((uint16_t)0x0014U)
#define PHY_10M_HALF                          ((uint16_t)0x0004U)

#define IS_PHY_SPEED_100Mbps(value)            (((value) & 0x0004U) == 0U)
#define IS_PHY_DUPLEX_FULL(value)              (((value) & 0x0010U) != 0U)

#endif /* STM32F4X7_ETH_CONF_H */
