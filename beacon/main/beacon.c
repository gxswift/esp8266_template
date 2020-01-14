/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <stdbool.h>
#include "stdlib.h"
#include <stdio.h>
#include "string.h"


#include "esp_libc.h"
// run-time variables
char emptySSID[32];
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint32_t currentTime = 0;
uint32_t packetSize = 0;
uint32_t packetCounter = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;

// beacon frame definition
uint8_t beaconPacket[109] = {
  /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00, // Type/Subtype: managment beacon frame
  /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
  /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source
  /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source

  // Fixed parameters
  /* 22 - 23 */ 0x00, 0x00, // Fragment & sequence number (will be done by the SDK)
  /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  /* 32 - 33 */ 0xe8, 0x03, // Interval: 0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
  /* 34 - 35 */ 0x31, 0x00, // capabilities Tnformation

  // Tagged parameters

  // SSID parameters
  /* 36 - 37 */ 0x00, 0x20, // Tag: Set SSID length, Tag length: 32
  /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, // SSID

  // Supported Rates
  /* 70 - 71 */ 0x01, 0x08, // Tag: Supported Rates, Tag length: 8
  /* 72 */ 0x82, // 1(B)
  /* 73 */ 0x84, // 2(B)
  /* 74 */ 0x8b, // 5.5(B)
  /* 75 */ 0x96, // 11(B)
  /* 76 */ 0x24, // 18
  /* 77 */ 0x30, // 24
  /* 78 */ 0x48, // 36
  /* 79 */ 0x6c, // 54

  // Current Channel
  /* 80 - 81 */ 0x03, 0x01, // Channel set, length
  /* 82 */      0x01,       // Current Channel

  // RSN information
  /*  83 -  84 */ 0x30, 0x18,
  /*  85 -  86 */ 0x01, 0x00,
  /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
  /*  91 -  92 */ 0x02, 0x00,
  /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04, /*Fix: changed 0x02(TKIP) to 0x04(CCMP) is default. WPA2 with TKIP not supported by many devices*/
  /* 101 - 102 */ 0x01, 0x00,
  /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
  /* 107 - 108 */ 0x00, 0x00
};


// ===== Settings ===== //
const uint8_t channels[] = {1, 6, 11}; // used Wi-Fi channels (available: 1-14)
const bool wpa2 = true; // WPA2 networks	false
const bool appendSpaces = true; // makes all SSIDs 32 characters long to improve performance

/*
  SSIDs:
  - don't forget the \n at the end of each SSID!
  - max. 32 characters per SSID
  - don't add duplicates! You have to change one character at least
*/
const char ssids[] = {
 /* "看有好多wifi\n"
  "真的好多wifi\n"
  "连上看看吧\n"
  "怎么连不上\n"
  "那连下一个\n"
  "再试试\n"
  "继续试\n"
  "总能连上一个的\n"
  "还是连不上\n"
  "那我们来学古诗吧\n"
  "来来来\n"
  "准备\n"
  "开始\n"
  "一片一片又一片\n"
  "两片三片四五片\n"
  "六片七片八九片\n"
  "飞入芦花都不见\n"
  "2M宽带\n"
  "4M宽带\n"
  "8M宽带\n"
  "20M宽带\n"
  "100M宽带\n"
  "好多M宽带\n"
  "连上一个就行了\n"*/
		"临江仙\n"
		"晏几道·宋\n"
		"梦后楼台高锁\n"
		"酒醒帘幕低垂\n"
		"去年春恨却来时\n"
		"落花人独立\n"
		"微雨燕双飞\n"
		"记得小苹初见\n"
		"两重心字罗衣\n"
		"琵琶弦上说相思\n"
		"当时明月在\n"
		"曾照彩云归\n"
/*
  "锦瑟\n"
  "李商隐\n"
  "锦瑟无端五十弦\n"
  "一弦一柱思华年\n"
  "庄生晓梦迷蝴蝶\n"
  "望帝春心托杜鹃\n"
  "沧海月明珠有泪\n"
  "蓝田日暖玉生烟\n"
  "此情可待成追忆\n"
  "只是当时已惘然\n"
  */
/*  "来，继续\n"
  "下一首\n"
  "wifi太多了\n"
  "好幸福\n"
  "低头思故乡\n"
  "举头望明月\n"
  "疑是地上霜\n"
  "床前明月光\n"
  "李白\n"
  "静夜思\n"*/
};
// goes to next channel
void nextChannel() {
  if(sizeof(channels) > 1){
    uint8_t ch = channels[channelIndex];
    channelIndex++;
    if (channelIndex > sizeof(channels)) channelIndex = 0;

    if (ch != wifi_channel && ch >= 1 && ch <= 14) {
      wifi_channel = ch;
      esp_wifi_set_channel(wifi_channel,0);
    }
  }
}





