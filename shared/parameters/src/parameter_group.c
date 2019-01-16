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

/*
 * parameter_group.c
 *
 *  Created on: 9 Feb 2016
 *      Author: billy
 */

#include <string.h>

#define ALLOCATE_PARAMETER_GROUP_TAGS_H
#include "parameter_group.h"
#include "parameter_set.h"
#include "string_utils.h"
#include "json_util.h"
#include "service.h"
#include "math_utils.h"


static const char S_REPEATABLE_GROUP_DELIMITER_PREFIX_S [] = "[";
static const char S_REPEATABLE_GROUP_DELIMITER_SUFFIX_S [] = "]";

static const char S_REPEATABLE_GROUP_DELIMITER_PREFIX_ESCAPED_REGEX_S [] = "\\[";
static const char S_REPEATABLE_GROUP_DELIMITER_SUFFIX_ESCAPED_REGEX_S [] = "\\]";

ParameterGroupNode *AllocateParameterGroupNode (ParameterGroup *group_p)
{
	ParameterGroupNode *param_group_node_p = (ParameterGroupNode *) AllocMemory (sizeof (ParameterGroupNode));

	if (param_group_node_p)
		{
			param_group_node_p -> pgn_param_group_p = group_p;
			param_group_node_p -> pgn_node.ln_prev_p = NULL;
			param_group_node_p -> pgn_node.ln_next_p = NULL;

			return param_group_node_p;
		}		/* if (param_group_node_p) */

	return NULL;
}



void FreeParameterGroupNode (ListItem *node_p)
{
	ParameterGroupNode *param_group_node_p = (ParameterGroupNode *) node_p;

	FreeParameterGroup (param_group_node_p -> pgn_param_group_p);
	FreeMemory (param_group_node_p);
}


ParameterGroup *AllocateParameterGroup (const char *name_s, const bool repeatable_flag, ServiceData *service_data_p)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			LinkedList *children_p = AllocateLinkedList (FreeParameterGroupNode);

			if (children_p)
				{
					/*
					 * The Parameters stored on this List are also known
					 * to the ParameterSet which is where they will get
					 * freed. So all we need to do on this LinkedList
					 * is free the memory allocated for the nodes.
					 */
					LinkedList *params_p = AllocateLinkedList (NULL);

					if (params_p)
						{
							ParameterGroup *param_group_p = (ParameterGroup *) AllocMemory (sizeof (ParameterGroup));

							if (param_group_p)
								{
									param_group_p -> pg_name_s = copied_name_s;
									param_group_p -> pg_params_p = params_p;
									param_group_p -> pg_visible_flag = true;

									param_group_p -> pg_full_display_flag = true;
									param_group_p -> pg_vertical_layout_flag = true;
									param_group_p -> pg_child_groups_p = children_p;
									param_group_p -> pg_repeatable_flag = repeatable_flag;
									param_group_p -> pg_current_repeatable_group_index = 0;
									param_group_p -> pg_service_data_p = service_data_p;

									if (service_data_p)
										{
											GetParameterGroupVisibility (service_data_p, name_s, & (param_group_p -> pg_visible_flag));
										}

									return param_group_p;
								}

							FreeLinkedList (params_p);
						}		/* if (params_p) */

					FreeLinkedList (children_p);
				}

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */

	return NULL;
}


void FreeParameterGroup (ParameterGroup *param_group_p)
{
	FreeCopiedString (param_group_p -> pg_name_s);

	FreeLinkedList (param_group_p -> pg_child_groups_p);

	FreeLinkedList (param_group_p -> pg_params_p);

	FreeMemory (param_group_p);
}


ParameterGroup *CreateAndAddParameterGroupToParameterSet (const char *name_s, const bool repeatable_flag, struct ServiceData *service_data_p, ParameterSet *param_set_p)
{
	ParameterGroup *group_p = AllocateParameterGroup (name_s, repeatable_flag, service_data_p);

	if (group_p)
		{
			if (!AddParameterGroupToParameterSet (param_set_p, group_p))
				{
					PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to add \"%s\" group to \"%s\" param set", name_s, param_set_p -> ps_name_s);
					FreeParameterGroup (group_p);
					group_p = NULL;
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Failed to create \"%s\" group", name_s);
		}

	return group_p;
}


