#include "soket_tcp_tt.h"
#include "global_config.h"
#define HOST_IP_ADDR "192.168.0.8"

#define PORT 8712

static const char *TAG = "soket_tcp";
static const char *payload = "Message from ESP32 ";

#define CONFIG_EXAMPLE_IPV4 1

/**
 * 创建一个TcpClient连接
 */
static struct sockaddr_in server_addr; // server地址
static int connect_socket = 0;         // 连接socket

esp_err_t create_tcp_client()
{
    ESP_LOGI(TAG, "will connect gateway ssid : %s port:%d",
             HOST_IP_ADDR, PORT);
    // 新建socket对象

    connect_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (connect_socket < 0)
    {
        ESP_LOGI(TAG, "Failed connect gateway ssid : %s port:%d",
                 HOST_IP_ADDR, PORT);
        // 新建失败后，关闭新建的socket，等待下次新建
        close(connect_socket);
        return ESP_FAIL;
    }
    // 配置连接服务器信息
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
    ESP_LOGI(TAG, "connectting server...");
    DELAY_MS(1000);
    // 连接服务器
    if (connect(connect_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        ESP_LOGE(TAG, "connect failed!");
        // 连接失败后，关闭之前新建的socket，等待下次新建
        close(connect_socket);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "connect success!");
    return ESP_OK;
}

bool g_rxtx_need_restart = false; // 异常后，重新连接标记
static void tcp_connect(void *pvParameters)
{
    while (1)
    {
        g_rxtx_need_restart = false;
        // 等待WIFI连接信号量(即等待ESP32作为STA连接到热点的信号)，死等
        xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);

        ESP_LOGI(TAG, "start tcp connected");
        TaskHandle_t tx_rx_task = NULL; // 任务句柄
        // 延时3S准备建立clien
        DELAY_MS(1000);
        ESP_LOGI(TAG, "create tcp Client");
        // 建立client
        int socket_ret = create_tcp_client();

        if (socket_ret == ESP_FAIL)
        {
            // 建立失败
            ESP_LOGI(TAG, "create tcp socket error,stop...");
            continue;
        }
        else
        {
            // 建立成功
            ESP_LOGI(TAG, "create tcp socket succeed...");
            // 建立tcp接收数据任务
            if (pdPASS != xTaskCreate(&recv_data_task, "recv_data", 4096, NULL, 4, NULL))
            {
                ESP_LOGE(TAG, "reStart Recv task create fail!");
            }
            else
            {
                ESP_LOGI(TAG, "reStart Recv task create succeed!");
            }
            DELAY_MS(100);
            if (pdPASS != xTaskCreate(&send_data_task, "send_data", 2048, NULL, 5, NULL))
            {
                ESP_LOGE(TAG, "reStart send_data create fail!");
            }
            else
            {
                ESP_LOGI(TAG, "reStart send_data create succeed!");
            }
        }
        while (1)
        { // 每3秒钟检查一次是否需要重新连接
            DELAY_MS(3000);
            if (g_rxtx_need_restart)
            {
                DELAY_MS(3000);
                ESP_LOGI(TAG, "reStart create tcp client...");
                // 建立client
                int socket_ret = create_tcp_client();

                if (socket_ret == ESP_FAIL)
                {
                    ESP_LOGE(TAG, "reStart create tcp socket error,stop...");
                    continue;
                }
                else
                {
                    ESP_LOGI(TAG, "reStart create tcp socket succeed...");
                    // 重新建立完成，清除标记
                    g_rxtx_need_restart = false;
                    // 建立tcp接收数据任务
                    if (pdPASS != xTaskCreate(&recv_data_task, "recv_data", 4096, NULL, 4, NULL))
                    {
                        ESP_LOGE(TAG, "reStart Recv task create fail!");
                    }
                    else
                    {
                        ESP_LOGI(TAG, "reStart Recv task create succeed!");
                    }
                    DELAY_MS(100);
                    if (pdPASS != xTaskCreate(&send_data_task, "send_data", 2048, NULL, 5, NULL))
                    {
                        ESP_LOGE(TAG, "reStart send_data create fail!");
                    }
                    else
                    {
                        ESP_LOGI(TAG, "reStart send_data create succeed!");
                    }
                }
            }
        }
    }

    vTaskDelete(NULL); // 删除任务本身
}
void send_data_task(void *pvParameters)
{
    char data[40];
    int err;
    ESP_LOGI(TAG, " send_data_task is runing");
    while (1)
    {
        xQueueReceive(uart_Queue, &(data[0]), portMAX_DELAY);
        err = send(connect_socket, data, strlen(data), 0);
        if (err < 0)
        {
            ESP_LOGE(TAG, "Error occurred during send");
        }
    }
}
void recv_data_task(void *pvParameters)
{
    int len = 0; // 长度
    int recv_cnt = 0;
    char databuff[100]; // 缓存
    while (1)
    {
        // 清空缓存
        memset(databuff, 0x00, sizeof(databuff));
        // 读取接收数据
        len = recv(connect_socket, databuff, sizeof(databuff), 0);
        bool g_rxtx_need_restart = false;
        if (len > 0)
        {
            // 打印接收到的数组
            ESP_LOGI(TAG, "cnt=%4d, recvData: %s", ++recv_cnt, databuff);
            // 接收数据回发
            // send(connect_socket, databuff, strlen(databuff), 0);
            if (xQueueSend(tcp_Queue, &databuff[0], (TickType_t)10) != pdPASS)
            {
                ESP_LOGE(TAG, " uart xQueueSend err");
            }
        }
        else
        {
            // 打印错误信息
            ESP_LOGI(TAG, "recvData error");
            // 连接异常，至断线重连标志为1，以便tcp_connect连接任务进行重连
            bool g_rxtx_need_restart = true;
            break;
        }
        DELAY_MS(10);
    }
    close_socket();
    bool g_rxtx_need_restart = true;
    vTaskDelete(NULL);
}

void close_socket()
{
    close(connect_socket); // 关闭客户端
}
void tcp_main_task(void *arg)
{
    char xReturned;
    ESP_LOGI(TAG, "tcp_main_task create task");
    xReturned = xTaskCreate(tcp_connect, "tcp_connect", 4096, NULL, 6, NULL);

    if (xReturned == pdPASS)
    {
        ESP_LOGI(TAG, "Creation is succeed");
        /* The task was created.  Use the task's handle to delete the task. */
    }
    else
    {
        ESP_LOGI(TAG, "Creation is failed");
    }
    tcp_Queue = xQueueCreate(10, 100 * sizeof(char));
    vTaskDelete(NULL);
}