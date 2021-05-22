#include "common.h"
#include "publisher.h"
#include "time_management.h"
#include "mqtt_message.h"

#define IOT_PLATFORM_GATEWAY_IP CONFIG_IOT_PLATFORM_GATEWAY_IP
#define IOT_PLATFORM_GATEWAY_PORT CONFIG_IOT_PLATFORM_GATEWAY_PORT
#define IOT_PLATFORM_GATEWAY_USERNAME CONFIG_IOT_PLATFORM_GATEWAY_USERNAME
#define IOT_PLATFORM_GATEWAY_PASSWORD CONFIG_IOT_PLATFORM_GATEWAY_PASSWORD

#define IOT_PLATFORM_GROUP CONFIG_IOT_PLATFORM_GROUP

#define IOT_PLATFORM_USER_ID CONFIG_IOT_PLATFORM_USER_ID
#define IOT_PLATFORM_DEVICE_ID CONFIG_IOT_PLATFORM_DEVICE_ID
#define IOT_PLATFORM_COUNT_SENSOR_NAME CONFIG_IOT_PLATFORM_COUNT_SENSOR_NAME
#define IOT_PLATFORM_RESTART_SENSOR_NAME CONFIG_IOT_PLATFORM_RESTART_SENSOR_NAME

#define MQTT_BROKER_URI "mqtt://" IOT_PLATFORM_GATEWAY_IP
#define MQTT_TOPIC            \
	STR(IOT_PLATFORM_USER_ID) \
	"_" STR(IOT_PLATFORM_DEVICE_ID)

#define MQTT_COUNT_MESSAGE_PATTERN "{\"username\":\"%s\",\"%s\":%d,\"device_id\":%d,\"timestamp\":%llu}"
#define MQTT_COUNT_MESSAGE_BUFFER_SIZE 100
static char MQTT_COUNT_MESSAGE_BUFFER[MQTT_COUNT_MESSAGE_BUFFER_SIZE];

#define MQTT_RESTART_MESSAGE_PATTERN "{\"username\":\"%s\",\"%s\":1,\"device_id\":%d,\"timestamp\":%llu}"
#define MQTT_RESTART_MESSAGE_BUFFER_SIZE 100
static char MQTT_RESTART_MESSAGE_BUFFER[MQTT_RESTART_MESSAGE_BUFFER_SIZE];

// 10^6 microsec * 60 * 16 = 15 min
#define TIMER_INTERVAL_PUBLISH_COUNT (1000000 * 60 * 15)

static esp_mqtt_client_handle_t count_mqtt_client;

static const char *TAG_PUB = "G4-PUB";

static void setup_periodic_timer();
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);
static void publish(char *message);
static void publish_restart();
static void periodic_timer_callback(void *arg);

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{

	switch (event->event_id)
	{
	case MQTT_EVENT_CONNECTED:
		ESP_LOGI(TAG_PUB, "MQTT_EVENT_CONNECTED");
		publish_restart();
		break;
	case MQTT_EVENT_DISCONNECTED:
		ESP_LOGI(TAG_PUB, "MQTT_EVENT_DISCONNECTED");
		break;
	case MQTT_EVENT_SUBSCRIBED:
		ESP_LOGI(TAG_PUB, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_UNSUBSCRIBED:
		ESP_LOGI(TAG_PUB, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		break;
	case MQTT_EVENT_PUBLISHED:
		ESP_LOGI(TAG_PUB, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		break;
	default:
		ESP_LOGI(TAG_PUB, "Other event id:%d", event->event_id);
		break;
	}

	return ESP_OK;
}

static void publish(char *message)
{
	ESP_LOGI(TAG_PUB, "Publish message: %s", message);
	esp_mqtt_client_publish(count_mqtt_client, MQTT_TOPIC, message, 0, 1, 0);
}

static void publish_restart()
{
	uint64_t current_epoch_time = read_epoch_time_in_msec();
	snprintf(MQTT_RESTART_MESSAGE_BUFFER, MQTT_RESTART_MESSAGE_BUFFER_SIZE, MQTT_RESTART_MESSAGE_PATTERN,
			 IOT_PLATFORM_GROUP, IOT_PLATFORM_RESTART_SENSOR_NAME, IOT_PLATFORM_DEVICE_ID, current_epoch_time);
	publish(MQTT_RESTART_MESSAGE_BUFFER);
}

void publish_count()
{
	uint64_t current_epoch_time = read_epoch_time_in_msec();
	snprintf(MQTT_COUNT_MESSAGE_BUFFER, MQTT_COUNT_MESSAGE_BUFFER_SIZE, MQTT_COUNT_MESSAGE_PATTERN,
			 IOT_PLATFORM_GROUP, IOT_PLATFORM_COUNT_SENSOR_NAME, count, IOT_PLATFORM_DEVICE_ID, current_epoch_time);
	publish(MQTT_COUNT_MESSAGE_BUFFER);
}

static void setup_periodic_timer()
{
	// Create timer
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &periodic_timer_callback,
		/* name is optional, but may help identify the timer when debugging */
		.name = "periodic"};
	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, TIMER_INTERVAL_PUBLISH_COUNT));
}

static void periodic_timer_callback(void *arg)
{
	publish_count();
}

void setup_publisher()
{
	ESP_LOGI(TAG_PUB, "Use URI: %s", MQTT_BROKER_URI);
	ESP_LOGI(TAG_PUB, "Use Topic: %s", MQTT_TOPIC);

	const esp_mqtt_client_config_t mqtt_cfg = {
		.uri = MQTT_BROKER_URI,
		.port = IOT_PLATFORM_GATEWAY_PORT,
		.username = IOT_PLATFORM_GATEWAY_USERNAME,
		.password = IOT_PLATFORM_GATEWAY_PASSWORD,
		.event_handle = mqtt_event_handler};

	count_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	esp_mqtt_client_start(count_mqtt_client);

	setup_periodic_timer();
}
