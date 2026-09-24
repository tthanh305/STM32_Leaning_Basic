#ifndef UART_H
#define UART_H

void UART1_Init(void);

void UART1_SendChar(char c);
void UART1_SendString(char *str);

char UART1_ReadChar(void);

#endif
