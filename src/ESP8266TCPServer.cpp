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
bool ESP8266TCPServer::start()
{
	// start server
	sint8 res = espconn_accept(esp_conn);
	if (res != ESPCONN_OK) {
		// error
		return false;
	}

	res = espconn_regist_time(esp_conn, timeout, 0);
	
	return res == ESPCONN_OK;
}


bool ESP8266TCPServer::stop()
{
	// stop server
	espconn_delete(esp_conn);
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
	clientConnection* client_conn = findConnectionAddrPort(clientConnections, clientAddr, connInfo.remote_port);
	
	if (!client_conn) {
		return UNKNOWN_ERROR;
	}
		
	
	// TODO: check for connected status!!
	while (client_conn->esp_conn->state == ESPCONN_WRITE) {
		esp_schedule(); // keep going?
		esp_yield();
	}
	
	if (client_conn->esp_conn->state == ESPCONN_CONNECT) {
		
		sint8 res = espconn_send(client_conn->esp_conn, data, length);
		return res;
	}
	
	return UNKNOWN_ERROR;
}

sint8 ESP8266TCPServer::send(uint8 clientId, const char* data)
{
	return send(clientId, (uint8*)data, strlen(data));
}

// send to all
sint8 ESP8266TCPServer::sendAll(uint8 *data, uint16 length)
{
	// two-pass sending
	clientConnection* busyClients = 0;
	
	// 1 - try to send everything
	clientConnection* conn = clientConnections;
	while (conn) {
		
		if (conn->esp_conn->state == ESPCONN_CONNECT) {
			
			sint8 res = espconn_send(conn->esp_conn, data, length);
			
		} else if (conn->esp_conn->state == ESPCONN_WRITE) {
			
			// add this to busy clients
			clientConnection* newConn = createConnection(conn->esp_conn);
			prependConnection(newConn, &busyClients);
			
		} else if (conn->esp_conn->state == ESPCONN_CLOSE) {
			
			// remove from list
			clientConnection* tofree = conn;
			conn = conn->next;
			
			removeConnection(tofree, &clientConnections);
			continue;
			
		} else {
			// what state?
			//
		}
		
		conn = conn->next;
	}
	
	// 2 - deal with busy clients...
	if (busyClients) {
		conn = busyClients;
		while (conn) {
			
			// deal with ESPCONN_NONE, ESPCONN_WAIT, ESPCONN_LISTEN?
			
			if (conn->esp_conn->state == ESPCONN_CONNECT) {
				sint8 res = espconn_send(conn->esp_conn, data, length);
				
				// remove from list
				clientConnection* tofree = conn;
				conn = conn->next;
				
				removeConnection(tofree, &busyClients);
				
				if (!conn) {
					if (!busyClients) {
						// all done
						break;
					} else {
						// start over
						conn = busyClients;
					}
				}
				
				continue;
				
			} else if (conn->esp_conn->state == ESPCONN_WRITE) {
				// give it some time
				esp_schedule(); // keep going?
				esp_yield();
			} else if (conn->esp_conn->state == ESPCONN_CLOSE) {
				
				// remove from list
				clientConnection* tofree = conn;
				conn = conn->next;
				
				removeConnection(tofree, &busyClients);
				
				if (!conn) {
					if (!busyClients) {
						// all done
						break;
					} else {
						// start over
						conn = busyClients;
					}
				}
				
			} else {
				// what state?
			}
			
			//
			conn = conn->next;
			if (!conn) {
				if (!busyClients) {
					// all done
					break;
				} else {
					// start over
					conn = busyClients;
				}
			}
		}
	}
	
	return ESPCONN_OK;
}

sint8 ESP8266TCPServer::sendAll(const char* data)
{
	return sendAll((uint8*)data, strlen(data));
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
	// new clientconnection
	clientConnection* newConnection = createConnection(pesp_conn);
	
	// prepend to chained list
	prependConnection(newConnection, &clientConnections);
	
	printConnections(clientConnections);
	
	// call user CB
	if (onClientConnectCb != 0) {
		ESP8266Client client(pesp_conn);
		onClientConnectCb(client);
	}
}

void ESP8266TCPServer::_onClientDisconnectCb(struct espconn *pesp_conn)
{
	// receiving servers espconn
	
	// get remote-ip-port from client
	int remoteP = pesp_conn->proto.tcp->remote_port;
	uint32_t remoteAddr;
	os_memcpy((void*)&remoteAddr, (void*)pesp_conn->proto.tcp->remote_ip, 4);
	
	// search for espconn in connections and remove
	removeConnectionAddrPort(&clientConnections, remoteAddr, remoteP);

	printConnections(clientConnections);
	
	// call user CB
	if (onClientDisconnectCb != 0) {
		onClientDisconnectCb();
	}
}

void ESP8266TCPServer::_onClientReconnectCb(struct espconn *pesp_conn, sint8 err)
{
	// receiving servers espconn
	
	// get remote-ip-port from client
	int remoteP = pesp_conn->proto.tcp->remote_port;
	uint32_t remoteAddr;
	os_memcpy((void*)&remoteAddr, (void*)pesp_conn->proto.tcp->remote_ip, 4);
	
	
	// remove client from list??
	clientConnection* clientConn = findConnectionAddrPort(clientConnections, remoteAddr, remoteP);
	if (clientConn) {
		removeConnection(clientConn, &clientConnections);
	}
	
	printConnections(clientConnections);
	
	// call user CB
	if (onClientReconnectCb != 0) {
		ESP8266Client client(pesp_conn);
		onClientReconnectCb(client, err);
	}
}

