/* SoftAP based Provisioning Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <lwip/err.h>
#include <lwip/sys.h>

#include <protocomm_security.h>
#include <wifi_provisioning/wifi_config.h>
#include <stdbool.h>

#include "lwip/apps/httpd.h"
#include "wifi_sta.h"
#include "web.h"
#include "tcp_task.h"
#include "rtc_memory.h"
#include "io.h"
#include "sntp.h"

#include "lwip/apps/netbiosns.h"


static const char *TAG = "app";


#define LOG_LOCAL_LEVEL ESP_LOG_NONE

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	system_event_info_t *info = &event->event_info;
    /* Invoke Provisioning event handler first */
  //  app_prov_event_handler(ctx, event);

    switch(event->event_id) {
    case SYSTEM_EVENT_AP_START:
        ESP_LOGI(TAG, "SoftAP started");
        break;
    case SYSTEM_EVENT_AP_STOP:
        ESP_LOGI(TAG, "SoftAP stopped");
        break;
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
    	getipv4_flag();
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI(TAG, "station:"MACSTR"leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGE(TAG, "Disconnect reason : %d", info->disconnected.reason);
        if (info->disconnected.reason == WIFI_REASON_BASIC_RATE_NOT_SUPPORT) {
            /*Switch to 802.11 bgn mode */
            esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCAL_11B | WIFI_PROTOCAL_11G | WIFI_PROTOCAL_11N);
        }
        esp_wifi_connect();
        loseipv4_flag();
#ifdef CONFIG_EXAMPLE_IPV6
        xEventGroupClearBits(wifi_event_group, IPV6_GOTIP_BIT);
#endif
        break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
#ifdef CONFIG_EXAMPLE_IPV6
        xEventGroupSetBits(wifi_event_group, IPV6_GOTIP_BIT);
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP6");

        char *ip6 = ip6addr_ntoa(&event->event_info.got_ip6.ip6_info.ip);
        ESP_LOGI(TAG, "IPv6: %s", ip6);
#endif
        break;
    default:
        break;
    }
    return ESP_OK;
}


#include "led_sample.h"
void app_main()
{
    /* Security version */
    int security = 0;
    /* Proof of possession */
    const protocomm_security_pop_t *pop = NULL;

#ifdef CONFIG_USE_SEC_1
    security = 1;
#endif

    /* Having proof of possession is optional */
#ifdef CONFIG_USE_POP
    const static protocomm_security_pop_t app_pop = {
        .data = (uint8_t *) CONFIG_POP,
        .len = (sizeof(CONFIG_POP)-1)
    };
    pop = &app_pop;
#endif

    /* Initialize networking stack */
    ESP_ERROR_CHECK( nvs_flash_init() );
    tcpip_adapter_init();

    /* Set our event handling */
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    /* Check if device is provisioned */
    io_config();
    xTaskCreate(restart_task, "restart", 1024, NULL, 8, NULL);

    uint32_t provisioned = rtc_mem_read(0);
    printf("rtc mem = %x,tick = %ld\r\n",provisioned,xTaskGetTickCount());
    if (provisioned == 0x12345678) {
        /* If not provisioned, start provisioning via soft AP */
        ESP_LOGI(TAG, "Starting WiFi SoftAP provisioning");

        const char *ssid = NULL;

#ifdef CONFIG_SOFTAP_SSID
        ssid = CONFIG_SOFTAP_SSID;
#else
        uint8_t eth_mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, eth_mac);

        char ssid_with_mac[33];
        snprintf(ssid_with_mac, sizeof(ssid_with_mac), "PROV_%02X%02X%02X",
                 eth_mac[3], eth_mac[4], eth_mac[5]);

        ssid = ssid_with_mac;
#endif
//--------------------------------------------------------------------------------------
        esp_err_t err = start_wifi_ap(ssid, "12345678");
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to start WiFi AP");
        }
        httpd_init();
        httpd_cgi_init();
//--------------------------------------------------------------------------------------
 //       app_prov_start_softap_provisioning(ssid, CONFIG_SOFTAP_PASS,security, pop);
    } else {
        /* Start WiFi station with credentials set during provisioning */
        ESP_LOGI(TAG, "Starting WiFi station");
        start_wifi_station();
        wait_for_ip();
#if 0
        xTaskCreate(tcp_client_task, "tcp_client", 4096, NULL, 5, NULL);
        xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);
        netbiosns_set_name("lwip_net");
        netbiosns_init();
#else
        xTaskCreate(main_task, "main", 1024*20, NULL, 5, NULL);
#endif
       // xTaskCreate(sntp_example_task, "sntp_example_task", 2048, NULL, 10, NULL);
    }
}
