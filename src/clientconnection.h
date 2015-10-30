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


typedef struct _clientConn clientConn;

struct _clientConn {
	clientConn* prev;
	clientConn* next;
	uint32_t addr;
	int port;
	struct espconn *esp_conn;
};

void prependConn(clientConn* newConn, clientConn** conn);
void appendConn(clientConn* newConn, clientConn** conn);
void removeConn(clientConn* conn, clientConn** root);
clientConn* findConn(clientConn* conn, struct espconn *pesp_conn);
clientConn* findConnAddrPort(clientConn* conn, uint32_t addr, int port);
void removeConnAddrPort(clientConn** root, uint32_t addr, int port);
uint32_t connCount(clientConn* conn);
clientConn* createConn(struct espconn *pesp_conn);
void clearConnections(clientConn* conn);
clientConn* removeConnGetNext(clientConn* conn, clientConn** root, bool loop);
clientConn* getNextConn(clientConn* conn, clientConn** root, bool loop);


#endif