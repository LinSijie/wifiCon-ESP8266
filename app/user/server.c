/*
 * server.c
 *
 *  Created on: 2018��10��17��
 *      Author: fanci
 */
#include "server.h"
#include "user_interface.h"
#include "gpio.h"


uint8 type = 0;
struct RGB_config RGB;
char isLedOn[6];

void ICACHE_FLASH_ATTR display(void){
	uint8 r = (uint8)atoi(RGB.r);
	uint8 g = (uint8)atoi(RGB.g);
	uint8 b = (uint8)atoi(RGB.b);
	if(strncmp(isLedOn, "true", 4) == 0){
		RGB_light_set_color(r, g, b);
	}
	else{
		RGB_light_set_color(0, 0, 0);
	}
	RGB_light_start();
}

void ICACHE_FLASH_ATTR data_send(void *arg, bool response_OK, char *psend){
	char http_header[256];
	uint16 length;
	struct espconn *pespconn = arg;
	char *pbuf = NULL;
	if(response_OK){
		//http response header
		os_sprintf(http_header, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nServer: lwIP/1.4.0\r\n", psend ? os_strlen(psend) : 0);
		if(psend){
			os_sprintf(http_header + strlen(http_header), "content-type: text/html; charset=utf-8\r\nPragma: no-cache\r\n\r\n");
			length = os_strlen(http_header) + os_strlen(psend);
			pbuf = (char *)os_zalloc(length + 1);
			os_memcpy(pbuf, http_header, os_strlen(http_header));
			os_memcpy(pbuf + os_strlen(http_header), psend, os_strlen(psend));
		}
		else{
			os_sprintf(http_header + os_strlen(http_header), "\r\n");
			length = os_strlen(http_header);
		}
	}
	else{
		os_sprintf(http_header, "HTTP/1.1 400 BadRequest\r\nContent-Length: %d\r\nServer: lwIP/1.4.0\r\n", psend ? os_strlen(psend) : 0);
		length = os_strlen(http_header);
	}

	if(psend){
		espconn_send(pespconn, pbuf, length);
	}
	else{
		espconn_send(pespconn, http_header, length);
	}

	if(pbuf){
		os_free(pbuf);
		pbuf = NULL;
	}
}

bool save_param(char *precv){
	//LedOn=false&Red=103&Green=255&Blue=48&
	//os_printf("in save_param function\r\n");
	char *PA;
	char *PB;
	//get LED status
	PA = strstr(precv, "LedOn");
	if(PA == NULL){
		return false;
	}
	PA += 6;
	PB = strstr(PA, "&");
	if(PB == NULL){
		return false;
	}
	memcpy(isLedOn, PA, PB-PA);
	isLedOn[PB-PA] = 0;
	//os_printf("isLedOn = %s\r\n",isLedOn);
	//get Red value
	PA = strstr(precv, "Red");
	if(PA == NULL){
		return false;
	}
	PA += 4;
	PB = strstr(PA, "&");
	if(PB == NULL){
		return false;
	}
	memcpy(RGB.r, PA, PB - PA);
	RGB.r[PB-PA] = 0;	//end string
	//os_printf("RGB.r = %s\r\n",RGB.r);
	//get Green value
	PA = strstr(precv, "Green");
	if(PA == NULL){
		return false;
	}
	PA += 6;
	PB = strstr(PA, "&");
	if(PB == NULL){
		return false;
	}
	os_memcpy(RGB.g, PA, PB - PA);
	RGB.g[PB-PA] = 0;
	//os_printf("RGB.g = %s\r\n",RGB.g);
	//get Blue value
	PA = strstr(precv, "Blue");
	if(PA == NULL){
		return false;
	}
	PA += 5;
	PB = strstr(PA, "&");
	if(PB == NULL){
		return false;
	}
	os_memcpy(RGB.b, PA, PB - PA);
	RGB.b[PB-PA] = 0;
	//os_printf("RGB.b = %s\r\n",RGB.b);
	
	return true;
	
}

bool ICACHE_FLASH_ATTR parse_url(char *precv, urlFrame *p_urlFrame){
	//os_printf("\r\n in parse_url function\r\n");
	char *pBuffer = NULL;
	char *str = NULL;
	uint16 length = 0;
	if(precv == NULL || p_urlFrame == NULL){
		os_printf("Wrong!\r\n");
		return false;
	}
	pBuffer = (char *)os_strstr(precv, "Host:");
	//os_printf("pBuffer1 is: %s\r\n", pBuffer);
	if(pBuffer != NULL){
		os_memset(p_urlFrame->pFilename, 0, URL_SIZE);
		if(os_strncmp(precv, "GET ", 4) == 0){
			p_urlFrame->Type = GET;
			pBuffer = precv + 4;
		}
		else if(os_strncmp(precv, "POST ", 5) == 0){
			p_urlFrame->Type = POST;
			pBuffer = precv + 5;
		}
		else{
			return false;
		}
		pBuffer++; //skip '/'
		str=(char *)os_strstr(pBuffer," HTTP");
		if(str != NULL){
			length = str - pBuffer;
			os_memcpy(p_urlFrame->pFilename, pBuffer, length);
		}
		else{
			os_printf("Cannot find Http\r\n");
			return false;
		}
	}
	else{
		os_printf("Cannot find Host\r\n");
		return false;
	}
}

void ICACHE_FLASH_ATTR server_recv(void *arg, char *pdata, unsigned short len){
	char index2[4096];
	urlFrame *pUrlFrame;
	char *pParseBuffer = NULL;
	pUrlFrame = (urlFrame *)os_zalloc(sizeof(urlFrame));
	char *pRecvData = (char *)os_zalloc(len + 1);
	os_memcpy(pRecvData, pdata, len);
	parse_url(pRecvData, pUrlFrame);
	switch(pUrlFrame->Type){
		case GET:
			//spi_flash_read(0x100 * 4096, (uint32 *)index2, 4096);
			//index2[INDEX_SIZE] = 0;
			os_printf("receive data from PC: %s\r\n", pdata);
			data_send(arg, true, "OK");
			break;
		case POST:
			pParseBuffer = (char *)os_strstr(pRecvData, "\r\n\r\n");
			if(pParseBuffer == NULL){
				data_send(arg, false, NULL);
				break;
			}

			pParseBuffer += 4;
			if(save_param(pParseBuffer) == true){
				display();
				//spi_flash_read(0x100 * 4096, (uint32 *)index2, 4096);
				//index2[INDEX_SIZE] = 0;
				os_printf("receive data from PC: %s\r\n", pdata);
				os_printf("LED color setup OK!\r\n");
				data_send(arg, true, "OK");
			}
			else{
				data_send(arg, false, NULL);
			}
			break;
	}
	os_free(pUrlFrame);
	pUrlFrame = NULL;
	os_free(pRecvData);
	pRecvData = NULL;

}

void ICACHE_FLASH_ATTR server_sent(void *arg){
	os_printf("Send OK!\r\n");
}

void ICACHE_FLASH_ATTR server_discon(void *arg){
	os_printf("disconnect OK!\r\n");
}

void ICACHE_FLASH_ATTR server_listen(void *arg){
	struct espconn *pespconn = arg;
	espconn_regist_recvcb(pespconn, server_recv);
	espconn_regist_sentcb(pespconn, server_sent);
	espconn_regist_disconcb(pespconn, server_discon);
}

void ICACHE_FLASH_ATTR server_recon(void *arg,sint8 err){
	os_printf("Connect error! Error No.%d\r\n", err);

}

void ICACHE_FLASH_ATTR my_server_init(struct ip_addr *local_ip, int port){
	LOCAL struct espconn esp_conn;
	// espconn parameter initialize
	esp_conn.type = ESPCONN_TCP;
	esp_conn.state = ESPCONN_NONE;
	esp_conn.proto.tcp = (esp_tcp *)os_malloc(sizeof(esp_tcp));
	os_memcpy(esp_conn.proto.tcp->local_ip, local_ip, 4);
	esp_conn.proto.tcp->local_port = port;

	espconn_regist_connectcb(&esp_conn,server_listen);
	espconn_regist_reconcb(&esp_conn,server_recon);

	espconn_accept(&esp_conn);

}

