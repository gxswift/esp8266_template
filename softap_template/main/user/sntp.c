#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "nvs_flash.h"

#include "lwip/apps/sntp.h"



static const char *TAG = "sntp_example";


void sntp_example_task(void *arg)
{
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

/*    time(&now);
    localtime_r(&now, &timeinfo);
*/
    // Is time set? If not, tm_year will be (1970 - 1900).
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

    // Set timezone to Eastern Standard Time and print local time
    // setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    // tzset();

    // Set timezone to China Standard Time
 /*   setenv("TZ", "CST-8", 1);
    tzset();*/

    while (1) {
        // update 'now' variable with current time
 /*       time(&now);
        localtime_r(&now, &timeinfo);*/
/*
		strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
		ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
*/
        ESP_LOGI(TAG, "Free heap size: %d\n", esp_get_free_heap_size());
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
