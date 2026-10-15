#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "led.h"
#include "button.h"

#include <stdint.h>

#define NUM_LED    3U
#define PWM_PERIOD 1000U
#define DUTY_STEP  100U
#define INIT_DUTY  500U

void Button_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_GPIOC,
        ENABLE
    );

    /* PB0, PB1, PB10: input pull-up */
    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpio);

    /* PC13: debug LED */
    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);

    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void Button_Task(void *param)
{
    uint16_t duty[NUM_LED] = {
        INIT_DUTY, INIT_DUTY, INIT_DUTY
    };

    uint8_t selected = 0U;
    uint8_t prev_sel = 1U;
    uint8_t prev_up = 1U;
    uint8_t prev_down = 1U;

    uint8_t sel, up, down;
    uint32_t i;

    (void)param;

    /* Khoi tao do sang cho 3 LED */
    for (i = 0U; i < NUM_LED; i++) {
        LED_SetDuty(i, duty[i]);
    }

    for (;;) {
        sel = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);
        up = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1);
        down = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10);

        /* Bao hieu khi nhan nut */
        if ((prev_sel && !sel) ||
            (prev_up && !up) ||
            (prev_down && !down)) {
            GPIOC->ODR ^= GPIO_Pin_13;
        }

        /* PB0: chon LED tiep theo */
        if (prev_sel && !sel) {
            selected = (selected + 1U) % NUM_LED;
        }

        /* PB1: tang do sang */
        if (prev_up && !up) {
            if (duty[selected] <= PWM_PERIOD - DUTY_STEP) {
                duty[selected] += DUTY_STEP;
            } else {
                duty[selected] = PWM_PERIOD;
            }

            LED_SetDuty(selected, duty[selected]);
        }

        /* PB10: giam do sang */
        if (prev_down && !down) {
            if (duty[selected] >= DUTY_STEP) {
                duty[selected] -= DUTY_STEP;
            } else {
                duty[selected] = 0U;
            }

            LED_SetDuty(selected, duty[selected]);
        }

        prev_sel = sel;
        prev_up = up;
        prev_down = down;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
