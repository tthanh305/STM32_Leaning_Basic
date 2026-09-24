#include "dma.h"
#include "uart.h"

void DMA1_ADC_Init(uint16_t *buffer, uint16_t size)
{
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel1->CCR &= ~DMA_CCR1_EN;

    /* ADC1->DR -> RAM */
    DMA1_Channel1->CPAR = (uint32_t)&ADC1->DR;
    DMA1_Channel1->CMAR = (uint32_t)buffer;
    DMA1_Channel1->CNDTR = size;

    DMA1_Channel1->CCR =
        DMA_CCR1_MINC  |
        DMA_CCR1_PSIZE_0 |
        DMA_CCR1_MSIZE_0 |
        DMA_CCR1_CIRC |
        DMA_CCR1_HTIE |
        DMA_CCR1_TCIE;

    /* Xóa cờ DMA */
    DMA1->IFCR = DMA_IFCR_CGIF1;

    /* ADC sử dụng DMA */
    ADC1->CR2 |= ADC_CR2_DMA;

    /* DMA1 Channel 1 = IRQ 11 */
    NVIC->ISER[0] |= (1 << 11);

    /* Enable DMA */
    DMA1_Channel1->CCR |= DMA_CCR1_EN;
}


void DMA1_Channel1_IRQHandler(void)
{
    uint16_t i;

    /* Half Transfer: 0 -> 49 */
    if (DMA1->ISR & DMA_ISR_HTIF1)
    {
        DMA1->IFCR = DMA_IFCR_CHTIF1;

        UART1_SendString("HT:\r\n");

        for (i = 0; i < 50; i++)
        {
            UART1_SendNumber(adc_buffer[i]);
            UART1_SendString("\r\n");
        }
    }

    /* Transfer Complete: 50 -> 99 */
    if (DMA1->ISR & DMA_ISR_TCIF1)
    {
        DMA1->IFCR = DMA_IFCR_CTCIF1;

        UART1_SendString("TC:\r\n");

        for (i = 50; i < 100; i++)
        {
            UART1_SendNumber(adc_buffer[i]);
            UART1_SendString("\r\n");
        }
    }
}
