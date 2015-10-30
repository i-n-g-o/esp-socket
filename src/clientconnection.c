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


void prependConn(clientConn* newConn, clientConn** conn)
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

void appendConn(clientConn* newConn, clientConn** conn)
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

void removeConn(clientConn* conn, clientConn** root)
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

clientConn* findConn(clientConn* conn, struct espconn *pesp_conn)
{
	while (conn) {
		if (conn->esp_conn == pesp_conn) {
			return conn;
		}
		conn = conn->next;
	}
	
	return 0;
}

clientConn* findConnAddrPort(clientConn* conn, uint32_t addr, int port)
{
	while (conn) {
		if (conn->addr == addr && conn->port == port) {
			return conn;
		}
		conn = conn->next;
	}
	
	return 0;
}


void removeConnAddrPort(clientConn** root, uint32_t addr, int port)
{
	clientConn* conn = *root;
	while (conn) {
		if (conn->addr == addr && conn->port == port) {
			// remove from chain
			removeConn(conn, root);
			// erase
			os_free(conn);
			// done
			break;
		}
		conn = conn->next;
	}
}

uint32_t connCount(clientConn* conn)
{
	uint32_t count = 0;
	while (conn) {
		count++;
		conn = conn->next;
	}
	return count;
}


clientConn* createConn(struct espconn *pesp_conn)
{
	// new clientconnection
	clientConn* newConn = (clientConn*)os_malloc(sizeof(clientConn));
	os_memset(newConn, 0, sizeof(clientConn));
	
	// set information
	os_memcpy((void*)&newConn->addr, (void*)pesp_conn->proto.tcp->remote_ip, 4);
	newConn->port = pesp_conn->proto.tcp->remote_port;
	newConn->esp_conn = pesp_conn;

	return newConn;
}


void clearConnections(clientConn* conn)
{
	// correct previous
	if (conn->prev != 0) {
		conn->prev->next = 0;
	}
	
	clientConn* tofree = 0;
	while (conn) {
		tofree = conn;
		conn = conn->next;
		
		os_free(tofree);
	}
}


clientConn* getNextConn(clientConn* conn, clientConn** root, bool loop)
{
	clientConn* next = conn->next;
	if (loop && !next) {
		if (!*root) {
			// all done
			return 0;
		} else {
			// start over
			next = *root;
		}
	}
	return next;
}

clientConn* removeConnGetNext(clientConn* conn, clientConn** root, bool loop)
{
	// remove from list
	clientConn* next = conn->next;
	
	removeConn(conn, root);
	
	if (loop && !next) {
		if (!*root) {
			// all done
			return 0;
		} else {
			// start over
			next = *root;
		}
	}
	
	return next;
}
