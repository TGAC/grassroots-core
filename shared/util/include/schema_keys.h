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
 * @file schema_keys.h
 * @brief The keys for the Grassroots Schema.
 *
 * The messages sent between the Grassroots Servers and Clients are JSON-based
 * and the various available keys are documented in this file.
 *
 * The keys are available as constant strings where the first word or phrase
 * shows the object which the key represents a member of. For instance ::PARAM_CURRENT_VALUE_S
 * and ::PARAM_SET_NAME_S refer to the current value of a Parameter and the name of
 * a ParameterSet respectively.
 *
 * Under normal usage you will not need to explicitly do the conversion of a JSON description to the
 * object that it represents in a step-by-step way as each object will have functions to do this e.g.
 * a ParameterSet has GetParameterSetAsJSON() to get the JSON message for a given ParameterSet and
 * CreateParameterSetFromJSON() to convert back from the JSON to a ParameterSet.
 */

#ifndef SHARED_SRC_UTIL_INCLUDE_SCHEMA_KEYS_H_
#define SHARED_SRC_UTIL_INCLUDE_SCHEMA_KEYS_H_


#include "grassroots_util_library.h"

/*
 * The following preprocessor macros allow us to declare
 * and define the variables in the same place. By default,
 * they will expand to
 *
 * 		extern const char *SERVICE_NAME_S;
 *
 * however if ALLOCATE_JSON_TAGS is defined then it will
 * become
 *
 * 		const char *SERVICE_NAME_S = "path";
 *
 * ALLOCATE_JSON_TAGS must be defined only once prior to
 * including this header file. Currently this happens in
 * scheme_keys.c.
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS


	#ifdef GRASSROOTS_UTIL_LIBRARY_EXPORTS // defined if we are building the LIB DLL (instead of using it)

		#ifdef ALLOCATE_SCHEMA_KEYS_TAGS
			#define SCHEMA_KEYS_PREFIX  GRASSROOTS_UTIL_API
			#define SCHEMA_KEYS_VAL(x) = x
			#define SCHEMA_KEYS_CONCAT_VAL(x,y) = x y
		#else
			#define SCHEMA_KEYS_PREFIX extern
			#define SCHEMA_KEYS_VAL(x)
			#define SCHEMA_KEYS_CONCAT_VAL(x,y)
		#endif
	#else
		#define SCHEMA_KEYS_PREFIX GRASSROOTS_UTIL_API
		#define SCHEMA_KEYS_VAL(x)
		#define SCHEMA_KEYS_CONCAT_VAL(x,y)
	#endif

#endif //	#ifndef DOXYGEN_SHOULD_SKIP_THIS



//#ifndef DOXYGEN_SHOULD_SKIP_THIS
//
//	#ifdef GRASSROOTS_UTIL_LIBRARY_EXPORTS // defined if we are building the LIB DLL (instead of using it)
//
//		#define SCHEMA_KEYS_PREFIX GRASSROOTS_UTIL_API
//
//		#ifdef ALLOCATE_SCHEMA_KEYS_TAGS
//			#define SCHEMA_KEYS_VAL(x)	= x
//			#define SCHEMA_KEYS_CONCAT_VAL(x,y) = x y
//		#else
//			#define SCHEMA_KEYS_VAL(x)
//			#define SCHEMA_KEYS_CONCAT_VAL(x,y)
//		#endif
//
//	#endif		//#ifdef GRASSROOTS_UTIL_LIBRARY_EXPORTS
//
//#endif		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */




