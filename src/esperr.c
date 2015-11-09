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
#include "esperr.h"

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


static const char* const ESPCONN_OK_STR = "ESPCONN_OK";
static const char* const ESPCONN_OK_DESC_STR = "No error, everything OK.";

static const char* const ESPCONN_MEM_STR = "ESPCONN_MEM";
static const char* const ESPCONN_MEM_DESC_STR = "Out of memory error.";

static const char* const ESPCONN_TIMEOUT_STR = "ESPCONN_TIMEOUT";
static const char* const ESPCONN_TIMEOUT_DESC_STR = "Timeout.";

static const char* const ESPCONN_RTE_STR = "ESPCONN_RTE";
static const char* const ESPCONN_RTE_DESC_STR = "Routing problem.";

static const char* const ESPCONN_INPROGRESS_STR = "ESPCONN_INPROGRESS";
static const char* const ESPCONN_INPROGRESS_DESC_STR = "Operation in progress.";


static const char* const ESPCONN_ABRT_STR = "ESPCONN_ABRT";
static const char* const ESPCONN_ABRT_DESC_STR = "Connection aborted.";

static const char* const ESPCONN_RST_STR = "ESPCONN_RST";
static const char* const ESPCONN_RST_DESC_STR = "Connection reset.";

static const char* const ESPCONN_CLSD_STR = "ESPCONN_CLSD";
static const char* const ESPCONN_CLSD_DESC_STR = "Connection closed.";

static const char* const ESPCONN_CONN_STR = "ESPCONN_CONN";
static const char* const ESPCONN_CONN_DESC_STR = "Not connected.";


static const char* const ESPCONN_ARG_STR = "ESPCONN_ARG";
static const char* const ESPCONN_ARG_DESC_STR = "Illegal argument.";

static const char* const ESPCONN_ISCONN_STR = "ESPCONN_ISCONN";
static const char* const ESPCONN_ISCONN_DESC_STR = "Already connected.";


static const char* const ESPCONN_HANDSHAKE_STR = "ESPCONN_HANDSHAKE";
static const char* const ESPCONN_HANDSHAKE_DESC_STR = "ssl handshake failed.";

#ifdef ESPCONN_SSL_INVALID_DATA
static const char* const ESPCONN_SSL_INVALID_DATA_STR = "ESPCONN_SSL_INVALID_DATA";
static const char* const ESPCONN_SSL_INVALID_DATA_DESC_STR = "ssl application invalid.";
#endif

// additional error
static const char* const ESP_UNKNOWN_ERROR_STR = "ESP_UNKNOWN_ERROR";
static const char* const ESP_UNKNOWN_ERROR_DESC_STR = "unknown error.";

// no such error
static const char* const NO_SUCH_ERROR_STR = "NO_SUCH_ERROR";
static const char* const NO_SUCH_ERROR_DESC_STR = "no such error.";


// return error string
const char* espErrorToStr(sint8 errNum)
{
	switch (errNum) {
		case ESPCONN_OK:
			return ESPCONN_OK_STR;
			break;
		case ESPCONN_MEM:
			return ESPCONN_MEM_STR;
			break;
		case ESPCONN_TIMEOUT:
			return ESPCONN_TIMEOUT_STR;
			break;
		case ESPCONN_RTE:
			return ESPCONN_RTE_STR;
			break;
		case ESPCONN_INPROGRESS:
			return ESPCONN_INPROGRESS_STR;
			break;
		case ESPCONN_ABRT:
			return ESPCONN_ABRT_STR;
			break;
		case ESPCONN_RST:
			return ESPCONN_RST_STR;
			break;
		case ESPCONN_CLSD:
			return ESPCONN_CLSD_STR;
			break;
		case ESPCONN_CONN:
			return ESPCONN_CONN_STR;
			break;
		case ESPCONN_ARG:
			return ESPCONN_ARG_STR;
			break;
		case ESPCONN_ISCONN:
			return ESPCONN_ISCONN_STR;
			break;
		case ESPCONN_HANDSHAKE:
			return ESPCONN_HANDSHAKE_STR;
			break;
#ifdef ESPCONN_SSL_INVALID_DATA
		case ESPCONN_SSL_INVALID_DATA:
			return ESPCONN_SSL_INVALID_DATA_STR;
#endif
		case ESP_UNKNOWN_ERROR:
			return ESP_UNKNOWN_ERROR_STR;
			break;
			
		default:
			break;
	}
	return NO_SUCH_ERROR_STR;
}


const char* espErrorDesc(sint8 errNum)
{
	switch (errNum) {
		case ESPCONN_OK:
			return ESPCONN_OK_DESC_STR;
			break;
		case ESPCONN_MEM:
			return ESPCONN_MEM_DESC_STR;
			break;
		case ESPCONN_TIMEOUT:
			return ESPCONN_TIMEOUT_DESC_STR;
			break;
		case ESPCONN_RTE:
			return ESPCONN_RTE_DESC_STR;
			break;
		case ESPCONN_INPROGRESS:
			return ESPCONN_INPROGRESS_DESC_STR;
			break;
		case ESPCONN_ABRT:
			return ESPCONN_ABRT_DESC_STR;
			break;
		case ESPCONN_RST:
			return ESPCONN_RST_DESC_STR;
			break;
		case ESPCONN_CLSD:
			return ESPCONN_CLSD_DESC_STR;
			break;
		case ESPCONN_CONN:
			return ESPCONN_CONN_DESC_STR;
			break;
		case ESPCONN_ARG:
			return ESPCONN_ARG_DESC_STR;
			break;
		case ESPCONN_ISCONN:
			return ESPCONN_ISCONN_DESC_STR;
			break;
		case ESPCONN_HANDSHAKE:
			return ESPCONN_HANDSHAKE_DESC_STR;
			break;
#ifdef ESPCONN_SSL_INVALID_DATA
		case ESPCONN_SSL_INVALID_DATA:
			return ESPCONN_SSL_INVALID_DATA_DESC_STR;
#endif
			
		case ESP_UNKNOWN_ERROR:
			return ESP_UNKNOWN_ERROR_DESC_STR;
			break;
			
		default:
			break;
	}
	return NO_SUCH_ERROR_DESC_STR;
}