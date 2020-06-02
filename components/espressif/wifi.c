

#include "esp_system.h"
#include "esp_err.h"
#include "esp_event_loop.h"
#include "esp_event.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_eth.h"

#include "rom/ets_sys.h"
#include "rom/gpio.h"

#include "soc/dport_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_sig_map.h"

#include "nvs_flash.h"
#include "driver/gpio.h"

#include "eth_phy/phy_lan8720.h"
#include "eth.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "esp_log.h"
#include "esp_event_loop.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"

#include "wifi.h"

void app_wifi_sta(const char *ssid , const char *passwd)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_FLASH));

	wifi_config_t wifi_config={0};
	strcpy((char*)wifi_config.sta.ssid,ssid);
	strcpy((char*)wifi_config.sta.password,passwd);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI("wifi", "wifi_init_sta finished.");
    ESP_LOGI("wifi", "connect to SSID:%s , password:%s \n", ssid,passwd);
}

void app_wifi_softap(char* in_ssid,char* in_pwd)
{
	tcpip_adapter_ip_info_t local_ip={0};
	int res_ap_get=0;
	int res_ap_set=0;

    tcpip_adapter_init();
    wifi_init_config_t cfg=WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
	wifi_config_t wifi_config = {0};
    strcpy((char*)wifi_config.ap.ssid , in_ssid);
    wifi_config.ap.ssid_len = strlen(in_ssid);
    strcpy((char*)wifi_config.ap.password , in_pwd);
    wifi_config.ap.max_connection = 2;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
      
    if (strlen(in_pwd) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP,&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI("wifi", "wifi_init_softap finished.SSID:%s password:%s",
             in_ssid, in_pwd);

    res_ap_get = tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &local_ip);
    //printf("self:"IPSTR"\n",IP2STR(&local_ip.ip));
    //printf("self:"IPSTR"\n",IP2STR(&local_ip.netmask));
    //printf("self:"IPSTR"\n",IP2STR(&local_ip.gw));

    tcpip_adapter_dhcps_stop(TCPIP_ADAPTER_IF_AP); 
    //memset(local_ip, 0, sizeof(tcpip_adapter_ip_info_t));

    IP4_ADDR(&local_ip.ip, 192, 168 , 1, 10);
    IP4_ADDR(&local_ip.gw, 192, 168 , 1, 10);
    IP4_ADDR(&local_ip.netmask, 255, 255 , 255, 0);

    printf("set ip:"IPSTR"\n",IP2STR(&local_ip.ip));
    printf("set netmask:"IPSTR"\n",IP2STR(&local_ip.netmask));
    printf("set gw:"IPSTR"\n",IP2STR(&local_ip.gw));

    res_ap_set = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_AP, &local_ip);


    tcpip_adapter_dhcps_start(TCPIP_ADAPTER_IF_AP);
}

int wifiGetIP(char* dstIP,char* dstMask,char* dstGW)
{
	dstIP[0]=0;
	dstMask[0]=0;
	dstGW[0]=0;

	tcpip_adapter_ip_info_t ip={0};
	memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
	if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip) == 0) {

			sprintf(dstIP,IPSTR, IP2STR(&ip.ip));
			sprintf(dstMask,IPSTR, IP2STR(&ip.netmask));
			sprintf(dstGW,IPSTR, IP2STR(&ip.gw));
			return 0;
	}
	return 1;
}