#include "bsp_eth.h"

#include "misc.h"
#include "stm32f4x7_eth.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_syscfg.h"

#define RVM_LAN8720_PHY_ADDRESS  0U

static void RVM_ETH_ResetPhy(void)
{
    GPIO_InitTypeDef gpio;
    volatile uint32_t delay;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOI, &gpio);

    GPIO_ResetBits(GPIOI, GPIO_Pin_1);
    for (delay = 0U; delay < 200000U; ++delay)
    {
        __NOP();
    }
    GPIO_SetBits(GPIOI, GPIO_Pin_1);
    for (delay = 0U; delay < 200000U; ++delay)
    {
        __NOP();
    }
}

static void RVM_ETH_ConfigurePins(void)
{
    GPIO_InitTypeDef gpio;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                           RCC_AHB1Periph_GPIOC |
                           RCC_AHB1Periph_GPIOG |
                           RCC_AHB1Periph_GPIOI,
                           ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);

    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio.GPIO_Speed = GPIO_Speed_100MHz;

    gpio.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

    gpio.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOC, &gpio);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);

    gpio.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_Init(GPIOG, &gpio);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
    GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);

    RVM_ETH_ResetPhy();
}

static void RVM_ETH_ConfigureInterrupt(void)
{
    NVIC_InitTypeDef nvic;

    nvic.NVIC_IRQChannel = ETH_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 6U;
    nvic.NVIC_IRQChannelSubPriority = 0U;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);
}

bool RVM_ETH_Init(void)
{
    ETH_InitTypeDef eth;
    uint32_t timeout;

    RVM_ETH_ConfigurePins();
    RVM_ETH_ConfigureInterrupt();

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC |
                           RCC_AHB1Periph_ETH_MAC_Tx |
                           RCC_AHB1Periph_ETH_MAC_Rx,
                           ENABLE);

    ETH_DeInit();
    ETH_SoftwareReset();
    timeout = 0U;
    while ((ETH_GetSoftwareResetStatus() == SET) && (timeout < 1000000U))
    {
        ++timeout;
    }
    if (ETH_GetSoftwareResetStatus() == SET)
    {
        return false;
    }

    ETH_StructInit(&eth);
    eth.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
    eth.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
    eth.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
    eth.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    eth.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
    eth.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
    eth.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
    eth.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
    eth.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
    eth.ETH_ChecksumOffload = ETH_ChecksumOffload_Disable;
    eth.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Disable;
    eth.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;
    eth.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
    eth.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;
    eth.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;
    eth.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
    eth.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;
    eth.ETH_FixedBurst = ETH_FixedBurst_Enable;
    eth.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;
    eth.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
    eth.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

    if (ETH_Init(&eth, RVM_LAN8720_PHY_ADDRESS) == ETH_ERROR)
    {
        return false;
    }

    ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);
    return true;
}

bool RVM_ETH_IsLinkUp(void)
{
    return (ETH_ReadPHYRegister(RVM_LAN8720_PHY_ADDRESS, PHY_BSR) &
            PHY_Linked_Status) != 0U;
}
