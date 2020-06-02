/*

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include <sys/socket.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "eth.h"
#include "dirent.h"
#include <lwip/api.h>
#include <lwip/dns.h>
#include <lwip/ip.h>
#include "wifi.h"
#include "tcpserv.h"

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:// station start
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED: //station disconnect from ap
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_CONNECTED: //station connect to ap
        break;
    case SYSTEM_EVENT_STA_GOT_IP:  //station got ip
		{
       char wifi_IP[50]={0};
			sprintf(wifi_IP,"%s",ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
    		ESP_LOGI("event", "SYSTEM_EVENT_STA_GOT_IP:%s\n", wifi_IP);		
		}
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:// a station connect to ap
    	ESP_LOGI("event", "station:"MACSTR" join,AID=%d\n",
		MAC2STR(event->event_info.sta_connected.mac),
		event->event_info.sta_connected.aid);
    	break;
    case SYSTEM_EVENT_AP_STADISCONNECTED://a station disconnect from ap
    	ESP_LOGI("event", "station:"MACSTR"leave,AID=%d\n",
		MAC2STR(event->event_info.sta_disconnected.mac),
		event->event_info.sta_disconnected.aid);
    	break;
    case SYSTEM_EVENT_ETH_CONNECTED:
		ESP_LOGI("event", "SYSTEM_EVENT_ETH_CONNECTED");
    	break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
		ESP_LOGI("event", "SYSTEM_EVENT_ETH_DISCONNECTED");
    case  SYSTEM_EVENT_ETH_GOT_IP:

    default:
        break;
    }
    return ESP_OK;
}


void tcpserv_read (const char *buf, int len)
{

}

void app_main()
{
    esp_event_loop_init(event_handler,NULL);
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    tcpip_adapter_init();

		printf("tcpserv init...port=%d\n",6666);
		tcpserv_init(tcpserv_read, 6666);
		xTaskCreate(app_tcpserv_task_accept, "tcpserv_accept", 4*1024, NULL, 5, NULL);
		xTaskCreate(app_tcpserv_task_recv, "tcpserv_recv", 6*1024, NULL, 5, NULL);

    app_wifi_softap("jiao_jiao_666","");
    printf(">>>Wifi Mode AP \n");

    //app_wifi_sta("xxxx","12345678");
    //printf(">>>Wifi Mode STA \n");

  ///////////////////////////////////////////////////////////////////////
	printf(">>>>print system info.\n");	
	esp_chip_info_t chip_info;
	esp_chip_info( &chip_info );
	printf("FLASH SIZE: %d MB (%s) \r\n", spi_flash_get_chip_size() / (1024 * 1024),
										  (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
	printf("Free memory: %d bytes \r\n", esp_get_free_heap_size());
	printf(">>>>successfully.\n");
	
}
