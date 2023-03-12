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
/**@file parameter.h
*/

#ifndef PARAMETER_H
#define PARAMETER_H

#include "uuid_defs.h"

#include "linked_list.h"
#include "grassroots_service_library.h"
#include "data_resource.h"
#include "hash_table.h"
#include "schema_version.h"
#include "parameter_type.h"
#include "remote_parameter_details.h"
#include "jansson.h"


/******* FORWARD DECLARATION *******/
struct Parameter;
struct ServiceData;
struct Service;

/**
 * The ParameterLevel defines the level that a user
 * should be to adjust the Parameter. The level of
 * Parameters that a user wishes to work with can
 * be adjusted within the user interface.
 *
 * @ingroup parameters_group
 */
typedef enum
{
	/**
	 * The value of a ParameterLevel for Parameters that
	 * are considered the most simple options.
	 */
	PL_SIMPLE,

	/**
	 * The value of a ParameterLevel for Parameters that
	 * are considered the options that require the most expertise.
	 */
	PL_ADVANCED,

	/**
	 * The value of a ParameterLevel for Parameters that
	 * are considered the options suitable for all levels
	 */
	PL_ALL
} ParameterLevel;


/**
 * This is a datatype that stores a read-only c-style string
 * along with a ParameterType.
 *
 * It's primary use is for error-checking of input parameters
 * within a Service as its parameters are being generated from
 * a json request object to make sure that the datatypes are
 * correct.
 *
 * @ingroup parameters_group
 */
typedef struct NamedParameterType
{
	/** The read-only name of the Parameter. */
	const char * const npt_name_s;

	/**
	 * The read-only ParameterType for the Parameter with the name
	 * given by npt_name_s.
	 */
	const ParameterType npt_type;
} NamedParameterType;


/**
 * A macro to provide a quick way to set the name and ParameterType for
 * NamedParameterType objects.
 *
 * @ingroup parameters_group
 */
#define SET_NAMED_PARAMETER_TYPE_TAGS(a,b) ({ .npt_name_s = a, .npt_type = b})

/* forward declaration */
typedef struct Parameter Parameter;



/**
 * @brief The datatype that stores all of the information about a Parameter.
 *
 * A Parameter stores all of the information required for a Service or Client to
 * set or get the information required for a settable option. This includes a name,
 * description, the datatype of this parameter and much more. The Parameter can also
 * hold arbitrary key-value pairs of char * for extra information if needed.
 *
 * @ingroup parameters_group
 */
struct Parameter
{
	/** The type of the parameter. */
	ParameterType pa_type;

	/** The name of the parameter. */
	char *pa_name_s;

	/** An optional user-friendly name of the parameter to use for client user interfaces. */
	char *pa_display_name_s;

	/** The description for this parameter. */
	char *pa_description_s;


	/**
	 * If the parameter can only take one of a
	 * constrained set of values, this will be
	 * a LinkedList of ParameterOptionNodes
	 * of the possible options. If it's
	 * NULL, then any value can be taken.
	 */
	LinkedList *pa_options_p;

	/**
	 * The level of the parameter.
	 */
	ParameterLevel pa_level;

	/**
	 * A map allowing the Parameter to store an arbitrary set of key-value
	 * pairs. Both the keys and values are char *.
	 */
	HashTable *pa_store_p;

	/**
	 * The ParameterGroup to which this Parameter belongs. If this
	 * Parameter is not in a ParameterGroup, then this will be
	 * <code>NULL</code>
	 */
	struct ParameterGroup *pa_group_p;


	/**
	 * A LinkedList of RemoteParameterNodes that hold the information
	 * for Parameters for PairedServices to the Service that owns
	 * this Parameter.
	 */
	LinkedList *pa_remote_parameter_details_p;


	/**
	 * Should this Parameter be displayed to the user or is it a
	 * hidden variable.
	 */
	bool pa_visible_flag;


	/**
	 *  When the value of this Parameter is changed in the client,
	 *  should it make a request to the server to refresh the
	 *  service definition?
	 *
	 *  This is ananlagous to the javascript onchange() event.
	 */
	bool pa_refresh_service_flag;


