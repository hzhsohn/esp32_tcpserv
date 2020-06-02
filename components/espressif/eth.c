
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
#include "lwip/dns.h"


#define DEFAULT_ETHERNET_PHY_CONFIG phy_lan8720_default_ethernet_config

#define PIN_SMI_MDC			GPIO_NUM_23
#define PIN_SMI_MDIO		GPIO_NUM_18
#define PIN_CLOCK_MODE		2	//PHY_CLOCK_GPIO16_OUT

bool eth_static_ip=false;


esp_err_t eth_config(const char * hostname, tcpip_adapter_ip_info_t local_ip, uint32_t dns1, uint32_t dns2);

static void eth_gpio_config_rmii(void)
{
    // RMII data pins are fixed:
    // TXD0 = GPIO19
    // TXD1 = GPIO22
    // TX_EN = GPIO21
    // RXD0 = GPIO25
    // RXD1 = GPIO26
    // CLK == GPIO0
    phy_rmii_configure_data_interface_pins();
    // MDC is GPIO 23, MDIO is GPIO 18
    phy_rmii_smi_configure_pins(PIN_SMI_MDC, PIN_SMI_MDIO);
}

void app_eth()
{

	//------------------------------------------------
		esp_err_t ret = ESP_OK;

		eth_config_t config = DEFAULT_ETHERNET_PHY_CONFIG;
		/* Set the PHY address in the example configuration */
		config.phy_addr = PHY0;
		config.gpio_config = eth_gpio_config_rmii;
		config.tcpip_input = tcpip_adapter_eth_input;
		//初始化GPIO16.这个50M叼信号线-------------------------
		config.clock_mode = PIN_CLOCK_MODE;
		//config.phy_power_enable = phy_device_power_enable_via_gpio;

		ret = esp_eth_init(&config);

		if(ret == ESP_OK) {
			esp_eth_enable();

		}
		else{
			ESP_LOGI(TAG_ETH, "ETH init fail.");
		}
}

int ethGetIP(char* dstIP,char* dstMask,char* dstGW)
{	
		dstIP[0]=0;
		dstMask[0]=0;
		dstGW[0]=0;

		tcpip_adapter_ip_info_t ip;
		memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));		
		if (tcpip_adapter_get_ip_info(ESP_IF_ETH, &ip) == 0) {
			sprintf(dstIP,IPSTR, IP2STR(&ip.ip));
			sprintf(dstMask,IPSTR, IP2STR(&ip.netmask));
			sprintf(dstGW,IPSTR, IP2STR(&ip.gw));
			return 0;
		}
		return 1;
}

bool ethFixIP(char* dstIP)
{
	tcpip_adapter_ip_info_t local_ip={0};	
	ip4_addr_t dns1;
	ip4_addr_t dns2;

	char *p1=dstIP;
	char *p2=strchr(p1,'.');
	if(NULL==p2) {return false;}
	*p2=0x00;
	p2++;
	char *p3=strchr(p2,'.');
	if(NULL==p3) {return false;}
	*p3=0x00;
	p3++;
	char *p4=strchr(p3,'.');
	if(NULL==p4) {return false;}
	*p4=0x00;
	p4++;

	int a,b,c,d;
	a=atoi(p1);
	b=atoi(p2);
	c=atoi(p3);
	d=atoi(p4);

    IP4_ADDR(&local_ip.ip, a, b , c, d);
    IP4_ADDR(&local_ip.gw, a, b , c, 1);
    IP4_ADDR(&local_ip.netmask, 255, 255 , 255, 0);
	
    IP4_ADDR(&dns1, 192, 168 , 1, 1);
    IP4_ADDR(&dns2, 192, 168 , 1, 1);

    eth_config("hx-kong-kg8",local_ip,0,0);

	return eth_static_ip;
}


esp_err_t eth_config(const char * hostname, tcpip_adapter_ip_info_t local_ip, uint32_t dns1, uint32_t dns2)
{
    esp_err_t err = ESP_OK;
	
    if (hostname != NULL) 
    {
        err = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, hostname);
    } 
    else 
    {
        err = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_ETH, "NetGate");
    }
    
    if(err != ESP_OK)
    {
        ESP_LOGE("eth","hostname could not be seted! Error: %d", err);
        return err;
    }   
   
    err = tcpip_adapter_dhcpc_stop(TCPIP_ADAPTER_IF_ETH);
    if(err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STOPPED)
    {
        ESP_LOGE("eth","DHCP could not be stopped! Error: %d", err);
        return err;
    }

    err = tcpip_adapter_set_ip_info(TCPIP_ADAPTER_IF_ETH, &local_ip);
    if(err != ESP_OK)
    {
        ESP_LOGE("eth","STA IP could not be configured! Error: %d", err);
        return err;
    }
    if(local_ip.ip.addr)
    {
        eth_static_ip = true;
    } 
    else 
    {
        err = tcpip_adapter_dhcpc_start(TCPIP_ADAPTER_IF_ETH);
        if(err != ESP_OK && err != ESP_ERR_TCPIP_ADAPTER_DHCP_ALREADY_STARTED)
        {
            ESP_LOGW("eth","DHCP could not be started! Error: %d", err);
            return err;
        }
        eth_static_ip = false;
    }

    ip_addr_t d;
    d.type = IPADDR_TYPE_V4;

    if(dns1 != (uint32_t)0x00000000) 
    {
        // Set DNS1-Server
        d.u_addr.ip4.addr = dns1;
        dns_setserver(0, &d);
    }

    if(dns2 != (uint32_t)0x00000000) 
    {
        // Set DNS2-Server
        d.u_addr.ip4.addr = dns2;
        dns_setserver(1, &d);
    }

    return ESP_OK;
}