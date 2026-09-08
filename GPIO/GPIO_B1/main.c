#include <stdint.h>

/* =========================
   RCC
   ========================= */

#define RCC_BASE        0x40021000UL

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

#define RCC ((RCC_TypeDef *)RCC_BASE)


/* =========================
   GPIO
   ========================= */

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

#define GPIOC_BASE      0x40011000UL

#define GPIOC ((GPIO_TypeDef *)GPIOC_BASE)



void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        for (uint32_t j = 0; j < 800; j++)
        {
            __asm__("nop");
        }
    }
}



int main(void)
{
    /* Enable GPIOC clock */
    RCC->APB2ENR |= (1 << 4);

    /* PC13 = output push-pull, 2 MHz */
    GPIOC->CRH &= ~(0xF << 20);
    GPIOC->CRH |=  (0x2 << 20);

    while (1)
    {
        GPIOC->ODR ^= (1 << 13);

        delay_ms(100);
    }
}