	/**
	 * Must this Parameter contain a valid value or can it be empty?
	 */
	bool pa_required_flag;


	void (*pa_clear_fn) (struct Parameter *param_p);


	bool (*pa_add_values_to_json_fn) (const struct Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);

	struct Parameter *(*pa_clone_fn) (const struct Parameter *param_p, const struct ServiceData *data_p);

	bool (*pa_set_value_from_string_fn) (struct Parameter *param_p, const char *value_s);


	/**
	 * Is the Parameter read-only?
	 */
	bool pa_read_only_flag;
};


/**
 * A datatype for storing Parameters in a
 * LinkedList.
 *
 * @extends ListItem
 * @ingroup parameters_group
 */
typedef struct ParameterNode
{
	/** The ListItem */
	ListItem pn_node;

	/** Pointer to the associated Parameter. */
	Parameter *pn_parameter_p;
} ParameterNode;



#ifndef DOXYGEN_SHOULD_SKIP_THIS


#ifdef ALLOCATE_PARAMETER_TAGS	
	#define PARAMETER_PREFIX  GRASSROOTS_SERVICE_API
	#define PARAMETER_VAL(x) = x
	#define PARAMETER_CONCAT_VAL(x,y) = x y
#else
	#define PARAMETER_PREFIX extern GRASSROOTS_SERVICE_API
	#define PARAMETER_VAL(x)
	#define PARAMETER_CONCAT_VAL(x,y)

#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


PARAMETER_PREFIX const char * const PA_TABLE_COLUMN_HEADERS_PLACEMENT_S PARAMETER_VAL("Column Headers Placement");

PARAMETER_PREFIX const char * const PA_TABLE_COLUMN_HEADINGS_S PARAMETER_VAL("Column Headings");

PARAMETER_PREFIX const char * const PA_TABLE_ROW_HEADINGS_S PARAMETER_VAL("Row Headings");

PARAMETER_PREFIX const char * const PA_TABLE_COLUMN_DELIMITER_S PARAMETER_VAL("Column Delimiter");

PARAMETER_PREFIX const char * const PA_TABLE_ADD_COLUMNS_FLAG_S PARAMETER_VAL("Add User-Definable Columns");

PARAMETER_PREFIX const char * const PA_TABLE_ROW_DELIMITER_S PARAMETER_VAL("Row Delimiter");

PARAMETER_PREFIX const char * const PA_TABLE_DEFAULT_COLUMN_DELIMITER_S PARAMETER_VAL(",");

PARAMETER_PREFIX const char * const PA_TABLE_DEFAULT_ROW_DELIMITER_S PARAMETER_VAL("\n");

PARAMETER_PREFIX const char * const PA_DOUBLE_PRECISION_S PARAMETER_VAL("precision");

PARAMETER_PREFIX const char * const PA_TYPE_STRING_ARRAY_S PARAMETER_VAL("string_array");

PARAMETER_PREFIX const char * const PA_TYPE_TIME_ARRAY_S PARAMETER_VAL("time_array");

PARAMETER_PREFIX const char * const PA_TYPE_STRING_S PARAMETER_VAL("string");

PARAMETER_PREFIX const char * const PA_TYPE_BOOLEAN_S PARAMETER_VAL("boolean");

PARAMETER_PREFIX const char * const PA_TYPE_INTEGER_S PARAMETER_VAL("integer");

PARAMETER_PREFIX const char * const PA_TYPE_NUMBER_S PARAMETER_VAL("number");

PARAMETER_PREFIX const char * const PA_TYPE_CHARACTER_S PARAMETER_VAL("character");


PARAMETER_PREFIX const char * const PA_TABLE_COLUMN_HEADERS_PLACEMENT_FIRST_ROW_S PARAMETER_VAL("Column Headers Placement On First Row");


