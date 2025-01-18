#ifndef _WIFI_TT_H_
#define _WIFI_TT_H_
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "soket_tcp_tt.h"

EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT 1<<0
#define WIFI_FAIL_BIT      1<<1
void wifi_conncet_task(void *arg);
void wifi_main_task(void *arg);

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

#endif 