/*
 * user_main.c
 *
 *  Created on: 2018��10��11��
 *      Author: fanci
 */
#include "user_main.h"



void ICACHE_FLASH_ATTR Wifi_connect(void){
	static uint8 count = 0;
	os_timer_disarm(&connect_timer);
	count++;
	uint8 status = wifi_station_get_connect_status();
	if(status == STATION_GOT_IP){
		os_printf("Wifi connect success!");
		struct ip_info info;
		wifi_get_ip_info(STATION_IF, &info);
		//my_station_init((struct ip_addr*)remote_ip, &info.ip, 1025);
		my_server_init(&info.ip, 80);

		return;
	}
	else{
		if(count >= 7){
			os_printf("Wifi connect fail!");
			return;
		}
	}
	os_timer_arm(&connect_timer,2000,NULL);
}

void to_scan(void) {
	struct station_config stationConf; 
	//choose ap
	//os_memcpy(&stationConf.ssid, "TP-LINK_B448", 32);
	//os_memcpy(&stationConf.password, "meiyoumima233", 64);
	os_memcpy(&stationConf.ssid, "serialTest-flasche", 32);
	os_memcpy(&stationConf.password, "qwertyuiop", 64);
	wifi_station_set_config(&stationConf);
	wifi_station_connect();
	os_timer_setfn(&connect_timer,Wifi_connect,NULL);
	os_timer_arm(&connect_timer,2000,NULL);
}

void user_init(){
	uart_init(115200,115200);
	wifi_set_opmode(0x03);
	RGB_light_init();
	system_init_done_cb(to_scan);
}
void user_rf_pre_init(){}




