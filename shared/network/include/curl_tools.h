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

#ifndef GRASSROOTS_CURL_TOOLS_H
#define GRASSROOTS_CURL_TOOLS_H

#include <stdio.h>

#include <curl/curl.h>

#include "jansson.h"

#include "typedefs.h"
#include "network_library.h"
#include "byte_buffer.h"


/**
 * An enum specifying where any resultant data from
 * running a CurlTool will be stored
 * @ingroup network_group
 */
typedef enum CurlMode
{
	/** The data will be stored in memory. */
	CM_MEMORY,

	/** The data will be written to disk. */
	CM_FILE,

	/** The number of available modes. */
	CM_NUM_MODES
} CurlMode;


typedef struct TemporaryFile
{
	/** @private */
	FILE *tf_temp_f;

	/** @private */
	char *tf_temp_file_contents_s;

	size_t tf_temp_file_size;

} TemporaryFile;


/**
 * @brief A tool for making http(s) requests and responses.
 *
 * A datatype that uses CURL to perform http(s) requests
 * and responses.
 * @ingroup network_group
 */
typedef struct CurlTool
{
	/** @private */
	CURL *ct_curl_p;


	/** @private */
	char *ct_username_s;

	/** @private */
	char *ct_password_s;


	/** @private */
	CurlMode ct_mode;

	/** @private */
	ByteBuffer *ct_buffer_p;

	/** @private */
	TemporaryFile *ct_temp_file_p;

	/** @private */
	struct curl_httppost *ct_form_p;

	/** @private */
	struct curl_httppost *ct_last_field_p;

	/** @private */
	struct curl_slist *ct_headers_list_p;

	bool ct_verbose_flag;

} CurlTool;



