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

#define ALLOCATE_KEY_VALUE_PAIR_TAGS (1)
#include "key_value_pair.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "streams.h"
#include "json_util.h"


KeyValuePair *AllocateKeyValuePair (const char *key_s, const char *value_s)
{
	char *copied_key_s = EasyCopyToNewString (key_s);

	if (copied_key_s)
		{
			char *copied_value_s = EasyCopyToNewString (value_s);

			if (copied_value_s)
				{
					KeyValuePair *kvp_p = (KeyValuePair *) AllocMemory (sizeof (KeyValuePair));

					if (kvp_p)
						{
							kvp_p -> kvp_key_s = copied_key_s;
							kvp_p -> kvp_value_s = copied_value_s;

							return kvp_p;
						}

					FreeCopiedString (copied_value_s);
				}		/* if (copied_value_s) */

			FreeCopiedString (copied_key_s);
		}		/* if (copied_key_s) */

	return NULL;
}


void FreeKeyValuePair (KeyValuePair *kvp_p)
{
	FreeCopiedString (kvp_p -> kvp_key_s);
	FreeCopiedString (kvp_p -> kvp_value_s);
	FreeMemory (kvp_p);
}



KeyValuePairNode *AllocateKeyValuePairNodeByParts (const char *key_s, const char *value_s)
{
	KeyValuePair *pair_p = AllocateKeyValuePair (key_s, value_s);

	if (pair_p)
		{
			KeyValuePairNode *node_p = AllocateKeyValuePairNode (pair_p);

			if (node_p)
				{
					return node_p;
				}

			FreeKeyValuePair (pair_p);
		}

	return NULL;
}


KeyValuePairNode *AllocateKeyValuePairNode (KeyValuePair *kvp_p)
{
	KeyValuePairNode *node_p = (KeyValuePairNode *) AllocMemory (sizeof (KeyValuePairNode));

	if (node_p)
		{
			InitListItem (& (node_p -> kvpn_node));
			node_p -> kvpn_pair_p = kvp_p;

			return node_p;
		}

	return NULL;
}


void FreeKeyValuePairNode (ListItem *node_p)
{
	KeyValuePairNode *kvp_node_p = (KeyValuePairNode *) node_p;

	FreeKeyValuePair (kvp_node_p -> kvpn_pair_p);
	FreeMemory (kvp_node_p);
}


json_t *GetKeyValuePairAsJSON (const KeyValuePair *kvp_p)
{
	json_t *kvp_json_p = json_object ();

	if (kvp_json_p)
		{
			if (SetJSONString (kvp_json_p, KVP_KEY_S, kvp_p -> kvp_key_s))
				{
					if (SetJSONString (kvp_json_p, KVP_VALUE_S, kvp_p -> kvp_value_s))
						{
							return kvp_json_p;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, kvp_json_p, "Failed to add \"%s\": \"%s\"",KVP_VALUE_S, kvp_p -> kvp_value_s);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, kvp_json_p, "Failed to add \"%s\": \"%s\"", KVP_KEY_S, kvp_p -> kvp_key_s);
				}

			json_decref (kvp_json_p);
		}		/* if (kvp_json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate JSON for KeyValuePair");
		}

	return NULL;
}


KeyValuePair *GetKeyValuePairFromJSON (const json_t *json_p)
{
	const char *key_s = GetJSONString (json_p, KVP_KEY_S);

	if (key_s)
		{
			const char *value_s = GetJSONString (json_p, KVP_VALUE_S);

			if (value_s)
				{
					KeyValuePair *kvp_p = AllocateKeyValuePair (key_s, value_s);

					if (kvp_p)
						{
							return kvp_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create KeyValuePair for \"%s\": \"%s\"", key_s, value_s);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to find \"%s\"", KVP_VALUE_S);
				}

		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to find \"%s\"", KVP_KEY_S);
		}

	return NULL;
}
