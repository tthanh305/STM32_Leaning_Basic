#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"

#include "led.h"
#include "button.h"

uint32_t SystemCoreClock = 8000000;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    if (LED_Init() != pdPASS) {
        for (;;) {}
    }

    Button_Init();

    if (xTaskCreate(LED_Task, "LED1",
                    configMINIMAL_STACK_SIZE,
                    (void *)0, 1, NULL) != pdPASS) {
        for (;;) {}
    }

    if (xTaskCreate(LED_Task, "LED2",
                    configMINIMAL_STACK_SIZE,
                    (void *)1, 1, NULL) != pdPASS) {
        for (;;) {}
    }

    if (xTaskCreate(LED_Task, "LED3",
                    configMINIMAL_STACK_SIZE,
                    (void *)2, 1, NULL) != pdPASS) {
        for (;;) {}
    }

    if (xTaskCreate(Button_Task, "BTN",
                    configMINIMAL_STACK_SIZE,
                    NULL, 2, NULL) != pdPASS) {
        for (;;) {}
    }

    vTaskStartScheduler();

    while (1) {}
}
