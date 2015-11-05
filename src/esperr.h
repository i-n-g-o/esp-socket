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

#ifndef ESP8266_ERR
#define ESP8266_ERR

#include "osapi.h"
#include "c_types.h"
#include "mem.h"
#include "ip_addr.h"
#include "espconn.h"


// copied from SDK 1.3.0
// /* Definitions for error constants. */
//
//#define ESPCONN_OK          0    /* No error, everything OK. */
//#define ESPCONN_MEM        -1    /* Out of memory error.     */
//#define ESPCONN_TIMEOUT    -3    /* Timeout.                 */
//#define ESPCONN_RTE        -4    /* Routing problem.         */
//#define ESPCONN_INPROGRESS  -5    /* Operation in progress    */
//
//#define ESPCONN_ABRT       -8    /* Connection aborted.      */
//#define ESPCONN_RST        -9    /* Connection reset.        */
//#define ESPCONN_CLSD       -10   /* Connection closed.       */
//#define ESPCONN_CONN       -11   /* Not connected.           */
//
//#define ESPCONN_ARG        -12   /* Illegal argument.        */
//#define ESPCONN_ISCONN     -15   /* Already connected.       */
//
//#define ESPCONN_HANDSHAKE  -28   /* ssl handshake failed	 */
//#define ESPCONN_SSL_INVALID_DATA  -61   /* ssl application invalid	 */

// additional error
#define ESP_UNKNOWN_ERROR       -100				/* Unknown error.           */

const char* espErrorToStr(sint8 errNum);
const char* espErrorDesc(sint8 errNum);

#endif