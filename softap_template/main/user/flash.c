#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "portmacro.h"
#include "flash.h"


#include "urlencode.h"



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

void wifi_info_read(char *ssid,char *password)
{
	spi_flash_read(CONNECT_ADD,&connect_info,sizeof(connect_info));

	strcpy(ssid,connect_info.ssid);
	strcpy(password,connect_info.password);
	printf("read ssid = %s,password = %s\r\n",connect_info.ssid,connect_info.password);
}

void wifi_info_write(const char *ssid,const char *password)
{
	char password_decode[64] = {0};
	memset(connect_info.ssid,0,sizeof(connect_info.ssid));
	memset(connect_info.password,0,sizeof(connect_info.password));


	memcpy(connect_info.ssid,ssid,strlen(ssid));

	memcpy(password_decode,password,strlen(password));
	urldecode(password_decode);
	memcpy(connect_info.password,password_decode,strlen(password_decode));

	spi_flash_erase_sector(CONNECT_ADD/4096);
	spi_flash_write(CONNECT_ADD,&connect_info,sizeof(connect_info));
	printf("write ssid = %s,password = %s\r\n",connect_info.ssid,connect_info.password);
}
