#include "adc.h"

void ADC1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* PA0 = analog input */
    GPIOA->CRL &= ~(0xF << 0);

    /* ADC clock = PCLK2 / 2 = 4 MHz */
    RCC->CFGR &= ~(3 << 14);

    /* Channel 0 */
    ADC1->SQR1 = 0;
    ADC1->SQR2 = 0;
    ADC1->SQR3 = 0;

    /* Sample time channel 0 */
    ADC1->SMPR2 &= ~(7 << 0);
    ADC1->SMPR2 |=  (5 << 0);

    /* External trigger = TIM3 TRGO */
    ADC1->CR2 &= ~(7 << 17);
    ADC1->CR2 |=  (4 << 17);

    /* Enable external trigger */
    ADC1->CR2 |= ADC_CR2_EXTTRIG;

    /* Enable ADC */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* Calibration */
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL);

    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);
}

uint16_t ADC1_Read(void)
{
    return ADC1->DR;
}
