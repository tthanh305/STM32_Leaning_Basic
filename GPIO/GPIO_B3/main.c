/*Cấu hình PA0-PA7 ở chế độ Input, PB8-PB15 ở chế độ Output.
Viết chương trình đọc dữ liệu từ PA0-PA7 sau đó đảo dữ liệu (Từ 0 thành 1, từ 1 thành
0), rồi ghi vào LED tại PB8-PB15*/

#include <stdint.h>

#define RCC_BASE    0x40021000UL
#define GPIOA_BASE  0x40010800UL
#define GPIOB_BASE  0x40010C00UL

typedef struct
{
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
} RCC_TypeDef;

typedef struct
{
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

#define RCC   ((RCC_TypeDef *)RCC_BASE)
#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *)GPIOB_BASE)

int main(void)
{
    // Bat clock GPIOA va GPIOB
    RCC->APB2ENR |= (1 << 2) | (1 << 3);

    // PA0-PA7: input pull-up
    GPIOA->CRL = 0x88888888;
    GPIOA->ODR |= 0x00FF;

    // PB8-PB15: output push-pull 2MHz
    GPIOB->CRH = 0x22222222;

    while (1)
    {
        uint8_t input;
        uint8_t output;

        input = GPIOA->IDR & 0xFF;

        output = (~input) & 0xFF;

        GPIOB->ODR = (GPIOB->ODR & 0x00FF) | ((uint32_t)output << 8);
    }
}
