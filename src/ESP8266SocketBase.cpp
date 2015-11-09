/*//-------------------------------------------------------------------------------
 *  ESP8266SocketBase.cpp
 *
 *  Implementation file for ESP8266SocketBase
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
#include "ESP8266SocketBase.h"
#include "ESP8266Client.h"

#define LOOP_TASK_PRIORITY 0

//----------------------------
// general callbacks
//----------------------------
LOCAL ICACHE_FLASH_ATTR
void client_sent_cb(void *arg)
{
	struct espconn *pConn = (struct espconn*)arg;
	if (pConn->reverse > 0) {
		((ESP8266SocketBase*)pConn->reverse)->_onClientSentCb();
	}
}


LOCAL ICACHE_FLASH_ATTR
void client_recv_cb(void *arg, char *pusrdata, unsigned short length)
{
	struct espconn *pConn = (struct espconn*)arg;
	
	if (pConn->reverse > 0) {
		ESP8266SocketBase* m_sock = (ESP8266SocketBase*)pConn->reverse;
		m_sock->_onClientDataCb(pConn, pusrdata, length);
	}
}


//----------------------------
// TCP callbacks
//----------------------------
LOCAL ICACHE_FLASH_ATTR
void client_connect_cb(void *arg)
{
	struct espconn *pConn = (struct espconn*)arg;
	if (pConn->reverse > 0) {
		ESP8266SocketBase* m_sock = (ESP8266SocketBase*)pConn->reverse;
		m_sock->_onClientConnectCb(pConn);
	}
}

LOCAL ICACHE_FLASH_ATTR
void client_discon_cb(void *arg)
{
	struct espconn *pConn = (struct espconn*)arg;
	if (pConn->reverse > 0) {
		ESP8266SocketBase* m_sock = (ESP8266SocketBase*)pConn->reverse;
		m_sock->_onClientDisconnectCb(pConn);
	}
}

LOCAL ICACHE_FLASH_ATTR
void client_recon_cb(void *arg, sint8 err)
{
	struct espconn *pConn = (struct espconn*)arg;
	if (pConn->reverse > 0) {
		ESP8266SocketBase* m_sock = (ESP8266SocketBase*)pConn->reverse;
		m_sock->_onClientReconnectCb(pConn, err);
	}
}


//----------------------------
//----------------------------
// constructor
//----------------------------
//----------------------------
ESP8266SocketBase::ESP8266SocketBase(espconn_type type) :
	m_bIsExternal(false)
	,reverse_external(0)
{
	sint8 res;
	
	if (type <= 0 || type > ESPCONN_UDP) {
		type = ESPCONN_TCP;
	}
	
	// create a control structure
	esp_conn = (struct espconn*)os_malloc(sizeof(struct espconn));
	os_memset(esp_conn, 0, sizeof(struct espconn));
	
	// set connection type
	esp_conn->type = type;
	esp_conn->state = ESPCONN_NONE;
	esp_conn->reverse = (void*)this;
	
	// set default callbacks
	res = espconn_regist_recvcb(esp_conn, client_recv_cb);
	res = espconn_regist_sentcb(esp_conn, client_sent_cb);
	
	
	// create tcp/udp structures
	if (esp_conn->type == ESPCONN_TCP) {
		
		// TCP
		esp_conn->proto.tcp = (esp_tcp*)os_malloc(sizeof(esp_tcp));
		os_memset(esp_conn->proto.tcp, 0, sizeof(esp_tcp));
		
		// set callbacks
		espconn_regist_connectcb(esp_conn, client_connect_cb);
		espconn_regist_reconcb(esp_conn, client_recon_cb);
		espconn_regist_disconcb(esp_conn, client_discon_cb);
		// not using
//		espconn_regist_write_finish
		
	} else if (esp_conn->type == ESPCONN_UDP) {
		
		// UDP
		esp_conn->proto.udp = (esp_udp*)os_malloc(sizeof(esp_udp));
		os_memset(esp_conn->proto.udp, 0, sizeof(esp_udp));
		
	}
}


ESP8266SocketBase::ESP8266SocketBase(struct espconn* _esp_conn) :
	m_bIsExternal(true)
{
	esp_conn = _esp_conn;
	
	// set reverse-pointer
	reverse_external = esp_conn->reverse;
	esp_conn->reverse = (void*)this;
}


ESP8266SocketBase::~ESP8266SocketBase()
{
	// esp_conn might still be alive
	// remove or reset reverse
	esp_conn->reverse = reverse_external;
	
	if (!m_bIsExternal) {
		
		espconn_delete(esp_conn);
		
		// cleanup
		if (esp_conn->type == ESPCONN_TCP) {
			os_free(esp_conn->proto.tcp);
		} else if (esp_conn->type == ESPCONN_UDP) {
			os_free(esp_conn->proto.udp);
		}
		
		os_free(esp_conn);
	}
}


//----------------------------
// sending
//----------------------------
sint8 ESP8266SocketBase::send(uint8* data, uint16 length)
{
	if (esp_conn->state == ESPCONN_CLOSE) {
		// return error ESPCONN_CONN -11
		return ESPCONN_CONN;
	}
	

	// wait while writing
	while (esp_conn->state == ESPCONN_WRITE) {
		esp_schedule(); // keep going?
		esp_yield();
	}
	
	// try to send
//	return espconn_send(esp_conn, data, length);
	
	// keep it backward compatible with SDK 1.2 for now
	return espconn_sent(esp_conn, data, length);
}

sint8 ESP8266SocketBase::send(const char* data)
{
	return send((uint8*)data, strlen(data));
}


//----------------------------
// logging
//----------------------------
// send info to user CB
void ESP8266SocketBase::info(const char* info)
{
	if (onInfoCb != 0) {
		onInfoCb(info);
	}
}

// send an error to user CB
void ESP8266SocketBase::error(const char* error, sint8 err)
{
	if (onErrorCb != 0) {
		onErrorCb(error, err);
	}
}


//----------------------------
//----------------------------
// espconn callbacks
//----------------------------
//----------------------------
void ESP8266SocketBase::_onClientSentCb()
{
	if (onClientSentCb != 0) {
		onClientSentCb();
	}
}

//----------------------------
//----------------------------
// client callback setter
//----------------------------
//----------------------------
// general
void ESP8266SocketBase::onSent( void(*function)() )
{
	onClientSentCb = function;
}

void ESP8266SocketBase::onData( void (*function)(ESP8266Client&, char *, unsigned short) )
{
	onClientDataCb = function;
}

// tcp
void ESP8266SocketBase::onConnected( void (*function)(ESP8266Client&) )
{
	onClientConnectCb = function;
}
void ESP8266SocketBase::onDisconnected( void (*function)() )
{
	onClientDisconnectCb = function;
}
void ESP8266SocketBase::onReconnect( void (*function)(ESP8266Client&, sint8) )
{
	onClientReconnectCb = function;
}

//----------------------------
//----------------------------
// info, error
//----------------------------
//----------------------------
void ESP8266SocketBase::onInfo( void(*function)(const char* info) )
{
	onInfoCb = function;
}

void ESP8266SocketBase::onError( void (*function)(const char*, sint8) )
{
	onErrorCb = function;
}