#ifdef __cplusplus
	extern "C" {
#endif



/**
 * Free a CurlTool
 *
 * @param curl_p The CurlTool to free.
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API void FreeCurlTool (CurlTool *curl_p);


/**
 * Free a CURL object.
 *
 * @param curl_p The CURL object to free.
 */
GRASSROOTS_NETWORK_LOCAL void FreeCurl (CURL *curl_p);

/**
 * @brief Set the cryptographic engine to use.
 *
 * @param curl_p The CurlTool instance to set the SSL engine for.
 * @param cryptograph_engine_name_s The name of the cryptographic engine
 * to use. For the valid names see the CURL documentation.
 * @return <code>true</code> if the SSL engine name was set successfully,
 * <code>false</code> otherwise.
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API bool SetSSLEngine (CurlTool *curl_p, const char *cryptograph_engine_name_s);


/**
 * Call a secure web address
 *
 * @param url_s The address to get the data from.
 * @param header_data_s The value to use for CURLOPT_CAINFO. See the Curl documentation.
 * @param ca_cert_name_s The value to use for CURLOPT_CAINFO. See the Curl documentation.
 * @param cert_name_s The value to use for CURLOPT_SSLCERT. See the Curl documentation.
 * @param key_name_s The value to use for CURLOPT_SSLKEY. See the Curl documentation.
 * @param verify_certs If this is true then CURLOPT_SSL_VERIFYPEER will be set. See the Curl documentation.
 * @param buffer_p Where the data from the address will be written to
 * @return <code>true</code> if the call was successful, <code>false</code> otherwise.
 */
GRASSROOTS_NETWORK_API bool CallSecureUrl (const char *url_s, const char *header_data_s, const char *ca_cert_name_s, const char * const cert_name_s, const char *key_name_s, const bool verify_certs, ByteBuffer *buffer_p);


/**
 * Add a callback to write the HTTP response for this CURL object to
 * the given buffer.
 *
 * @param buffer_p The ByteBuffer which will store the CURL object's response. *
 * @param curl_p The CurlTool object to add the callback for.
 * @return <code>true</code> if the CurlTool was updated successfully,
 * <code>false</code> otherwise.
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API bool AddCurlCallback (CurlTool *curl_tool_p, curl_write_callback callback_fn, void *callback_data_p);


/**
 * Set the URI that the CurlTool will try to get the html data from.
 *
 * @param tool_p The CurlTool to update.
 * @param uri_s The URI to set the CurlTool for.
 * @return <code>true</code> if the CurlTool was updated successfully,
 * <code>false</code> otherwise.
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API bool SetUriForCurlTool (CurlTool *tool_p, const char * const uri_s);


/**
 * @brief Run a CurlTool.
 * This will get the CurlTool to get all of the data from its given URI.
 *
 * @param tool_p The CurlTool to run.
 * @return CURLE_OK if successful or an error code upon failure.
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API CURLcode RunCurlTool (CurlTool *tool_p);


/**
 * @brief Set up a CurlTool to do a JSON post when it is run.
 *
 * @param tool_p The CurlTool to update.
 * @return <code>true</code> if the CurlTool was updated successfully,
 * <code>false</code> otherwise.
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API bool SetCurlToolForJSONPost (CurlTool *tool_p);


/**
 * @brief Get the downloaded data from a CurlTool.
 *
 * If the CurlTool has been run successfully, this will get a read-only
 * version of the downloaded data. RunCurlTool() must have been
 * run prior to this.
 * @param tool_p The CurlTool to get the data from.
 * @return The downloaded data or <code>NULL</code> upon error.
 * @see RunCurlTool
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API const char *GetCurlToolData (CurlTool * const tool_p);



/**
 * @brief Get the size in bytes of the downloaded data from a CurlTool.
 *
 * If the CurlTool has been run successfully, this will get the length
 * of the downloaded data. RunCurlTool() must have been
 * run prior to this.
 * @param tool_p The CurlTool to get the length from.
 * @return The length of the downloaded data.
 * @see RunCurlTool
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API size_t GetCurlToolDataSize (CurlTool * const tool_p);


/**
 * @brief Send a JSON request using a CurlTool.
 * @param tool_p The CurlTool to use.
 * @param req_p The json data to send.
 * @return <code>true</code> if the CurlTool made the request and received a response
 * successfully. The resultant data can be retrieved using <code>GetCurlToolData</code>.
 * If the call was unsuccessful, then <code>false</code> will be returned.
 * @see GetCurlToolData
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API bool MakeRemoteJSONCallFromCurlTool (CurlTool *tool_p, const json_t *req_p);


/**
 * Add a key value pair to the request header that a CurlTool will send when it is run.
 *
 * @param tool_p The CurlTool to add the header to
 * @param key_s The key to add. This will be copied so this value does not need to stay in scope.
 * @param value_s The value to add. This will be copied so this value does not need to stay in scope.
 * @return <code>true</code> if the header values were set successfully, <code>false</code>
 * otherwise.
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API bool SetCurlToolHeader (CurlTool *tool_p, const char *key_s, const char *value_s);


/**
 *
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API bool SetCurlToolAuth (CurlTool *tool_p, const char *username_s, const char *password_s);


/**
 *
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API void ClearCurlToolAuth (CurlTool *tool_p);



/**
 * Get the URL encoded version of a string.
 *
 * @param tool_p The CurlTool to use to generate the URL encoded string.
 * @param src_s The string to encode.
 * @return The URL encoded version of the input string. This must be freed with
 * FreeURLEscapedString.
 * @see GetURLEscapedString
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API char *GetURLEscapedString (CurlTool *tool_p, const char *src_s);


/**
 * Free a previously allocated string created by GetURLEscapedString.
 *
 * @param value_s The string to free. This must have been created by GetURLEscapedString.
 * @see GetURLEscapedString
 * @memberof CurlTool
 */
GRASSROOTS_NETWORK_API void FreeURLEscapedString (char *value_s);



GRASSROOTS_NETWORK_API CurlTool *AllocateFileCurlTool (const char * const filename_s);



GRASSROOTS_NETWORK_API CurlTool *AllocateMemoryCurlTool (const size_t buffer_size);


GRASSROOTS_NETWORK_API char *GetWebData (const char *url_s);



GRASSROOTS_NETWORK_API void SetCurlToolTimeout (CurlTool *tool_p, const long timeout);


GRASSROOTS_NETWORK_LOCAL TemporaryFile *AllocateTemporaryFile (const char * const filename_s);



GRASSROOTS_NETWORK_LOCAL void FreeTemporaryFile (TemporaryFile *temp_p);


GRASSROOTS_NETWORK_LOCAL char *GetTemporaryFileContentsAsString (TemporaryFile *temp_p);


GRASSROOTS_NETWORK_API void ClearCurlToolData (CurlTool *tool_p);


GRASSROOTS_NETWORK_API void SetCurlToolVerbose (CurlTool *tool_p, const bool verbose_flag);



GRASSROOTS_NETWORK_API bool DownloadFile (CurlTool * const curl_p, const char * const url_s, const char * const output_filename_s);


#ifdef __cplusplus
}
#endif


#endif		/* #ifndef GRASSROOTS_CURL_TOOLS_H */