#ifdef __cplusplus
	extern "C" {
#endif



/**
 * Allocate a Parameter
 *
 * @param service_data_p The ServiceData for the Service that is allocating this Parameter.
 * @param type The ParameterType for this Parameter.
 * @param name_s The name of the Parameter. The Parameter will store a copy of this string so this value does not need to remain in scope.
 * @param display_name_s An optional name to display for the Parameter for use in Clients. The Parameter will store a copy of this string so this value does not need to remain in scope.
 * This can be <code>NULL</code>.
 * @param description_s The description of the Parameter. The Parameter will store a copy of this string so this value does not need to remain in scope.
 * @param options_p This can be used to constrain the Parameter to a fixed set of values. If this is <code>NULL</code> then the Parameter can be set to any value.
 * @param default_value The default value for this Parameter.
 * @param current_value_p If this is not <code>NULL</code>, then copy this value as the current value of the Parameter. If this is <code>NULL</code>, then current value for this Parameter
 * will be set to be a copy of its default value.
 * @param bounds_p If this is not <code>NULL</code>, then this will be used to specify the minimum and maximum values that this Parameter can take. If this is <code>NULL</code>,
 * then the Parameter can take any value.
 * @param level The ParameterLevel for this Parameter. This determines when the Client should display this Parameter to the user.
 * @param check_value_fn If this is not <code>NULL</code>, then this will be used to check whether the Parameter has been set to a valid value.
 * @return A newly-allocated Parameter or <code>NULL</code> upon error.
 * @memberof Parameter
 */
//GRASSROOTS_PARAMS_API Parameter *AllocateParameter (const struct ServiceData *service_data_p, ParameterType type, const char * const name_s, const char * const display_name_s, const char * const description_s, LinkedList *options_p, SharedType default_value, SharedType *current_value_p, ParameterBounds *bounds_p, ParameterLevel level, const char *(*check_value_fn) (const Parameter * const parameter_p, const void *value_p));



GRASSROOTS_SERVICE_API bool InitParameter (Parameter *param_p, const struct ServiceData *service_data_p, ParameterType type, const char * const name_s,
																					const char * const display_name_s, const char * const description_s, ParameterLevel level,
																					void (*clear_fn) (Parameter *param_p),
																					bool (*add_values_to_json_fn) (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag),
																					struct Parameter *(*clone_fn) (const Parameter *param_p, const struct ServiceData *service_data_p),
																					bool (*set_value_from_string_fn) (struct Parameter *param_p, const char *value_s)
);


GRASSROOTS_SERVICE_API bool InitParameterFromJSON (Parameter *param_p, const json_t * const root_p, const struct Service *service_p, const bool concise_flag, const ParameterType *pt_p);


GRASSROOTS_SERVICE_API bool AddParameterValuesToJSON (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag);


/**
 * Clone a Parameter
 *
 * @param src_p The Parameter to clone.
 * @return A newly-allocated Parameter or <code>NULL</code> upon error.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API Parameter *CloneParameter (const Parameter * const src_p, const struct ServiceData *data_p);


/**
 * Free a Parameter.
 *
 * @param param_p The Parameter to free.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API void FreeParameter (Parameter *param_p);


/**
 * Clear a Parameter.
 *
 * @param param_p The Parameter to clear.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API void ClearParameter (Parameter *param_p);




/**
 * Test whether a given ParameterLevel matches or exceeds another.
 *
 * @param param_level The level to check.
 * @param threshold The level to check against.
 * @return <code>true</code> if the param_level matched or exceeded the threshold, <code>false</code> otherwise.
 */
GRASSROOTS_SERVICE_API bool CompareParameterLevels (const ParameterLevel param_level, const ParameterLevel threshold);



/**
 * Check whether the value of a Parameter is a non-negative real value.
 *
 * @param parameter_p The Parameter to check.
 * @param value_p The value to check.
 * @return An error message string on failure or <code>NULL</code> if the value
 * is a non-negative real number.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API const char *CheckForSignedReal (const Parameter * const parameter_p, const void *value_p);


/**
 * Check whether the value of a Parameter is not <code>NULL</code>.
 *
 * @param parameter_p The Parameter to check.
 * @param value_p The value to check.
 * @return An error message string on failure or <code>NULL</code> if the value
 * is a not <code>NULL</code>.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API const char *CheckForNotNull (const Parameter * const parameter_p, const void *value_p);


/**
 * Add a key value pair to a Parameter.
 *
 * @param parameter_p The Parameter to update.
 * @param key_s The key to add to the Parameter. A deep copy of this will be made by the
 * Parameter so the value passed in can go out of scope withut issues.
 * can go out of scope
 * @param value_s The value to add to the Parameter. A deep copy of this will be made by the
 * Parameter so the value passed in can go out of scope withut issues.
 * @return <code>true</code> if the Parameter was updated successfully, <code>false</code> otherwise.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API bool AddParameterKeyStringValuePair (Parameter * const parameter_p, const char *key_s, const char *value_s);


GRASSROOTS_SERVICE_API bool AddParameterKeyJSONValuePair (Parameter * const parameter_p, const char *key_s, const json_t *value_p);


GRASSROOTS_SERVICE_API bool AddColumnParameterHint (const char *name_s, const char *description_s, const ParameterType param_type, const bool required_flag, json_t *array_p);


/**
 * Remove a key value pair from a Parameter.
 *
 * @param parameter_p The Parameter to update.
 * @param key_s The key to remove from the Parameter. Its associated value will also be removed.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API void RemoveParameterKeyValuePair (Parameter * const parameter_p, const char *key_s);


/**
 * Get a value from a Parameter.
 *
 * @param parameter_p The Parameter to query.
 * @param key_s The key used to get the associated value from the Parameter.
 * @return The matching value or <code>NULL</code> if the key did not exist for the Parameter.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API const char *GetParameterKeyValue (const Parameter * const parameter_p, const char *key_s);


/**
 * Get the json-based representation of a Parameter.
 *
 * @param parameter_p The Parameter to get.
 * @param sv_p If you wish to create a JSON fragment for a different version of the Grassroots system,
 * then you can set this value to the version that you require. If this is <code>NULL</code>, then the
 * current version of the running Grassroots system will be used.
 * @param full_definition_flag If this is <code>true</code> then all of the details for this
 * Parameter will get added. If this is <code>false</code> then just the name and current value
 * will get added. This is useful is you just want to send the values to use when running a
 * service.
 * @return A newly-allocated json-based description of the Parameter or <code>NULL</code>
 * upon error. When you no longer require the value you need to call json_decref upon it.
 * @see GetRunnableParameterAsJSON
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API json_t *GetParameterAsJSON (const Parameter * const parameter_p, const SchemaVersion * const sv_p, const bool full_definition_flag);


/**
 * Create a Parameter from a json-based representation.
 *
 * @param json_p The json-based decription of the Parameter.
 * @param concise_flag If this is <code>true</code>, then just the Parameter
 * values that are needed to run the Service are added. If this is <code>false</code>
 * then user-facing attributes such as description, parameter level, group, etc. will
 * be added.
 * @return A newly-allocated Parameter derived from the json-based description
 * or <code>NULL</code> upon error.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API Parameter *CreateParameterFromJSON (const json_t * const root_p, struct Service *service_p, const bool concise_flag);



/**
 * Does the JSON fragment describe a full set of a Parameter's features or
 * just enough to get its current value.
 *
 * @param json_p The JSON fragment
 * @return <code>true</code> if the JSON fragment contains only enough data
 * to get the current value of the Parameter. If the fragment contains data such
 * as the display name, description, etc. then this will return <code>false</code>.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API bool IsJSONParameterConcise (const json_t * const json_p);



/**
 * Get the name to use for a Client to use for this Parameter.
 *
 * @param parameter_p The Parameter to get the value for.
 * @return The Parameter's display name if it is not <code>NULL</code>, else
 * the Parameter's name.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API const char *GetUIName (const Parameter * const parameter_p);


/**
 * Get the current value of a Parameter as a string.
 *
 * @param param_p The Parameter to get the current value for.
 * @param alloc_flag_p If the returned value had to be newly created, for example if
 * the type of this Parameter is a number, then this will be set to <code>true</code>
 * and the returned value will need to be freed using FreeCopiedString to avoid a memory
 * leak. If this is <code>false</code> then the returned value points directly to a string
 * within the Parameter's current value.
 * @return The Parameter value as a string or <code>NULL</code> if there was an error.
 * @see FreeCopiedString
 * @see SetParameterValueFromString
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API char *GetParameterValueAsString (const Parameter * const param_p, bool *alloc_flag_p);


/**
 * Set the current value of a Parameter from a string.
 *
 * @param param_p The Parameter to set the current value for.
 * @param value_s The Parameter value as a string.
 * @return <code>true</code> if the Paremeter value was set successfully, <code>false</code> otherwise.
 * @see GetParameterValueAsString
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API bool SetParameterValueFromString (Parameter * const param_p, const char *value_s);


/**
 * Create a RemoteParameterDetails and add it to the given Parameter.
 *
 *
 * @param param_p The Parameter to add the new RemoteParameterDetails to.
 * @param uri_s The URI of the ExternalServer that runs the PairedService that this RemoteParameter belongs to.
 * @param name_s The name of this Parameter on the PairedService.
 * @return <code>true</code> if the RemoteParameterDetails was added successfully or <code>false</code> otherwise.
 * @see AllocateRemoteParameterDetails
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API bool AddRemoteDetailsToParameter (Parameter *param_p, const char * const uri_s, const char * const name_s);


/**
 * Make a deep copy of all of the RemoteParameterDetails stored on one Parameter to another.
 *
 * @param src_param_p The Parameter to copy of all of the RemoteParameterDetails from.
 * @param dest_param_p The Parameter to copy of all of the RemoteParameterDetails to.
 * @return <code>true</code> if the RemoteParameterDetails was copied successfully or <code>false</code> otherwise.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API bool CopyRemoteParameterDetails (const Parameter * const src_param_p, Parameter *dest_param_p);


/**
 * Get the human-readable name for a given ParameterType.
 *
 * @param param_type The ParameterType to get the name for.
 * @return The human-readable name or <code>NULL</code> upon error.
 */
GRASSROOTS_SERVICE_API const char *GetGrassrootsTypeAsString (const ParameterType param_type);


/**
 * Get the ParameterType from its human-readable name.
 *
 * @param param_type_s The human-readable name of a ParameterType.
 * @param param_type_p Pointer to where the ParameterType will be set.
 * @return <code>true</code> if the ParameterType was set successfully, <code>false</code> otherwise.
 */
GRASSROOTS_SERVICE_API bool GetGrassrootsTypeFromString (const char *param_type_s, ParameterType *param_type_p);


/**
 * Get the configured visibility value for a given ParameterGroup.
 *
 * @param service_data_p The ServiceData for the Service that the given ParameterGroup belongs to.
 * @param group_name_s The name of the ParameterGroup to check.
 * @param visibility_p Pointer to where the value for configured visibility value for the given ParameterGroup
 * will be stored.
 * @return <code>true</code> if the visibility flag was set successfully, <code>false</code> otherwise.
 * @memberof ParameterGroup
 */
GRASSROOTS_SERVICE_API bool GetParameterGroupVisibility (const struct ServiceData *service_data_p, const char *group_name_s, bool *visibility_p);


/**
 * Get the configured description for a given Parameter.
 *
 * @param service_data_p The ServiceData for the Service that the given ParameterGroup belongs to.
 * @param param_name_s The name of the Parameter to check.
 * @param description_ss Pointer to where the value for configured default value for the given Parameter
 * will be stored.
 * @return <code>true</code> if the description was set successfully, <code>false</code> otherwise.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API bool GetParameterDescriptionFromConfig (const struct ServiceData *service_data_p, const char *param_name_s, char **description_ss);


/**
 * Get the configured display name for a given Parameter.
 *
 * @param service_data_p The ServiceData for the Service that the given ParameterGroup belongs to.
 * @param param_name_s The name of the Parameter to check.
 * @param display_name_ss Pointer to where the value for configured default value for the given Parameter
 * will be stored.
 * @return <code>true</code> if the description was set successfully, <code>false</code> otherwise.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API bool GetParameterDisplayNameFromConfig (const struct ServiceData *service_data_p, const char *param_name_s, char **display_name_ss);


/**
 * Get the configured ParameterLevel for a given Parameter.
 *
 * @param service_data_p The ServiceData for the Service that the given ParameterGroup belongs to.
 * @param param_name_s The name of the Parameter to check.
 * @param level_p Pointer to where the value for configured ParameterLevel of the given Parameter
 * will be stored.
 * @return <code>true</code> if the ParameterLevel was set successfully, <code>false</code> otherwise.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API bool GetParameterLevelFromConfig (const struct ServiceData *service_data_p, const char *param_name_s, ParameterLevel *level_p);


/**
 * Get the string representation of a given ParameterLevel.
 *
 * @param level The ParameterLevel to get the string representation for.
 * will be stored.
 * @return The string or <code>NULL</code> upon error.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API const char *GetParameterLevelAsString (const ParameterLevel level);



/**
 * Get the ParameterLevel from a string representation.
 *
 * @param level_s The string representation of the ParameterLevel
 * @param level The ParameterLevel which will be set.
 * @return code>true</code> if the ParameterLevel was set successfully, <code>false</code> otherwise.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API bool GetParameterLevelFromString (const char *level_s, ParameterLevel *level_p);


/**
 * Allocate a ParameterNode for a given Parameter so that it can be stored
 * upon a LinkedList.
 *
 * @param param_p The Parameter that the ParameterNode will reference.
 * @return the newly-allocated ParameterNode or <code>NULL</code> upon error.
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API ParameterNode *AllocateParameterNode (Parameter *param_p);


/**
 * Free a ParameterNode.
 *
 * @param node_p The ParameterNode to free.
 * @memberof Parameter,
 */
GRASSROOTS_SERVICE_API void FreeParameterNode (ListItem *node_p);


/**
 * Get the minimal json-based representation of a Parameter needed to run one or
 * more ServiceJobs. This is typically used when sending a request to a Server to
 * run a Service.
 *
 * @param name_s The name of the Parameter.
 * @param value_p The SharedValue containing the current value of the Parameter.
 * @param param_type The ParameterType of the Parameter.
 * @param sv_p If you wish to create a JSON fragment for a different version of the Grassroots system,
 * then you can set this value to the version that you require. If this is <code>NULL</code>, then the
 * current version of the running Grassroots system will be used.
 * @param full_definition_flag If this is <code>true</code> then all of the details for this
 * Parameter will get added. If this is <code>false</code> then just the name and current value
 * will get added. This is useful is you just want to send the values to use when running a
 * service.
 * @return A newly-allocated json-based description of the Parameter or <code>NULL</code>
 * upon error. When you no longer require the value you need to call json_decref upon it.
 * @see GetParameterAsJSON
 * @memberof Parameter
 */
GRASSROOTS_SERVICE_API json_t *GetRunnableParameterAsJSON (const Parameter *param_p, const SchemaVersion * const sv_p, const bool full_definition_flag);


GRASSROOTS_SERVICE_API bool AddParameterLevelToJSON (const ParameterLevel level, json_t *root_p, const SchemaVersion * const UNUSED_PARAM (sv_p));



GRASSROOTS_SERVICE_API void SetParameterCallbacks (Parameter *param_p, void (*clear_fn) (Parameter *param_p),
														bool (*add_values_to_json_fn) (const Parameter *param_p, json_t *param_json_p, const bool full_definition_flag),
														Parameter *(*clone_fn) (const Parameter *param_p, const struct ServiceData *data_p),
														bool (*set_value_from_string_fn) (Parameter *param_p, const char *value_s));


GRASSROOTS_SERVICE_API const json_t *GetParameterFromConfig (const json_t *service_config_p, const char * const param_name_s);



GRASSROOTS_SERVICE_LOCAL bool AddNullParameterValueToJSON (json_t *param_json_p, const char *key_s);



GRASSROOTS_SERVICE_API bool SetParameterCurrentValueFromJSON (Parameter *param_p, const json_t *value_p);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef PARAMETER_H */

