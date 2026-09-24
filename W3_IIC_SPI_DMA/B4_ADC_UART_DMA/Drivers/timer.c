#include "timer.h"

void TIM3_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    TIM3->PSC = 7999;
    TIM3->ARR = 9;

    /* TIM3 Update -> TRGO */
    TIM3->CR2 &= ~(7 << 4);
    TIM3->CR2 |= (2 << 4);

    TIM3->EGR = TIM_EGR_UG;

    TIM3->SR &= ~TIM_SR_UIF;

    TIM3->CR1 |= TIM_CR1_CEN;
}

/* TEST 4 không dùng TIM3 interrupt */
void TIM3_IRQHandler(void)
{
    TIM3->SR &= ~TIM_SR_UIF;
}
