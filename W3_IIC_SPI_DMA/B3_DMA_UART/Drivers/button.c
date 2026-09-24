#include "stm32f10x.h"

void BUTTON_Init(void)
{
    /* Enable GPIOB */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    /*
     * PB0 input pull-up/pull-down
     */
    GPIOB->CRL &= ~(GPIO_CRL_MODE0 |
                    GPIO_CRL_CNF0);

    GPIOB->CRL |= GPIO_CRL_CNF0_1;

    /*
     * Pull-up
     */
    GPIOB->ODR |= GPIO_ODR_ODR0;
}

unsigned char BUTTON_Pressed(void)
{
    /*
     * Button active LOW
     */
    if (!(GPIOB->IDR & GPIO_IDR_IDR0))
    {
        return 1;
    }

    return 0;
}
