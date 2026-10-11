#include "bsp_debug_usart.h"

#define RVM_USART_TX_DMA_STREAM   DMA2_Stream7
#define RVM_USART_TX_DMA_CHANNEL  DMA_Channel_4
#define RVM_USART_TX_DMA_FLAGS    (DMA_FLAG_FEIF7 | DMA_FLAG_DMEIF7 | \
                                   DMA_FLAG_TEIF7 | DMA_FLAG_HTIF7 | \
                                   DMA_FLAG_TCIF7)

static volatile uint8_t s_rx_ring[RVM_SERIAL_RX_RING_SIZE];
static volatile uint16_t s_rx_head;
static volatile uint16_t s_rx_tail;

static volatile bool s_binary_active;
static volatile bool s_tx_dma_busy;
static volatile bool s_tx_dma_complete;
static volatile bool s_tx_dma_error;

static uint8_t s_deferred_log[RVM_SERIAL_DEFERRED_LOG_SIZE];
static volatile uint16_t s_log_head;
static volatile uint16_t s_log_tail;

static void Debug_USART_SendPolling(uint8_t byte)
{
    while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET)
    {
    }
    USART_SendData(DEBUG_USART, byte);
}

static void Debug_USART_QueueLogByte(uint8_t byte)
{
    uint16_t next = (uint16_t)((s_log_head + 1U) %
                               RVM_SERIAL_DEFERRED_LOG_SIZE);

    if (next != s_log_tail)
    {
        s_deferred_log[s_log_head] = byte;
        s_log_head = next;
    }
}

static void Debug_USART_FlushDeferredLog(void)
{
    while (s_log_tail != s_log_head)
    {
        Debug_USART_SendPolling(s_deferred_log[s_log_tail]);
        s_log_tail = (uint16_t)((s_log_tail + 1U) %
                                RVM_SERIAL_DEFERRED_LOG_SIZE);
    }
}

void Debug_USART_Config(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    RCC_AHB1PeriphClockCmd(DEBUG_USART_RX_GPIO_CLK |
                           DEBUG_USART_TX_GPIO_CLK |
                           RCC_AHB1Periph_DMA2, ENABLE);
    RCC_APB2PeriphClockCmd(DEBUG_USART_CLK, ENABLE);

    GPIO_PinAFConfig(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_SOURCE,
                     DEBUG_USART_RX_AF);
    GPIO_PinAFConfig(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_SOURCE,
                     DEBUG_USART_TX_AF);

    GPIO_StructInit(&gpio);
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Pin = DEBUG_USART_TX_PIN;
    GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &gpio);
    gpio.GPIO_Pin = DEBUG_USART_RX_PIN;
    GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &gpio);

    USART_StructInit(&usart);
    usart.USART_BaudRate = DEBUG_USART_BAUDRATE;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(DEBUG_USART, &usart);

    DMA_DeInit(RVM_USART_TX_DMA_STREAM);
    while (DMA_GetCmdStatus(RVM_USART_TX_DMA_STREAM) != DISABLE)
    {
    }
    DMA_ClearFlag(RVM_USART_TX_DMA_STREAM, RVM_USART_TX_DMA_FLAGS);

    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 6U;
    nvic.NVIC_IRQChannelSubPriority = 0U;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    nvic.NVIC_IRQChannel = DMA2_Stream7_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 6U;
    nvic.NVIC_IRQChannelSubPriority = 1U;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    USART_ITConfig(DEBUG_USART, USART_IT_RXNE, ENABLE);
    USART_DMACmd(DEBUG_USART, USART_DMAReq_Tx, ENABLE);
    USART_Cmd(DEBUG_USART, ENABLE);
}

bool Debug_USART_ReadByte(uint8_t *byte)
{
    if ((byte == 0) || (s_rx_tail == s_rx_head))
    {
        return false;
    }

    *byte = s_rx_ring[s_rx_tail];
    s_rx_tail = (uint16_t)((s_rx_tail + 1U) % RVM_SERIAL_RX_RING_SIZE);
    return true;
}

bool Debug_USART_BeginBinary(void)
{
    uint32_t primask = __get_PRIMASK();
    bool acquired = false;

    __disable_irq();
    if ((!s_binary_active) && (!s_tx_dma_busy))
    {
        s_binary_active = true;
        acquired = true;
    }
    if (primask == 0U)
    {
        __enable_irq();
    }
    return acquired;
}

void Debug_USART_EndBinary(void)
{
    while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TC) == RESET)
    {
    }
    s_binary_active = false;
    Debug_USART_FlushDeferredLog();
}

