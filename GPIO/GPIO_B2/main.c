#include <stdint.h>

/* =========================
   RCC
   ========================= */

#define RCC_BASE 0x40021000UL

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

#define RCC ((RCC_TypeDef *)RCC_BASE)


/* =========================
   GPIO
   ========================= */

#define GPIOA_BASE 0x40010800UL

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

#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)


/* =========================
   Delay
   ========================= */

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


/* =========================
   Main
   ========================= */

int main(void)
{
    /* Bat clock GPIOA */
    RCC->APB2ENR |= (1 << 2);

    /* PA0 - PA7: output push-pull, 2 MHz */
    GPIOA->CRL = 0x22222222;

    uint8_t pos = 0;
    int8_t direction = 1;

    while (1)
    {
    
    GPIOA->BSRR = (1 << pos) | (0xFF & ~(1 << pos))<<16;

        pos += direction;

        if (pos == 7)
        {
            direction = -1;
        }
        else if (pos == 0)
        {
            direction = 1;
        }

        delay_ms(100);
    }
}
