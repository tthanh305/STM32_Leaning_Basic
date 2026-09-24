#include "stm32f10x.h"

#include "uart.h"
#include "sd.h"
#include "fat32.h"
#include "command.h"


#define BUFFER_SIZE 128

#define FILE_NAME "TEST.TXT"


char file_buffer[BUFFER_SIZE];


/* Compare two strings */
unsigned char StringCompare(char *a, char *b)
{
    unsigned int i = 0;

    while (a[i] != 0 && b[i] != 0)
    {
        if (a[i] != b[i])
        {
            return 0;
        }

        i++;
    }

    if (a[i] == 0 && b[i] == 0)
    {
        return 1;
    }

    return 0;
}


/* Get string length */
unsigned long StringLength(char *str)
{
    unsigned long length = 0;

    while (str[length] != 0)
    {
        length++;
    }

    return length;
}


/* Process command */
void ProcessCommand(char *command)
{
    char *data;
    unsigned long size;


    /* Mount FAT32 */
    if (StringCompare(command, "mount"))
    {
        if (FAT32_Mount() == FAT32_OK)
        {
            UART1_SendString("FAT32 MOUNT OK\r\n");
        }
        else
        {
            UART1_SendString("FAT32 MOUNT ERROR\r\n");
        }

        return;
    }


    /* Read file */
    if (StringCompare(command, "read"))
    {
        size = FAT32_ReadFile(
            FILE_NAME,
            (unsigned char *)file_buffer,
            BUFFER_SIZE
        );

        if (size == 0)
        {
            UART1_SendString("READ ERROR\r\n");
            return;
        }

        UART1_SendString("FILE DATA:\r\n");

        UART1_SendString(file_buffer);

        UART1_SendString("\r\n");

        return;
    }


    /* Write file */
    if (StringCompare(command, "write"))
    {
        UART1_SendString("DATA> ");

        /*
         * Wait for data from UART.
         * command.c only receives the string.
         */
        while (1)
        {
            data = Command_Get();

            if (data != 0)
            {
                break;
            }
        }


        size = StringLength(data);


        if (FAT32_WriteFile(
            FILE_NAME,
            (unsigned char *)data,
            size
        ) == FAT32_OK)
        {
            UART1_SendString("WRITE OK\r\n");
        }
        else
        {
            UART1_SendString("WRITE ERROR\r\n");
        }

        return;
    }


    /* Unknown command */
    UART1_SendString("Invalid command\r\n");
}


int main(void)
{
    char *command;


    /* Initialize UART */
    UART1_Init();

    /* Initialize command receiver */
    Command_Init();


    UART1_SendString("\r\n");
    UART1_SendString("STM32 FAT32 TEST\r\n");


    /* Initialize SD Card */
    if (SD_Init() != SD_OK)
    {
        UART1_SendString("SD INIT ERROR\r\n");

        while (1);
    }

    UART1_SendString("SD INIT OK\r\n");


    /* Mount FAT32 */
    if (FAT32_Mount() != FAT32_OK)
    {
        UART1_SendString("FAT32 MOUNT ERROR\r\n");

        while (1);
    }

    UART1_SendString("FAT32 MOUNT OK\r\n");

    UART1_SendString("> ");


    while (1)
    {
        command = Command_Get();


        if (command != 0)
        {
            ProcessCommand(command);

            UART1_SendString("> ");
        }
    }
}
