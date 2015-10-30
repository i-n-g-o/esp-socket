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


//----------------------------
//----------------------------
// constructor
//----------------------------
//----------------------------
ESP8266Client::ESP8266Client(const char* addr, int port, espconn_type type) :
	ESP8266SocketBase(type)
{
	setPort(port);
	setAddress(ipaddr_addr(addr));
}

ESP8266Client::ESP8266Client(struct espconn* _esp_conn) : ESP8266SocketBase(_esp_conn)
	,m_bIsConnected(true)
	,m_bIsConnecting(false)
{
	// set remotePort from espconn
	if (isTcp()) {
		remotePort = esp_conn->proto.tcp->remote_port;
	} else if (isUdp()) {
		remotePort = esp_conn->proto.udp->remote_port;
	}
}


ESP8266Client::~ESP8266Client()
{
	if (!m_bIsExternal) {
		disconnect();
	}
}


//----------------------------
//----------------------------
// methods
//----------------------------
//----------------------------

// address
void ESP8266Client::setAddress(uint32_t address)
{
	if (isTcp()) {
		os_memcpy(esp_conn->proto.tcp->remote_ip, &address, 4);
	} else if (isUdp()) {
		os_memcpy(esp_conn->proto.udp->remote_ip, &address, 4);
	}
}

void ESP8266Client::setAddress(uint8 ip0, uint8 ip1, uint8 ip2, uint8 ip3)
{
	if (isTcp()) {

		// set remote ip
		esp_conn->proto.tcp->remote_ip[0] = ip0;
		esp_conn->proto.tcp->remote_ip[1] = ip1;
		esp_conn->proto.tcp->remote_ip[2] = ip2;
		esp_conn->proto.tcp->remote_ip[3] = ip3;

	} else if (isUdp()) {
		
		// set remote ip
		esp_conn->proto.udp->remote_ip[0] = ip0;
		esp_conn->proto.udp->remote_ip[1] = ip1;
		esp_conn->proto.udp->remote_ip[2] = ip2;
		esp_conn->proto.udp->remote_ip[3] = ip3;
	}
}

uint32_t ESP8266Client::getAddress()
{
	uint32_t addr = 0;
	
	if (isTcp()) {
		os_memcpy(&addr, esp_conn->proto.tcp->remote_ip, 4);
	} else if (isUdp()) {
		os_memcpy(&addr, esp_conn->proto.udp->remote_ip, 4);
	}
	
	return addr;
}

// port
void ESP8266Client::setPort(int port)
{
	remotePort = port;
	
	if (isTcp()) {
		esp_conn->proto.tcp->remote_port = port;
	} else if (isUdp()) {
		
		esp_conn->proto.udp->remote_port = port;
		//???
		esp_conn->proto.udp->local_port = port; //espconn_port();
	}
}

int ESP8266Client::getPort()
{
	if (isTcp()) {
		return esp_conn->proto.tcp->remote_port;
	} else if (isUdp()) {
		return esp_conn->proto.udp->remote_port;
	}
	
	return remotePort;
}


//----------------------------
// connect / disconnect
//----------------------------
bool ESP8266Client::connect()
{
	// already connecting?
	if (m_bIsConnecting) {
		return false;
	}
	
	sint8 res = ESPCONN_OK;
	
	// if connected... disconnect first
	if (m_bIsConnected) {
		disconnect();
	}
	
	if (isTcp()) {
		res = espconn_connect(esp_conn);
		if (res == ESPCONN_OK) {
			m_bIsConnecting = true;
		}
	} else if (isUdp()) {
		res = espconn_create(esp_conn);
		m_bIsConnected = true;
		m_bIsConnecting = false;
	}
	
	if (res != ESPCONN_OK) {
		error("could not connect: ", res);
	}
	
	return res == ESPCONN_OK;
}

bool ESP8266Client::disconnect()
{
	sint8 res = ESPCONN_OK;
	
	m_bIsConnecting = false;
	
	if (isTcp()) {
		res = espconn_disconnect(esp_conn);
	} else {
		espconn_delete(esp_conn);
		m_bIsConnected = false;
	}
	
	if (res != ESPCONN_OK) {
		error("could not disconnect: ", res);
	}
	
	return res == ESPCONN_OK;
}


//----------------------------
//----------------------------
// espconn callbacks
//----------------------------
//----------------------------
void ESP8266Client::_onClientDataCb(struct espconn *pesp_conn, char *data, unsigned short length)
{
	if (onClientDataCb != 0) {
		onClientDataCb(*this, data, length);
	}
}

//----------------------------
//----------------------------
// TCP callbacks
//----------------------------
//----------------------------
void ESP8266Client::_onClientConnectCb(struct espconn *pesp_conn_client)
{
	m_bIsConnected = true;
	m_bIsConnecting = false;
	
	if (onClientConnectCb != 0) {
		onClientConnectCb(*this);
	}
}

void ESP8266Client::_onClientDisconnectCb(struct espconn *pesp_conn_client)
{
	// set internal state
	m_bIsConnected = false;
	m_bIsConnecting = false;
	
	if (onClientDisconnectCb != 0) {
		onClientDisconnectCb();
	}
}

void ESP8266Client::_onClientReconnectCb(struct espconn *pesp_conn_client, sint8 err)
{
	// set internal state
	m_bIsConnected = false;
	m_bIsConnecting = false;
	
	if (onClientReconnectCb != 0) {
		onClientReconnectCb(*this, err);
	}
}

