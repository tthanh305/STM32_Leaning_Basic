#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"

uint32_t SystemCoreClock = 8000000;

typedef struct
{
    GPIO_TypeDef *GPIOx;
    uint16_t GPIO_Pin;
    uint32_t period_ms;
} LED_TypeDef;

LED_TypeDef led1 = {GPIOA, GPIO_Pin_0, 10000};
LED_TypeDef led2 = {GPIOA, GPIO_Pin_3, 1000};
LED_TypeDef led3 = {GPIOA, GPIO_Pin_5, 100};

void GPIO_Clock_Enable(GPIO_TypeDef *GPIOx)
{
    if (GPIOx == GPIOA)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    else if (GPIOx == GPIOB)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    else if (GPIOx == GPIOC)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
}

void led_task(void *task_parameter)
{
    LED_TypeDef *led = (LED_TypeDef *)task_parameter;
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_Clock_Enable(led->GPIOx);

    GPIO_InitStructure.GPIO_Pin = led->GPIO_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(led->GPIOx, &GPIO_InitStructure);

    GPIO_ResetBits(led->GPIOx, led->GPIO_Pin);

    while (1)
    {
        GPIO_SetBits(led->GPIOx, led->GPIO_Pin);
        vTaskDelay((TickType_t)(led->period_ms >> 1));

        GPIO_ResetBits(led->GPIOx, led->GPIO_Pin);
        vTaskDelay((TickType_t)(led->period_ms >> 1));
    }
}

int main(void)
{
    xTaskCreate(led_task, "LED1", configMINIMAL_STACK_SIZE, &led1, 1, NULL);
    xTaskCreate(led_task, "LED2", configMINIMAL_STACK_SIZE, &led2, 1, NULL);
    xTaskCreate(led_task, "LED3", configMINIMAL_STACK_SIZE, &led3, 1, NULL);

    vTaskStartScheduler();

    while (1)
    {
    }
}
