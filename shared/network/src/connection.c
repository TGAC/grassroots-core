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

#ifdef _WIN32

#else
#include <unistd.h>
#endif

#include "connection.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "json_tools.h"
#include "request_tools.h"
#include "streams.h"


#include "raw_connection.h"


#ifdef _DEBUG
	#define CONNECTION_DEBUG	(STM_LEVEL_FINE)
#else
	#define CONNECTION_DEBUG	(STM_LEVEL_NONE)
#endif



static void FreeWebConnection (WebConnection *connection_p);



/******************************/
/***** METHOD DEFINITIONS *****/
/******************************/




bool InitConnection (Connection *connection_p, ConnectionType type)
{
	bool success_flag = true;

	connection_p -> co_id = 1;
	connection_p -> co_type = type;

	return success_flag;
}


void ReleaseConnection (Connection * UNUSED_PARAM (connection_p))
{

}



Connection *AllocateWebServerConnection (const char * const full_uri_s)
{
	WebConnection *connection_p = (WebConnection *) AllocMemory (sizeof (WebConnection));

	if (connection_p)
		{
			CurlTool *curl_p = AllocateCurlTool (CM_MEMORY);

			if (curl_p)
				{
					char *uri_s = EasyCopyToNewString (full_uri_s);

					if (uri_s)
						{
							connection_p -> wc_uri_s = uri_s;

							if (SetUriForCurlTool (curl_p, uri_s))
								{
									if (SetCurlToolForJSONPost (curl_p))
										{
											if (InitConnection (& (connection_p -> wc_base), CT_WEB))
												{
													connection_p -> wc_curl_p = curl_p;

													return (& (connection_p -> wc_base));
												}		/* if (InitConnection (& (connection_p -> wc_base))) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "InitConnection failed for %s", uri_s);
												}
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "SetCurlToolForJSONPost failed for %s", uri_s);
										}
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to SetUriForCurlTool to %s", uri_s);
								}

							FreeCopiedString (connection_p -> wc_uri_s);
						}		/* if (uri_s) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate copy %s", full_uri_s);
						}

					FreeCurlTool (curl_p);
				}		/* if (curl_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate CurlTool to %s", full_uri_s);
				}

			FreeMemory (connection_p);
		}		/* if (connection_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate WebConnection to %s", full_uri_s);
		}

	return NULL;
}


void FreeConnection (Connection *connection_p)
{

	switch (connection_p -> co_type)
		{
			case CT_RAW:
				FreeRawConnection ((struct RawConnection *) connection_p);
				break;

			case CT_WEB:
				FreeWebConnection ((WebConnection *) connection_p);
				break;

			default:
				break;
		}

	ReleaseConnection (connection_p);

	FreeMemory (connection_p);
}



const char *MakeRemoteJsonCallViaConnection (Connection *connection_p, const json_t *req_p)
{
	bool success_flag = false;

	if (connection_p -> co_type == CT_RAW)
		{
			success_flag =  MakeRemoteJsonCallViaRawConnection (connection_p, req_p);
		}
	else if (connection_p -> co_type == CT_WEB)
		{
			WebConnection *web_connection_p = (WebConnection *) connection_p;

			if (MakeRemoteJSONCallFromCurlTool (web_connection_p -> wc_curl_p, req_p))
				{
					success_flag = true;
				}		/* if (MakeRemoteJSONCallFromCurlTool (web_connection_p -> wc_curl_p, req_p)) */
		}


	return (success_flag ? GetConnectionData (connection_p) : NULL);
}



int SendJsonRequestViaRawConnection (struct RawConnection *connection_p, const json_t *json_p)
{
	int res = -1;
	char *req_s = json_dumps (json_p, 0);

	if (req_s)
		{
			res = AtomicSendStringViaRawConnection (req_s, connection_p);
			free (req_s);
		}

	return res;
}



static void FreeWebConnection (WebConnection *connection_p)
{
	FreeCurlTool (connection_p -> wc_curl_p);

	if (connection_p -> wc_uri_s)
		{
			FreeCopiedString (connection_p -> wc_uri_s);
		}
}


const char *GetConnectionData (Connection *connection_p)
{
	const char *data_s = NULL;

	if (connection_p -> co_type == CT_RAW)
		{
		data_s = GetRawConnectionData(connection_p);
		}
	else if (connection_p -> co_type == CT_WEB)
		{
			WebConnection *web_connection_p = (WebConnection *) connection_p;

			data_s = GetCurlToolData (web_connection_p -> wc_curl_p);
		}

	return data_s;
}


bool SetConnectionCredentials (Connection *connection_p, const char *username_s, const char *password_s)
{
	bool success_flag = false;

	if (connection_p -> co_type == CT_WEB)
		{
			WebConnection *web_conn_p = (WebConnection *) connection_p;

			success_flag = SetCurlToolAuth (web_conn_p -> wc_curl_p, username_s, password_s);
		}

	return success_flag;
}



