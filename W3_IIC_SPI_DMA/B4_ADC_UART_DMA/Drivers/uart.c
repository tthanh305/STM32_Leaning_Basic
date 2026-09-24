#include "uart.h"

void UART1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9: Alternate Function Push-Pull, 50 MHz */
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);

    /* PA10: Floating input */
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8);

    /* PCLK2 = 8 MHz, baud = 115200 */
    USART1->BRR = 0x45;

    USART1->CR1 = USART_CR1_TE |
                  USART_CR1_RE |
                  USART_CR1_UE;
}

void UART1_SendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE));

    USART1->DR = c;
}

void UART1_SendString(char *str)
{
    while (*str)
    {
        UART1_SendChar(*str++);
    }
}

void UART1_SendNumber(uint16_t value)
{
    char buf[5];
    int i = 0;

    if (value == 0)
    {
        UART1_SendChar('0');
        return;
    }

    while (value > 0)
    {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0)
    {
        UART1_SendChar(buf[--i]);
    }
}
