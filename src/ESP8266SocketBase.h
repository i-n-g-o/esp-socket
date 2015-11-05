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

#ifndef ESP8266_SOCKET_BASE
#define ESP8266_SOCKET_BASE

#include <stddef.h>
#include <string.h>


extern "C" {
	#include "ets_sys.h"
	#include "osapi.h"
	#include "mem.h"
	#include "ip_addr.h"
	#include "espconn.h"
	
	#include "esperr.h"
	
	extern void esp_schedule();
	extern void esp_yield();
}


#define TCP ESPCONN_TCP
#define UDP ESPCONN_UDP


class ESP8266Client;

class ESP8266SocketBase
{
public:
	// constructor, deconstructor
	ESP8266SocketBase(espconn_type type = TCP);
	ESP8266SocketBase(struct espconn* _esp_conn);
	~ESP8266SocketBase();

	// methods
	bool isTcp() { return esp_conn->type == ESPCONN_TCP; };
	bool isUdp() { return esp_conn->type == ESPCONN_UDP; };
	
	virtual sint8 send(uint8 *data, uint16 length);
	virtual sint8 send(const char* data);
	
	bool isSending() { return esp_conn->state == ESPCONN_WRITE; };
	
	//----------------------------
	// set user callback functions
	// general
	void onSent( void(*)() );
	void onData( void(*)(ESP8266Client&, char *, unsigned short) );
	
	// tcp callbacks
	void onConnected( void(*)(ESP8266Client&) );
	void onDisconnected( void(*)() );
	void onReconnect( void(*)(ESP8266Client&, sint8) );
	
	// info, error
	void onInfo( void(*)(const char*) );
	void onError( void(*)(const char*, sint8) );
	
	//----------------------------
	// internal callbacks
	// general
	virtual void _onClientSentCb();
	virtual void _onClientDataCb(struct espconn *pesp_conn, char *data, unsigned short length) = 0;
	
	// tcp callbacks
	virtual void _onClientConnectCb(struct espconn *pesp_conn_client) = 0;
	virtual void _onClientDisconnectCb(struct espconn *pesp_conn_client) = 0;
	virtual void _onClientReconnectCb(struct espconn *pesp_conn_client, sint8 err) = 0;
	
protected:
	void info(const char* info);
	void error(const char* error, sint8 err);
	
	struct espconn*		esp_conn;
	bool				m_bIsExternal;
	
	// user callback functions
	// general
	void (*onClientSentCb)() = 0;
	void (*onClientDataCb)(ESP8266Client& client, char *data, unsigned short length) = 0;
	
	// tcp callbacks
	void (*onClientConnectCb)(ESP8266Client& client) = 0;
	void (*onClientDisconnectCb)() = 0;
	void (*onClientReconnectCb)(ESP8266Client& client, sint8 err) = 0;
	
	// info, error
	void (*onInfoCb)(const char* error) = 0;
	void (*onErrorCb)(const char* error, sint8 err) = 0;
	
private:
	void* reverse_external;
	
};


#endif