bool AddParameterGroupAsJSON (ParameterGroup *param_group_p, json_t *groups_array_p)
{
	bool success_flag = false;
	json_t *group_json_p = GetParameterGroupAsJSON (param_group_p);

	if (group_json_p)
		{
			if (json_array_append_new (groups_array_p, group_json_p) == 0)
				{
					success_flag = true;

					if ((param_group_p -> pg_child_groups_p) && (param_group_p -> pg_child_groups_p -> ll_size > 0))
						{
							ParameterGroupNode *node_p = (ParameterGroupNode *) (param_group_p -> pg_child_groups_p -> ll_head_p);

							while (node_p && success_flag)
								{
									success_flag = AddParameterGroupAsJSON (node_p -> pgn_param_group_p, groups_array_p);
									node_p = (ParameterGroupNode *) (node_p -> pgn_node.ln_next_p);
								}
						}
				}
		}

	return success_flag;
}


json_t *GetParameterGroupAsJSON (ParameterGroup *param_group_p)
{
	json_error_t err;
	json_t *value_p = json_pack_ex (&err, 0, "{s:s,s:b,s:b}", PARAM_GROUP_NAME_S, param_group_p -> pg_name_s, PARAM_GROUP_VISIBLE_S, param_group_p -> pg_visible_flag, PARAM_GROUP_REPEATABLE_S, param_group_p -> pg_repeatable_flag);

	if (value_p)
		{
			bool success_flag = true;
			ParameterNode *node_p = (ParameterNode *) (param_group_p -> pg_params_p -> ll_head_p);

			if (node_p)
				{
					bool all_levels_same_flag = true;
					ParameterLevel l = node_p -> pn_parameter_p -> pa_level;

					node_p = (ParameterNode *) node_p -> pn_node.ln_next_p;

					while (node_p && all_levels_same_flag)
						{
							if (node_p -> pn_parameter_p -> pa_level == l)
								{
									node_p = (ParameterNode *) node_p -> pn_node.ln_next_p;
								}
							else
								{
									all_levels_same_flag = false;
								}
						}

					if (all_levels_same_flag)
						{
							const char *level_s = NULL;

							success_flag = false;

							switch (l)
								{
									case PL_SIMPLE:
										level_s = PARAM_LEVEL_TEXT_SIMPLE_S;
										break;

									case PL_ADVANCED:
										level_s = PARAM_LEVEL_TEXT_ADVANCED_S;
										break;

									case PL_ALL:
										level_s = PARAM_LEVEL_TEXT_ALL_S;
										break;

									default:
										PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Unknown ParameterLevel %d", l);
										break;
								}		/* switch (param_p -> pa_level) */

							if (level_s)
								{
									if (SetJSONString (value_p, PARAM_LEVEL_S, level_s))
										{
											success_flag = true;
										}
								}

						}		/* if (all_levels_same_flag) */

				}		/* if (node_p) */

			if (success_flag)
				{
					return value_p;
				}

			json_decref (value_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetParameterGroupAsJSON failed for \"%s\"", param_group_p -> pg_name_s);
		}

	return NULL;
}



bool AddParameterToParameterGroup (ParameterGroup *group_p, Parameter *param_p)
{
	bool success_flag = false;
	ParameterNode *node_p = AllocateParameterNode (param_p);

	if (node_p)
		{
			LinkedListAddTail (group_p -> pg_params_p, & (node_p -> pn_node));
			param_p -> pa_group_p = group_p;
			success_flag = true;
		}

	return success_flag;
}


bool AddParameterGroupChild (ParameterGroup *parent_group_p, ParameterGroup *child_group_p)
{
	bool success_flag = false;
	ParameterGroupNode *node_p = AllocateParameterGroupNode (child_group_p);

	if (node_p)
		{
			LinkedListAddTail (parent_group_p -> pg_child_groups_p, & (node_p -> pgn_node));
			success_flag = true;
		}

	return success_flag;
}


bool CloneParameters (const ParameterGroup * const src_group_p, ParameterGroup *dest_group_p)
{
	bool success_flag = true;
	const ParameterNode *src_node_p = (const ParameterNode *) (src_group_p -> pg_params_p -> ll_head_p);

	while (src_node_p)
		{
			Parameter *dest_param_p = CloneParameter (src_node_p -> pn_parameter_p);

			if (dest_param_p)
				{
					if (!AddParameterToParameterGroup (dest_group_p, dest_param_p))
						{
							success_flag = false;
						}
				}
			else
				{
					success_flag = false;

				}

			if (success_flag)
				{
					src_node_p = (const ParameterNode *) (src_node_p -> pn_node.ln_next_p);
				}

		}		/* while (src_node_p) */

	return success_flag;
}


