#ifndef I2C_H
#define I2C_H

#include "stm32f10x.h"

void I2C1_Init(void);

uint8_t I2C1_WriteByte(uint8_t devAddr,
                       uint8_t reg,
                       uint8_t data);

uint8_t I2C1_ReadByte(uint8_t devAddr,
                      uint8_t reg,
                      uint8_t *data);

uint8_t I2C1_ReadBytes(uint8_t devAddr,
                       uint8_t reg,
                       uint8_t *data,
                       uint8_t len);

#endif
