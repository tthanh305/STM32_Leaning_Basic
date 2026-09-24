#include "stm32f10x.h"
#include "uart.h"
#include "i2c.h"
#include "mpu6500.h"

static void delay(void)
{
    volatile uint32_t i;

    for (i = 0; i < 500000; i++);
}

static void UART1_SendNumber(int16_t value)
{
    char buf[8];
    uint8_t i = 0;

    if (value < 0)
    {
        UART1_SendChar('-');
        value = -value;
    }

    if (value == 0)
    {
        UART1_SendChar('0');
        return;
    }

    while (value > 0)
    {
        buf[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i)
        UART1_SendChar(buf[--i]);
}

int main(void)
{
    MPU6500_Data imu;
    uint8_t who_am_i;

    UART1_Init();
    I2C1_Init();

    UART1_SendString("\r\nB1 MPU6500\r\n");

    if (!MPU6500_Init())
    {
        UART1_SendString("MPU6500 ERROR\r\n");

        while (1);
    }

    MPU6500_ReadWhoAmI(&who_am_i);

    UART1_SendString("MPU6500 OK\r\n");
    UART1_SendString("WHO_AM_I = 0x");

    if (who_am_i == 0x70)
        UART1_SendString("70\r\n");
    else if (who_am_i == 0x71)
        UART1_SendString("71\r\n");
    else
        UART1_SendString("??\r\n");

    while (1)
    {
        if (MPU6500_ReadData(&imu))
        {
            UART1_SendString("ACC: ");

            UART1_SendNumber(imu.ax);
            UART1_SendString(", ");

            UART1_SendNumber(imu.ay);
            UART1_SendString(", ");

            UART1_SendNumber(imu.az);

            UART1_SendString(" | GYRO: ");

            UART1_SendNumber(imu.gx);
            UART1_SendString(", ");

            UART1_SendNumber(imu.gy);
            UART1_SendString(", ");

            UART1_SendNumber(imu.gz);

            UART1_SendString("\r\n");
        }
        else
        {
            UART1_SendString("I2C READ ERROR\r\n");
        }

        delay();
    }
}
