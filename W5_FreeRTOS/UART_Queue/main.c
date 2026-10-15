#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "button_exti.h"
#include "uart1.h"

/* UART Task configuration */
#define UART_TASK_STACK_SIZE    128U
#define UART_TASK_PRIORITY      (tskIDLE_PRIORITY + 1U)

/* Queue configuration */
#define BUTTON_QUEUE_LENGTH     10U

/* System clock: default HSI clock */
uint32_t SystemCoreClock = 8000000U;

/* Queue handle shared by main and UART Task */
static QueueHandle_t button_queue;


/* =========================================
 * Send unsigned integer through UART
 * ========================================= */
static void UART_SendNumber(uint32_t number)
{
    char buffer[11];
    uint32_t index = 0U;

    if (number == 0U)
    {
        UART1_SendString("0");
        return;
    }

    while (number > 0U)
    {
        buffer[index++] = (char)('0' + (number % 10U));
        number /= 10U;
    }

    while (index > 0U)
    {
        index--;
        UART1_SendChar(buffer[index]);
    }
}


/* =========================================
 * UART Task
 * Wait for button count from Queue
 * ========================================= */
static void UART_Task(void *argument)
{
    uint32_t count;

    (void)argument;

    UART1_SendString("\r\n=== EXTI Queue Test ===\r\n");
    UART1_SendString("Press PB0 button\r\n");
    UART1_SendString("UART Task running\r\n");

    while (1)
    {
        /*
         * Block until button count is available.
         * No polling or busy waiting.
         */
        if (xQueueReceive(
                button_queue,
                &count,
                portMAX_DELAY) == pdPASS)
        {
            UART1_SendString("Button count: ");
            UART_SendNumber(count);
            UART1_SendString("\r\n");
        }
    }
}


/* =========================================
 * Main
 * ========================================= */
int main(void)
{
    BaseType_t task_result;

    /* Initialize USART1 */
    UART1_Init();

    UART1_SendString("\r\nSystem starting...\r\n");

    /* Create Queue for button events */
    button_queue = xQueueCreate(
        BUTTON_QUEUE_LENGTH,
        sizeof(uint32_t)
    );

    if (button_queue == NULL)
    {
        UART1_SendString("ERROR: Queue creation failed\r\n");

        while (1)
        {
        }
    }

    /*
     * Initialize PB0 and EXTI0.
     * This function also creates the debounce task.
     */
    BUTTON_EXTI_Init(button_queue);

    /* Create UART receiving task */
    task_result = xTaskCreate(
        UART_Task,
        "UART_Task",
        UART_TASK_STACK_SIZE,
        NULL,
        UART_TASK_PRIORITY,
        NULL
    );

    if (task_result != pdPASS)
    {
        UART1_SendString("ERROR: UART task creation failed\r\n");

        while (1)
        {
        }
    }

    /* Start FreeRTOS scheduler */
    vTaskStartScheduler();

    /* Reached only if scheduler cannot start */
    UART1_SendString("ERROR: Scheduler failed\r\n");

    while (1)
    {
    }
}
