#include <string.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_wifi.h>
#include <nvs_flash.h>
#include <nvs.h>

#include "freertos/event_groups.h"

static const char *TAG = "ap";
esp_err_t start_wifi_ap(const char *ssid, const char *pass)
{
    /* Initialize WiFi with default configuration */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init WiFi : %d", err);
        return err;
    }

    /* Build WiFi configuration for AP mode */
    wifi_config_t wifi_config = {
        .ap = {
            .max_connection = 5,
        },
    };

    strncpy((char *) wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    wifi_config.ap.ssid_len = strlen(ssid);

    if (strlen(pass) == 0) {
        memset(wifi_config.ap.password, 0, sizeof(wifi_config.ap.password));
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    } else {
        strncpy((char *) wifi_config.ap.password, pass, sizeof(wifi_config.ap.password));
        wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    }

    /* Start WiFi in AP mode with configuration built above */
    err = esp_wifi_set_mode(WIFI_MODE_AP);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi mode : %d", err);
        return err;
    }
    err = esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi config : %d", err);
        return err;
    }
    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start WiFi : %d", err);
        return err;
    }
    return ESP_OK;
}



//------------------------------------------------------------------------------------------------------

#define CONFIG_WIFI_SSID "Mototek"
#define CONFIG_WIFI_PASSWORD "Mototek@"


#define EXAMPLE_WIFI_SSID CONFIG_WIFI_SSID
#define EXAMPLE_WIFI_PASS CONFIG_WIFI_PASSWORD



static EventGroupHandle_t wifi_event_group;

const int IPV4_GOTIP_BIT = BIT0;
#ifdef CONFIG_EXAMPLE_IPV6
const int IPV6_GOTIP_BIT = BIT1;
#endif
//esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
void start_wifi_station(void)
{
    wifi_event_group = xEventGroupCreate();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
            .password = EXAMPLE_WIFI_PASS,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
 //   ESP_ERROR_CHECK( esp_wifi_set_channel(11,0) );//set channel  connect in 6s,without set in 10s
    ESP_ERROR_CHECK( esp_wifi_start() );
}

void getipv4_flag()
{
	xEventGroupSetBits(wifi_event_group, IPV4_GOTIP_BIT);
}
void loseipv4_flag()
{
	xEventGroupClearBits(wifi_event_group, IPV4_GOTIP_BIT);
}
void wait_for_ip(void)
{
#ifdef CONFIG_EXAMPLE_IPV6
    uint32_t bits = IPV4_GOTIP_BIT | IPV6_GOTIP_BIT;
#else
    uint32_t bits = IPV4_GOTIP_BIT;
#endif

    ESP_LOGI(TAG, "Waiting for AP connection...");
    xEventGroupWaitBits(wifi_event_group, bits, false, true, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to AP");
}


