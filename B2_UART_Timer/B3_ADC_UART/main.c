#include "stm32f10x.h"

volatile unsigned int adc_value = 0;

void ADC1_Init(void)
{
    /* Enable GPIOA + ADC1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* PA0 = Analog input */
    GPIOA->CRL &= ~(0xF << 0);

    /* ADC clock = PCLK2 / 6 */
    RCC->CFGR &= ~(0x3 << 14);
    RCC->CFGR |=  (0x2 << 14);

    /* ADC1 Channel 0 */
    ADC1->SQR1 = 0;
    ADC1->SQR2 = 0;
    ADC1->SQR3 = 0;

    /* Sample time Channel 0 = 55.5 cycles */
    ADC1->SMPR2 &= ~(0x7 << 0);
    ADC1->SMPR2 |=  (0x5 << 0);

    /* Enable ADC */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* Reset calibration */
    ADC1->CR2 |= ADC_CR2_RSTCAL;
    while (ADC1->CR2 & ADC_CR2_RSTCAL);

    /* Start calibration */
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL);
}


unsigned int ADC1_Read(void)
{
    /* Start conversion */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* Wait conversion complete */
    while (!(ADC1->SR & ADC_SR_EOC));

    /* Read ADC value */
    return ADC1->DR;
}


void UART1_Init(void)
{
    /* Enable GPIOA + USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9 = TX, AF Push-Pull */
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);

    /* PA10 = RX, Floating input */
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8);

    /* PCLK2 = 8 MHz, Baudrate = 115200 */
    USART1->BRR = 0x45;

    /* Enable TX + RX + USART */
    USART1->CR1 = USART_CR1_TE |
                  USART_CR1_RE |
                  USART_CR1_UE;
}


void UART1_SendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE));

    USART1->DR = c;
}


void UART1_SendString(const char *str)
{
    while (*str)
    {
        UART1_SendChar(*str++);
    }
}


void UART1_SendNumber(unsigned int number)
{
    char buffer[10];
    int i = 0;

    if (number == 0)
    {
        UART1_SendChar('0');
        return;
    }

    while (number > 0)
    {
        buffer[i++] = (number % 10) + '0';
        number /= 10;
    }

    while (i > 0)
    {
        UART1_SendChar(buffer[--i]);
    }
}


void Delay_1s(void)
{
    volatile unsigned int i;

    for (i = 0; i < 800000; i++);
}


int main(void)
{
    unsigned int voltage;

    ADC1_Init();
    UART1_Init();

    while (1)
    {
        adc_value = ADC1_Read();

        /* V = ADC * 3.3 / 4095 */
        voltage = (adc_value * 3300) / 4095;

        UART1_SendString("ADC = ");
        UART1_SendNumber(adc_value);

        UART1_SendString("  Voltage = ");
        UART1_SendNumber(voltage / 1000);
        UART1_SendChar('.');
        UART1_SendNumber(voltage % 1000);
        UART1_SendString(" V\r\n");

        Delay_1s();
    }
}

