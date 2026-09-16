#include "stm32f10x.h"

#define BUFFER_SIZE 32

volatile char rx_buffer[BUFFER_SIZE];
volatile unsigned int rx_index = 0;
volatile unsigned char message_ready = 0;

volatile unsigned int pwm_percent = 50;
volatile unsigned char led_state = 0;


/* =========================
   UART1
   PA9  -> TX
   PA10 -> RX
   ========================= */

void UART1_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    /* PA9: Alternate Function Push-Pull */
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |=  (0xB << 4);

    /* PA10: Input Floating */
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |=  (0x4 << 8);

    /* 115200 baud, PCLK2 = 8 MHz */
    USART1->BRR = 0x45;

    USART1->CR1 = USART_CR1_TE |
                  USART_CR1_RE |
                  USART_CR1_RXNEIE |
                  USART_CR1_UE;

    /* Enable USART1 interrupt */
    NVIC->ISER[1] |= (1 << 5);
}


/* =========================
   UART Send
   ========================= */

void UART1_SendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE));

    USART1->DR = c;
}


void UART1_SendString(const char *str)
{
    while (*str)
        UART1_SendChar(*str++);
}


/* =========================
   PWM TIM2_CH1
   PA0 -> PWM
   ========================= */

void PWM_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* PA0: Alternate Function Push-Pull */
    GPIOA->CRL &= ~(0xF << 0);
    GPIOA->CRL |=  (0xB << 0);

    /*
       Timer clock = 8 MHz

       PSC = 7
       ARR = 999

       8 MHz / 8 / 1000 = 1 kHz
    */

    TIM2->PSC = 7;
    TIM2->ARR = 999;

    /* Initial PWM = 50% */
    TIM2->CCR1 = 500;

    /* PWM Mode 1 */
    TIM2->CCMR1 &= ~(0xFF);
    TIM2->CCMR1 |= (6 << 4);

    /* Enable preload */
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;

    /* Initially OFF */
    TIM2->CCER &= ~TIM_CCER_CC1E;

    /* Auto reload preload */
    TIM2->CR1 |= TIM_CR1_ARPE;

    /* Update registers */
    TIM2->EGR |= TIM_EGR_UG;

    /* Start timer */
    TIM2->CR1 |= TIM_CR1_CEN;
}


/* =========================
   Set PWM %
   ========================= */

void PWM_SetPercent(unsigned int percent)
{
    if (percent > 100)
        percent = 100;

    pwm_percent = percent;

    TIM2->CCR1 = (1000 * percent) / 100;
}


/* =========================
   Process Command
   ========================= */

void Process_Command(void)
{
    unsigned int value;


    /* =====================
       ON!
       ===================== */

    if (rx_buffer[0] == 'O' &&
        rx_buffer[1] == 'N' &&
        rx_buffer[2] == '\0')
    {
        led_state = 1;

        TIM2->CCER |= TIM_CCER_CC1E;

        UART1_SendString("LED ON\r\n");
    }


    /* =====================
       OFF!
       ===================== */

    else if (rx_buffer[0] == 'O' &&
             rx_buffer[1] == 'F' &&
             rx_buffer[2] == 'F' &&
             rx_buffer[3] == '\0')
    {
        led_state = 0;

        TIM2->CCER &= ~TIM_CCER_CC1E;

        UART1_SendString("LED OFF\r\n");
    }


    /* =====================
       Status!
       ===================== */

    else if (rx_buffer[0] == 'S' &&
             rx_buffer[1] == 't' &&
             rx_buffer[2] == 'a' &&
             rx_buffer[3] == 't' &&
             rx_buffer[4] == 'u' &&
             rx_buffer[5] == 's' &&
             rx_buffer[6] == '\0')
    {
        UART1_SendString("LED: ");

        if (led_state)
            UART1_SendString("ON");
        else
            UART1_SendString("OFF");

        UART1_SendString(", PWM: ");

        if (pwm_percent == 100)
        {
            UART1_SendString("100");
        }
        else if (pwm_percent >= 10)
        {
            UART1_SendChar((pwm_percent / 10) + '0');
            UART1_SendChar((pwm_percent % 10) + '0');
        }
        else
        {
            UART1_SendChar(pwm_percent + '0');
        }

        UART1_SendString("%\r\n");
    }


    /* =====================
       P6!
       P25!
       P90!
       P100!
       ===================== */

    else if (rx_buffer[0] == 'P')
    {
        value = 0;

        /* Digit 1 */
        if (rx_buffer[1] >= '0' &&
            rx_buffer[1] <= '9')
        {
            value = rx_buffer[1] - '0';

            /* Digit 2 */
            if (rx_buffer[2] >= '0' &&
                rx_buffer[2] <= '9')
            {
                value = value * 10 +
                        (rx_buffer[2] - '0');

                /* Digit 3 */
                if (rx_buffer[3] >= '0' &&
                    rx_buffer[3] <= '9')
                {
                    value = value * 10 +
                            (rx_buffer[3] - '0');
                }
            }
        }

        PWM_SetPercent(value);

        UART1_SendString("PWM updated\r\n");
    }


    /* =====================
       Invalid
       ===================== */

    else
    {
        UART1_SendString("Invalid command\r\n");
    }
}


/* =========================
   USART1 Interrupt
   ========================= */

void USART1_IRQHandler(void)
{
    char c;

    if (USART1->SR & USART_SR_RXNE)
    {
        c = USART1->DR;

        /*
           Nếu message trước chưa được
           main xử lý thì bỏ qua ký tự mới
        */
        if (message_ready)
            return;


        /* Kết thúc lệnh */
        if (c == '!')
        {
            rx_buffer[rx_index] = '\0';

            message_ready = 1;
        }


        /* Bỏ qua CR và LF */
        else if (c == '\r' || c == '\n')
        {
            return;
        }


        /* Lưu ký tự vào buffer */
        else
        {
            if (rx_index < BUFFER_SIZE - 1)
            {
                rx_buffer[rx_index++] = c;
            }
        }
    }
}


/* =========================
   Main
   ========================= */

int main(void)
{
    /* Vector table */
    SCB->VTOR = 0x08000000;

    /* Enable global interrupt */
    __enable_irq();


    PWM_Init();
    UART1_Init();


    UART1_SendString("Bai 5 UART PWM OK\r\n");


    while (1)
    {
        if (message_ready)
        {
            Process_Command();

            /* Chuẩn bị nhận lệnh mới */
            rx_index = 0;
            message_ready = 0;
        }
    }
}
