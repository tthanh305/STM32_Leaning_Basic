#ifndef DMA_H
#define DMA_H

void DMA_USART1_TX_Init(void);
void DMA_USART1_Send(char *buffer, unsigned int length);

#endif
