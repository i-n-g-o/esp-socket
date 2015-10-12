/*//-------------------------------------------------------------------------------
 *  ESP8266Client.cpp
 *
 *  Implementation file for ESP8266Client
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
#include "ESP8266Client.h"

/*
 * general callbacks
 */
LOCAL ICACHE_FLASH_ATTR
void client_sent_cb(void *arg)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266Client* client = (ESP8266Client*)pConn->reverse;
	
	if (client->onClientSentCb != 0) {
		client->onClientSentCb();
	}
}


LOCAL ICACHE_FLASH_ATTR
void client_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266Client* client = (ESP8266Client*)pConn->reverse;
	
	if (client->onClientDataCb != 0) {
		client->onClientDataCb(*client, pusrdata, length);
	}
}


/* 
 * TCP callbacks
 */
LOCAL ICACHE_FLASH_ATTR
void client_connect_cb(void *arg)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266Client* server = (ESP8266Client*)pConn->reverse;
	
	// consider:
	//	enum espconn_option{
	//		ESPCONN_START = 0x00,
	//		ESPCONN_REUSEADDR = 0x01,
	//		ESPCONN_NODELAY = 0x02,
	//		ESPCONN_COPY = 0x04,
	//		ESPCONN_KEEPALIVE = 0x08,
	//		ESPCONN_END
	//	};
	// first enable keepalive
	//	sint8 espconn_set_opt( struct espconn *espconn, uint8 opt)
	// then turn it on
//	enum espconn_level{
//		ESPCONN_KEEPIDLE,
//		ESPCONN_KEEPINTVL,
//		ESPCONN_KEEPCNT
//	};
	// sint8 espconn_set_keepalive(struct espconn *espconn, uint8 level, void* optarg)
	
	if (server->onClientConnectCb != 0) {
		server->onClientConnectCb(pConn);
	}
}

LOCAL ICACHE_FLASH_ATTR
void client_discon_cb(void *arg)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266Client* server = (ESP8266Client*)pConn->reverse;
	
	if (server->onClientDisconnectCb != 0) {
		server->onClientDisconnectCb(pConn);
	}
}

LOCAL ICACHE_FLASH_ATTR
void client_recon_cb(void *arg, sint8 err)
{
	struct espconn *pConn = (struct espconn*)arg;
	ESP8266Client* server = (ESP8266Client*)pConn->reverse;
	
	if (server->onClientReconnectCb != 0) {
		server->onClientReconnectCb(pConn, err);
	}
}


/*
 * Constructor
 */
ESP8266Client::ESP8266Client() :
	external(false)
{
	sint8 res;
	
	// create a control structure
	esp_conn = (struct espconn*)os_malloc(sizeof(struct espconn));
	os_memset(esp_conn, 0, sizeof(struct espconn));
	
	esp_conn->state = ESPCONN_NONE;
	esp_conn->reverse = (void*)this;
	
	// set callbacks
	res = espconn_regist_recvcb(esp_conn, client_recv_cb);
	res = espconn_regist_sentcb(esp_conn, client_sent_cb);
}

ESP8266Client::ESP8266Client(IPAddress& address, int port, espconn_type type) : ESP8266Client()
{
	// safety
	if (type <= 0 || type > ESPCONN_UDP) {
		type = ESPCONN_TCP;
	}
	
	// set connection type
	esp_conn->type = type;
	
	// create tcp/udp structures
	if (esp_conn->type == ESPCONN_TCP) {
		
		esp_conn->proto.tcp = (esp_tcp*)os_malloc(sizeof(esp_tcp));
		os_memset(esp_conn->proto.tcp, 0, sizeof(esp_tcp));
		
	} else if (esp_conn->type == ESPCONN_UDP) {
		
		esp_conn->proto.udp = (esp_udp*)os_malloc(sizeof(esp_udp));
		os_memset(esp_conn->proto.udp, 0, sizeof(esp_udp));
		
	}

	setPort(port);
	setAddress(address);
}

