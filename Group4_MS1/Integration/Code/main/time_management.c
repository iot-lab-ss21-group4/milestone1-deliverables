#include "common.h"
#include "time_management.h"

#define SNTP_SERVER "pool.ntp.org"
#define SNTP_SYNC_RETRY 10

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void obtain_time(void)
{
    int retry = 0;
    const int retry_count = SNTP_SYNC_RETRY;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry++ < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    if (retry == retry_count)
    {
        ESP_LOGE(TAG, "Could not retrieve time.!\n");
        esp_restart();
    }
    // Set timezone to Europe/Berlin:
    // https://github.com/jdlambert/micro_tz_db/blob/98ddc684cd96727cb10213218434818474edc409/zones.c
    // Format: https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    ESP_LOGI(TAG, "System time set.");
}

struct tm read_time()
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    return timeinfo;
}

uint64_t read_epoch_time_in_msec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t time_in_ms = (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
    return time_in_ms;
}

void setup_time_management()
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, SNTP_SERVER);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    sntp_init();
    obtain_time();
    current_time = read_time();
}

void loop_time()
{
    struct tm tmp_current_time = read_time();
    if (current_time.tm_hour == 23 && tmp_current_time.tm_hour == 0)
    {
        count = 0;
        esp_restart();
    }
    if (tmp_current_time.tm_min != current_time.tm_min)
    {
        current_time = tmp_current_time;
        // TODO: esthetic feature: different change type for oled queue so that the whole display does not need to be erased
        xQueueSend(count_display_q, (const void *)&count, portMAX_DELAY);
    }
}
