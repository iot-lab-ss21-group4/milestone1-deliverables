#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_sntp.h"
#include "mqtt_client.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sys.h"

#define INNER_BARRIER_PIN CONFIG_INNER_BARRIER_PIN
#define OUTER_BARRIER_PIN CONFIG_OUTER_BARRIER_PIN
#define TRIGGER_PIN_IN CONFIG_TRIGGER_PIN_IN
#define TRIGGER_PIN_OUT CONFIG_TRIGGER_PIN_OUT
#define ONBOARD_LED_PIN 19
#define GET_CLOUD_EVENTS CONFIG_GET_CLOUD_EVENTS
#define TEST_ROOM_COMMANDS CONFIG_TEST_ROOM_COMMANDS
#define BARRIER_EVT_Q_SIZE 32
#define COUNT_DISPLAY_Q_SIZE 32
// minimum count of people in the room
#define MIN_ROOM_COUNT 0
// assuming that count_display_q_item is an unsigned type
#define MAX_ROOM_COUNT ((count_display_q_item)(-1))
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)       \
    ((byte)&0x80 ? '1' : '0'),     \
        ((byte)&0x40 ? '1' : '0'), \
        ((byte)&0x20 ? '1' : '0'), \
        ((byte)&0x10 ? '1' : '0'), \
        ((byte)&0x08 ? '1' : '0'), \
        ((byte)&0x04 ? '1' : '0'), \
        ((byte)&0x02 ? '1' : '0'), \
        ((byte)&0x01 ? '1' : '0')
#define FIRST_N_BITMASK(n) ((1 << n) - 1)
#define POWER_OF_TWO(x) (1 << (x))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define LOGICAL_NOT(c) ((c) ? false : true)
#define STATIC_ASSERT(COND, MSG) typedef char static_assertion_##MSG[(COND) ? 1 : -1]
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

typedef uint8_t barrier_evt_q_item;
typedef uint8_t count_display_q_item;

extern const char *TAG;
extern volatile RTC_NOINIT_ATTR uint8_t count;
extern xQueueHandle barrier_evt_q;
extern xQueueHandle count_display_q;
extern xQueueHandle count_publish_q;
extern volatile bool in_testing_scenario;
extern struct tm current_time;

void init_logging();
void init_common_queues();
void init_esp_dependencies();
void init_counter();

#endif
