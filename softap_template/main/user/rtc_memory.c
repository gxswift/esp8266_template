#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <esp_log.h>
#include "esp_spi_flash.h"
#include "portmacro.h"
#include "freertos/queue.h"

static const char *TAG = "rtc";
#define RTC_MEM                  (RTC_USER_BASE)


uint32_t rtc_mem_read(uint8_t add)
{
	return READ_PERI_REG(RTC_MEM+add);
}


void rtc_mem_write(uint8_t add,uint32_t num)
{
	WRITE_PERI_REG(RTC_MEM+add,num);
}



//---------------------------------------------------------------------------------------------------
static xQueueHandle restart_evt_queue = NULL;
void restart_to_ap()
{
	uint32_t num = 0;
	xQueueSend(restart_evt_queue, &num, NULL);
}

void restart_to_sta()
{
	uint32_t num = 1;
	xQueueSend(restart_evt_queue, &num, NULL);
}

void restart_task(void *pvParameters)
{
	uint32_t restart;
	restart_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    while(1)
    {
        if (xQueueReceive(restart_evt_queue, &restart, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "restart val: %d\n", restart);
            if (restart == 0)//ap
            {
            	rtc_mem_write(0,0x12345678);
            }
            else
            {
            	rtc_mem_write(0,0xEFEFEFEF);

            }
            vTaskDelay(1000 / portTICK_RATE_MS);
  			esp_restart();//restart
        }
    }
}













