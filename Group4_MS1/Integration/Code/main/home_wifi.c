#include "common.h"
#include "home_wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASS CONFIG_ESP_WIFI_PASS
#define ESP_WIFI_MAXIMUM_RETRY CONFIG_ESP_WIFI_MAXIMUM_RETRY

static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
static volatile bool connected = false;

const char *WIFI_TAG = "G4-WIFI";

static void event_handler(void *arg, esp_event_base_t event_base,
						  int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		esp_wifi_connect();
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		connected = false;
		if (s_retry_num < ESP_WIFI_MAXIMUM_RETRY)
		{
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(WIFI_TAG, "retry to connect to the AP");
		}
		else
		{
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(WIFI_TAG, "connect to the AP fail");
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
		ESP_LOGI(WIFI_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		connected = true;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
	else
	{
		ESP_LOGI(WIFI_TAG, "Unknown event received");
	}
}

void wait_until_connection()
{
	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
										   WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
										   pdFALSE,
										   pdFALSE,
										   portMAX_DELAY);

	if (bits & WIFI_FAIL_BIT)
	{
		ESP_LOGW(WIFI_TAG, "No connection after %d tries. Restart.", ESP_WIFI_MAXIMUM_RETRY);
		esp_restart();
	}
	ESP_LOGI(WIFI_TAG, "connected to ap SSID:%s password:%s", ESP_WIFI_SSID, ESP_WIFI_PASS);
	xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
}

void loop_wifi()
{
	if (!connected)
	{
		ESP_LOGW(WIFI_TAG, "Connection lost. Start reconnection.");
		wait_until_connection();
	}
}

void setup_wifi()
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	// Creates a station network interface object
	esp_netif_create_default_wifi_sta();

	// Obtain a default configuration
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	// Create a WiFi driver task and initialize the driver
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&event_handler,
														NULL,
														&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														IP_EVENT_STA_GOT_IP,
														&event_handler,
														NULL,
														&instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = ESP_WIFI_SSID,
			.password = ESP_WIFI_PASS,
			/* Setting a password implies station will connect to all security modes including WEP/WPA.
				* However these modes are deprecated and not advisable to be used. Incase your Access point
				* doesn't support WPA2, these mode can be enabled by commenting below line */
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
			.pmf_cfg = {
				.capable = true,
				.required = false},
		},
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(WIFI_TAG, "wifi_init_sta finished.");

	wait_until_connection();
}
