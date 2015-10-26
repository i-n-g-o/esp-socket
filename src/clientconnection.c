/*//-------------------------------------------------------------------------------
 *  clientconnection.c
 *
 *  Implementation file for clientconnection
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
#include "clientconnection.h"


void prependConnection(clientConnection* newConn, clientConnection** conn)
{
	if (!conn || !newConn) return;
	
	newConn->next = *conn;
	newConn->prev = 0;
	
	if (!*conn) {
		// set and return
		*conn = newConn;
		return;
	}
	
	newConn->prev = (*conn)->prev;
	
	if ((*conn)->prev) {
		(*conn)->prev->next = newConn;
	} else {
		(*conn)->prev = newConn;
		*conn = newConn;
	}
}

void appendConnection(clientConnection* newConn, clientConnection** conn)
{
	if (!conn || !newConn) return;

	newConn->prev = *conn;
	newConn->next = 0;
	
	if (!*conn) {
		// set and return
		*conn = newConn;
		return;
	}
	
	newConn->next = (*conn)->next;
	
	if ((*conn)->next) {
		(*conn)->next->prev = newConn;
	}
	(*conn)->next = newConn;
}

void removeConnection(clientConnection* conn, clientConnection** root)
{
	if (!conn || !root) return;
	
	// last element?
	if (!conn->next && !conn->prev) {
		*root = 0;
		return;
	}
		
	// remove
	if (conn->next) {
		conn->next->prev = conn->prev;
	} else if (conn->prev) {
		conn->prev->next = 0;
	}
	
	if (conn->prev) {
		conn->prev->next = conn->next;
	} else if (conn->next) {
		conn->next->prev = 0;
		if (*root) {
			*root = conn->next;
		}
	}
}

clientConnection* findConnection(clientConnection* conn, struct espconn *pesp_conn)
{
	while (conn) {
		if (conn->esp_conn == pesp_conn) {
			return conn;
		}
		conn = conn->next;
	}
	
	return 0;
}

clientConnection* findConnectionAddrPort(clientConnection* conn, uint32_t addr, int port)
{
	while (conn) {
		if (conn->addr == addr && conn->port == port) {
			return conn;
		}
		conn = conn->next;
	}
	
	return 0;
}


void removeConnectionAddrPort(clientConnection** root, uint32_t addr, int port)
{
	clientConnection* conn = *root;
	while (conn) {
		if (conn->addr == addr && conn->port == port) {
			// remove from chain
			removeConnection(conn, root);
			// erase
			os_free(conn);
			// done
			break;
		}
		conn = conn->next;
	}
}

uint32_t connectionCount(clientConnection* conn)
{
	uint32_t count = 0;
	while (conn) {
		count++;
		conn = conn->next;
	}
	return count;
}


clientConnection* createConnection(struct espconn *pesp_conn)
{
	// new clientconnection
	clientConnection* newConn = (clientConnection*)os_malloc(sizeof(clientConnection));
	os_memset(newConn, 0, sizeof(clientConnection));
	
	// set information
	os_memcpy((void*)&newConn->addr, (void*)pesp_conn->proto.tcp->remote_ip, 4);
	newConn->port = pesp_conn->proto.tcp->remote_port;
	newConn->esp_conn = pesp_conn;

	return newConn;
}


void clearConnections(clientConnection* conn)
{
	while (conn) {
		clientConnection* tofree = conn;
		conn = conn->next;
		
		os_free(tofree);
	}
}

