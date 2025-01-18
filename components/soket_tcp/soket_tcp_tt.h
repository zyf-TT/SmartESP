#ifndef _SOKET_TCP_TT_H_
#define _SOKET_TCP_TT_H_
 
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
// #include "protocol_examples_common.h"
// #include "addr_from_stdin.h"
#include "lwip/err.h"
#include "lwip/sockets.h"

#include "wifi_tt.h"
#include "uart_tt.h"
void send_data_task(void *pvParameters);
void recv_data_task(void *pvParameters);
void tcp_main_task(void *arg);
static void tcp_connect(void *pvParameters);
esp_err_t create_tcp_client();
void close_socket();
QueueHandle_t tcp_Queue;

#endif 