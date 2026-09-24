#include "stm32f10x.h"
#include "uart.h"


void UART1_Init(void)
{
    /* GPIOA + USART1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;


    /*
     * PA9  = TX
     * PA10 = RX
     */

    GPIOA->CRH &= ~(
        GPIO_CRH_MODE9 |
        GPIO_CRH_CNF9 |
        GPIO_CRH_MODE10 |
        GPIO_CRH_CNF10
    );


    /* PA9 = Alternate function push-pull, 50 MHz */
    GPIOA->CRH |=
        GPIO_CRH_MODE9_0 |
        GPIO_CRH_MODE9_1 |
        GPIO_CRH_CNF9_1;


    /* PA10 = Floating input */
    GPIOA->CRH |=
        GPIO_CRH_CNF10_0;


    /*
     * PCLK2 = 8 MHz
     * Baudrate = 115200
     */

    USART1->BRR = 0x45;


    /*
     * Enable:
     * TX
     * RX
     * USART
     */

    USART1->CR1 =
        USART_CR1_TE |
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
        UART1_SendChar(*str);
        str++;
    }
}


char UART1_ReadChar(void)
{
    while (!(USART1->SR & USART_SR_RXNE));

    return USART1->DR;
}
