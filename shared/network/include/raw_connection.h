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
 * raw_connection.h
 *
 *  Created on: 16 Mar 2015
 *      Author: billy
 *
 * @brief A datatype to encapsulate Server-Client connections.
 * @addto@group network_group
 * @{
 */

#ifndef RAW_CONNECTION_H_
#define RAW_CONNECTION_H_

#include "jansson.h"

#include "network_library.h"
#include "typedefs.h"

/* forward declarations */
struct RawConnection;
struct Connection;


#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a RawConnection to a Server.
 *
 * @param hostname_s The hostname of the Server to connect to.
 * @param port_s The Server's port to connect to.
 * @return A Connection to the given Server or <code>NULL</code> upon error.
 * @memberof Connection
 */
GRASSROOTS_NETWORK_API struct Connection *AllocateRawServerConnection (const char * const hostname_s, const char * const port_s);




/**
 * Receive data over a RawConnection
 *
 * @param connection_p The RawConnection to use to receive the data with.
 * @return A positive integer for the number of bytes received upon success. When negative,
 * it indicates that there was an error with this value being -(num bytes received) that were
 * sent successfully before the error occurred. If this is zero, it means that there was
 * an error sending the initial message containing the length header.
 * @memberof RawConnection
 */
GRASSROOTS_NETWORK_API int AtomicReceiveViaRawConnection (struct RawConnection *connection_p);



/**
 * Send a string over a network connection.
 *
 * @param data_s The string to send.
 * @param connection_p The RawConnection to use to send the data with.
 * @return A positive integer for the number of bytes sent upon success. When negative,
 * it indicates that there was an error with this value being -(num bytes sent) that were
 * sent successfully before the error occurred. If this is zero, it means that there was
 * an error sending the initial message containing the length header.
 * @memberof RawConnection
 */
GRASSROOTS_NETWORK_API int AtomicSendStringViaRawConnection (const char *data_s, struct RawConnection *connection_p);


/**
 * Send a string over a network connection.
 *
 * @param buffer_p The buffer to send the message from.
 * @param num_to_send The length of the buffer to send.
 * @param connection_p The RawConnection to use to send the data with.
 * @return A positive integer for the number of bytes sent upon success. When negative,
 * it indicates that there was an error with this value being -(num bytes sent) that were
 * sent successfully before the error occurred. If this is zero, it means that there was
 * an error sending the initial message containing the length header.
 * @memberof RawConnection
 */
GRASSROOTS_NETWORK_API int AtomicSendViaRawConnection (const char *buffer_p, uint32 num_to_send, struct RawConnection *connection_p);


GRASSROOTS_NETWORK_LOCAL int SendJsonRequestViaRawConnection (struct RawConnection *connection_p, const json_t *json_p);


GRASSROOTS_NETWORK_LOCAL const char *GetRawConnectionData (struct Connection *connection_p);


GRASSROOTS_NETWORK_LOCAL bool MakeRemoteJsonCallViaRawConnection (struct Connection *connection_p, const json_t *req_p);

GRASSROOTS_NETWORK_LOCAL void FreeRawConnection (struct RawConnection *conn_p);

#ifdef __cplusplus
}
#endif



#endif		/* #ifndef RAW_CONNECTION_H_ */

