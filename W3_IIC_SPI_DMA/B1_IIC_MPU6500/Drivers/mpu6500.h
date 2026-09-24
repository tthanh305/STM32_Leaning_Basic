#ifndef MPU6500_H
#define MPU6500_H

#include "stm32f10x.h"

#define MPU6500_ADDR       0x68
#define MPU6500_WHO_AM_I   0x75
#define MPU6500_PWR_MGMT_1 0x6B
#define MPU6500_ACCEL_CFG   0x1C
#define MPU6500_GYRO_CFG    0x1B
#define MPU6500_ACCEL_XOUT  0x3B

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t temp;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} MPU6500_Data;

uint8_t MPU6500_Init(void);
uint8_t MPU6500_ReadWhoAmI(uint8_t *id);
uint8_t MPU6500_ReadData(MPU6500_Data *data);

#endif
