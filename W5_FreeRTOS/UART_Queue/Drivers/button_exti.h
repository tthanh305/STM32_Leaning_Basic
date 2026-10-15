#ifndef BUTTON_EXTI_H
#define BUTTON_EXTI_H

#include "FreeRTOS.h"
#include "queue.h"

void BUTTON_EXTI_Init(QueueHandle_t queue);

extern volatile uint32_t exti_isr_count;
extern volatile uint32_t queue_send_ok;
extern volatile uint32_t queue_send_fail;

#endif
