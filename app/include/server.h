/*
 * server.h
 *
 *  Created on: 2018��10��17��
 *      Author: fanci
 */

#ifndef APP_INCLUDE_SERVER_H_
#define APP_INCLUDE_SERVER_H_

#include "user_main.h"
#define URL_SIZE 125
#define RGB_VALUE_SIZE 5

void server_init(struct ip_addr *local_ip, int port);
typedef enum ProtocolType{
    GET = 0,
    POST,
} ProtocolType;

typedef struct urlFrame{
    enum ProtocolType Type;
    char pFilename[URL_SIZE];
}urlFrame;

typedef struct RGB_config{
    char r[RGB_VALUE_SIZE];
    char g[RGB_VALUE_SIZE];
    char b[RGB_VALUE_SIZE];
}RGB_config;

#endif /* APP_INCLUDE_SERVER_H_ */
