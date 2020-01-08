#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "portmacro.h"
#include "flash.h"

void get_chip_info()
{
    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP8266 chip with %d CPU cores, WiFi, ",
            chip_info.cores);

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
}

#define CONNECT_ADD	500*4096


wifi_connect_info connect_info;

void wifi_info_read()
{
	spi_flash_read(CONNECT_ADD,&connect_info,sizeof(connect_info));
	printf("read ssid = %s,len = %d,password = %s,len = %d\r\n",connect_info.ssid,connect_info.ssidlen,connect_info.password,connect_info.passwordlen);
}

void wifi_info_write()
{
	spi_flash_erase_sector(CONNECT_ADD/4096);
	spi_flash_write(CONNECT_ADD,&connect_info,sizeof(connect_info));
	printf("write ssid = %s,len = %d,password = %s,len = %d\r\n",connect_info.ssid,connect_info.ssidlen,connect_info.password,connect_info.passwordlen);
}
