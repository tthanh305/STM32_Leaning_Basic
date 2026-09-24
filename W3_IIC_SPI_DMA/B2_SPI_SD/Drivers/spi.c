#include "spi.h"

void SPI1_Init(void)
{
    /* GPIOA + SPI1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /*
     * PA5  SCK  -> Alternate Function Push-Pull
     * PA6  MISO -> Input Floating
     * PA7  MOSI -> Alternate Function Push-Pull
     */

    GPIOA->CRL &= ~(0xFFF << 20);

    GPIOA->CRL |= (0xB << 20);   /* PA5 */
    GPIOA->CRL |= (0x4 << 24);   /* PA6 */
    GPIOA->CRL |= (0xB << 28);   /* PA7 */

    /*
     * SPI1:
     * Master
     * Software NSS
     * Mode 0
     * Prescaler /256
     *
     * 8 MHz / 256 = 31.25 kHz
     * dùng khi khởi tạo SD
     */

    SPI1->CR1 = 0;

    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 |= SPI_CR1_SSM;
    SPI1->CR1 |= SPI_CR1_SSI;

    SPI1->CR1 |= SPI_CR1_BR_2 |
                 SPI_CR1_BR_1 |
                 SPI_CR1_BR_0;

    SPI1->CR1 &= ~SPI_CR1_CPOL;
    SPI1->CR1 &= ~SPI_CR1_CPHA;

    SPI1->CR1 |= SPI_CR1_SPE;
}


unsigned char SPI1_SendByte(unsigned char data)
{
    while (!(SPI1->SR & SPI_SR_TXE));

    SPI1->DR = data;

    while (!(SPI1->SR & SPI_SR_RXNE));

    return SPI1->DR;
}
