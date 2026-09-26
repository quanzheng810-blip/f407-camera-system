#include "ethernetif.h"

#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "stm32f4x7_eth.h"

#define RVM_ETH_INPUT_STACK_WORDS  512U
#define RVM_ETH_INPUT_PRIORITY     6U
#define RVM_ETH_INPUT_WAIT_TICKS   pdMS_TO_TICKS(250U)
#define RVM_ETH_TX_WAIT_TICKS      pdMS_TO_TICKS(100U)

static const uint8_t s_mac_address[6] = {0x02U, 0x40U, 0x7AU, 0x10U, 0x00U, 0x01U};
static SemaphoreHandle_t s_rx_semaphore;
static SemaphoreHandle_t s_tx_mutex;

extern ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB];
extern ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB];
extern uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE];
extern uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE];
extern ETH_DMADESCTypeDef *DMATxDescToSet;
extern ETH_DMA_Rx_Frame_infos *DMA_RX_FRAME_infos;

static err_t RVM_ETH_LowLevelOutput(struct netif *netif, struct pbuf *packet)
{
    struct pbuf *part;
    uint8_t *buffer;
    uint16_t offset = 0U;

    (void)netif;
    if ((packet->tot_len > ETH_TX_BUF_SIZE) ||
        (xSemaphoreTake(s_tx_mutex, RVM_ETH_TX_WAIT_TICKS) != pdTRUE))
    {
        return ERR_BUF;
    }
    if ((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != 0U)
    {
        (void)xSemaphoreGive(s_tx_mutex);
        return ERR_USE;
    }

    buffer = (uint8_t *)DMATxDescToSet->Buffer1Addr;
    for (part = packet; part != NULL; part = part->next)
    {
        (void)memcpy(&buffer[offset], part->payload, part->len);
        offset = (uint16_t)(offset + part->len);
    }
    (void)ETH_Prepare_Transmit_Descriptors(offset);
    (void)xSemaphoreGive(s_tx_mutex);
    return ERR_OK;
}

static struct pbuf *RVM_ETH_LowLevelInput(void)
{
    FrameTypeDef frame;
    struct pbuf *packet = NULL;
    struct pbuf *part;
    __IO ETH_DMADESCTypeDef *descriptor;
    uint32_t segment;
    uint16_t offset = 0U;

    frame = ETH_Get_Received_Frame_interrupt();
    if ((frame.descriptor != NULL) &&
        ((frame.descriptor->Status & ETH_DMARxDesc_ES) == 0U))
    {
        packet = pbuf_alloc(PBUF_RAW, (u16_t)frame.length, PBUF_POOL);
        if (packet != NULL)
        {
            for (part = packet; part != NULL; part = part->next)
            {
                (void)memcpy(part->payload,
                             &((uint8_t *)frame.buffer)[offset],
                             part->len);
                offset = (uint16_t)(offset + part->len);
            }
        }
    }

    descriptor = (DMA_RX_FRAME_infos->Seg_Count > 1U) ?
                 DMA_RX_FRAME_infos->FS_Rx_Desc : frame.descriptor;
    for (segment = 0U;
         (segment < DMA_RX_FRAME_infos->Seg_Count) && (descriptor != NULL);
         ++segment)
    {
        descriptor->Status = ETH_DMARxDesc_OWN;
        descriptor = (__IO ETH_DMADESCTypeDef *)descriptor->Buffer2NextDescAddr;
    }
    DMA_RX_FRAME_infos->Seg_Count = 0U;

    if ((ETH->DMASR & ETH_DMASR_RBUS) != 0U)
    {
        ETH->DMASR = ETH_DMASR_RBUS;
        ETH->DMARPDR = 0U;
    }
    return packet;
}

static void RVM_ETH_InputTask(void *argument)
{
    struct netif *netif = (struct netif *)argument;

    for (;;)
    {
        if (xSemaphoreTake(s_rx_semaphore, RVM_ETH_INPUT_WAIT_TICKS) == pdTRUE)
        {
            struct pbuf *packet = RVM_ETH_LowLevelInput();
            if (packet != NULL)
            {
                if (netif->input(packet, netif) != ERR_OK)
                {
                    pbuf_free(packet);
                }
            }
        }
    }
}

static void RVM_ETH_LowLevelInit(struct netif *netif)
{
    uint32_t index;

    netif->hwaddr_len = ETH_HWADDR_LEN;
    (void)memcpy(netif->hwaddr, s_mac_address, sizeof(s_mac_address));
    netif->mtu = 1500U;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

    s_rx_semaphore = xSemaphoreCreateCounting(20U, 0U);
    s_tx_mutex = xSemaphoreCreateMutex();

    ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr);
    ETH_DMATxDescChainInit(DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);
    ETH_DMARxDescChainInit(DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
    for (index = 0U; index < ETH_RXBUFNB; ++index)
    {
        ETH_DMARxDescReceiveITConfig(&DMARxDscrTab[index], ENABLE);
    }

    (void)xTaskCreate(RVM_ETH_InputTask,
                      "ETH-RX",
                      RVM_ETH_INPUT_STACK_WORDS,
                      netif,
                      RVM_ETH_INPUT_PRIORITY,
                      NULL);
    ETH_Start();
}

err_t ethernetif_init(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", netif != NULL);

#if LWIP_NETIF_HOSTNAME
    netif->hostname = "rvm-f407";
#endif
    netif->name[0] = 'e';
    netif->name[1] = '0';
    netif->output = etharp_output;
    netif->linkoutput = RVM_ETH_LowLevelOutput;
    RVM_ETH_LowLevelInit(netif);
    return ERR_OK;
}

void ETH_IRQHandler(void)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    if (ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET)
    {
        if (s_rx_semaphore != NULL)
        {
            (void)xSemaphoreGiveFromISR(s_rx_semaphore,
                                        &higher_priority_task_woken);
        }
    }
    ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}
