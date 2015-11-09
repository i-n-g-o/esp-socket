/*//-------------------------------------------------------------------------------
 *  ESP8266TCPServer.cpp
 *
 *  Implementation file for ESP8266TCPServer
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
#include "ESP8266TCPServer.h"

//----------------------------
//----------------------------
// constructor
//----------------------------
//----------------------------
ESP8266TCPServer::ESP8266TCPServer(int port) : ESP8266SocketBase()
	,timeout(0)
	,maxClients(16)
	,bIsSecure(0)
	,clientConnections(0)
{
	sint8 res;
	
	esp_conn->proto.tcp->local_port = port;
	
	// default never timeout clients
	setTimeout(timeout);
	// set default max clients
	setMaxClients(maxClients);
}


//----------------------------
//----------------------------
// methods
//----------------------------
//----------------------------
sint8 ESP8266TCPServer::start()
{
	// start server
	sint8 res = espconn_accept(esp_conn);
	if (res != ESPCONN_OK) {
		// error
		return res;
	}

	return espconn_regist_time(esp_conn, timeout, 0);
}


sint8 ESP8266TCPServer::stop()
{
	// disconnect all clients, clean list
	clientConn* conn = clientConnections;
	while (conn) {
		sint8 res = espconn_disconnect(conn->esp_conn);
		if (res != ESPCONN_OK &&
			res != ESPCONN_ARG)
		{
			error("could not disconnect client. reason: ", res);
			// we need to leave the client in the list
			conn = getNextConn(conn, &clientConnections, true);
		} else {
			conn = removeConnGetNext(conn, &clientConnections, true);
		}
	}

	// do we need a timeout here?
	while (esp_conn->state == ESPCONN_CONNECT) {
		esp_schedule(); // keep going?
		esp_yield();
	}
	
	// stop server
	return espconn_delete(esp_conn);
}


/*
 *
 */
bool ESP8266TCPServer::setTimeout(uint32 interval)
{
	timeout = interval;
	
	// set timeout
	sint8 res = espconn_regist_time(esp_conn, timeout, 0);
	return res == ESPCONN_OK;
}

bool ESP8266TCPServer::setMaxClients(uint8 max)
{
	maxClients = max;
	
	// set max clients
	sint8 res = espconn_tcp_set_max_con_allow(esp_conn, maxClients);
	return res == ESPCONN_OK;
}

remot_info* ESP8266TCPServer::connectionInfo()
{
	remot_info* clients;
	sint8 res = espconn_get_connection_info(esp_conn, &clients, bIsSecure);
	
	return clients;
}

//----------------------------
// send methods
//----------------------------
// send to one
sint8 ESP8266TCPServer::send(uint8 clientId, uint8 *data, uint16 length)
{
	if (clientId >= maxClients) {
		return false;
	}
	
	remot_info* clients;
	sint8 res = espconn_get_connection_info(esp_conn, &clients, bIsSecure);
	
	remot_info connInfo = clients[clientId];
	// validate?
	
	uint32_t clientAddr;
	os_memcpy((void*)&clientAddr, (void*)connInfo.remote_ip, 4);
	
	
	// get espconn of client
	clientConn* client_conn = findConnAddrPort(clientConnections, clientAddr, connInfo.remote_port);
	
	if (!client_conn) {
		return ESP_UNKNOWN_ERROR;
	}
		
	
	// TODO: check for connected status!!
	while (client_conn->esp_conn->state == ESPCONN_WRITE) {
		esp_schedule(); // keep going?
		esp_yield();
	}
	
	if (client_conn->esp_conn->state == ESPCONN_CONNECT) {
		
		// SDK 1.3 call
//		sint8 res = espconn_send(client_conn->esp_conn, data, length);
		
		// keep it backward compatible with SDK 1.2 for now
		sint8 res = espconn_sent(client_conn->esp_conn, data, length);
		
		return res;
	}
	
	return ESP_UNKNOWN_ERROR;
}

sint8 ESP8266TCPServer::send(uint8 clientId, const char* data)
{
	return send(clientId, (uint8*)data, strlen(data));
}

