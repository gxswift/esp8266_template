#pragma once


uint32_t rtc_mem_read(uint8_t add);
void rtc_mem_write(uint8_t add,uint32_t num);


void restart_to_ap();
void restart_to_sta();
void restart_task(void *pvParameters);
