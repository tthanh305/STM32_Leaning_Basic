#include "stm32f10x.h"

void UART1_Init(void)
{
    /* GPIOA + USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9: TX - Alternate Function Push-Pull 50MHz
       PA10: RX - Floating input */

    GPIOA->CRH &= ~(GPIO_CRH_MODE9 |
                    GPIO_CRH_CNF9  |
                    GPIO_CRH_MODE10 |
                    GPIO_CRH_CNF10);

    GPIOA->CRH |= GPIO_CRH_MODE9_0 |
                  GPIO_CRH_MODE9_1 |
                  GPIO_CRH_CNF9_1;

    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    /*
     * PCLK2 = 8MHz
     * Baudrate = 115200
     */
    USART1->BRR = 0x45;

    USART1->CR1 = USART_CR1_TE |
                  USART_CR1_RE |
                  USART_CR1_UE;
}
