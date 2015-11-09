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

#include "ESP8266SocketBase.h"
#include "ESP8266Client.h"

extern "C" {
	#include "ets_sys.h"
	#include "osapi.h"
	#include "mem.h"
	#include "ip_addr.h"
	#include "espconn.h"
	
	#include "clientconnection.h"
	#include "esperr.h"
}

/* TODO
 - use secure calls
 - how about udp?
 */

class ESP8266TCPServer : public ESP8266SocketBase
{
public:
	ESP8266TCPServer(int port);
	
	sint8 start();
	sint8 stop();
	bool setTimeout(uint32 interval);
	bool setMaxClients(uint8 max);
	uint8 getMaxClients() {return maxClients; };
	
	remot_info* connectionInfo();
	
	sint8 send(uint8 clientId, uint8* data, uint16 length); // send data to client with id
	sint8 send(uint8 clientId, const char* data); // send data to client with id
	sint8 sendAll(uint8* data, uint16 length); // send data to all connected clients
	sint8 sendAll(const char* data); // send data to all connected clients
	
	
	//----------------------------
	// internal callbacks - override
	void _onClientDataCb(struct espconn *pesp_conn, char *data, unsigned short length);	
	
	// tcp callbacks
	void _onClientConnectCb(struct espconn *pesp_conn_client);
	void _onClientDisconnectCb(struct espconn *pesp_conn_client);
	void _onClientReconnectCb(struct espconn *pesp_conn_client, sint8 err);
	
	
private:
	void printConnections(clientConn* conn);
	
	uint8				bIsSecure;
	uint32				timeout;
	uint8				maxClients;
	
	clientConn* clientConnections;
};


#endif