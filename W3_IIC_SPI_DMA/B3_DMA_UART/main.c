#include "stm32f10x.h"

#include "uart.h"
#include "dma.h"
#include "button.h"

#define BUFFER_SIZE 32

char buffer[BUFFER_SIZE] = "MT02-02:BTN:";

unsigned int button_value = 0;


/* =========================
   Convert number to string
   ========================= */
unsigned int NumberToString(
    unsigned int value,
    char *str)
{
    char temp[10];
    unsigned int i = 0;
    unsigned int j = 0;

    if (value == 0)
    {
        str[0] = '0';
        return 1;
    }

    while (value > 0)
    {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0)
    {
        str[j++] = temp[--i];
    }

    return j;
}


/* =========================
   Create message

   MT02-02:BTN:value\n\r
   ========================= */
unsigned int MakeMessage(
    char *buffer,
    unsigned int value)
{
    unsigned int index;

    index = NumberToString(
        value,
        &buffer[12]);

    buffer[12 + index] = '\n';
    buffer[13 + index] = '\r';

    return 14 + index;
}


int main(void)
{
    unsigned int length;

    UART1_Init();
    DMA_USART1_TX_Init();
    BUTTON_Init();

    while (1)
    {
        if (BUTTON_Pressed())
        {
            button_value++;

            length = MakeMessage(
                buffer,
                button_value);

            DMA_USART1_Send(
                buffer,
                length);

            /*
             * Wait for button release
             */
            while (BUTTON_Pressed())
            {
            }

            /*
             * Debounce
             */
            for (volatile unsigned int i = 0;
                 i < 50000;
                 i++)
            {
            }
        }
    }
}
