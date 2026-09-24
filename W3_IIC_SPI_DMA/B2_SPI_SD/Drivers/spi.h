#ifndef SPI_H
#define SPI_H

#include "stm32f10x.h"

void SPI1_Init(void);
unsigned char SPI1_SendByte(unsigned char data);

#endif
