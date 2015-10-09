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


/*
 * general callbacks
 */
LOCAL ICACHE_FLASH_ATTR
void server_sent_cb(void *arg)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266TCPServer* server = (ESP8266TCPServer*)pConn->reverse;
	
	if (server->onServerSentCb != 0) {
		server->onServerSentCb();
	}
}

LOCAL ICACHE_FLASH_ATTR
void server_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266TCPServer* server = (ESP8266TCPServer*)pConn->reverse;
	
	ESP8266Client client(pConn);
	
	if (server->onClientDataCb != 0) {
		server->onClientDataCb(client, pusrdata, length);
	}
}

/*
 * TCP callbacks
 */
LOCAL ICACHE_FLASH_ATTR
void server_connect_cb(void *arg)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266TCPServer* server = (ESP8266TCPServer*)pConn->reverse;
	
	ESP8266Client client(pConn);
	
	if (server->onClientConnectCb != 0) {
		server->onClientConnectCb(client);
	}
}

LOCAL ICACHE_FLASH_ATTR
void server_discon_cb(void *arg)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266TCPServer* server = (ESP8266TCPServer*)pConn->reverse;
	
	ESP8266Client client(pConn);
	
	if (server->onClientDisconnectCb != 0) {
		server->onClientDisconnectCb(client);
	}
}

LOCAL ICACHE_FLASH_ATTR
void server_recon_cb(void *arg, sint8 err)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266TCPServer* server = (ESP8266TCPServer*)pConn->reverse;
	
	ESP8266Client client(pConn);
	
	if (server->onClientReconnectCb != 0) {
		server->onClientReconnectCb(client, err);
	}
}



/*
 */
ESP8266TCPServer::ESP8266TCPServer(int port) :
	timeout(0)
	,maxClients(16)
	,bIsSecure(0)
{
	sint8 res;
	
	esp_conn_server.type = ESPCONN_TCP;
	esp_conn_server.state = ESPCONN_NONE;
	esp_conn_server.proto.tcp = &esptcp;
	esp_conn_server.proto.tcp->local_port = port;
	
	esp_conn_server.reverse = (void*)this;
	
	// general callbacks
	res = espconn_regist_sentcb(&esp_conn_server, server_sent_cb);
	res = espconn_regist_recvcb(&esp_conn_server, server_recv_cb);
	
	// set tcp callbacks
	res = espconn_regist_connectcb(&esp_conn_server, server_connect_cb);
	res = espconn_regist_reconcb(&esp_conn_server, server_recon_cb);
	res = espconn_regist_disconcb(&esp_conn_server, server_discon_cb);
	
	// default never timeout clients
	setTimeout(timeout);
	// set default max clients
	setMaxClients(maxClients);
}


ESP8266TCPServer::~ESP8266TCPServer()
{
	stop();
}



/*
 *
 */
bool ESP8266TCPServer::start()
{
	// start server
	sint8 res = espconn_accept(&esp_conn_server);
	if (res != ESPCONN_OK) {
		// error
		return false;
	}
	
	res = espconn_regist_time(&esp_conn_server, timeout, 0);
	
	return res == ESPCONN_OK;
}


bool ESP8266TCPServer::stop()
{
	// stop server
	espconn_delete(&esp_conn_server);
}


/*
 *
 */
bool ESP8266TCPServer::setTimeout(uint32 interval)
{
	timeout = interval;
	
	// set timeout
	sint8 res = espconn_regist_time(&esp_conn_server, timeout, 0);
	return res == ESPCONN_OK;
}

bool ESP8266TCPServer::setMaxClients(uint8 max)
{
	maxClients = max;
	
	// set max clients
	sint8 res = espconn_tcp_set_max_con_allow(&esp_conn_server, maxClients);
	return res == ESPCONN_OK;	
}

remot_info* ESP8266TCPServer::connectionInfo()
{
	remot_info* clients;
	sint8 res = espconn_get_connection_info(&esp_conn_server, &clients, bIsSecure);
	
//	for (int i=0; i<maxClients; i++) {
//		remot_info conn = clients[i];
//		if (conn.state == ESPCONN_CONNECT) {
//			
//		}
//	}
}

/*
 *
 */
bool ESP8266TCPServer::send(uint8 *data, uint16 length)
{
	// send
	sint8 res = espconn_send(&esp_conn_server, data, length);
	return res == ESPCONN_OK;
}

bool ESP8266TCPServer::send(uint8 clientId, uint8 *data, uint16 length)
{
	if (clientId >= maxClients) {
		return false;
	}
	
	remot_info* clients;
	sint8 res = espconn_get_connection_info(&esp_conn_server, &clients, bIsSecure);
	
	remot_info conn = clients[clientId];
	
	if (conn.state == ESPCONN_CONNECT && conn.remote_port > 0) {
		// setup remote info
		esptcp.remote_port = conn.remote_port;
		os_memcpy((void*)esptcp.remote_ip, (void*)conn.remote_ip, 4);
		
		 return send(data, length);
	}
	
	return false;
}

bool ESP8266TCPServer::sendAll(uint8 *data, uint16 length)
{
	remot_info* clients;
	sint8 res = espconn_get_connection_info(&esp_conn_server, &clients, bIsSecure);
	
	for (int i=0; i<maxClients; i++) {
		remot_info conn = clients[i];
		// check state
		if (conn.state == ESPCONN_CONNECT && conn.remote_port > 0) {
			// setup remote info
			esptcp.remote_port = conn.remote_port;
			os_memcpy((void*)esptcp.remote_ip, (void*)conn.remote_ip, 4);
			
			bool result = send(data, length);
			if (!result) {
				// handle error
			}
		}
	}
}


/*
 * set callbacks
 */
void ESP8266TCPServer::onSent( void (*function)() )
{
	onServerSentCb = function;
}
void ESP8266TCPServer::onData( void (*function)(ESP8266Client&, char *, unsigned short) )
{
	onClientDataCb = function;
}

// tcp callbacks
void ESP8266TCPServer::onConnected( void (*function)(ESP8266Client&) )
{
	onClientConnectCb = function;
}
void ESP8266TCPServer::onDisconnected( void (*function)(ESP8266Client&) )
{
	onClientDisconnectCb = function;
}
void ESP8266TCPServer::onReconnected( void (*function)(ESP8266Client&, sint8) )
{
	onClientReconnectCb = function;
}

