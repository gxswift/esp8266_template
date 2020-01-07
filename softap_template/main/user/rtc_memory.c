#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "portmacro.h"



#define RTC_MEM                  (RTC_USER_BASE)


uint32_t rtc_mem_read(uint8_t add)
{
	return READ_PERI_REG(RTC_MEM+add);
}


void rtc_mem_write(uint8_t add,uint32_t num)
{

	WRITE_PERI_REG(RTC_MEM+add,num);
}

















