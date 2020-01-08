#pragma once


typedef struct
{
	uint8_t ssid[20];
	uint8_t ssidlen;
	uint8_t password[20];
	uint8_t passwordlen;
	uint8_t channel_primary;
	uint8_t channel_second;

}wifi_connect_info;
extern wifi_connect_info connect_info;

void wifi_info_read();

void wifi_info_write();









