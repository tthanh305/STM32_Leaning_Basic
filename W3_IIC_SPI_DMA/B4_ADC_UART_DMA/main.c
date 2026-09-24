#include "stm32f10x.h"
#include "uart.h"
#include "adc.h"
#include "dma.h"
#include "timer.h"

uint16_t adc_buffer[100];

int main(void)
{
    UART1_Init();
    ADC1_Init();
    DMA1_ADC_Init(adc_buffer, 100);
    TIM3_Init();

    UART1_SendString("START\n\r");

    while (1)
    {
    }
}
