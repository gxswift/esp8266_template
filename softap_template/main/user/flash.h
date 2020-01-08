#pragma once


typedef struct
{
	char ssid[32];
	char password[64];
	uint8_t channel_primary;
	uint8_t channel_second;
}wifi_connect_info;



void wifi_info_read(char *ssid,char *password);

void wifi_info_write(const char *ssid,const char *password);









