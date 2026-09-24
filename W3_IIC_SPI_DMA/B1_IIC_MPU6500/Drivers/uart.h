#ifndef UART_H
#define UART_H

#include "stm32f10x.h"

void UART1_Init(void);
void UART1_SendChar(char c);
void UART1_SendString(char *str);

#endif
