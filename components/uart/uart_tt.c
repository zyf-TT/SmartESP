#include "uart_tt.h"
/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define ECHO_TEST_TXD (1)
#define ECHO_TEST_RXD (0)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM (0)
#define ECHO_UART_BAUD_RATE (115200)
#define ECHO_TASK_STACK_SIZE (CONFIG_EXAMPLE_TASK_STACK_SIZE)
static const char *TAG = "tt_BigWifi";

#define BUF_SIZE (512)

void echo_task(void *arg)
{
    // Message_tt UartMessage;
    int Chang_Count_tt = 0;
    int rec_Count_tt = 0;
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,      // 波特率
        .data_bits = UART_DATA_8_BITS,         /*!< UART byte size*/
        .parity = UART_PARITY_DISABLE,         // 奇偶校验方式
        .stop_bits = UART_STOP_BITS_1,         // 停止位数
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, // 硬件流控方式
        .source_clk = UART_SCLK_APB,           // 时钟源
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_set_pin(
        ECHO_UART_PORT_NUM, // 编号
        ECHO_TEST_TXD,      // TX GPIO
        ECHO_TEST_RXD,      // RX GPIO
        ECHO_TEST_RTS,
        ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    uint8_t *data_tt = (uint8_t *)malloc(BUF_SIZE);
    vTaskDelay(pdMS_TO_TICKS(100));
    while (1)
    {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, (BUF_SIZE - 1), 20);
        // Write data back to the UART
        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *)data, len);
        if (len)
        {
            data[len] = '\0';
            Chang_Count_tt = 0;
            for (int i = 0; i < len; i++)
            {
                data_tt[Chang_Count_tt] = data[i] & 240;
                data_tt[Chang_Count_tt] /= 16;
                Chang_Count_tt++;

                data_tt[Chang_Count_tt] = data[i] & 15;
                Chang_Count_tt++;
            }
            data_tt[Chang_Count_tt] = '\0';
            for (int i = 0; i < Chang_Count_tt; i++)
            {
                // ESP_LOGI(TAG, "Recv str: %d", data_tt[i]);
                if (data_tt[i] >= 0 && data_tt[i] <= 9)
                {
                    /* code */
                    data_tt[i] += 0x30;
                }
                if (data_tt[i] >= 10 && data_tt[i] <= 15)
                {
                    /* code */
                    data_tt[i] += 0x41 - 10;
                }
            }
            rec_Count_tt++;
            if (xQueueSend(uart_Queue, &data_tt[18], (TickType_t)10) != pdPASS)
            {
                ESP_LOGE(TAG, " uart xQueueSend err");
            }
            ESP_LOGI(TAG, "count = %4d;Recv str: %s", rec_Count_tt, &data_tt[18]);
        }
    }
}

void uart_Send_task(void *args)
{
    char *databuff = (char *)malloc(BUF_SIZE);

    vTaskDelay(pdMS_TO_TICKS(100));
    while (1)
    {
        xQueueReceive(tcp_Queue, &(databuff[0]), portMAX_DELAY);
        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *)databuff, strlen(databuff));

        vTaskDelay(pdMS_TO_TICKS(50));
    }
    // 正常不会执行到这里
    vTaskDelete(NULL);
}

void uart_main_task(void *arg)
{
    char xReturned;
    ESP_LOGI(TAG, "echo_task create task");
    xReturned = xTaskCreate(echo_task, "echo_task", 4096, NULL, 5, NULL);
    xReturned = xTaskCreate(uart_Send_task, "uart_Send_task", 4096, NULL, 5, NULL);

    uart_Queue = xQueueCreate(10, 100 * sizeof(char));

    vTaskDelete(NULL);
}