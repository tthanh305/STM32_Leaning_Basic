#include <stdint.h>

#define RCC_BASE    0x40021000UL
#define GPIOA_BASE  0x40010800UL
#define GPIOB_BASE  0x40010C00UL

typedef struct {
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

typedef struct {
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

#define BTN_PIN   (1U << 0)
#define LED_PIN   (1U << 8)

void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms; i++)
    {
        for (volatile uint32_t j = 0; j < 800; j++)
        {
            __asm__("nop");
        }
    }
}


int main(void)
{
    uint8_t old_state = 1;
    uint8_t new_state;

    RCC->APB2ENR |= (1U << 2) | (1U << 3);

    GPIOA->CRL &= ~(0xFU << 0);
    GPIOA->CRL |=  (0x8U << 0);
    GPIOA->ODR |= BTN_PIN;

    GPIOB->CRH &= ~(0xFU << 0);
    GPIOB->CRH |=  (0x2U << 0);

    while (1)
    {
        new_state = (GPIOA->IDR & BTN_PIN) ? 1 : 0;

        if (old_state == 1 && new_state == 0)
        {
            delay_ms(20);

            if ((GPIOA->IDR & BTN_PIN) == 0)
            {
                GPIOB->ODR ^= LED_PIN;
            }
        }

        old_state = new_state;
    }
}