// send to all
sint8 ESP8266TCPServer::sendAll(uint8 *data, uint16 length)
{
	// two-pass sending
	clientConn* busyClients = 0;
	
	// 1 - try to send everything
	clientConn* conn = clientConnections;
	while (conn) {
		
		if (conn->esp_conn->state == ESPCONN_CONNECT) {
			
			// SDK 1.3 call
//			sint8 res = espconn_send(conn->esp_conn, data, length);
			
			// keep it backward compatible with SDK 1.2 for now
			sint8 res = espconn_sent(conn->esp_conn, data, length);

			
		} else if (conn->esp_conn->state == ESPCONN_WRITE) {
			
			// add this to busy clients
			clientConn* newConn = createConn(conn->esp_conn);
			prependConn(newConn, &busyClients);
			
		} else if (conn->esp_conn->state == ESPCONN_CLOSE) {
			
			// remove from list
			info("sendAll - closed - remove");
			clientConn* tofree = conn;
			conn = conn->next;
			
			removeConn(tofree, &clientConnections);
			continue;
			
		} else {
			// what state?
			error("sending client in state: ", conn->esp_conn->state);
		}
		
		conn = conn->next;
	}
	
	// 2 - deal with busy clients...
	if (busyClients) {
		conn = busyClients;
		while (conn) {
			
			// deal with ESPCONN_WAIT, ESPCONN_LISTEN?
			
			if (conn->esp_conn->state == ESPCONN_CONNECT) {
				
				// SDK 1.3 call
//				sint8 res = espconn_send(conn->esp_conn, data, length);

				// keep it backward compatible with SDK 1.2 for now
				sint8 res = espconn_sent(conn->esp_conn, data, length);
				
				// remove from list - get next elements, loop
				conn = removeConnGetNext(conn, &busyClients, true);
				
				continue;
				
			} else if (conn->esp_conn->state == ESPCONN_WRITE) {
				// give it some time
				esp_schedule(); // keep going?
				esp_yield();
			} else if (conn->esp_conn->state == ESPCONN_CLOSE ||
					   conn->esp_conn->state == ESPCONN_NONE)
			{
				// remove from from original list...
				clientConn* origConn = findConn(clientConnections, conn->esp_conn);
				if (origConn) {
					removeConn(origConn, &clientConnections);
				} else {
					// client should be in list?
					// unless it was removed by disconnect CB
				}

				// remove from list - get next elements, loop
				conn = removeConnGetNext(conn, &busyClients, true);
				
			} else {
				// what state?				
				error("busyClient in state", conn->esp_conn->state);
			}

			// get next element, loop
			conn = getNextConn(conn, &busyClients, true);
		}
	}
	
	return ESPCONN_OK;
}

sint8 ESP8266TCPServer::sendAll(const char* data)
{
	return sendAll((uint8*)data, strlen(data));
}


// log connections to info
void ESP8266TCPServer::printConnections(clientConn* conn)
{
	char buf[1024];
	memset(buf, 0, 1024);
	
	
	info("-----clients-----");
	while (conn) {
		
		os_sprintf(buf, "%u: %u ---> %lu", conn->addr, conn->port, (unsigned long)conn->next);
		info(buf);
		
		conn = conn->next;
	}
}


//----------------------------
//----------------------------
// espconn callbacks
//----------------------------
//----------------------------
void ESP8266TCPServer::_onClientDataCb(struct espconn *pesp_conn, char *data, unsigned short length)
{
	// set incoming client as current connection
	esp_conn->proto.tcp->remote_port = pesp_conn->proto.tcp->remote_port;
	os_memcpy((void*)esp_conn->proto.tcp->remote_ip, (void*)pesp_conn->proto.tcp->remote_ip, 4);

	// call user CB
	if (onClientDataCb != 0) {
		ESP8266Client client(pesp_conn);
		onClientDataCb(client, data, length);
	}
}
//----------------------------
//----------------------------
// TCP callbacks
//----------------------------
//----------------------------
void ESP8266TCPServer::_onClientConnectCb(struct espconn *pesp_conn)
{
	info("server _clientConnect");
	
	// new clientconnection
	clientConn* newConnection = createConn(pesp_conn);
	
	// prepend to chained list
	prependConn(newConnection, &clientConnections);
	
//	printConnections(clientConnections);
	
	// call user CB
	if (onClientConnectCb != 0) {
		ESP8266Client client(pesp_conn);
		onClientConnectCb(client);
	}
}

void ESP8266TCPServer::_onClientDisconnectCb(struct espconn *pesp_conn)
{
	// receiving servers espconn
	info("server _clientDisconnect");
	
	// get remote-ip-port from client
	int remoteP = pesp_conn->proto.tcp->remote_port;
	uint32_t remoteAddr;
	os_memcpy((void*)&remoteAddr, (void*)pesp_conn->proto.tcp->remote_ip, 4);
	
	// search for espconn in connections and remove
	removeConnAddrPort(&clientConnections, remoteAddr, remoteP);

//	printConnections(clientConnections);
	
	// call user CB
	if (onClientDisconnectCb != 0) {
		onClientDisconnectCb();
	}
}

void ESP8266TCPServer::_onClientReconnectCb(struct espconn *pesp_conn, sint8 err)
{
	// receiving servers espconn
	info("server _clientReconnect");
	
	// get remote-ip-port from client
	int remoteP = pesp_conn->proto.tcp->remote_port;
	uint32_t remoteAddr;
	os_memcpy((void*)&remoteAddr, (void*)pesp_conn->proto.tcp->remote_ip, 4);
	
	
	// remove client from list??
	clientConn* clientConn = findConnAddrPort(clientConnections, remoteAddr, remoteP);
	if (clientConn) {
		removeConn(clientConn, &clientConnections);
	} else {
		// why?
		// this client client should be in list
		error("disconnect: did not find client in list", ESP_UNKNOWN_ERROR);
	}
	
//	printConnections(clientConnections);
	
	// call user CB
	if (onClientReconnectCb != 0) {
		ESP8266Client client(pesp_conn);
		onClientReconnectCb(client, err);
	}
}