#ifdef __cplusplus
extern "C"
{
#endif


#define CONTEXT_PREFIX_SCHEMA_ORG_S "so:"
SCHEMA_KEYS_PREFIX const char *CONTEXT_URL_SCHEMA_ORG_S SCHEMA_KEYS_VAL("http://schema.org/");

#define CONTEXT_PREFIX_EDAM_ONTOLOGY_S "eo:"
SCHEMA_KEYS_PREFIX const char *CONTEXT_URL_EDAM_ONOTOLOGY_S SCHEMA_KEYS_VAL("http://edamontology.org/");

#define CONTEXT_PREFIX_EXPERIMENTAL_FACTOR_ONTOLOGY_S "efo:"
SCHEMA_KEYS_PREFIX const char *CONTEXT_URL_EXPERIMENTAL_FACTOR_ONOTOLOGY_S SCHEMA_KEYS_VAL("http://www.ebi.ac.uk/efo/");

#define CONTEXT_PREFIX_SOFTWARE_ONTOLOGY_S "swo:"
SCHEMA_KEYS_PREFIX const char *CONTEXT_URL_SOFTWARE_ONOTOLOGY_S SCHEMA_KEYS_VAL("http://www.ebi.ac.uk/swo/");

#define CONTEXT_PREFIX_CROP_ONTOLOGY_S "co:"
SCHEMA_KEYS_PREFIX const char *CONTEXT_URL_CROP_ONTOLOGY_S SCHEMA_KEYS_VAL("http://www.cropontology.org/terms/");

#define CONTEXT_PREFIX_ENVIRONMENT_ONTOLOGY_S "envo:"
SCHEMA_KEYS_PREFIX const char *CONTEXT_URL_ENVIRONMENT_ONTOLOGY_S SCHEMA_KEYS_VAL("http://purl.obolibrary.org/obo/ENVO_");

#define CONTEXT_PREFIX_AGRONOMY_ONTOLOGY_S "agro:"
SCHEMA_KEYS_PREFIX const char *CONTEXT_URL_AGRONOMY_ONTOLOGY_S SCHEMA_KEYS_VAL("http://purl.obolibrary.org/obo/AGRO_");

#define CONTEXT_PREFIX_NCI_THESAUSUS_ONTOLOGY_S "ncit:"
SCHEMA_KEYS_PREFIX const char *CONTEXT_URL_NCI_THESAUSUS_ONTOLOGY_S SCHEMA_KEYS_VAL("http://purl.obolibrary.org/obo/NCIT_");


#define CONTEXT_PREFIX_STATS_ONTOLOGY_S "stato:"
PREFIX const char *CONTEXT_URL_STATS_ONTOLOGY_S VAL("http://purl.obolibrary.org/obo/STATO_");


	/**
	 *  @nosubgrouping
	 *
	 */

	/**
	 * The JSON key for the errors object of the server response.
	 *
	 * @ingroup SCHEMA_ERROR_GROUP
	 */
	SCHEMA_KEYS_PREFIX const char *ERROR_S SCHEMA_KEYS_VAL("error");

	/**
	 * The JSON key for the header object of the server response.
	 */
	SCHEMA_KEYS_PREFIX const char *HEADER_S SCHEMA_KEYS_VAL("header");

	/**
	 * The JSON key for a request.
	 */
	SCHEMA_KEYS_PREFIX const char *REQUEST_S SCHEMA_KEYS_VAL("request");
	SCHEMA_KEYS_PREFIX const char *REQUEST_VERBOSE_S SCHEMA_KEYS_VAL("verbose");


	/**
	 * @name Schema definitions for specifying the Grassroots schema version.
	 */
	/* Start of doxygen member group */
	/**@{*/

	/**
	 * The JSON key for the schema object of the server response.
	 * This appears at the top level of JSON request objects.
	 */
	SCHEMA_KEYS_PREFIX const char *SCHEMA_S SCHEMA_KEYS_VAL("schema");

	/**
	 * The JSON key for specifying the major version of the schema.
	 * This is a child of @link SCHEMA_S @endlink
	 */
	SCHEMA_KEYS_PREFIX const char *VERSION_MAJOR_S SCHEMA_KEYS_VAL("major");

	/**
	 * The JSON key for specifying the minor version of the schema.
	 * This is a child of @link SCHEMA_S @endlink
	 */
	SCHEMA_KEYS_PREFIX const char *VERSION_MINOR_S SCHEMA_KEYS_VAL("minor");


	SCHEMA_KEYS_PREFIX const char *VERSION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "softwareVersion");

	/* End of doxygen member group */
	/**@}*/


	/** @name The Schema definitions for specifying the Service Providers.
	 * @ingroup SCHEMA_SERVICE_PROVIDERS_GROUP
	 */
	/* Start of doxygen member group */
	/**@{*/
	/**
	 * @brief The JSON key for specifying an array of external Grassroots Servers.
	 *
	 * The objects contained in this array each have ::PROVIDER_NAME_S,
	 * ::PROVIDER_DESCRIPTION_S and ::PROVIDER_URI_S keys to describe each of the
	 * Servers.
	 */
	SCHEMA_KEYS_PREFIX const char *SERVER_MULTIPLE_PROVIDERS_S SCHEMA_KEYS_VAL("providers");

	/**
	 * @brief The JSON key for specifying details about this Grassroots Server.
	 *
	 * The child key-value pairs of this object are ::PROVIDER_NAME_S,
	 * ::PROVIDER_DESCRIPTION_S and ::PROVIDER_URI_S keys to describe each of the
	 * Servers.
	 */
	SCHEMA_KEYS_PREFIX const char *SERVER_PROVIDER_S SCHEMA_KEYS_VAL("provider");

	/**
	 * @brief The JSON key for the name of the Server to show to Clients and other connected
	 * Servers.
	 *
	 * @see ::SERVER_MULTIPLE_PROVIDERS_S
	 * @see :: SERVER_PROVIDER_S
	 */
	SCHEMA_KEYS_PREFIX const char *PROVIDER_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

	/**
	 * @brief The JSON key for the description of the Server to show to Clients and other connected
	 * Servers.
	 *
	 * @see ::SERVER_MULTIPLE_PROVIDERS_S
	 * @see :: SERVER_PROVIDER_S
	 */
	SCHEMA_KEYS_PREFIX const char *PROVIDER_DESCRIPTION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");


	/**
	 * @brief The JSON key for the url of a logo of the Server to show to Clients and other connected
	 * Servers.
	 *
	 * @see ::SERVER_MULTIPLE_PROVIDERS_S
	 * @see :: SERVER_PROVIDER_S
	 */
	SCHEMA_KEYS_PREFIX const char *PROVIDER_LOGO_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "logo");


	/**
	 * @brief The JSON key for the URI that accepts Grassroots message on the Server.
	 *
	 * @see ::SERVER_MULTIPLE_PROVIDERS_S
	 * @see :: SERVER_PROVIDER_S
	 */
	SCHEMA_KEYS_PREFIX const char *PROVIDER_URI_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "url");
	/* End of doxygen member group */
	/**@}*/


	/** @name The Schema definitions for specifying Servers. */
	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *SERVERS_MANAGER_S SCHEMA_KEYS_VAL("servers_manager");
	SCHEMA_KEYS_PREFIX const char *SERVERS_S SCHEMA_KEYS_VAL("servers");
	SCHEMA_KEYS_PREFIX const char *SERVER_UUID_S SCHEMA_KEYS_VAL("server_uuid");
	SCHEMA_KEYS_PREFIX const char *SERVER_NAME_S SCHEMA_KEYS_VAL("server_name");
	SCHEMA_KEYS_PREFIX const char *SERVER_CONNECTION_TYPE_S SCHEMA_KEYS_VAL("server_connection");
	SCHEMA_KEYS_PREFIX const char *SERVER_URI_S SCHEMA_KEYS_VAL("server_uri");
	/* End of doxygen member group */
	/**@}*/


	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *SERVER_PAIRED_SERVICES_S SCHEMA_KEYS_VAL("paired_services");
	SCHEMA_KEYS_PREFIX const char *SERVER_LOCAL_PAIRED_SERVCE_S SCHEMA_KEYS_VAL("local");
	SCHEMA_KEYS_PREFIX const char *SERVER_REMOTE_PAIRED_SERVCE_S SCHEMA_KEYS_VAL("remote");
	/**@}*/



	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *CONNECTION_RAW_S SCHEMA_KEYS_VAL("connection_raw");
	SCHEMA_KEYS_PREFIX const char *CONNECTION_WEB_S SCHEMA_KEYS_VAL("connection_web");
	/* End of doxygen member group */
	/**@}*/

	/* Start of doxygen member group */
	/**@{*/
	/**
	 * @deprecated Use ::SERVER_OPERATION_S instead.
	 */
	SCHEMA_KEYS_PREFIX const char *SERVER_OPERATIONS_S SCHEMA_KEYS_VAL("operations");
	SCHEMA_KEYS_PREFIX const char *SERVER_OPERATION_S SCHEMA_KEYS_VAL("operation");

	/**
	 * The key to specify the operation id.
	 * This uses the numeric values from enum Operation.
	 *
	 * @deprecated Use ::OPERATION_S instead.
	 */
	SCHEMA_KEYS_PREFIX const char *OPERATION_ID_OLD_S SCHEMA_KEYS_VAL("operationId");

	/**
	 * The key to specify the operation id.
	 * This uses the numeric values from enum Operation.
	 *
	 * @deprecated Use ::OPERATION_S instead.
	 */
	SCHEMA_KEYS_PREFIX const char *OPERATION_ID_S SCHEMA_KEYS_VAL("so:name");

	/**
	 * The key to specify the operation id.
	 * This uses the string values returned from
	 * GetOperationAsString().
	 *
	 */
	SCHEMA_KEYS_PREFIX const char *OPERATION_S SCHEMA_KEYS_VAL("operation");
	SCHEMA_KEYS_PREFIX const char *OPERATION_DESCRIPTION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");
	SCHEMA_KEYS_PREFIX const char *OPERATION_INFORMATION_URI_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "url");
	SCHEMA_KEYS_PREFIX const char *OPERATION_ICON_URI_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "image");
	SCHEMA_KEYS_PREFIX const char *OPERATION_ICON_DATA_S SCHEMA_KEYS_VAL("icon_data");
	SCHEMA_KEYS_PREFIX const char *OPERATION_SYNCHRONOUS_S SCHEMA_KEYS_VAL("synchronous");
	/* End of doxygen member group */
	/**@}*/

	/** @name The Schema definitions for specifying Services. */
	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *SERVICES_NAME_S SCHEMA_KEYS_VAL("services");
	SCHEMA_KEYS_PREFIX const char *SERVICES_ID_S SCHEMA_KEYS_VAL("path");
	SCHEMA_KEYS_PREFIX const char *SERVICE_DESCRIPTION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");
	SCHEMA_KEYS_PREFIX const char *SERVICE_RUN_S SCHEMA_KEYS_VAL("start_service");
	SCHEMA_KEYS_PREFIX const char *SERVICE_REFRESH_S SCHEMA_KEYS_VAL("refresh_service");
	SCHEMA_KEYS_PREFIX const char *SERVICES_STATUS_S SCHEMA_KEYS_VAL("status");
	SCHEMA_KEYS_PREFIX const char *SERVICES_STATUS_DEFAULT_S SCHEMA_KEYS_VAL("default");
	/* End of doxygen member group */
	/**@}*/


	/** @name The Schema definitions for specifying Services. */
	/* Start of doxygen member group */
	/**@{*/

	SCHEMA_KEYS_PREFIX const char *TYPE_SERVICE_S SCHEMA_KEYS_VAL("grassroots_service");

	SCHEMA_KEYS_PREFIX const char *SERVICE_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");
	SCHEMA_KEYS_PREFIX const char *SERVICE_ALIAS_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "alternateName");
	SCHEMA_KEYS_PREFIX const char *SERVICE_HELP_URL_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "url");
	SCHEMA_KEYS_PREFIX const char *SERVICE_UUID_S SCHEMA_KEYS_VAL("service_uuid");
	SCHEMA_KEYS_PREFIX const char *SERVICE_STATUS_VALUE_S SCHEMA_KEYS_VAL("status");
	SCHEMA_KEYS_PREFIX const char *SERVICE_STATUS_S SCHEMA_KEYS_VAL("status_text");
	SCHEMA_KEYS_PREFIX const char *SERVICE_RESULTS_S SCHEMA_KEYS_VAL("results");
	SCHEMA_KEYS_PREFIX const char *SERVICE_CATEGORY_S SCHEMA_KEYS_VAL("category");
	SCHEMA_KEYS_PREFIX const char *SERVICE_JOBS_S SCHEMA_KEYS_VAL("jobs");
	SCHEMA_KEYS_PREFIX const char *SERVICE_CONFIG_S SCHEMA_KEYS_VAL("config");
	SCHEMA_KEYS_PREFIX const char *SERVICE_TYPE_VALUE_S SCHEMA_KEYS_VAL("Grassroots:Service");

	SCHEMA_KEYS_PREFIX const char *SERVICE_RUN_MODE_S SCHEMA_KEYS_VAL("run_mode");


	/* End of doxygen member group */
	/**@}*/


	SCHEMA_KEYS_PREFIX const char *SERVICE_METADATA_APPLICATION_CATEGORY_S SCHEMA_KEYS_VAL("application_category");
	SCHEMA_KEYS_PREFIX const char *SERVICE_METADATA_APPLICATION_SUBCATEGORY_S SCHEMA_KEYS_VAL("application_subcategory");
	SCHEMA_KEYS_PREFIX const char *SERVICE_METADATA_APPLICATION_INPUT_S SCHEMA_KEYS_VAL("input");
	SCHEMA_KEYS_PREFIX const char *SERVICE_METADATA_APPLICATION_OUTPUT_S SCHEMA_KEYS_VAL("output");

	SCHEMA_KEYS_PREFIX const char *SCHEMA_TERM_URL_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "sameAs");
	SCHEMA_KEYS_PREFIX const char *SCHEMA_TERM_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");
	SCHEMA_KEYS_PREFIX const char *SCHEMA_TERM_DESCRIPTION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");
	SCHEMA_KEYS_PREFIX const char *SCHEMA_TERM_ABBREVIATION_S SCHEMA_KEYS_VAL("abbreviation");



	/** @name The Schema definitions for specifying LinkedServices. */
	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *LINKED_SERVICES_S SCHEMA_KEYS_VAL("linked_services");
	SCHEMA_KEYS_PREFIX const char *LINKED_SERVICE_S SCHEMA_KEYS_VAL("linked_service");
	SCHEMA_KEYS_PREFIX const char *LINKED_SERVICE_FUNCTION_S SCHEMA_KEYS_VAL("function");
	SCHEMA_KEYS_PREFIX const char *LINKED_SERVICE_CONFIG_S SCHEMA_KEYS_VAL("config");
	SCHEMA_KEYS_PREFIX const char *MAPPED_PARAMS_LIST_S SCHEMA_KEYS_VAL("mappings");
	SCHEMA_KEYS_PREFIX const char *MAPPED_PARAM_INPUT_S SCHEMA_KEYS_VAL("input");
	SCHEMA_KEYS_PREFIX const char *MAPPED_PARAM_OUTPUT_S SCHEMA_KEYS_VAL("output");
	SCHEMA_KEYS_PREFIX const char *MAPPED_PARAM_REQUIRED_S SCHEMA_KEYS_VAL("required");
	SCHEMA_KEYS_PREFIX const char *MAPPED_PARAMS_ROOT_S SCHEMA_KEYS_VAL("input_root");
	SCHEMA_KEYS_PREFIX const char *MAPPED_PARAM_MULTI_VALUED_S SCHEMA_KEYS_VAL("multiple");
	/* End of doxygen member group */
	/**@}*/


	/** @name The Schema definitions for specifying ServiceJobs. */
	/* Start of doxygen member group */
	/**@{*/


	SCHEMA_KEYS_PREFIX const char *JOBS_MANAGER_S SCHEMA_KEYS_VAL("jobs_manager");

	/**
	 * The JSON key for the name of a ServiceJob.
	 */
	SCHEMA_KEYS_PREFIX const char *JOB_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

	SCHEMA_KEYS_PREFIX const char *JOB_S SCHEMA_KEYS_VAL("job");

	/**
	 * The JSON key of the Service name for a ServiceJob.
	 */
	SCHEMA_KEYS_PREFIX const char *JOB_SERVICE_S SCHEMA_KEYS_VAL("service_name");
	SCHEMA_KEYS_PREFIX const char *JOB_DESCRIPTION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");


	SCHEMA_KEYS_PREFIX const char *JOB_RESULTS_S SCHEMA_KEYS_VAL("results");
	SCHEMA_KEYS_PREFIX const char *JOB_OMITTED_RESULTS_S SCHEMA_KEYS_VAL("results_omitted");
	SCHEMA_KEYS_PREFIX const char *JOB_ERRORS_S SCHEMA_KEYS_VAL("errors");
	SCHEMA_KEYS_PREFIX const char *JOB_ERROR_S SCHEMA_KEYS_VAL("error");
	/**
	 * The JSON key to specify errors for ServiceJobs where the parameters
	 * have been valid, yet the job has failed whilst running.
	 */
	SCHEMA_KEYS_PREFIX const char *JOB_RUNTIME_ERRORS_S SCHEMA_KEYS_VAL("runtime_errors");
	SCHEMA_KEYS_PREFIX const char *JOB_METADATA_S SCHEMA_KEYS_VAL("metadata");
	SCHEMA_KEYS_PREFIX const char *JOB_UUID_S SCHEMA_KEYS_VAL("job_uuid");
	SCHEMA_KEYS_PREFIX const char *JOB_REMOTE_URI_S  SCHEMA_KEYS_VAL("remote_uri");
	SCHEMA_KEYS_PREFIX const char *JOB_REMOTE_UUID_S  SCHEMA_KEYS_VAL("remote_uuid");
	SCHEMA_KEYS_PREFIX const char *JOB_REMOTE_SERVICE_S  SCHEMA_KEYS_VAL("remote_service");
	SCHEMA_KEYS_PREFIX const char *JOB_REMOTE_S  SCHEMA_KEYS_VAL("remote_job");
	SCHEMA_KEYS_PREFIX const char *JOB_TYPE_S  SCHEMA_KEYS_VAL("job_type");

	/* End of doxygen member group */
	/**@}*/


	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *PLUGIN_NAME_S SCHEMA_KEYS_VAL("plugin");
	/* End of doxygen member group */
	/**@}*/


	/** @name  The Schema definitions for specifying ParameterSets. */
	/* Start of doxygen member group */
	/**@{*/

	/**
	 * The JSON key for the ParameterSet object of the server response.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_SET_KEY_S SCHEMA_KEYS_VAL("parameter_set");

	/**
	 * The JSON key for the name of ParameterSet object of the server response.
	 * This is a child of @link PARAM_SET_KEY_S @endlink
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_SET_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

	/**
	 * The JSON key for the description of ParameterSet object of the server response.
	 * This is a child of @link PARAM_SET_KEY_S @endlink
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_SET_DESCRIPTION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");

	/**
	 * The JSON key for the child object detailing all of the Parameters within a ParameterSet.
	 *
	 * This is a child of @link PARAM_SET_KEY_S @endlink
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_SET_PARAMS_S SCHEMA_KEYS_VAL("parameters");

	/**
	 * The JSON key for the child object containing the names of all of the
	 * ParameterGroups within a ParameterSet.

	 * This is a child of @link PARAM_SET_KEY_S @endlink
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_SET_GROUPS_S SCHEMA_KEYS_VAL("groups");
	/* End of doxygen member group */
	/**@}*/


	/**
	 * @name The Schema definitions for specifying Parameters
	 * Each Parameter is an object in a json array value which
	 * is a child of @link PARAM_SET_PARAMS_S @endlink.
	 */
	/* Start of doxygen member group */
	/**@{*/

	/**
	 * The JSON key for the name of Parameter.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_NAME_S SCHEMA_KEYS_VAL("param");

	/**
	 * The JSON key for the display name of Parameter.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_DISPLAY_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");


	SCHEMA_KEYS_PREFIX const char *PARAM_CONCISE_DEFINITION_S SCHEMA_KEYS_VAL("concise");

	/**
	 * The JSON key for the description of Parameter.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_DESCRIPTION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");

	SCHEMA_KEYS_PREFIX const char *PARAM_STORE_S SCHEMA_KEYS_VAL("store");

	SCHEMA_KEYS_PREFIX const char *PARAM_REMOTE_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");


	SCHEMA_KEYS_PREFIX const char *PARAM_TYPE_S SCHEMA_KEYS_VAL("type");
	SCHEMA_KEYS_PREFIX const char *PARAM_GRASSROOTS_TYPE_INFO_S SCHEMA_KEYS_VAL("grassroots_type_number");
	SCHEMA_KEYS_PREFIX const char *PARAM_GRASSROOTS_TYPE_INFO_TEXT_S SCHEMA_KEYS_VAL("grassroots_type");
	SCHEMA_KEYS_PREFIX const char *PARAM_GRASSROOTS_S SCHEMA_KEYS_VAL("parameter_type");
	SCHEMA_KEYS_PREFIX const char *PARAM_COMPOUND_TEXT_S SCHEMA_KEYS_VAL("text");
	SCHEMA_KEYS_PREFIX const char *PARAM_COMPOUND_VALUE_S SCHEMA_KEYS_VAL("value");


	SCHEMA_KEYS_PREFIX const char *PARAM_REFRESH_S SCHEMA_KEYS_VAL("refresh");

	SCHEMA_KEYS_PREFIX const char *PARAM_REQUIRED_S SCHEMA_KEYS_VAL("required");


	SCHEMA_KEYS_PREFIX const char *PARAM_READ_ONLY_S SCHEMA_KEYS_VAL("read_only");


	/**
	 * The JSON key for the default value of Parameter.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_DEFAULT_VALUE_S  SCHEMA_KEYS_VAL("default_value");

	/**
	 * The JSON key for the current value of Parameter.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_CURRENT_VALUE_S  SCHEMA_KEYS_VAL("current_value");

	SCHEMA_KEYS_PREFIX const char *PARAM_OPTIONS_S  SCHEMA_KEYS_VAL("enum");


	SCHEMA_KEYS_PREFIX const char *PARAM_VISIBLE_S  SCHEMA_KEYS_VAL("visible");


	/**
	 * The JSON key for the name of the ParameterGroup for this Parameter.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_GROUP_S SCHEMA_KEYS_VAL("group");


	/**
	 * The JSON key for the name of the ParameterGroup for this Parameter.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_REPEATED_GROUPS_S SCHEMA_KEYS_VAL("groups");

	/**
	 * The JSON key for specifying the name of a ParameterGroup
	 * to a user.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_GROUP_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

	/**
	 * The JSON key for specifying the default visibility of a ParameterGroup
	 * to a user.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_GROUP_VISIBLE_S SCHEMA_KEYS_VAL("visible");



	/**
	 * The JSON key for specifying whether the parameters within a ParameterGroup
	 * can be repeated multiple times.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_GROUP_REPEATABLE_S SCHEMA_KEYS_VAL("repeatable");


	/**
	 * If the parameters are repeatable, specify the parameter to use the entry labels.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_GROUP_REPEATABLE_LABEL_S SCHEMA_KEYS_VAL("repeatable_label");


	/**
	 * If the parameters are repeatable, specify the parameter to use the entry labels.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_GROUP_PARAMS_S SCHEMA_KEYS_VAL("parameters");


	/**
	 * The JSON key for specifying the minimum value that a numeric Parameter
	 * can take.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_MIN_S  SCHEMA_KEYS_VAL("min");


	/**
	 * The JSON key for specifying the maximum value that a numeric Parameter
	 * can take.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_MAX_S  SCHEMA_KEYS_VAL("max");

	/**
	 * The JSON key for specifying the level of the Parameter.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_LEVEL_S  SCHEMA_KEYS_VAL("level");
	SCHEMA_KEYS_PREFIX const char *PARAM_LEVEL_TEXT_S  SCHEMA_KEYS_VAL("level_text");


	/**
	 * The JSON key for specifying that a Parameter is suitable for
	 * all levels.
	 */

	SCHEMA_KEYS_PREFIX const char *PARAM_LEVEL_TEXT_ALL_S  SCHEMA_KEYS_VAL("all");

	/**
	 * The JSON key for specifying that a Parameter to be available
	 * for a client at the beginner level.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_LEVEL_TEXT_SIMPLE_S  SCHEMA_KEYS_VAL("simple");


	/**
	 * The JSON key for specifying that a Parameter to be available
	 * for a client at the advanced level.
	 */
	SCHEMA_KEYS_PREFIX const char *PARAM_LEVEL_TEXT_ADVANCED_S  SCHEMA_KEYS_VAL("advanced");

	SCHEMA_KEYS_PREFIX const char *PARAM_ERRORS_S  SCHEMA_KEYS_VAL("errors");

	SCHEMA_KEYS_PREFIX const char *PARAM_SERVER_ID_S  SCHEMA_KEYS_VAL("server_id");

	SCHEMA_KEYS_PREFIX const char *PARAM_REMOTE_URI_S  SCHEMA_KEYS_VAL("remote_uri");

	SCHEMA_KEYS_PREFIX const char *PARAM_REMOTE_S  SCHEMA_KEYS_VAL("remote_details");

	SCHEMA_KEYS_PREFIX const char *PARAM_VALUE_SET_FROM_TEXT_S  SCHEMA_KEYS_VAL("value_as_text");


	SCHEMA_KEYS_PREFIX const char *KEYWORDS_QUERY_S SCHEMA_KEYS_VAL("query");
	/* End of doxygen member group */
	/**@}*/


	/** @name  The Schema definitions for specifying SharedType values. */
	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *SHARED_TYPE_DESCRIPTION_S  SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");
	SCHEMA_KEYS_PREFIX const char *SHARED_TYPE_VALUE_S  SCHEMA_KEYS_VAL("value");
	/* End of doxygen member group */
	/**@}*/


	/** @name The Schema definitions for specifying Resources.
	 * The JSON keys for defining a Resource
	 */
	/**@{*/

	/**
	 * The JSON key for specifying a Resource object.
	 */
	SCHEMA_KEYS_PREFIX const char *RESOURCE_S  SCHEMA_KEYS_VAL("resource");

	/**
	 * The JSON key for specifying the protocol for a Resource.
	 */
	SCHEMA_KEYS_PREFIX const char *RESOURCE_PROTOCOL_S  SCHEMA_KEYS_VAL("protocol");

	/**
	 * The JSON key for specifying the value of a Resource.
	 * This could be the filename, uri, etc. depending upon
	 * the protocol for this Resource.
	 */
	SCHEMA_KEYS_PREFIX const char *RESOURCE_VALUE_S  SCHEMA_KEYS_VAL("value");

	/**
	 * The JSON key for specifying the a user-friendly description
	 * of a Resource.
	 */
	SCHEMA_KEYS_PREFIX const char *RESOURCE_DESCRIPTION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");

	/**
	 * The JSON key for specifying a title to display to Clients
	 * and external Servers, rather than show the name.
	 */
	SCHEMA_KEYS_PREFIX const char *RESOURCE_TITLE_S  SCHEMA_KEYS_VAL("title");


	/**
	 * The JSON key for specifying any inline data for a Resource.
	 */
	SCHEMA_KEYS_PREFIX const char *RESOURCE_DATA_S  SCHEMA_KEYS_VAL("data");
	/* End of doxygen member group */
	/**@}*/


	/** @name The Schema definitions for specifying Parameters.
	 * */
	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *TAG_INPUT_NAME_S SCHEMA_KEYS_VAL("input");
	SCHEMA_KEYS_PREFIX const char *TAG_OUTPUT_NAME_S SCHEMA_KEYS_VAL("output");
	/* End of doxygen member group */
	/**@}*/


	SCHEMA_KEYS_PREFIX const char *CONFIG_S SCHEMA_KEYS_VAL("config");


	/** @name The Schema definitions for specifying user credentials. */
	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_S SCHEMA_KEYS_VAL("credentials");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_USERNAME_S SCHEMA_KEYS_VAL("username");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_PASSWORD_S SCHEMA_KEYS_VAL("password");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_UUID_S SCHEMA_KEYS_VAL("user_uuid");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_TOKEN_KEY_S SCHEMA_KEYS_VAL("token_key");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_TOKEN_SECRET_S SCHEMA_KEYS_VAL("token_secret");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_APP_KEY_S SCHEMA_KEYS_VAL("app_key");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_APP_SECRET_S SCHEMA_KEYS_VAL("app_secret");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_ENCRYPTION_METHOD_S SCHEMA_KEYS_VAL("encrypt_method");
	SCHEMA_KEYS_PREFIX const char *CREDENTIALS_ENCRYPTION_KEY_S SCHEMA_KEYS_VAL("encrypt_key");
	/* End of doxygen member group */
	/**@}*/


	/** @name The Schema definitions for specifying DRMAA details. */
	/* Start of doxygen member group */
	/**@{*/
	SCHEMA_KEYS_PREFIX const char *DRMAA_S SCHEMA_KEYS_VAL("drmaa");
	SCHEMA_KEYS_PREFIX const char *DRMAA_PROGRAM_NAME_S SCHEMA_KEYS_VAL("program_name");
	SCHEMA_KEYS_PREFIX const char *DRMAA_ENVIRONMENT_S SCHEMA_KEYS_VAL("env_vars");
	SCHEMA_KEYS_PREFIX const char *DRMAA_JOB_NAME_S SCHEMA_KEYS_VAL("job_name");
	SCHEMA_KEYS_PREFIX const char *DRMAA_QUEUE_S SCHEMA_KEYS_VAL("queue");
	SCHEMA_KEYS_PREFIX const char *DRMAA_WORKING_DIR_S SCHEMA_KEYS_VAL("working_dir");
	SCHEMA_KEYS_PREFIX const char *DRMAA_OUTPUT_FILE_S SCHEMA_KEYS_VAL("output_file");
	SCHEMA_KEYS_PREFIX const char *DRMAA_ID_S SCHEMA_KEYS_VAL("drmma_job_id");
	SCHEMA_KEYS_PREFIX const char *DRMAA_UUID_S SCHEMA_KEYS_VAL("drmma_grassroots_uuid");
	SCHEMA_KEYS_PREFIX const char *DRMAA_OUT_ID_S SCHEMA_KEYS_VAL("drmma_job_out_id");
	SCHEMA_KEYS_PREFIX const char *DRMAA_HOSTNAME_S SCHEMA_KEYS_VAL("host");
	SCHEMA_KEYS_PREFIX const char *DRMAA_USERNAME_S SCHEMA_KEYS_VAL("user");
	SCHEMA_KEYS_PREFIX const char *DRMAA_EMAILS_S SCHEMA_KEYS_VAL("emails");
	SCHEMA_KEYS_PREFIX const char *DRMAA_ARGS_S SCHEMA_KEYS_VAL("args");
	SCHEMA_KEYS_PREFIX const char *DRMAA_NUM_CORES_S SCHEMA_KEYS_VAL("num_cores");
	SCHEMA_KEYS_PREFIX const char *DRMAA_MEM_USAGE_S SCHEMA_KEYS_VAL("mem");
	/* End of doxygen member group */
	/**@}*/


	SCHEMA_KEYS_PREFIX const char *WEB_SERVICE_METHOD_S SCHEMA_KEYS_VAL("method");
	SCHEMA_KEYS_PREFIX const char *WEB_SERVICE_URL_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "url");


	SCHEMA_KEYS_PREFIX const char *KVP_KEY_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");
	SCHEMA_KEYS_PREFIX const char *KVP_VALUE_S SCHEMA_KEYS_VAL("value");




	SCHEMA_KEYS_PREFIX const char *INDEXING_NAME_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "name");
	SCHEMA_KEYS_PREFIX const char *INDEXING_SERVICE_NAME_S SCHEMA_KEYS_VAL("service");
	SCHEMA_KEYS_PREFIX const char *INDEXING_SERVICE_ALIAS_S SCHEMA_KEYS_VAL("service_alternate_name");
	SCHEMA_KEYS_PREFIX const char *INDEXING_DESCRIPTION_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "description");
	SCHEMA_KEYS_PREFIX const char *INDEXING_TYPE_S SCHEMA_KEYS_VAL("@type");
	SCHEMA_KEYS_PREFIX const char *INDEXING_TYPE_DESCRIPTION_S SCHEMA_KEYS_VAL("type_description");
	SCHEMA_KEYS_PREFIX const char *INDEXING_TYPE_SERVICE_GRASSROOTS_S SCHEMA_KEYS_VAL("Grassroots:Service");
	SCHEMA_KEYS_PREFIX const char *INDEXING_TYPE_DESCRIPTION_SERVICE_GRASSROOTS_S SCHEMA_KEYS_VAL("Service");
	SCHEMA_KEYS_PREFIX const char *INDEXING_PAYLOAD_DATA_S SCHEMA_KEYS_VAL("payload");
	SCHEMA_KEYS_PREFIX const char *INDEXING_PAYLOAD_URL_S SCHEMA_KEYS_VAL("internal_url");
	SCHEMA_KEYS_PREFIX const char *INDEXING_ID_S SCHEMA_KEYS_VAL("id");
	SCHEMA_KEYS_PREFIX const char *INDEXING_ICON_URI_S SCHEMA_KEYS_CONCAT_VAL(CONTEXT_PREFIX_SCHEMA_ORG_S, "image");



/**
 * Expand a context ontology term to its fully qualified version. For instance
 * "so:name"  will become "http://schema.org/name" since "so:" is a shortcut
 * for "http://schmema.org"
 *
 * @param term_s The term to expand.
 * @return The newly-allocated fully-qualified term url or <code>NULL</code>
 * upon error. This will need to be freed with FreeCopiedString() to avoid a memory leak
 */
GRASSROOTS_UTIL_API char *GetExpandedContextTerm (const char *term_s);

#ifdef __cplusplus
}
#endif


#endif /* SHARED_SRC_UTIL_INCLUDE_SCHEMA_KEYS_H_ */
