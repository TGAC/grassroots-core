

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
#include <string.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "raw_connection.h"
#include "string_utils.h"
#include "math_utils.h"
#include "memory_allocations.h"

/*****************************/
/***** STATIC PROTOTYPES *****/
/*****************************/

static int SendData (int socket_fd, const void *buffer_p, const size_t num_to_send);

static int ReceiveData (int socket_fd, void *buffer_p, const size_t num_to_receive);

static int ReceiveDataIntoByteBuffer (int socket_fd, ByteBuffer *buffer_p, const size_t num_to_receive, bool append_flag);


/******************************/
/***** METHOD DEFINITIONS *****/
/******************************/

bool MakeRemoteJsonCallViaConnection (Connection *connection_p, const json_t *req_p)
{
	bool success_flag = false;

	RawConnection *raw_connection_p = (RawConnection *) connection_p;

	if (SendJsonRequestViaRawConnection (raw_connection_p, req_p) > 0)
		{
			if (AtomicReceiveViaRawConnection (raw_connection_p) > 0)
				{
					success_flag = true;
				}
		}

	return success_flag;
}


const char *GetRawConnectionData (Connection *connection_p)
{
	RawConnection *raw_connection_p = (RawConnection *) connection_p;

	const char *data_s = GetByteBufferData (raw_connection_p -> rc_data_buffer_p);

	return data_s;
}


void FreeRawConnection (RawConnection *conn_p)
{
	RawConnection *connection_p = (RawConnection *) conn_p;

	if (connection_p -> rc_server_connection_flag)
		{
			if (connection_p -> rc_data.rc_server_p)
				{
					freeaddrinfo (connection_p -> rc_data.rc_server_p);
				}
		}
	else
		{
			if (connection_p -> rc_data.rc_client_p)
				{
					FreeMemory (connection_p -> rc_data.rc_client_p);
				}
		}

	FreeByteBuffer (connection_p -> rc_data_buffer_p);


	close (connection_p -> rc_sock_fd);
}




int AtomicSendStringViaRawConnection (const char *data_s, RawConnection *connection_p)
{
	return AtomicSendViaRawConnection (data_s, strlen (data_s), connection_p);
}




/**
 * Make sure that we keep sending until the complete message has been
 * transferred.
 *
 * @param socket_fd The socket to send to.
 * @param buffer_p The buffer to send the message from.
 * @param num_to_send The length of the buffer to send.
 * @return A positive integer for the number of bytes sent upon success. When negative,
 * it indicates that there was an error with this value being -(num bytes sent) that were
 * sent successfully before the error occurred. If this is zero, it means that there was
 * an error sending the initial message containing the length header.
 */
static int ReceiveDataIntoByteBuffer (int socket_fd, ByteBuffer *buffer_p, const size_t num_to_receive, bool append_flag)
{
	size_t num_received = 0;
	int i;
	bool loop_flag = true;


	if (append_flag)
		{
			size_t space = GetRemainingSpaceInByteBuffer (buffer_p);

			if (num_to_receive >= space)
				{
					if (!ExtendByteBuffer (buffer_p, num_to_receive - space + 1))
						{
							return 0;
						}
				}
		}
	else
		{
			ResetByteBuffer (buffer_p);

			/* Receive the message */
			if (num_to_receive >= buffer_p -> bb_size)
				{
					if (!ResizeByteBuffer (buffer_p, num_to_receive + 1))
						{
							return 0;
						}
				}
		}

	while (loop_flag)
		{
			char buffer [1024];
			i = recv (socket_fd, buffer, 1024, 0);

			if (i != -1)
				{
					num_received += i;

					if (AppendToByteBuffer (buffer_p, buffer, i))
						{
							loop_flag = (num_received < num_to_receive);
						}
					else
						{
							loop_flag = false;
							num_received = -num_received;
						}
				}
			else
				{
					loop_flag = false;
					num_received = -num_received;
				}
		}

	return num_received;
}


/**
 * @brief A Connection that uses raw socket-based communication.
 *
 * @extends Connection
 *
 * @ingroup network_group
 */
typedef struct RawConnection
{
	/** The base Connection */
	Connection rc_base;

	/** The buffer where all of the received data is stored */
	ByteBuffer *rc_data_buffer_p;

	/** The socket the RawConnection is using. */
	int rc_sock_fd;

	/**
	 * <code>true</code> if the Connection is to a Server, <code>
	 * false</code> if it is to a Client.
	 */
	bool rc_server_connection_flag;

	/**
	 * Dependent upon rc_server_connection_flag this stores the underlying
	 * connection data.
	 */
	union
		{
			/** If this RawConnection is to a Server this will be used. */
			struct addrinfo *rc_server_p;

			/** If this RawConnection is to a Client this will be used. */
			struct sockaddr *rc_client_p;
		}
	rc_data;

} RawConnection;


