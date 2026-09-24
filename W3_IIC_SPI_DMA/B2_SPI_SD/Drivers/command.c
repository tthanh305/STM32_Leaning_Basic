#include "command.h"
#include "uart.h"

#define COMMAND_BUFFER_SIZE 128

static char command_buffer[COMMAND_BUFFER_SIZE];
static unsigned int command_index;

void Command_Init(void)
{
    command_index = 0;
    command_buffer[0] = 0;
}

char *Command_Get(void)
{
    char c;

    c = UART1_ReadChar();

    if (c == '\r' || c == '\n')
    {
        if (command_index != 0)
        {
            command_buffer[command_index] = 0;
            command_index = 0;

            UART1_SendString("\r\n");

            return command_buffer;
        }

        return 0;
    }

    if (c == '\b')
    {
        if (command_index > 0)
        {
            command_index--;
            UART1_SendString("\b \b");
        }

        return 0;
    }

    if (command_index < COMMAND_BUFFER_SIZE - 1)
    {
        command_buffer[command_index] = c;
        command_index++;

        UART1_SendChar(c);
    }

    return 0;
}
