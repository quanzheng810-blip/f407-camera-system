#include "dcmi_framebuffer.h"

#include "stm32f4xx.h"

#define RVM_DCMI_DMA_STREAM  DMA2_Stream1
#define RVM_DCMI_DMA_FLAGS   (DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | \
                              DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1 | \
                              DMA_FLAG_TCIF1)

static volatile bool s_frame_event;
static volatile bool s_capture_error;

void RVM_DcmiFrame_Init(void)
{
    DCMI_InitTypeDef dcmi;
    NVIC_InitTypeDef nvic;

    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_DCMI, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    DCMI_DeInit();
    DCMI_StructInit(&dcmi);
    dcmi.DCMI_CaptureMode = DCMI_CaptureMode_SnapShot;
    dcmi.DCMI_SynchroMode = DCMI_SynchroMode_Hardware;
    dcmi.DCMI_PCKPolarity = DCMI_PCKPolarity_Rising;
    dcmi.DCMI_VSPolarity = DCMI_VSPolarity_High;
    dcmi.DCMI_HSPolarity = DCMI_HSPolarity_Low;
    dcmi.DCMI_CaptureRate = DCMI_CaptureRate_All_Frame;
    dcmi.DCMI_ExtendedDataMode = DCMI_ExtendedDataMode_8b;
    DCMI_Init(&dcmi);

    nvic.NVIC_IRQChannel = DCMI_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 6U;
    nvic.NVIC_IRQChannelSubPriority = 0U;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    DCMI_ITConfig(DCMI_IT_FRAME | DCMI_IT_OVF | DCMI_IT_ERR, ENABLE);
}

bool RVM_DcmiFrame_Start(uint8_t *destination, uint32_t byte_count)
{
    DMA_InitTypeDef dma;
    uint32_t word_count;

    if ((destination == 0) || (((uint32_t)destination & 3U) != 0U) ||
        ((byte_count & 3U) != 0U) || (byte_count == 0U))
    {
        return false;
    }

    word_count = byte_count / 4U;
    if (word_count > 0xFFFFU)
    {
        return false;
    }

    DCMI_CaptureCmd(DISABLE);
    DCMI_Cmd(DISABLE);
    DMA_Cmd(RVM_DCMI_DMA_STREAM, DISABLE);
    while (DMA_GetCmdStatus(RVM_DCMI_DMA_STREAM) != DISABLE)
    {
    }
    DMA_DeInit(RVM_DCMI_DMA_STREAM);
    DMA_ClearFlag(RVM_DCMI_DMA_STREAM, RVM_DCMI_DMA_FLAGS);

    DMA_StructInit(&dma);
    dma.DMA_Channel = DMA_Channel_1;
    dma.DMA_PeripheralBaseAddr = (uint32_t)&DCMI->DR;
    dma.DMA_Memory0BaseAddr = (uint32_t)destination;
    dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma.DMA_BufferSize = word_count;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    dma.DMA_Mode = DMA_Mode_Normal;
    dma.DMA_Priority = DMA_Priority_VeryHigh;
    dma.DMA_FIFOMode = DMA_FIFOMode_Enable;
    dma.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    dma.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
    dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(RVM_DCMI_DMA_STREAM, &dma);

    s_frame_event = false;
    s_capture_error = false;
    DCMI_ClearITPendingBit(DCMI_IT_FRAME | DCMI_IT_OVF | DCMI_IT_ERR);
    DMA_Cmd(RVM_DCMI_DMA_STREAM, ENABLE);
    DCMI_Cmd(ENABLE);
    DCMI_CaptureCmd(ENABLE);
    return true;
}

void RVM_DcmiFrame_Stop(void)
{
    DCMI_CaptureCmd(DISABLE);
    DCMI_Cmd(DISABLE);
    DMA_Cmd(RVM_DCMI_DMA_STREAM, DISABLE);
}

bool RVM_DcmiFrame_IsComplete(void)
{
    if (!s_frame_event)
    {
        return false;
    }

    return (DMA_GetCurrDataCounter(RVM_DCMI_DMA_STREAM) == 0U) ||
           (DMA_GetFlagStatus(RVM_DCMI_DMA_STREAM, DMA_FLAG_TCIF1) == SET);
}

bool RVM_DcmiFrame_HasError(void)
{
    return s_capture_error ||
           (DMA_GetFlagStatus(RVM_DCMI_DMA_STREAM, DMA_FLAG_TEIF1) == SET) ||
           (DMA_GetFlagStatus(RVM_DCMI_DMA_STREAM, DMA_FLAG_DMEIF1) == SET) ||
           (DMA_GetFlagStatus(RVM_DCMI_DMA_STREAM, DMA_FLAG_FEIF1) == SET);
}

bool RVM_DcmiFrame_OnIrq(void)
{
    bool frame = false;

    if (DCMI_GetITStatus(DCMI_IT_FRAME) == SET)
    {
        DCMI_ClearITPendingBit(DCMI_IT_FRAME);
        s_frame_event = true;
        frame = true;
    }
    if (DCMI_GetITStatus(DCMI_IT_OVF) == SET)
    {
        DCMI_ClearITPendingBit(DCMI_IT_OVF);
        s_capture_error = true;
    }
    if (DCMI_GetITStatus(DCMI_IT_ERR) == SET)
    {
        DCMI_ClearITPendingBit(DCMI_IT_ERR);
        s_capture_error = true;
    }
    return frame;
}