static int ConnectToServer (const char *hostname_s, const char *port_s, struct addrinfo **server_pp);


Connection *AllocaterRawClientConnection (int server_socket_fd)
{
	RawConnection *connection_p = (RawConnection *) AllocMemory (sizeof (RawConnection));

	if (connection_p)
		{
			if (InitConnection (& (connection_p -> rc_base), CT_RAW))
				{
					connection_p -> rc_data_buffer_p = AllocateByteBuffer (1024);

					if (connection_p -> rc_data_buffer_p)
						{
							struct sockaddr *remote_p = (struct sockaddr *) AllocMemory (sizeof (struct sockaddr));

							if (remote_p)
								{
									socklen_t t = sizeof (struct sockaddr);
									int client_socket_fd = accept (server_socket_fd, remote_p, &t);

									if (client_socket_fd != -1)
										{
											connection_p -> rc_sock_fd = client_socket_fd;
											connection_p -> rc_data.rc_client_p = remote_p;
											connection_p -> rc_server_connection_flag = false;

											return (& (connection_p -> rc_base));
										}

									FreeMemory (remote_p);
								}		/* if (remote_p) */

							FreeByteBuffer (connection_p -> rc_data_buffer_p);
						}		/* if (connection_p -> rc_data_buffer_p) */

					ReleaseConnection (& (connection_p -> rc_base));
				}		/* if (InitConnection (& (connection_p -> rc_base))) */

			FreeMemory (connection_p);
		}		/* if (connection_p) */

	return NULL;
}


Connection *AllocateRawServerConnection (const char * const hostname_s, const char * const port_s)
{
	RawConnection *connection_p = (RawConnection *) AllocMemory (sizeof (RawConnection));

	if (connection_p)
		{
			if (InitConnection (& (connection_p -> rc_base), CT_RAW))
				{
					connection_p -> rc_data_buffer_p = AllocateByteBuffer (1024);

					if (connection_p -> rc_data_buffer_p)
						{
							struct addrinfo *server_p = NULL;
							int fd = ConnectToServer (hostname_s, port_s, &server_p);

							if (fd >= 0)
								{
									connection_p -> rc_sock_fd = fd;
									connection_p -> rc_data.rc_server_p = server_p;
									connection_p -> rc_server_connection_flag = true;

									return (& (connection_p -> rc_base));
								}		/* if (fd >= 0) */

							FreeByteBuffer (connection_p -> rc_data_buffer_p);
						}

					ReleaseConnection (& (connection_p -> rc_base));
				}		/* if (InitConnection (& (connection_p -> rc_base))) */

			FreeMemory (connection_p);
		}		/* if (connection_p) */


	return NULL;
}


