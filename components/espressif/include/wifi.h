#ifndef _WIFI_H
#define _WIFI_H

#define TAG_WIFI "wifi:"

void app_wifi_softap(char* in_ssid,char* in_pwd);
void app_wifi_sta(const char *ssid , const char *passwd);
int wifiGetIP(char* dstIP,char* dstMask,char* dstGW);

#endif