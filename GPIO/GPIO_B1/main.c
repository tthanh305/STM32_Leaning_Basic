#include <stm32f10x.h>

void delay_ms(uint32_t ms) {
    // Delay thô bằng vòng lặp - phụ thuộc tốc độ CPU
    // STM32F103C8T6 mặc định chạy HSI 8MHz (chưa cấu hình PLL)
    for (uint32_t i = 0; i < ms; i++) {
        for (uint32_t j = 0; j < 800; j++) {
            __asm__("nop");
        }
    }
}

int main(void) {
    RCC->APB2ENR |= (1 << 4); // Bật clock cho GPIOC (bit 4)

    GPIOC->CRH &= ~(0xF << 20);   // Clear config PC13 (bit 20-23 trong CRH)
    GPIOC->CRH |= (0x2 << 20);    // PC13: output push-pull, 2MHz (MODE=10, CNF=00)

    while (1) {
        GPIOC->ODR ^= (1 << 13); // Toggle PC13
        delay_ms(100);
    }
}
