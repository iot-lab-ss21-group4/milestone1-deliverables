#include "common.h"
#include "ssd1306.h"
#include "oled.h"
#include "time_management.h"

#define IOT_LAB_COURSE_GROUP 4
#define GROUP_PATTERN "G%d"
#define TIME_PATTERN "%H:%M"
#define GROUP_AND_TIME_PATTERN "%s  %s"
#define COUNTING_PATTERN "%d %d"

#define GROUP_BUFFER_SIZE 3
#define TIME_BUFFER_SIZE 6
#define GROUP_AND_TIME_BUFFER_SIZE (GROUP_BUFFER_SIZE + 2 + TIME_BUFFER_SIZE)
#define COUNTING_BUFFER_SIZE 9

static char GROUP_BUFFER[GROUP_BUFFER_SIZE];
static char TIME_BUFFER[TIME_BUFFER_SIZE];
static char GROUP_AND_TIME_BUFFER[GROUP_AND_TIME_BUFFER_SIZE];
static char COUNTING_BUFFER[COUNTING_BUFFER_SIZE];

static void showRoomState();
static void oled_update_task(void *);

static void showRoomState()
{
    ssd1306_clearScreen();

    snprintf(GROUP_BUFFER, GROUP_BUFFER_SIZE, GROUP_PATTERN, IOT_LAB_COURSE_GROUP);
    strftime(TIME_BUFFER, TIME_BUFFER_SIZE, TIME_PATTERN, &current_time);
    snprintf(GROUP_AND_TIME_BUFFER, GROUP_AND_TIME_BUFFER_SIZE, GROUP_AND_TIME_PATTERN, GROUP_BUFFER, TIME_BUFFER);

    // TODO: why in the slides 00  00 ?? One counter for the esp and one for the counter of another ESP ?
    snprintf(COUNTING_BUFFER, COUNTING_BUFFER_SIZE, COUNTING_PATTERN, count, 0);

    ssd1306_printFixedN(0, 0, GROUP_AND_TIME_BUFFER, STYLE_NORMAL, 1);
    ssd1306_printFixedN(0, 24, COUNTING_BUFFER, STYLE_NORMAL, 2);
}

static void oled_update_task(void *_)
{
    count_display_q_item local_count = 0;
    while (1)
    {
        while (xQueueReceive(count_display_q, &local_count, portMAX_DELAY) != pdTRUE)
            ;
        showRoomState();
    }
}

void setup_oled()
{
    ssd1306_128x64_i2c_init();
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    showRoomState();
    xTaskCreate(oled_update_task, "oled_update_task", 4096, NULL, 10, NULL);
}