void task(void *pvParameters)
{
    for (int i = 0; i < 32; i++)
      emptySSID[i] = ' ';
	  packetSize = sizeof(beaconPacket);
	  if (wpa2) {
	    beaconPacket[34] = 0x31;
	  } else {
	    beaconPacket[34] = 0x21;
	    packetSize -= 26;
	  }

	  macAddr[0] = 0xD8;
	  macAddr[1] = 0x8F;
	  macAddr[2] = 0x76;
	  macAddr[3] = 0xAD;
	  macAddr[4] = 0xD8;

	while(1)
	{
	currentTime = xTaskGetTickCount();

	  // send out SSIDs
	  if (currentTime - attackTime > 100/portTICK_PERIOD_MS) {
	    attackTime = currentTime;

	    // temp variables
	    int i = 0;
	    int j = 0;
	    int ssidNum = 1;
	    char tmp;
	    int ssidsLen = strlen(ssids);

	    // go to next channel
	    nextChannel();
	    while (i < ssidsLen) {
	      // read out next SSID
	      j = 0;
	      do {
	        tmp = ssids [ i + j];
	        j++;
	      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

	      uint8_t ssidLen = j - 1;
	      // set MAC address
	      macAddr[5] = ssidNum;
	      ssidNum++;
	      // write MAC address into beacon frame
	      memcpy(&beaconPacket[10], macAddr, 6);
	      memcpy(&beaconPacket[16], macAddr, 6);
	      // reset SSID
	      memcpy(&beaconPacket[38], emptySSID, 32);
	      // write new SSID into beacon frame
	      memcpy(&beaconPacket[38], &ssids[i], ssidLen);
	      // set channel for beacon frame
	      beaconPacket[82] = wifi_channel;
	      // send packet
	      if(appendSpaces){
	        for(int k=0;k<3;k++){
	        	if(esp_wifi_80211_tx(WIFI_IF_STA,beaconPacket, packetSize, 0) == 0)
	        	{
	        		packetCounter++;
	        	}
	          vTaskDelay(10/ portTICK_PERIOD_MS);
	        }
	      }
	      // remove spaces
	      else {
	        uint16_t tmpPacketSize = (packetSize - 32) + ssidLen; // calc size
	        uint8_t* tmpPacket = malloc(tmpPacketSize); // create packet buffer
	        memcpy(&tmpPacket[0], &beaconPacket[0], 38 + ssidLen); // copy first half of packet into buffer
	        tmpPacket[37] = ssidLen; // update SSID length byte
	        memcpy(&tmpPacket[38 + ssidLen], &beaconPacket[70], wpa2 ? 39 : 13); // copy second half of packet into buffer
	        // send packet
	        for(int k=0;k<3;k++){
	        	if(esp_wifi_80211_tx(WIFI_IF_STA,beaconPacket, packetSize, 0) == 0)
	        		packetCounter++;
	          vTaskDelay(10/ portTICK_PERIOD_MS);
	        }
	        free( tmpPacket); // free memory of allocated buffer
	      }

	      i += j;
	    }
	  }
	  // show packet-rate each second
	  if (currentTime - packetRateTime > 1000) {
	    packetRateTime = currentTime;
	    packetCounter = 0;
	  }
	}
}



#define TAG "beacon"

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* For accessing reason codes in case of disconnection */
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
        break;

    case SYSTEM_EVENT_AP_STA_GOT_IP6:
    default:
        break;
    }
    return ESP_OK;
}


static void initialise_wifi(void)
{
    tcpip_adapter_init();
	ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
   // esp_wifi_set_channel(channels[0],WIFI_SECOND_CHAN_NONE);
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
}

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();
    xTaskCreate(task, "task", 4096, NULL, 5, NULL);
}



