#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"

#include "rtc_memory.h"

#define LOG_LOCAL_LEVEL ESP_LOG_NONE



static const char *TAG = "io";

//GPIO16 	LED
//GPIO0		FLASH_KEY
#define GPIO_OUTPUT_IO_0    16

#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0))

#define GPIO_INPUT_IO_0     0
#define GPIO_INPUT_IO_1     5
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))

static xQueueHandle gpio_evt_queue = NULL;
static xQueueHandle led_evt_queue = NULL;
static xQueueHandle led_fre_queue = NULL;

static void gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void *arg)
{
    uint32_t io_num;
    for (;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            ESP_LOGI(TAG, "GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
            if (io_num == 0 && gpio_get_level(io_num) == 1)
            {
            	if (xTaskGetTickCount() > 2000*portTICK_RATE_MS)
            	restart_to_ap();
            }
        }
    }
}

void led_function(uint32_t fun)
{
    uint32_t fun_num = fun;
    xQueueSend(led_evt_queue, &fun_num, NULL);
}

void led_frequency(uint32_t fre)
{
    uint32_t fre_num = fre;
    xQueueSend(led_fre_queue, &fre_num, NULL);
}


void led_task(void *pvParameters)
{
	int cnt = 0;
	uint32_t fun_num;
	static uint32_t led_flag = 2;
	static uint32_t frequency = 1;
	led_evt_queue = xQueueCreate(10, sizeof(uint32_t));
	led_fre_queue = xQueueCreate(10, sizeof(uint32_t));
	while(1)
	{
		if (xQueueReceive(led_evt_queue, &fun_num, 1))
		{
			led_flag = fun_num;
		}
		if (xQueueReceive(led_fre_queue, &fun_num, 1))
		{
			frequency = fun_num;
		}
		//-------------------------------------------
		if (led_flag == 0)
		{
			gpio_set_level(GPIO_OUTPUT_IO_0, 1);
		}
		else if(led_flag == 1)
		{
			gpio_set_level(GPIO_OUTPUT_IO_0, 0);
		}
		else if (led_flag == 2)
		{
			vTaskDelay((500-frequency) / portTICK_RATE_MS);
			gpio_set_level(GPIO_OUTPUT_IO_0, (cnt++) % 2);
		}
	}
}

void io_config(void)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void *) GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void *) GPIO_INPUT_IO_1);

    //remove isr handler for gpio number.
    gpio_isr_handler_remove(GPIO_INPUT_IO_0);
    //hook isr handler for specific gpio pin again
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void *) GPIO_INPUT_IO_0);


    xTaskCreate(led_task, "led", 512, NULL, 3, NULL);
}


