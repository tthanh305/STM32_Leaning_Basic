#include <stdint.h>

extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

void Reset_Handler(void);

static void Default_Handler(void)
{
    while (1) {}
}

/* Core handlers */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void DMA1_Channel1_IRQHandler(void);

/* TIM3 interrupt */
void TIM3_IRQHandler(void);

/* Vector table */
__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) = {

    /* Cortex-M3 */
    (void (*)(void))&_estack,
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,

    0,
    0,
    0,
    0,

    SVC_Handler,
    DebugMon_Handler,

    0,

    PendSV_Handler,
    SysTick_Handler,

    /* STM32F103 peripheral IRQ 0 -> 28 */
    Default_Handler,    /*  0: WWDG */
    Default_Handler,    /*  1: PVD */
    Default_Handler,    /*  2: TAMPER */
    Default_Handler,    /*  3: RTC */
    Default_Handler,    /*  4: FLASH */
    Default_Handler,    /*  5: RCC */
    Default_Handler,    /*  6: EXTI0 */
    Default_Handler,    /*  7: EXTI1 */
    Default_Handler,    /*  8: EXTI2 */
    Default_Handler,    /*  9: EXTI3 */
    Default_Handler,                 /* 10: EXTI4 */
    DMA1_Channel1_IRQHandler,        /* 11: DMA1_Channel1 */
    Default_Handler,    /* 12: DMA1_Channel2 */
    Default_Handler,    /* 13: DMA1_Channel3 */
    Default_Handler,    /* 14: DMA1_Channel4 */
    Default_Handler,    /* 15: DMA1_Channel5 */
    Default_Handler,    /* 16: DMA1_Channel6 */
    Default_Handler,    /* 17: DMA1_Channel7 */
    Default_Handler,    /* 18: ADC1_2 */
    Default_Handler,    /* 19: USB_HP_CAN_TX */
    Default_Handler,    /* 20: USB_LP_CAN_RX0 */
    Default_Handler,    /* 21: CAN_RX1 */
    Default_Handler,    /* 22: CAN_SCE */
    Default_Handler,    /* 23: EXTI9_5 */
    Default_Handler,    /* 24: TIM1_BRK */
    Default_Handler,    /* 25: TIM1_UP */
    Default_Handler,    /* 26: TIM1_TRG_COM */
    Default_Handler,    /* 27: TIM1_CC */
    Default_Handler,    /* 28: TIM2 */

    TIM3_IRQHandler      /* 29: TIM3 */
};

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    while (dst < &_edata)
    {
        *dst++ = *src++;
    }

    dst = &_sbss;

    while (dst < &_ebss)
    {
        *dst++ = 0;
    }

    extern int main(void);
    main();

    while (1) {}
}
