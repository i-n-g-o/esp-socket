/*//-------------------------------------------------------------------------------
 *  clientconnection.h
 *
 *  Header file for clientconnection
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

#ifndef ESP8266_SOCKET_CLIENTCONNECTION
#define ESP8266_SOCKET_CLIENTCONNECTION

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "ip_addr.h"
#include "espconn.h"


typedef struct _clientConnection clientConnection;

struct _clientConnection {
	clientConnection* prev;
	clientConnection* next;
	uint32_t addr;
	int port;
	struct espconn *esp_conn;
};

void prependConnection(clientConnection* newConn, clientConnection** conn);
void appendConnection(clientConnection* newConn, clientConnection** conn);
void removeConnection(clientConnection* conn, clientConnection** root);
clientConnection* findConnection(clientConnection* conn, struct espconn *pesp_conn);
clientConnection* findConnectionAddrPort(clientConnection* conn, uint32_t addr, int port);
void removeConnectionAddrPort(clientConnection** root, uint32_t addr, int port);
uint32_t connectionCount(clientConnection* conn);
clientConnection* createConnection(struct espconn *pesp_conn);
void clearConnections(clientConnection* conn);


#endif