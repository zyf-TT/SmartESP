#ifndef _UART_TT_H_
#define _UART_TT_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

#include "soket_tcp_tt.h"

void echo_task(void *arg);
void uart_Send_task(void *args);
void uart_main_task(void *arg);
// typedef struct Message
// {
//     char MessageID;
//     char Data[ 20 ];
// }Message_tt;

QueueHandle_t uart_Queue;
#endif 