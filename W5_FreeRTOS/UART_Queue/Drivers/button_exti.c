#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "button_exti.h"

#define BUTTON_PIN             0U
#define DEBOUNCE_TIME_MS      20U
#define RELEASE_CHECK_MS       5U
#define BUTTON_TASK_STACK     128U
#define BUTTON_TASK_PRIORITY  (tskIDLE_PRIORITY + 2U)

static QueueHandle_t button_queue;
static TaskHandle_t button_task_handle;

static uint32_t press_count;

volatile uint32_t exti_isr_count = 0U;
volatile uint32_t queue_send_ok = 0U;
volatile uint32_t queue_send_fail = 0U;


/* =========================================
 * Button debounce task
 * ========================================= */
static void BUTTON_Task(void *argument)
{
    uint32_t count;

    (void)argument;

    while (1)
    {
        /* Cho ISR thong bao */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* Cho tiep diem het doi */
        vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));

        /* LOW: nut van dang duoc nhan */
        if ((GPIOB->IDR & (1U << BUTTON_PIN)) == 0U)
        {
            press_count++;
            count = press_count;

            /* Gui so lan nhan vao Queue */
            if (xQueueSend(button_queue, &count, 0U) == pdPASS)
            {
                queue_send_ok++;
            }
            else
            {
                queue_send_fail++;
            }
        }

        /*
         * Cho nut duoc nha.
         * Trong thoi gian nay EXTI0 van bi khoa.
         */
        while ((GPIOB->IDR & (1U << BUTTON_PIN)) == 0U)
        {
            vTaskDelay(pdMS_TO_TICKS(RELEASE_CHECK_MS));
        }

        /*
         * Xac nhan nut da o muc HIGH on dinh.
         * Neu lai LOW, tiep tuc cho nha.
         */
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));

            if ((GPIOB->IDR & (1U << BUTTON_PIN)) != 0U)
            {
                break;
            }

            while ((GPIOB->IDR & (1U << BUTTON_PIN)) == 0U)
            {
                vTaskDelay(pdMS_TO_TICKS(RELEASE_CHECK_MS));
            }
        }

        /* Xoa co ngat cu */
        EXTI->PR = (1U << BUTTON_PIN);

        /* Cho phep nhan lan tiep theo */
        EXTI->IMR |= (1U << BUTTON_PIN);
    }
}


/* =========================================
 * Initialize PB0 and EXTI0
 * ========================================= */
void BUTTON_EXTI_Init(QueueHandle_t queue)
{
    button_queue = queue;

    press_count = 0U;
    exti_isr_count = 0U;
    queue_send_ok = 0U;
    queue_send_fail = 0U;

    if (button_queue == NULL)
    {
        return;
    }

    /* Enable GPIOB and AFIO clocks */
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN |
                    RCC_APB2ENR_AFIOEN;

    /* PB0: input pull-up */
    GPIOB->CRL &= ~(0xFU << (BUTTON_PIN * 4U));
    GPIOB->CRL |=  (0x8U << (BUTTON_PIN * 4U));

    GPIOB->ODR |= (1U << BUTTON_PIN);

    /* Route EXTI0 to PB0 */
    AFIO->EXTICR[0] &= ~(0xFU << 0U);
    AFIO->EXTICR[0] |=  (0x1U << 0U);

    /* Disable EXTI0 while configuring */
    EXTI->IMR &= ~(1U << BUTTON_PIN);

    /* Falling edge only */
    EXTI->RTSR &= ~(1U << BUTTON_PIN);
    EXTI->FTSR |=  (1U << BUTTON_PIN);

    /* Clear pending flag */
    EXTI->PR = (1U << BUTTON_PIN);

    /* Create debounce task */
    if (xTaskCreate(
            BUTTON_Task,
            "ButtonTask",
            BUTTON_TASK_STACK,
            NULL,
            BUTTON_TASK_PRIORITY,
            &button_task_handle) != pdPASS)
    {
        button_task_handle = NULL;
        return;
    }

    /* EXTI priority must be valid for FreeRTOS FromISR APIs */
    NVIC_SetPriority(
        EXTI0_IRQn,
        configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
    );

    NVIC_EnableIRQ(EXTI0_IRQn);

    /* Enable EXTI0 */
    EXTI->IMR |= (1U << BUTTON_PIN);
}


/* =========================================
 * EXTI0 interrupt handler
 * ========================================= */
void EXTI0_IRQHandler(void)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    if ((EXTI->PR & (1U << BUTTON_PIN)) != 0U)
    {
        /* Clear pending interrupt */
        EXTI->PR = (1U << BUTTON_PIN);

        exti_isr_count++;

        /* Mask EXTI0 to block bounce interrupts */
        EXTI->IMR &= ~(1U << BUTTON_PIN);

        /* Notify debounce task */
        if (button_task_handle != NULL)
        {
            vTaskNotifyGiveFromISR(
                button_task_handle,
                &higher_priority_task_woken
            );
        }

        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}
