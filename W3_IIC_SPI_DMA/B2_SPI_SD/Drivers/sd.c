#include "sd.h"
#include "spi.h"

#define CMD0       0
#define CMD8       8
#define CMD16      16
#define CMD17      17
#define CMD24      24
#define CMD55      55
#define CMD58      58
#define ACMD41     41

static unsigned char SD_SendCommand(
    unsigned char cmd,
    unsigned long arg,
    unsigned char crc
)
{
    unsigned char response;
    unsigned int i;

    SPI1_SendByte(0xFF);

    SPI1_SendByte(0x40 | cmd);

    SPI1_SendByte((unsigned char)(arg >> 24));
    SPI1_SendByte((unsigned char)(arg >> 16));
    SPI1_SendByte((unsigned char)(arg >> 8));
    SPI1_SendByte((unsigned char)arg);

    SPI1_SendByte(crc);

    for (i = 0; i < 1000; i++)
    {
        response = SPI1_SendByte(0xFF);

        if (!(response & 0x80))
            return response;
    }

    return 0xFF;
}


/* -------------------------------------------------
 * Chờ SD card sẵn sàng
 * ------------------------------------------------- */

static void SD_WaitReady(void)
{
    unsigned int i;

    for (i = 0; i < 50000; i++)
    {
        if (SPI1_SendByte(0xFF) == 0xFF)
            return;
    }
}


/* -------------------------------------------------
 * Khởi tạo GPIO CS
 * ------------------------------------------------- */

static void SD_CS_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    /*
     * PA4 = GPIO Output Push-Pull
     * 50 MHz
     */

    GPIOA->CRL &= ~(0xF << 16);
    GPIOA->CRL |=  (0x3 << 16);

    SD_CS_HIGH();
}


/* -------------------------------------------------
 * Đổi SPI sang tốc độ cao hơn sau khi init
 * ------------------------------------------------- */

static void SD_SPI_Fast(void)
{
    SPI1->CR1 &= ~SPI_CR1_SPE;

    SPI1->CR1 &= ~(
        SPI_CR1_BR_2 |
        SPI_CR1_BR_1 |
        SPI_CR1_BR_0
    );

    /*
     * 8 MHz / 8 = 1 MHz
     */

    SPI1->CR1 |= SPI_CR1_BR_1;

    SPI1->CR1 |= SPI_CR1_SPE;
}


/* -------------------------------------------------
 * SD Card Init
 * ------------------------------------------------- */

unsigned char SD_Init(void)
{
    unsigned char response;
    unsigned char r7[4];

    unsigned int i;

    SD_CS_Init();
    SPI1_Init();

    /*
     * SD yêu cầu ít nhất 74 clock
     * với CS = HIGH
     */

    SD_CS_HIGH();

    for (i = 0; i < 10; i++)
        SPI1_SendByte(0xFF);

    /*
     * CMD0
     * Reset SD về SPI mode
     */

    SD_CS_LOW();

    response = SD_SendCommand(
        CMD0,
        0,
        0x95
    );

    SD_CS_HIGH();
    SPI1_SendByte(0xFF);

    if (response != 0x01)
        return SD_ERROR;


    /*
     * CMD8
     * Kiểm tra SD v2
     */

    SD_CS_LOW();

    response = SD_SendCommand(
        CMD8,
        0x000001AA,
        0x87
    );

    if (response == 0x01)
    {
        r7[0] = SPI1_SendByte(0xFF);
        r7[1] = SPI1_SendByte(0xFF);
        r7[2] = SPI1_SendByte(0xFF);
        r7[3] = SPI1_SendByte(0xFF);

        SD_CS_HIGH();
        SPI1_SendByte(0xFF);

        /*
         * ACMD41
         */

        for (i = 0; i < 1000; i++)
        {
            SD_CS_LOW();

            SD_SendCommand(
                CMD55,
                0,
                0x01
            );

            SD_CS_HIGH();
            SPI1_SendByte(0xFF);

            SD_CS_LOW();

            response = SD_SendCommand(
                ACMD41,
                0x40000000,
                0x01
            );

            SD_CS_HIGH();
            SPI1_SendByte(0xFF);

            if (response == 0x00)
                break;
        }

        if (response != 0x00)
            return SD_ERROR;


        /*
         * CMD58
         * Đọc OCR
         */

        SD_CS_LOW();

        response = SD_SendCommand(
            CMD58,
            0,
            0x01
        );

        SPI1_SendByte(0xFF);
        SPI1_SendByte(0xFF);
        SPI1_SendByte(0xFF);
        SPI1_SendByte(0xFF);

        SD_CS_HIGH();
        SPI1_SendByte(0xFF);

        if (response != 0x00)
            return SD_ERROR;
    }
    else
    {
        /*
         * SD v1 / MMC không xử lý ở phiên bản
         * driver cơ bản này.
         */

        SD_CS_HIGH();
        return SD_ERROR;
    }

    SD_SPI_Fast();

    return SD_OK;
}


/* -------------------------------------------------
 * Đọc 1 block 512 byte
 * CMD17
 * ------------------------------------------------- */

unsigned char SD_ReadBlock(
    unsigned long sector,
    unsigned char *buffer
)
{
    unsigned char response;
    unsigned int i;
    unsigned long address;

    /*
     * SDHC dùng địa chỉ theo sector.
     * Với SDHC, sector = 0,1,2,...
     */

    address = sector;

    SD_CS_LOW();

    response = SD_SendCommand(
        CMD17,
        address,
        0x01
    );

    if (response != 0x00)
    {
        SD_CS_HIGH();
        SPI1_SendByte(0xFF);
        return SD_ERROR;
    }

    /*
     * Chờ token 0xFE
     */

    for (i = 0; i < 10000; i++)
    {
        response = SPI1_SendByte(0xFF);

        if (response == 0xFE)
            break;
    }

    if (response != 0xFE)
    {
        SD_CS_HIGH();
        SPI1_SendByte(0xFF);
        return SD_ERROR;
    }

    /*
     * Nhận 512 byte
     */

    for (i = 0; i < 512; i++)
        buffer[i] = SPI1_SendByte(0xFF);

    /*
     * CRC
     */

    SPI1_SendByte(0xFF);
    SPI1_SendByte(0xFF);

    SD_CS_HIGH();
    SPI1_SendByte(0xFF);

    return SD_OK;
}


/* -------------------------------------------------
 * Ghi 1 block 512 byte
 * CMD24
 * ------------------------------------------------- */

unsigned char SD_WriteBlock(
    unsigned long sector,
    unsigned char *buffer
)
{
    unsigned char response;
    unsigned char token;
    unsigned int i;

    SD_CS_LOW();

    response = SD_SendCommand(
        CMD24,
        sector,
        0x01
    );

    if (response != 0x00)
    {
        SD_CS_HIGH();
        SPI1_SendByte(0xFF);
        return SD_ERROR;
    }

    /*
     * Start block token
     */

    SPI1_SendByte(0xFE);

    /*
     * Gửi 512 byte
     */

    for (i = 0; i < 512; i++)
        SPI1_SendByte(buffer[i]);

    /*
     * CRC dummy
     */

    SPI1_SendByte(0xFF);
    SPI1_SendByte(0xFF);

    /*
     * Data response
     */

    token = SPI1_SendByte(0xFF);

    if ((token & 0x1F) != 0x05)
    {
        SD_CS_HIGH();
        SPI1_SendByte(0xFF);
        return SD_ERROR;
    }

    /*
     * Chờ ghi xong
     */

    SD_WaitReady();

    SD_CS_HIGH();
    SPI1_SendByte(0xFF);

    return SD_OK;
}
