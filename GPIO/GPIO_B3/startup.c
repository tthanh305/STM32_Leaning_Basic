#include <stdint.h>

/* Các symbol này được định nghĩa trong linker script (.ld) */
extern uint32_t _sidata; /* địa chỉ .data trong FLASH (LMA) */
extern uint32_t _sdata;  /* bắt đầu .data trong RAM (VMA)   */
extern uint32_t _edata;  /* kết thúc .data trong RAM        */
extern uint32_t _sbss;   /* bắt đầu .bss                    */
extern uint32_t _ebss;   /* kết thúc .bss                   */
extern uint32_t _estack; /* đỉnh stack = cuối vùng RAM       */

void Reset_Handler(void);
static void Default_Handler(void) { while (1) {} }

/* Các handler ngoại lệ core Cortex-M, weak alias -> Default_Handler
 * nếu ta không định nghĩa lại ở nơi khác */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/* Vector table phải nằm đúng ở đầu FLASH (0x08000000) */
__attribute__((section(".isr_vector")))
void (* const vector_table[])(void) = {
    (void (*)(void))&_estack,   /* initial stack pointer */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0, 0, 0, 0,                 /* reserved */
    SVC_Handler,
    DebugMon_Handler,
    0,                          /* reserved */
    PendSV_Handler,
    SysTick_Handler,
};

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    /* copy .data từ FLASH lên RAM */
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* zero-init .bss */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    extern int main(void);
    main();

    while (1) {}
}
