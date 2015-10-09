/*//-------------------------------------------------------------------------------
 *  ESP8266Client.h
 *
 *  Header file for ESP8266Client
 *
 *  c++ wrapper for ESP8266 sockets written by Ingo Randolf - 2015
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 //-------------------------------------------------------------------------------*/

#ifndef ESP8266_SOCKET_CLIENT
#define ESP8266_SOCKET_CLIENT

#include "IPAddress.h"

#include <stddef.h>
//#include <stdint.h>
//#include <stdbool.h>
//#include <stdarg.h>
//#include <string.h>


extern "C" {
//	#include "ets_sys.h"
//	#include "osapi.h"
//	#include "os_type.h"
//	#include "c_types.h"
	#include "mem.h"
	#include "ip_addr.h"
	#include "espconn.h"
}


#define TCP ESPCONN_TCP
#define UDP ESPCONN_UDP

class ESP8266Client
{
public:
	ESP8266Client();
	ESP8266Client(IPAddress& address, int port, espconn_type type = TCP);
	ESP8266Client(struct espconn* _esp_conn);
	~ESP8266Client();
	
	void setAddress(IPAddress& address);
	void setPort(int port);
	
	bool isTcp() { return esp_conn->type == ESPCONN_TCP; };
	bool isUdp() { return esp_conn->type == ESPCONN_UDP; };

	
	bool connect();
	bool disconnect();
	bool send(uint8 *data, uint16 length);
	
	// set callbacks
	void onSent( void(*)() );
	void onData( void (*)(struct espconn *, char *, unsigned short) );
	
	// tcp callbacks
	void onConnected( void (*)(struct espconn *) );
	void onDisconnected( void (*)(struct espconn *) );
	void onReconnect( void (*)(struct espconn *, sint8) );
	
	
	// user callbacks
	// general
	void (*onClientSentCb)() = 0;
	void (*onClientDataCb)(struct espconn *pesp_conn_client, char *data, unsigned short length) = 0;
	
	// tcp callbacks
	void (*onClientConnectCb)(struct espconn *pesp_conn_client) = 0;
	void (*onClientDisconnectCb)(struct espconn *pesp_conn_client) = 0;
	void (*onClientReconnectCb)(struct espconn *pesp_conn_client, sint8 err) = 0;
	
private:
	struct espconn*		esp_conn;
	int					remotePort;
	bool				external;	
};


#endif