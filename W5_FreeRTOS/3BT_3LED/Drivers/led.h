#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "FreeRTOS.h"

BaseType_t LED_Init(void);
void LED_SetDuty(uint8_t id, uint16_t duty);
void LED_Task(void *param);

#endif
