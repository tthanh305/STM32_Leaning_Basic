#ifndef SD_H
#define SD_H

#include "stm32f10x.h"

#define SD_CS_LOW()   (GPIOA->BSRR = GPIO_BSRR_BR4)
#define SD_CS_HIGH()  (GPIOA->BSRR = GPIO_BSRR_BS4)

#define SD_OK          0
#define SD_ERROR       1

unsigned char SD_Init(void);

unsigned char SD_ReadBlock(
    unsigned long sector,
    unsigned char *buffer
);

unsigned char SD_WriteBlock(
    unsigned long sector,
    unsigned char *buffer
);

#endif
