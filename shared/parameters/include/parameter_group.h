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
 * parameter_group.h
 *
 *  Created on: 9 Feb 2016
 *      Author: billy
 */

#ifndef PARAMETERS_INCLUDE_PARAMETER_GROUP_H_
#define PARAMETERS_INCLUDE_PARAMETER_GROUP_H_

#include "jansson.h"
#include "linked_list.h"

#include "parameter.h"



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
 * json_util.c.
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_PARAMETER_GROUP_TAGS_H
	#define PARAMETER_GROUP_PREFIX GRASSROOTS_PARAMS_API
	#define PARAMETER_GROUP_VAL(x)	= x
#else
	#define PARAMETER_GROUP_PREFIX extern
	#define PARAMETER_GROUP_VAL(x)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


/*
PARAMETER_GROUP_PREFIX const char *PARAM_GROUP_DELIMITER_S PARAMETER_GROUP_VAL("/");
*/

/* forward declaration */
struct ServiceData;
struct ParameterGroup;
struct ParameterSet;

/**
 * A datatype to tell the system that certain
 * parameters should be grouped together in the
 * client's user interface if possible.
 *
 * @ingroup parameters_group
 */
typedef struct ParameterGroup
{
	/** The name of the ParameterGroup */
	char *pg_name_s;

	/** Should this ParameterGroup initially be visible? */
	bool pg_visible_flag;


	/**
	 * Should this ParameterGroup have a full display e.g. a titled
	 * legend or simply be a minimal grouping
	 */
	bool pg_full_display_flag;


	/**
	 * Should the ParameterGroup layout its child parameters
	 * horizontally or vertically?
	 */
	bool pg_vertical_layout_flag;


	/**
	 * Can the parameters in this group be repeated, analogous
	 * to a row in a table, <code>false</code> if not.
	 */
	bool pg_repeatable_flag;


	/**
	 * If the ParameterGroup is repeatable, use this named
	 * parameter as the label in the list of the parameters
	 * in this ParameterGroup.
	 */
	Parameter *pg_repeatable_param_p;

	/**
	 * If the parameters can be repeated, this is the current
	 * index e.g. row number in a table.
	 */
	uint32 pg_current_repeatable_group_index;


	/** The number of Parameters in this ParameterGroup */
	uint32 pg_num_params;

	/**
	 * A list of ParameterNodes for the parameters in this group
	 */
	LinkedList *pg_params_p;


	/**
	 * A list of ParmameterGroupNodes for any
	 * child parameter groups.
	 */
	LinkedList *pg_child_groups_p;


	/**
	 * The ServiceData for the Service that has created this ParametetrGroup
	 */
	const struct ServiceData *pg_service_data_p;

} ParameterGroup;



/**
 * A datatype for storing a ParameterGroup on a LinkedList.
 *
 * @extends ListItem
 * @ingroup parameters_group
 */
typedef struct ParameterGroupNode
{
	/** The base list node */
	ListItem pgn_node;

	/** Pointer to the associated ParameterGroup */
	ParameterGroup *pgn_param_group_p;
} ParameterGroupNode;




#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a ParameterGroupNode that will take ownership of
 * a given ParameterGroup.
 *
 * @param group_p The ParameterGroup that the ParameterGroupNode will reference.
 * @return The new ParameterGroupNode or <code>NULL</code> upon error.
 * @memberof ParameterGroupNode
 */
GRASSROOTS_PARAMS_API ParameterGroupNode *AllocateParameterGroupNode (ParameterGroup *group_p);


/**
 * Free a given ParameterGroupNode.
 *
 * This will also free the associated ParameterGroup.
 *
 * @param node_p The ParameterGroupNode to free.
 * @memberof ParameterGroupNode
 */
GRASSROOTS_PARAMS_API void FreeParameterGroupNode (ListItem *node_p);