bool Debug_USART_StartTxDma(const uint8_t *data, uint16_t length)
{
    DMA_InitTypeDef dma;

    if ((data == 0) || (length == 0U) || (!s_binary_active) ||
        s_tx_dma_busy)
    {
        return false;
    }

    DMA_Cmd(RVM_USART_TX_DMA_STREAM, DISABLE);
    while (DMA_GetCmdStatus(RVM_USART_TX_DMA_STREAM) != DISABLE)
    {
    }
    DMA_DeInit(RVM_USART_TX_DMA_STREAM);
    DMA_ClearFlag(RVM_USART_TX_DMA_STREAM, RVM_USART_TX_DMA_FLAGS);

    DMA_StructInit(&dma);
    dma.DMA_Channel = RVM_USART_TX_DMA_CHANNEL;
    dma.DMA_PeripheralBaseAddr = (uint32_t)&DEBUG_USART->DR;
    dma.DMA_Memory0BaseAddr = (uint32_t)data;
    dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma.DMA_BufferSize = length;
    dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    dma.DMA_Mode = DMA_Mode_Normal;
    dma.DMA_Priority = DMA_Priority_High;
    dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
    dma.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(RVM_USART_TX_DMA_STREAM, &dma);
    /* Direct-mode and FIFO error interrupts are not useful for this simple
     * byte-wide USART stream. Enabling FE while direct mode is selected can
     * report a false FIFO error as a short transfer finishes. TC and the real
     * transfer/bus error are sufficient here. */
    DMA_ITConfig(RVM_USART_TX_DMA_STREAM, DMA_IT_TC | DMA_IT_TE, ENABLE);

    s_tx_dma_complete = false;
    s_tx_dma_error = false;
    s_tx_dma_busy = true;
    DMA_Cmd(RVM_USART_TX_DMA_STREAM, ENABLE);
    return true;
}

bool Debug_USART_TxDmaBusy(void)
{
    return s_tx_dma_busy;
}

bool Debug_USART_TakeTxDmaComplete(void)
{
    bool complete;
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    complete = s_tx_dma_complete;
    s_tx_dma_complete = false;
    if (primask == 0U)
    {
        __enable_irq();
    }
    return complete;
}

bool Debug_USART_TakeTxDmaError(void)
{
    bool error;
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    error = s_tx_dma_error;
    s_tx_dma_error = false;
    if (primask == 0U)
    {
        __enable_irq();
    }
    return error;
}

void Debug_USART_OnIrq(void)
{
    if (USART_GetITStatus(DEBUG_USART, USART_IT_RXNE) != RESET)
    {
        uint8_t byte = (uint8_t)USART_ReceiveData(DEBUG_USART);
        uint16_t next = (uint16_t)((s_rx_head + 1U) %
                                   RVM_SERIAL_RX_RING_SIZE);
        if (next != s_rx_tail)
        {
            s_rx_ring[s_rx_head] = byte;
            s_rx_head = next;
        }
    }

    if (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_ORE) != RESET)
    {
        (void)DEBUG_USART->SR;
        (void)DEBUG_USART->DR;
    }
}

void Debug_USART_OnTxDmaIrq(void)
{
    if (DMA_GetITStatus(RVM_USART_TX_DMA_STREAM, DMA_IT_TEIF7) != RESET)
    {
        DMA_ClearITPendingBit(RVM_USART_TX_DMA_STREAM, DMA_IT_TEIF7 |
                              DMA_IT_DMEIF7 | DMA_IT_FEIF7 |
                              DMA_IT_TCIF7);
        DMA_Cmd(RVM_USART_TX_DMA_STREAM, DISABLE);
        s_tx_dma_busy = false;
        s_tx_dma_error = true;
        return;
    }

    if (DMA_GetITStatus(RVM_USART_TX_DMA_STREAM, DMA_IT_TCIF7) != RESET)
    {
        DMA_ClearITPendingBit(RVM_USART_TX_DMA_STREAM, DMA_IT_TCIF7 |
                              DMA_IT_DMEIF7 | DMA_IT_FEIF7);
        DMA_Cmd(RVM_USART_TX_DMA_STREAM, DISABLE);
        s_tx_dma_busy = false;
        s_tx_dma_complete = true;
    }
}

int fputc(int ch, FILE *stream)
{
    (void)stream;
    if (s_binary_active)
    {
        Debug_USART_QueueLogByte((uint8_t)ch);
    }
    else
    {
        Debug_USART_SendPolling((uint8_t)ch);
    }
    return ch;
}

int fgetc(FILE *stream)
{
    uint8_t byte;
    (void)stream;

    while (!Debug_USART_ReadByte(&byte))
    {
    }
    return (int)byte;
}