ESP8266Client::ESP8266Client(struct espconn* _esp_conn)
{
	external = true;
	esp_conn = _esp_conn;
	
	// set remotePort
	if (esp_conn->type == ESPCONN_TCP) {
		
		remotePort = esp_conn->proto.tcp->remote_port;
		
	} else if (esp_conn->type == ESPCONN_UDP) {
		
		remotePort = esp_conn->proto.udp->remote_port;
	}
}


ESP8266Client::~ESP8266Client()
{
	if (!external) {
		disconnect();
		// cleanup
		if (esp_conn->type == ESPCONN_TCP) {
			os_free(esp_conn->proto.tcp);
		} else if (esp_conn->type == ESPCONN_UDP) {
			os_free(esp_conn->proto.udp);
		}
		
		os_free(esp_conn);
	}
}


/*
 * methods
 */
void ESP8266Client::setAddress(IPAddress& address)
{
	if (esp_conn->type == ESPCONN_TCP) {

		// set remote ip
		esp_conn->proto.tcp->remote_ip[0] = address[0];
		esp_conn->proto.tcp->remote_ip[1] = address[1];
		esp_conn->proto.tcp->remote_ip[2] = address[2];
		esp_conn->proto.tcp->remote_ip[3] = address[3];
		
		// set callbacks
		espconn_regist_connectcb(esp_conn, client_connect_cb);
		espconn_regist_reconcb(esp_conn, client_recon_cb);
		espconn_regist_disconcb(esp_conn, client_discon_cb);
		// not using
//		espconn_regist_write_finish

	} else if (esp_conn->type == ESPCONN_UDP) {
		
		// set remote ip
		esp_conn->proto.udp->remote_ip[0] = address[0];
		esp_conn->proto.udp->remote_ip[1] = address[1];
		esp_conn->proto.udp->remote_ip[2] = address[2];
		esp_conn->proto.udp->remote_ip[3] = address[3];
	}
}

void ESP8266Client::setPort(int port)
{
	remotePort = port;
	
	if (esp_conn->type == ESPCONN_TCP) {
		
		esp_conn->proto.tcp->remote_port = port;
		
	} else if (esp_conn->type == ESPCONN_UDP) {
		
		esp_conn->proto.udp->remote_port = port;
		esp_conn->proto.udp->local_port = port;
		
	}
}


//
bool ESP8266Client::connect()
{
	sint8 res = 0;
	if (esp_conn->type == ESPCONN_TCP) {
		res = espconn_connect(esp_conn);
	} else if (esp_conn->type == ESPCONN_UDP) {
		res = espconn_create(esp_conn);
	}
	
	return res == ESPCONN_OK;
}

bool ESP8266Client::disconnect()
{
	sint8 res;
	if (esp_conn->type == ESPCONN_TCP) {
		res = espconn_disconnect(esp_conn);
	} else {
		espconn_delete(esp_conn);
		return true;
	}
	return res == ESPCONN_OK;
}


//
bool ESP8266Client::send(uint8 *data, uint16 length)
{
	// safety
	setPort(remotePort);
	
	// send
	sint8 res = espconn_send(esp_conn, data, length);
	return res == ESPCONN_OK;
}


// set callbacks
void ESP8266Client::onSent( void(*function)() )
{
	onClientSentCb = function;
}

void ESP8266Client::onData( void (*function)(ESP8266Client&, char *, unsigned short) )
{
	onClientDataCb = function;
}

// tcp
void ESP8266Client::onConnected( void (*function)(struct espconn *) )
{
	onClientConnectCb = function;
}
void ESP8266Client::onDisconnected( void (*function)(struct espconn *) )
{
	onClientDisconnectCb = function;
}
void ESP8266Client::onReconnect( void (*function)(struct espconn *, sint8) )
{
	onClientReconnectCb = function;
}



