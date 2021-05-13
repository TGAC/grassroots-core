/*
** Copyright 2014-2016 The Earlham Institute
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/**
 * @file
 * @brief
 */
/*
 * connection.h
 *
 *  Created on: 16 Mar 2015
 *      Author: billy
 *
 * @brief A datatype to encapsulate Server-Client connections.
 * @addto@group network_group
 * @{
 */

#ifndef CONNECTION_H_
#define CONNECTION_H_


#include "typedefs.h"
#include "byte_buffer.h"
#include "curl_tools.h"
#include "network_library.h"
#include "jansson.h"

/**
 * An enumeration listing the different types of Connections
 *
 * @ingroup network_group
 */
typedef enum
{
	/** A raw socket-based connection */
	CT_RAW,

	/** A connection using http(s) */
	CT_WEB,

	/** The number of possible ConnectionTypes */
	CT_NUM_TYPES
} ConnectionType;


/**
 * @brief The base class for building a Server-Client connection.
 *
 * @ingroup network_group
 */
typedef struct Connection
{
	/** The internal id for this Connection */
	uint32 co_id;

	/** The ConnectionType for this Connection */
	ConnectionType co_type;
} Connection;




/**
 * @brief A Connection that uses http(s) communication.
 *
 * @extends Connection
 *
 * @ingroup network_group
 */
typedef struct WebConnection
{
	/** The base Connection. */
	Connection wc_base;

	/** The CurlTool that encapsulates the web-based calls. */
	CurlTool *wc_curl_p;

	/** The URI that the WebConnection is to. */
	char *wc_uri_s;
} WebConnection;


#ifdef __cplusplus
extern "C"
{
#endif



/**
 * Allocate a WebConnection to a Server.
 *
 * @param full_uri_s The URI to connect to.
 * @return A Connection to the given Server or <code>NULL</code> upon error.
 * @memberof Connection
 */
GRASSROOTS_NETWORK_API Connection *AllocateWebServerConnection (const char * const full_uri_s);


/**
 * Allocate a RawConnection to a Client.
 *
 * @param server_socket_fd The Client's socket_fd on the Server.
 * @return A Connection to the given Client or <code>NULL</code> upon error.
 * @memberof Connection
 */
GRASSROOTS_NETWORK_API Connection *AllocaterRawClientConnection (int server_socket_fd);


/**
 * Get the data stored in a Connection's buffer.
 *
 * @param connection_p The Connection to get the data from.
 * @return The data stored in a Connection's buffer.
 * @memberof Connection
 */
GRASSROOTS_NETWORK_API const char *GetConnectionData (Connection *connection_p);


/**
 * Free a Connection.
 *
 * @param connection_p The Connection to free.
 * @memberof Connection
 */
GRASSROOTS_NETWORK_API void FreeConnection (Connection *connection_p);


/**
 * Send JSON-based data over a Connection.
 *
 * @param connection_p The Connection to use.
 * @param req_p The data to send.
 * @return The response or <code>NULL</code> upon error.
 * @memberof Connection
 */
GRASSROOTS_NETWORK_API const char *MakeRemoteJsonCallViaConnection (Connection *connection_p, const json_t *req_p);



/**
 * @memberof Connection
 */
GRASSROOTS_NETWORK_API bool SetConnectionCredentials (Connection *connection_p, const char *username_s, const char *password_s);


GRASSROOTS_NETWORK_API bool InitConnection (Connection *connection_p, ConnectionType type);


GRASSROOTS_NETWORK_API void ReleaseConnection (Connection *connection_p);



/** @} */


#ifdef __cplusplus
}
#endif


#endif /* CONNECTION_H_ */