/**
 * Allocate a ParameterGroup.
 *
 * @param name_s The name of the ParameterGroup that will be displayed to the user.
 * @param key_s An optional internal key to use by the owning Service. This can be <code>NULL</code>.
 * @param service_data_p The ServiceData for the Service that generates this ParameterGroup.
 * @param repeatable_flag <code>true</code> if the parameters in this group can be repeated, analogous
 * to a row in a table, <code>false</code> if not.
 * @return The new ParameterGroup or <code>NULL</code> upon error.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API ParameterGroup *AllocateParameterGroup (const char *name_s,  const bool repeatable_flag, const struct ServiceData *service_data_p);


/**
 * Free a given ParameterGroup.
 *
 * @param param_group_p The ParameterGroup to free.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API void FreeParameterGroup (ParameterGroup *param_group_p);


/**
 * Add the JSON fragment detailing a given ParameterGroup to a given JSON array.
 *
 * @param param_group_p The ParameterGroup to get the JSON for.
 * @param groups_array_p The JSON array to add the JSON representation of the ParameterGroup to.
 * @return <code>true</code> if the ParameterGroup was added successfully, <code>false</code> otherwise.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API bool AddParameterGroupAsJSON (ParameterGroup *param_group_p, json_t *groups_array_p, const SchemaVersion * const sv_p);


/**
 * Get the JSON fragment detailing a given ParameterGroup.
 *
 * @param param_group_p The ParameterGroup to get the JSON for.
 * @return The JSON fragment or <code>NULL</code> upon error.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API json_t *GetParameterGroupAsJSON (ParameterGroup *param_group_p, const bool include_params_flag, const bool full_definition_flag, const SchemaVersion * const sv_p);


/**
 * Add a ParameterGroup as a child to another ParameterGroup.
 *
 * @param parent_group_p The ParameterGroup to add the child ParameterGroup to.
 * @param child_group_p The ParameterGroup to add.
 * @return <code>true</code> if the ParameterGroup was added successfully, <code>false</code> otherwise.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API bool AddParameterGroupChild (ParameterGroup *parent_group_p, ParameterGroup *child_group_p);


/**
 * Create and add a ParameterGroup as a child to another ParameterGroup.
 *
 * @param parent_group_p The ParameterGroup to create and add the child ParameterGroup to.
 * @param name_s The name of the ParameterGroup that will be displayed to the user.
 * @param repeatable_flag <code>true</code> if the parameters in this group can be repeated, analogous
 * to a row in a table, <code>false</code> if not.
 * @param add_params_flag If this is <code>true</code> then all of the Parameters in parent_group_p will be
 * cloned into this newly-created ParameterGroup using ClonedParameters(). If this is <code>false</code>
 * then the Parameters will not be copied.
 * @return The child ParameterGroup or <code>NULL</code> if there was an error.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API ParameterGroup *CreateAndAddParameterGroupChild (ParameterGroup *parent_group_p, const char *name_s, const bool repeatable_flag, const bool add_params_flag);


/**
 * Remove a ParameterGroup child from another ParameterGroup.
 *
 * @param parent_group_p The ParameterGroup to remove the child ParameterGroup from.
 * @param child_group_p The ParameterGroup to remove.
 * @return <code>true</code> if the ParameterGroup was removed successfully, <code>false</code> otherwise.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API bool RemoveParameterGroupChild (ParameterGroup *parent_group_p, ParameterGroup *child_group_p);


/**
 * Clone all of the Parameters from one ParameterGroup to another.
 *
 * @param src_group_p The ParameterGroup to clone the Parameters from
 * @param dest_group_p The ParameterGroup to add the cloned Parameters to.
 * @return <code>true</code> if all of the Parameters were cloned successfully, <code>false</code> otherwise.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API bool CloneParameters (const ParameterGroup * const src_group_p, ParameterGroup *dest_group_p);


/**
 * Add a Parameter to a ParameterGroup.
 *
 * @param parent_group_p The ParameterGroup to add the Parameter to.
 * @param param_p The Parameter to add.
 * @return <code>true</code> if the Parameter was added successfully, <code>false</code> otherwise.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API bool AddParameterToParameterGroup (ParameterGroup *parent_group_p, Parameter *param_p);


/**
 * Create a ParameterGroup and add it to a given ParameterSet.
 *
 * @param name_s The name of the ParameterGroup that will be displayed to the user.
 * @param repeatable_flag <code>true</code> if the parameters in this group can be repeated, analogous
 * to a row in a table, <code>false</code> if not.
 * @param service_data_p The ServiceData for the Service that generates this ParameterGroup.
 * @param param_set_p The ParameterSet to add the ParameterGroup to.
 * @return The new ParameterGroup or <code>NULL</code> upon error.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API ParameterGroup *CreateAndAddParameterGroupToParameterSet (const char *name_s, const bool repeatable_flag, const struct ServiceData *service_data_p, struct ParameterSet *param_set_p);


/**
 * Get the ParameterNode for a Parameter with a given name from a ParameterGroup.
 *
 * @param group_p The ParameterGroup to search.
 * @param name_s The Parameter name to try and match.
 * @return  The ParameterNode with the matching Parameter name or <code>NULL</code> if it could not
 * be found
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API ParameterNode *GetParameterNodeFromParameterGroupByName (const ParameterGroup * const group_p, const char * const name_s);


/**
 * Get the Parameter with a given name from a ParameterGroup.
 *
 * @param group_p The ParameterGroup to search.
 * @param name_s The Parameter name to try and match.
 * @return  The Parameter with the matching name or <code>NULL</code> if it could not
 * be found
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API Parameter *GetParameterFromParameterGroupByName (const ParameterGroup * const group_p, const char * const name_s);


/**
 * Get the name to use for a child of a repeatable ParameterGroup.
 *
 * @param group_p The ParameterSet to get the name from.
 * @return  The newly-allocated name or <code>NULL</code> if there was an error.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API char *GetRepeatableParameterGroupName (ParameterGroup * const group_p);


/**
 * Get the string to use for matching ParameterGroups as children of
 * a repeatable ParameterGroup.
 *
 * @param group_p The ParameterSet to get the regular expresion string from.
 * @return  The newly-allocated regular expression string or <code>NULL</code> if there was an error.
 * This value will need to be freed using FreeCopiedString() to avoid a memory leak.
 * @memberof ParameterGroup
 */
GRASSROOTS_PARAMS_API char *GetRepeatableParameterGroupRegularExpression (const ParameterGroup * const group_p);


#ifdef __cplusplus
}
#endif


#endif /* PARAMETERS_INCLUDE_PARAMETER_GROUP_H_ */
