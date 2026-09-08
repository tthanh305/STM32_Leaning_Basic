#include <stm32f10x.h>

void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms; i++) {
        for (volatile uint32_t j = 0; j < 800; j++) {
            __asm__("nop");
        }
    }
}

int main(void) {
    RCC->APB2ENR |= (1 << 2); // Bật clock cho GPIOA (bit 2)

    /* Cấu hình PA0-PA7 làm output push-pull, 2MHz
     * CRL quản lý PA0-PA7, mỗi chân chiếm 4 bit (MODE + CNF)
     * MODE=10 (output 2MHz), CNF=00 (push-pull) -> mỗi chân = 0x2
     * 8 chân x 4 bit = toàn bộ 32 bit của CRL -> 0x22222222 */
    GPIOA->CRL = 0x22222222;

    uint8_t pos = 0;      // vị trí LED đang sáng (0-7)
    int8_t direction = 1; // 1: chạy sang phải, -1: chạy sang trái

    while (1) {
        GPIOA->BSRR = (1 << pos) | (0xFF & ~(1 << pos))<<16;

        pos += direction;

        // Đảo chiều khi chạm 2 đầu
        if (pos == 7) {
            direction = -1;
        } else if (pos == 0) {
            direction = 1;
        }

        delay_ms(100);
    }
}
