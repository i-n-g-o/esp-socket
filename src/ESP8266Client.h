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

#include "ESP8266SocketBase.h"

extern "C" {
	#include "ets_sys.h"
	#include "osapi.h"
	#include "mem.h"
	#include "ip_addr.h"
	#include "espconn.h"
	
	#include "esperr.h"
}

/* TODO
 - remotePort necessary??
 - use internal buffer for data to send... if buffer is full yield?
 - use secure calls
 */


class ESP8266Client : public ESP8266SocketBase
{
public:
	ESP8266Client(const char* addr, int port, espconn_type type = TCP);
	ESP8266Client(struct espconn* _esp_conn);
	~ESP8266Client();
	
	void setAddress(uint32_t address);
	void setAddress(uint8 ip0, uint8 ip1, uint8 ip2, uint8 ip3);
	void setPort(int port);
	
	uint32_t getAddress();
	int getPort();
	
	bool connect();
	bool disconnect();

	bool isConnected() { return m_bIsConnected; };
	bool isConnecting() { return m_bIsConnecting; };
	
	//----------------------------
	// internal callbacks - override
	void _onClientDataCb(struct espconn *pesp_conn, char *data, unsigned short length);
	
	void _onClientConnectCb(struct espconn *pesp_conn_client);
	void _onClientDisconnectCb(struct espconn *pesp_conn_client);
	void _onClientReconnectCb(struct espconn *pesp_conn_client, sint8 err);
	
	
private:
	int					remotePort;
	
	bool				m_bIsConnected;
	bool				m_bIsConnecting;
};


#endif