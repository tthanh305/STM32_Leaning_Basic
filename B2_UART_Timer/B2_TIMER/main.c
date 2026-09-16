#include "stm32f10x.h"

volatile uint32_t cnt1 = 0;
volatile uint32_t cnt2 = 0;
volatile uint32_t cnt3 = 0;

/* Init 3 LED */
void LED_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    GPIOA->CRL &= ~(0xFFF << 0);
    GPIOA->CRL |=  (0x222 << 0);
}

/* SysTick 1ms */
void SysTick_Init(void)
{
    SysTick->LOAD = 8000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = 0x07;
}

/* SysTick interrupt */
void SysTick_Handler(void)
{
    cnt1++;
    cnt2++;
    cnt3++;

    /* 0.1 Hz */
    if (cnt1 >= 5000)
    {
        GPIOA->ODR ^= (1 << 0);
        cnt1 = 0;
    }

    /* 1 Hz */
    if (cnt2 >= 500)
    {
        GPIOA->ODR ^= (1 << 1);
        cnt2 = 0;
    }

    /* 10 Hz */
    if (cnt3 >= 50)
    {
        GPIOA->ODR ^= (1 << 2);
        cnt3 = 0;
    }
}

int main(void)
{
    LED_Init();
    SysTick_Init();

    while (1)
    {
    }
}
