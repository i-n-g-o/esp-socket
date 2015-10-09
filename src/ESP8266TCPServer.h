/*//-------------------------------------------------------------------------------
 *  ESP8266TCPServer.h
 *
 *  Header file for ESP8266TCPServer
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

#ifndef ESP8266_SOCKET_SERVER
#define ESP8266_SOCKET_SERVER

#include "IPAddress.h"
#include "ESP8266Client.h"

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
	#include "ip_addr.h"
	#include "espconn.h"
}

// TODO. use secure calls

class ESP8266TCPServer
{
public:
	ESP8266TCPServer(int port);
	~ESP8266TCPServer();
	
	bool start();
	bool stop();
	bool setTimeout(uint32 interval);
	bool setMaxClients(uint8 max);
	uint8 getMaxClients() {return maxClients; };
	
	remot_info* connectionInfo();
	
	bool send(uint8 *data, uint16 length); // send to currently set remote ip:port
	bool send(uint8 clientId, uint8 *data, uint16 length); // get client with id and send
	bool sendAll(uint8 *data, uint16 length); // send to all clients
	
	// set callbacks
	void onSent( void (*)() );
	void onData( void (*)(ESP8266Client&, char *, unsigned short) );
	//
	void onConnected( void (*)(ESP8266Client&) );
	void onDisconnected( void (*)(ESP8266Client&) );
	void onReconnected( void (*)(ESP8266Client&, sint8) );
	
	
	// user callbacks
	void (*onServerSentCb)() = 0;
	void (*onClientDataCb)(ESP8266Client& client, char *data, unsigned short length) = 0;
	
	// tcp callbacks
	void (*onClientConnectCb)(ESP8266Client& client) = 0;
	void (*onClientDisconnectCb)(ESP8266Client& client) = 0;
	void (*onClientReconnectCb)(ESP8266Client& client, sint8 err) = 0;
	
	
private:
	struct espconn		esp_conn_server;
	esp_tcp				esptcp;
	
	uint8				bIsSecure;
	
	uint32				timeout;
	uint8				maxClients;
};


#endif