/*
 * user_main.c
 *
 *  Created on: 2018��10��11��
 *      Author: fanci
 */
#include "user_main.h"
#include "smartconfig.h"

void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata){
    switch(status) {
        case SC_STATUS_WAIT:
            os_printf("SC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            os_printf("SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
			sc_type *type = pdata;
            if (*type == SC_TYPE_ESPTOUCH) {
                os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
            } else {
                os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
            }
            break;
        case SC_STATUS_LINK:
            os_printf("SC_STATUS_LINK\n");
            struct station_config *sta_conf = pdata;

	        wifi_station_set_config(sta_conf);
	        wifi_station_disconnect();
	        wifi_station_connect();
	        struct ip_info info;
	        wifi_get_ip_info(STATION_IF, &info);
	        my_server_init(&info.ip, 80);
            break;
        case SC_STATUS_LINK_OVER:
            os_printf("SC_STATUS_LINK_OVER\n");
            if (pdata != NULL) {
                uint8 phone_ip[4] = {0};

                os_memcpy(phone_ip, (uint8*)pdata, 4);
                os_printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            }
            smartconfig_stop();
            break;
    }
}


void ICACHE_FLASH_ATTR done_cb1(void){
	wifi_station_disconnect();
	smartconfig_start(smartconfig_done);
}

void user_init(){
	uart_init(115200,115200);
	wifi_set_opmode(0x01);
	RGB_light_init();
	system_init_done_cb(done_cb1);
}
void user_rf_pre_init(){}
