#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "led.h"

#include <stdint.h>

#define NUM_LED        3U
#define PWM_PERIOD     1000U
#define TIMER_CLOCK_HZ 8000000U

static QueueHandle_t led_queue[NUM_LED];

/* Khoi tao PWM va Queue */
BaseType_t LED_Init(void)
{
    GPIO_InitTypeDef gpio;
    TIM_TimeBaseInitTypeDef tb;
    TIM_OCInitTypeDef oc;
    uint32_t i;

    /* GPIOA va TIM2 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* Cau hinh timer */
    TIM_TimeBaseStructInit(&tb);
    tb.TIM_Prescaler = TIMER_CLOCK_HZ / 1000000U - 1U;
    tb.TIM_Period = PWM_PERIOD - 1U;
    tb.TIM_CounterMode = TIM_CounterMode_Up;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &tb);

    /* Cau hinh 3 kenh PWM */
    TIM_OCStructInit(&oc);
    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_OCPolarity = TIM_OCPolarity_High;
    oc.TIM_Pulse = 500U;

    TIM_OC1Init(TIM2, &oc);
    TIM_OC2Init(TIM2, &oc);
    TIM_OC3Init(TIM2, &oc);

    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM2, ENABLE);

    TIM_GenerateEvent(TIM2, TIM_EventSource_Update);
    TIM_Cmd(TIM2, ENABLE);

    /* Moi LED co mot Queue */
    for (i = 0U; i < NUM_LED; i++) {
        led_queue[i] = xQueueCreate(1U, sizeof(uint16_t));

        if (led_queue[i] == NULL) {
            return pdFAIL;
        }
    }

    return pdPASS;
}

/* Gui muc sang den Queue cua LED */
void LED_SetDuty(uint8_t id, uint16_t duty)
{
    if (id < NUM_LED && led_queue[id] != NULL) {
        if (duty > PWM_PERIOD) {
            duty = PWM_PERIOD;
        }

        xQueueOverwrite(led_queue[id], &duty);
    }
}

/* Task cap nhat PWM */
void LED_Task(void *param)
{
    uint32_t id = (uint32_t)(uintptr_t)param;
    uint16_t duty;

    for (;;) {
        if (xQueueReceive(
                led_queue[id],
                &duty,
                portMAX_DELAY
            ) == pdPASS) {

            if (id == 0U) {
                TIM_SetCompare1(TIM2, duty);
            } else if (id == 1U) {
                TIM_SetCompare2(TIM2, duty);
            } else if (id == 2U) {
                TIM_SetCompare3(TIM2, duty);
            }
        }
    }
}
