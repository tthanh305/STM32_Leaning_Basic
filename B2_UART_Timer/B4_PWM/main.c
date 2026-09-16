#include "stm32f10x.h"

void GPIO_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // PA0-PA3: Alternate Function Push-Pull, 2 MHz
    GPIOA->CRL &= ~(0xFFFF);
    GPIOA->CRL |=  (0xAAAA);
}

void TIM2_PWM_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // TIM2 = 8 MHz
    // 8 MHz / (7 + 1) = 1 MHz
    TIM2->PSC = 7;

    // 1 MHz / (999 + 1) = 1 kHz
    TIM2->ARR = 999;

    // Duty cycle
    TIM2->CCR1 = 100;   // 10%
    TIM2->CCR2 = 300;   // 30%
    TIM2->CCR3 = 500;   // 50%
    TIM2->CCR4 = 700;   // 70%

    // PWM mode 1
    TIM2->CCMR1 = 0;
    TIM2->CCMR2 = 0;

    TIM2->CCMR1 |= (6 << 4);    // CH1
    TIM2->CCMR1 |= (6 << 12);   // CH2
    TIM2->CCMR2 |= (6 << 4);    // CH3
    TIM2->CCMR2 |= (6 << 12);   // CH4

    // Enable CH1-CH4
    TIM2->CCER |= TIM_CCER_CC1E;
    TIM2->CCER |= TIM_CCER_CC2E;
    TIM2->CCER |= TIM_CCER_CC3E;
    TIM2->CCER |= TIM_CCER_CC4E;

    // Update registers
    TIM2->EGR = TIM_EGR_UG;

    // Start timer
    TIM2->CR1 |= TIM_CR1_CEN;
}

int main(void)
{
    GPIO_Init();
    TIM2_PWM_Init();

    while (1)
    {
    }
}
