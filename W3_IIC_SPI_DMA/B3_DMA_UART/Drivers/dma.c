#include "stm32f10x.h"

void DMA_USART1_TX_Init(void)
{
    /* Enable DMA1 clock */
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    /* USART1 TX = DMA1 Channel4 */
    DMA1_Channel4->CCR = 0;

    DMA1_Channel4->CPAR =
        (unsigned int)&USART1->DR;

    /*
     * Memory -> Peripheral
     * Memory increment
     * 8-bit memory
     * 8-bit peripheral
     */
    DMA1_Channel4->CCR =
        DMA_CCR1_DIR |
        DMA_CCR1_MINC;

    /* Enable USART1 TX DMA */
    USART1->CR3 |= USART_CR3_DMAT;
}


void DMA_USART1_Send(
    char *buffer,
    unsigned int length)
{
    /*
     * Disable DMA channel
     * before configuring new transfer
     */
    DMA1_Channel4->CCR &= ~DMA_CCR1_EN;

    /* Clear DMA1 Channel4 flags */
    DMA1->IFCR = DMA_IFCR_CGIF4;

    /* Memory address */
    DMA1_Channel4->CMAR =
        (unsigned int)buffer;

    /* Number of bytes */
    DMA1_Channel4->CNDTR = length;

    /* Start DMA */
    DMA1_Channel4->CCR |= DMA_CCR1_EN;
}
