#include "common.h"

const char *TAG = "G4";
volatile RTC_NOINIT_ATTR uint8_t count;
xQueueHandle barrier_evt_q = NULL;
xQueueHandle count_display_q = NULL;
xQueueHandle count_publish_q = NULL;
volatile bool in_testing_scenario = false;
struct tm current_time;

void init_logging()
{
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "Start");
}

void init_common_queues()
{
    if (barrier_evt_q == NULL)
    {
        barrier_evt_q = xQueueCreate(BARRIER_EVT_Q_SIZE, sizeof(barrier_evt_q_item));
    }
    if (count_display_q == NULL)
    {
        count_display_q = xQueueCreate(COUNT_DISPLAY_Q_SIZE, sizeof(count_display_q_item));
    }
    if (count_publish_q == NULL)
    {
        count_publish_q = xQueueCreate(COUNT_DISPLAY_Q_SIZE, sizeof(count_display_q_item));
    }
}

void init_esp_dependencies()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
}

void init_counter()
{
	esp_reset_reason_t reason = esp_reset_reason();
	switch (reason)
	{
	case ESP_RST_POWERON:
		count = 0;
		ESP_LOGI(TAG, "Power reset");
		break;
	case ESP_RST_SW:
		ESP_LOGI(TAG, "Software reset");
		break;
	default:
		ESP_LOGW(TAG, "Unknown reset reason.");
		break;
	}
}
