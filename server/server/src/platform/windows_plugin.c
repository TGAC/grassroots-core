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

#include <windows.h>

#include "streams.h"
#include "hash_table.h"
#include "memory_allocations.h"
#include "plugin.h"
#include "string_utils.h"


typedef struct WindowsPlugin
{
	Plugin wp_plugin;
	HINSTANCE wp_handle;
} WindowsPlugin;


/*
Plugin *CopyPlugin (const Plugin * const src_p)
{
	Plugin *dest_plugin_p = AllocatePlugin (src_p -> pl_name_s, src_p -> pl_path_s);

	if (dest_plugin_p)
		{
			CopyBasePlugin (src_p, dest_plugin_p);

			((WindowsPlugin *) dest_plugin_p) -> wp_handle = ((WindowsPlugin *) src_p) -> wp_handle;
		}

	return dest_plugin_p;
}
*/


Plugin *AllocatePlugin (const char * const path_s, GrassrootsServer *server_p)
{
	WindowsPlugin *plugin_p = (WindowsPlugin *) AllocMemory (sizeof (WindowsPlugin));

	if (plugin_p)
		{
			InitBasePlugin ((Plugin *) plugin_p, path_s, server_p);
			plugin_p -> wp_handle = NULL;
		}

	return ((Plugin *) plugin_p);
}


void ClosePlugin (Plugin * const plugin_p)
{
	WindowsPlugin *windows_plugin_p = (WindowsPlugin *) plugin_p;

//	RemovePluginFromPluginManager (plugin_p);
	ClearBasePlugin (plugin_p);

	if (windows_plugin_p -> wp_handle)
		{
			FreeLibrary (windows_plugin_p -> wp_handle);
			windows_plugin_p -> wp_handle = NULL;
		}
}


void FreePlugin (Plugin * const plugin_p)
{
	ClosePlugin (plugin_p);
	FreeMemory ((WindowsPlugin *) plugin_p);
}



bool OpenPlugin (Plugin *const plugin_p)
{
	WindowsPlugin *windows_plugin_p = (WindowsPlugin *) plugin_p;
	bool success_flag = false;



	windows_plugin_p -> wp_handle = LoadLibraryA (plugin_p -> pl_path_s);

	if (windows_plugin_p->wp_handle) 
		{
			success_flag = true;   
		} 
	else 
		{
    }


	return success_flag;
}


char *MakePluginName (const char * const name)
{
  const char * const suffix_s = ".dll";
	const size_t l = strlen (name);
	char *c_p = (char *) AllocMemory ((l + strlen (suffix_s) + 1) * sizeof (char));

	if (c_p)
		{
			strcpy (c_p, name);
      strcpy (c_p + l, suffix_s);
		}

	return c_p;
}

