#ifndef DMA_H
#define DMA_H

#include "stm32f10x.h"

extern uint16_t adc_buffer[100];

void DMA1_ADC_Init(uint16_t *buffer, uint16_t size);

#endif