ParameterGroup *CreateAndAddParameterGroupChild (ParameterGroup *parent_group_p, const char *name_s, const bool repeatable_flag, const bool add_params_flag)
{
	ParameterGroup *child_p = AllocateParameterGroup (name_s, repeatable_flag, parent_group_p -> pg_service_data_p);

	if (child_p)
		{
			bool success_flag = true;

			if (add_params_flag)
				{
					if (!CloneParameters (parent_group_p, child_p))
						{
							success_flag = false;
						}
				}

			if (success_flag)
				{
					if (AddParameterGroupChild (parent_group_p, child_p))
						{
							return child_p;
						}
				}

			FreeParameterGroup (child_p);
		}		/* if (child_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create child \"%s\" ParameterGroup", name_s);
		}

	return NULL;
}


bool RemoveParameterGroupChild (ParameterGroup *parent_group_p, ParameterGroup *child_group_p)
{
	bool success_flag = false;

	if (parent_group_p -> pg_child_groups_p)
		{
			ParameterGroupNode *node_p = (ParameterGroupNode *) (parent_group_p -> pg_child_groups_p -> ll_head_p);

			while (node_p)
				{
					if (node_p -> pgn_param_group_p == child_group_p)
						{
							LinkedListRemove (parent_group_p -> pg_child_groups_p, & (node_p -> pgn_node));
							success_flag = true;
						}
					else
						{
							node_p = (ParameterGroupNode *) (node_p -> pgn_node.ln_next_p);
						}

				}		/* while (node_p) */

		}		/* if (parent_group_p -> pg_child_groups_p) */

	return success_flag;
}




ParameterNode *GetParameterNodeFromParameterGroupByName (const ParameterGroup * const group_p, const char * const name_s)
{
	ParameterNode *node_p = (ParameterNode *) (group_p -> pg_params_p -> ll_head_p);

	while (node_p)
		{
			Parameter *param_p = node_p -> pn_parameter_p;

			if (strcmp (param_p -> pa_name_s, name_s) == 0)
				{
					return node_p;
				}
			else
				{
					node_p = (ParameterNode *) (node_p -> pn_node.ln_next_p);
				}
		}		/* while (node_p) */

	return NULL;
}


Parameter *GetParameterFromParameterGroupByName (const ParameterGroup * const group_p, const char * const name_s)
{
	ParameterNode *node_p = GetParameterNodeFromParameterGroupByName (group_p, name_s);

	if (node_p)
		{
			return node_p -> pn_parameter_p;
		}

	return NULL;
}


bool GetParameterValueFromParameterGroup (const ParameterGroup * const params_p, const char * const name_s, SharedType *value_p, const bool current_value_flag)
{
	bool success_flag = false;
	Parameter *param_p = GetParameterFromParameterGroupByName (params_p, name_s);

	if (param_p)
		{
			*value_p = current_value_flag ? param_p -> pa_current_value : param_p -> pa_default;
			success_flag = true;
		}

	return success_flag;
}



char *GetRepeatableParameterGroupRegularExpression (const ParameterGroup * const group_p)
{
	char *reg_ex_s = ConcatenateVarargsStrings (group_p -> pg_name_s, " ", S_REPEATABLE_GROUP_DELIMITER_PREFIX_ESCAPED_REGEX_S, ".*", S_REPEATABLE_GROUP_DELIMITER_SUFFIX_ESCAPED_REGEX_S, NULL);

	if (!reg_ex_s)
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Couldn't create the regular expression for parameter group \"%s\"", group_p -> pg_name_s);
		}

	return reg_ex_s;
}


char *GetRepeatableParameterGroupName (ParameterGroup * const group_p)
{
	char *value_s = NULL;
	char *index_s = ConvertIntegerToString (group_p -> pg_current_repeatable_group_index);

	if (index_s)
		{
			value_s = ConcatenateVarargsStrings (group_p ->  pg_name_s, " ", S_REPEATABLE_GROUP_DELIMITER_PREFIX_S, index_s, S_REPEATABLE_GROUP_DELIMITER_SUFFIX_S, NULL);

			if (value_s)
				{
					++ (group_p -> pg_current_repeatable_group_index);
				}

			FreeCopiedString (index_s);
		}

	return value_s;
}
