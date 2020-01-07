#pragma once


esp_err_t start_wifi_ap(const char *ssid, const char *pass);

void start_wifi_station(void);

void wait_for_ip(void);

void getipv4_flag();
void loseipv4_flag();
