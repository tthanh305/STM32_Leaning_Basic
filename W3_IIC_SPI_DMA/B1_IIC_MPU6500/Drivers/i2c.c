#include "i2c.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_i2c.h"

void I2C1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    /* Clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    /* PB6 = SCL, PB7 = SDA */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* I2C1 */
    I2C_InitStructure.I2C_ClockSpeed = 100000;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress =
        I2C_AcknowledgedAddress_7bit;

    I2C_Init(I2C1, &I2C_InitStructure);

    I2C_Cmd(I2C1, ENABLE);
}

static uint8_t I2C_WaitEvent(uint32_t event)
{
    uint32_t timeout = 100000;

    while (!I2C_CheckEvent(I2C1, event))
    {
        if (--timeout == 0)
            return 0;
    }

    return 1;
}

uint8_t I2C1_WriteByte(uint8_t devAddr,
                       uint8_t reg,
                       uint8_t data)
{
    I2C_GenerateSTART(I2C1, ENABLE);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
        return 0;

    I2C_Send7bitAddress(I2C1, devAddr << 1,
                        I2C_Direction_Transmitter);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        return 0;

    I2C_SendData(I2C1, reg);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        return 0;

    I2C_SendData(I2C1, data);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        return 0;

    I2C_GenerateSTOP(I2C1, ENABLE);

    return 1;
}

uint8_t I2C1_ReadByte(uint8_t devAddr,
                      uint8_t reg,
                      uint8_t *data)
{
    I2C_GenerateSTART(I2C1, ENABLE);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
        return 0;

    I2C_Send7bitAddress(I2C1, devAddr << 1,
                        I2C_Direction_Transmitter);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        return 0;

    I2C_SendData(I2C1, reg);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        return 0;

    I2C_GenerateSTART(I2C1, ENABLE);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
        return 0;

    I2C_Send7bitAddress(I2C1, devAddr << 1,
                        I2C_Direction_Receiver);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        return 0;

    I2C_AcknowledgeConfig(I2C1, DISABLE);

    I2C_GenerateSTOP(I2C1, ENABLE);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
        return 0;

    *data = I2C_ReceiveData(I2C1);

    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return 1;
}

uint8_t I2C1_ReadBytes(uint8_t devAddr,
                       uint8_t reg,
                       uint8_t *data,
                       uint8_t len)
{
    uint8_t i;

    if (len == 0)
        return 0;

    I2C_GenerateSTART(I2C1, ENABLE);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
        return 0;

    I2C_Send7bitAddress(I2C1, devAddr << 1,
                        I2C_Direction_Transmitter);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        return 0;

    I2C_SendData(I2C1, reg);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        return 0;

    I2C_GenerateSTART(I2C1, ENABLE);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
        return 0;

    I2C_Send7bitAddress(I2C1, devAddr << 1,
                        I2C_Direction_Receiver);

    if (!I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        return 0;

    for (i = 0; i < len; i++)
    {
        if (i == len - 1)
        {
            I2C_AcknowledgeConfig(I2C1, DISABLE);
            I2C_GenerateSTOP(I2C1, ENABLE);
        }

        if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
            return 0;

        data[i] = I2C_ReceiveData(I2C1);
    }

    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return 1;
}