static int ConnectToServer (const char *hostname_s, const char *port_s, struct addrinfo **server_pp)
{
	struct addrinfo hints;
	int i;
	int sock_fd = -1;

	memset (&hints, 0, sizeof (hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	i = getaddrinfo (hostname_s, port_s, &hints, server_pp);

	if (i == 0)
		{
			struct addrinfo *addr_p = *server_pp;
			int loop_flag = 1;

			/* loop through all the results and connect to the first we can */
			while (loop_flag)
				{
					sock_fd = socket (addr_p -> ai_family, addr_p -> ai_socktype, addr_p -> ai_protocol);

					if (sock_fd != -1)
						{
							i = connect (sock_fd, addr_p -> ai_addr, addr_p -> ai_addrlen);

							if (i != -1)
								{

									loop_flag = 0;
								}
							else
								{
									close (sock_fd);
									sock_fd = -1;
								}
						}

					if (loop_flag)
						{
							addr_p = addr_p -> ai_next;
							loop_flag = (addr_p != NULL);
						}
				}		/* while (addr_p) */

		}		/* if (i == 0) */

	return sock_fd;
}


/**
 * Make sure that we keep sending until the complete message has been
 * transferred.
 *
 * @param socket_fd The socket to send to.
 * @param buffer_p The buffer to send the message from.
 * @param num_to_send The length of the buffer to send.
 * @return A positive integer for the number of bytes received upon success. When negative,
 * it indicates that there was an error with this value being -(num bytes sent) that were
 * sent successfully before the error occurred. If this is zero, it means that there was
 * an error sending the initial message containing the length header.
 */
int AtomicReceiveViaRawConnection (RawConnection *connection_p)
{
	int num_received = 0;
	const int header_size = sizeof (uint32);
	char header_s [header_size];

	/* if the buffer isn't empty, clear it */
	if (GetByteBufferSize (connection_p -> rc_data_buffer_p) > 0)
		{
			ResetByteBuffer (connection_p -> rc_data_buffer_p);
		}


	/* Get the length of the message */
	num_received = ReceiveData (connection_p -> rc_sock_fd, header_s, header_size);

	if (num_received == header_size)
		{
			uint32 *val_p = (uint32 *) header_s;
			uint32 message_size = ntohl (*val_p);

			/* Get the id of the message */
			num_received = ReceiveData (connection_p -> rc_sock_fd, header_s, header_size);

			if (num_received == header_size)
				{
					val_p = (uint32 *) header_s;
					uint32 id_val = ntohl (*val_p);

					num_received = ReceiveDataIntoByteBuffer (connection_p -> rc_sock_fd, connection_p -> rc_data_buffer_p, message_size, false);
				}
		}

	return num_received;
}



/**
 * Make sure that we keep sending until the complete message has been
 * transferred.
 *
 * @param socket_fd The socket to send to.
 * @param buffer_p The buffer to send the message from.
 * @param num_to_send The length of the buffer to send.
 * @return A positive integer for the number of bytes sent upon success. When negative,
 * it indicates that there was an error with this value being -(num bytes sent) that were
 * sent successfully before the error occurred. If this is zero, it means that there was
 * an error sending the initial message containing the length header.
 */
int AtomicSendViaRawConnection (const char *buffer_p, const uint32 num_to_send, RawConnection *connection_p)
{
	int num_sent = 0;
	const size_t header_size = sizeof (uint32);
	char header_s [header_size];

	/* Get the length of the message */
	uint32 i = htonl (num_to_send);

	/*
	 * Send the header size. Note that header_s
	 * isn't a valid c string as it is not null-
	 * terminated.
	 */
	memcpy (header_s, &i, header_size);
	num_sent = SendData (connection_p -> rc_sock_fd, (const void *) header_s, header_size);

	if (num_sent >= 0)
		{
			if ((size_t) num_sent == header_size)
				{
					/*
					 * Send the id. Note that header_s
					 * isn't a valid c string as it is not null-
					 * terminated.
					 */
					i = htonl (connection_p -> rc_base.co_id);
					memcpy (header_s, &i, header_size);
					num_sent = SendData (connection_p -> rc_sock_fd, (const void *) header_s, header_size);

					if (num_sent >= 0)
						{
							if ((size_t) num_sent == header_size)
								{
									/* Send the message */
									num_sent = SendData (connection_p -> rc_sock_fd, buffer_p, num_to_send);
								}
						}
				}
		}

	return num_sent;
}


/**
 * Make sure that we keep sending until the complete message has been
 * transferred.
 *
 * @param socket_fd The socket to send to.
 * @param buffer_p The buffer to send the message from.
 * @param num_to_send The length of the buffer to send.
 * @return A positive integer for the number of bytes sent upon success. When negative,
 * it indicates that there was an error with this value being -(num bytes sent) that were
 * sent successfully before the error occurred. If this is zero, it means that there was
 * an error sending the initial message containing the length header.
 */
static int SendData (int socket_fd, const void *buffer_p, const size_t num_to_send)
{
	size_t num_sent = 0;
	int res = 0;
	int i;
	bool loop_flag = true;

	while (loop_flag)
		{
			i = send (socket_fd, buffer_p, num_to_send, 0);

			if (i != -1)
				{
					num_sent += i;
					buffer_p += i;

					loop_flag = (num_sent < num_to_send);
				}
			else
				{
					loop_flag = false;
					res = -num_sent;
				}
		}

	return res;
}


/**
 * Make sure that we keep sending until the complete message has been
 * transferred.
 *
 * @param socket_fd The socket to send to.
 * @param buffer_p The buffer to send the message from.
 * @param num_to_send The length of the buffer to send.
 * @return A positive integer for the number of bytes sent upon success. When negative,
 * it indicates that there was an error with this value being -(num bytes sent) that were
 * sent successfully before the error occurred. If this is zero, it means that there was
 * an error sending the initial message containing the length header.
 */
static int ReceiveData (int socket_fd, void *buffer_p, const size_t num_to_receive)
{
	size_t num_received = 0;
	int i;
	bool loop_flag = true;

	while (loop_flag)
		{
			i = recv (socket_fd, buffer_p, num_to_receive, 0);

			if (i != -1)
				{
					num_received += i;
					buffer_p += i;

					loop_flag = (num_received < num_to_receive);
				}
			else
				{
					loop_flag = false;
					num_received = -num_received;
				}
		}

	return num_received;
}

