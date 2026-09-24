#include "mpu6500.h"
#include "i2c.h"

uint8_t MPU6500_Init(void)
{
    uint8_t id;

    if (!MPU6500_ReadWhoAmI(&id))
        return 0;

    if (id != 0x70 && id != 0x71)
        return 0;

    I2C1_WriteByte(MPU6500_ADDR,
                   MPU6500_PWR_MGMT_1,
                   0x00);

    I2C1_WriteByte(MPU6500_ADDR,
                   MPU6500_ACCEL_CFG,
                   0x00);

    I2C1_WriteByte(MPU6500_ADDR,
                   MPU6500_GYRO_CFG,
                   0x00);

    return 1;
}

uint8_t MPU6500_ReadWhoAmI(uint8_t *id)
{
    return I2C1_ReadByte(MPU6500_ADDR,
                          MPU6500_WHO_AM_I,
                          id);
}

uint8_t MPU6500_ReadData(MPU6500_Data *data)
{
    uint8_t buf[14];

    if (!I2C1_ReadBytes(MPU6500_ADDR,
                        MPU6500_ACCEL_XOUT,
                        buf,
                        14))
    {
        return 0;
    }

    data->ax = (int16_t)((buf[0] << 8) | buf[1]);
    data->ay = (int16_t)((buf[2] << 8) | buf[3]);
    data->az = (int16_t)((buf[4] << 8) | buf[5]);

    data->temp = (int16_t)((buf[6] << 8) | buf[7]);

    data->gx = (int16_t)((buf[8] << 8) | buf[9]);
    data->gy = (int16_t)((buf[10] << 8) | buf[11]);
    data->gz = (int16_t)((buf[12] << 8) | buf[13]);

    return 1;
}
