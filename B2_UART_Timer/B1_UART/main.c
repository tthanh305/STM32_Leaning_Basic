#include "stm32f10x.h"

#define BUFFER_SIZE 128

#define CLASS_CODE  "MT02"
#define GROUP_CODE  "02"

char buffer[BUFFER_SIZE];
unsigned int index = 0;


/* =========================
   UART1 Init
   PA9  = TX
   PA10 = RX
   ========================= */
void UART1_Init(void)
{
    /* Enable GPIOA + USART1 */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9 = TX, AF Push-Pull */
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);

    /* PA10 = RX, Floating input */
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8);

    /* PCLK2 = 8 MHz, Baudrate = 115200 */
    USART1->BRR = 0x45;

    /* Enable TX + RX + USART */
    USART1->CR1 = USART_CR1_TE |
                  USART_CR1_RE |
                  USART_CR1_UE;
}


/* =========================
   Send 1 character
   ========================= */
void UART1_SendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE));

    USART1->DR = c;
}


/* =========================
   Send string
   ========================= */
void UART1_SendString(const char *str)
{
    while (*str)
    {
        UART1_SendChar(*str++);
    }
}


/* =========================
   Send received message
   ========================= */
void UART1_SendMessage(void)
{
    UART1_SendString(CLASS_CODE);
    UART1_SendString(GROUP_CODE);
    UART1_SendString(": ");
    UART1_SendString(buffer);
    UART1_SendString("\r\n");
}


int main(void)
{
    UART1_Init();

    while (1)
    {
        /* Check received data */
        if (USART1->SR & USART_SR_RXNE)
        {
            char c = USART1->DR;

            /* '!' = end of message */
            if (c == '!')
            {
                buffer[index] = '\0';

                UART1_SendMessage();

                /* Reset buffer */
                index = 0;
            }
            else
            {
                /* Store character */
                if (index < BUFFER_SIZE - 1)
                {
                    buffer[index++] = c;
                }
            }
        }
    }
}
