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

#include "plugin.h"

#include "string_utils.h"
#include "streams.h"
#include "grassroots_server.h"


#ifdef _DEBUG
	#define PLUGIN_DEBUG	(STM_LEVEL_INFO)
#else
	#define PLUGIN_DEBUG	(STM_LEVEL_NONE)
#endif




bool InitBasePlugin (Plugin * const plugin_p, const char * const path_s, GrassrootsServer *server_p)
{
	bool success_flag = false;

	plugin_p -> pl_name_s = NULL;
	plugin_p -> pl_path_s = NULL;
	plugin_p -> pl_path_mem = MF_ALREADY_FREED;

	plugin_p -> pl_status = PS_UNSET;
	plugin_p -> pl_open_count = 0;

	plugin_p -> pl_server_p = server_p;

	ClearPluginValue (plugin_p);


	if (path_s)
		{
			plugin_p -> pl_path_s = CopyToNewString (path_s, 0, true);

			if (plugin_p -> pl_path_s)
				{
					plugin_p -> pl_path_mem = MF_DEEP_COPY;
					
					plugin_p -> pl_name_s = DeterminePluginName (path_s);
					
					if (plugin_p -> pl_name_s)
						{
							success_flag = true;
						}
				}
		}

	return success_flag;
}


void IncrementPluginOpenCount (Plugin *plugin_p)
{
	++ (plugin_p -> pl_open_count);
}


void DecrementPluginOpenCount (Plugin *plugin_p)
{
	#if PLUGIN_DEBUG >= STM_LEVEL_FINER
	PrintErrors (STM_LEVEL_FINER, __FILE__, __LINE__, "plugin %s open count " INT32_FMT, plugin_p -> pl_name_s, plugin_p -> pl_open_count);
	#endif

	if (plugin_p -> pl_open_count == 1)
		{
			FreePlugin (plugin_p);
		}
	else
		{
			-- (plugin_p -> pl_open_count);
		}
}


void ClearPluginPath (Plugin * const plugin_p)
{
	if (plugin_p -> pl_path_s)
		{
			switch (plugin_p -> pl_path_mem)
				{
					case MF_DEEP_COPY:
					case MF_SHALLOW_COPY:
						FreeMemory (plugin_p -> pl_path_s);
						break;

					case MF_SHADOW_USE:
					default:
						break;
				}

			plugin_p -> pl_path_s = NULL;
			plugin_p -> pl_path_mem = MF_ALREADY_FREED;
		}
}


void ClearBasePlugin (Plugin * const plugin_p)
{
	/* this must be before resetting plugin_p -> pl_type */
	//DeallocatePluginService (plugin_p);

	#if PLUGIN_DEBUG >= STM_LEVEL_FINER
	PrintErrors (STM_LEVEL_FINER, __FILE__, __LINE__, "ClearBasePlugin %s" , plugin_p -> pl_name_s);
	#endif

	if (plugin_p -> pl_name_s)
		{
			FreeCopiedString (plugin_p -> pl_name_s);
			plugin_p -> pl_name_s = NULL;
		}

	ClearPluginPath (plugin_p);
}


void ClearPluginValue (Plugin * const plugin_p)
{
	memset (& (plugin_p -> pl_value), 0, sizeof (PluginValue));

	plugin_p -> pl_type = PN_UNKNOWN;
}



void FreePluginListNode (ListItem * const node_p)
{
	PluginListNode *plugin_node_p = (PluginListNode *) node_p;
	Plugin *plugin_p = plugin_node_p -> pln_plugin_p;

	plugin_node_p -> pln_plugin_p = NULL;
	ClosePlugin (plugin_p);
	FreeMemory (plugin_node_p);
}


PluginListNode *AllocatePluginListNode (Plugin * const plugin_p)
{
	PluginListNode *node_p = (PluginListNode *) AllocMemory (sizeof (PluginListNode));

	if (node_p)
		{
			InitListItem (& (node_p -> pln_node));

			node_p -> pln_plugin_p = (Plugin *) plugin_p;
			node_p -> pln_plugin_flag = MF_SHALLOW_COPY;
		}

	return node_p;
}
