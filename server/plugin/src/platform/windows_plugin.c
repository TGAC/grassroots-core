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

//#include <winsock2.h>
#include <windows.h>

#include "streams.h"
#include "hash_table.h"
#include "memory_allocations.h"
#include "plugin.h"
#include "string_utils.h"
#include "filesystem_utils.h"
#include "grassroots_server.h"


typedef struct WindowsPlugin
{
	Plugin wp_plugin;
	HMODULE  wp_handle_p;
} WindowsPlugin;


/*
Plugin *CopyPlugin (const Plugin * const src_p)
{
	Plugin *dest_plugin_p = AllocatePlugin (src_p -> pl_name_s, src_p -> pl_path_s);

	if (dest_plugin_p)
		{
			CopyBasePlugin (src_p, dest_plugin_p);

			((WindowsPlugin *) dest_plugin_p) -> wp_handle_p = ((WindowsPlugin *) src_p) -> wp_handle;
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
			plugin_p -> wp_handle_p = NULL;
		}

	return ((Plugin *) plugin_p);
}


void ClosePlugin (Plugin * const plugin_p)
{
	WindowsPlugin *windows_plugin_p = (WindowsPlugin *) plugin_p;

//	RemovePluginFromPluginManager (plugin_p);
	ClearBasePlugin (plugin_p);

	if (windows_plugin_p -> wp_handle_p)
		{
			FreeLibrary (windows_plugin_p -> wp_handle_p);
			windows_plugin_p -> wp_handle_p = NULL;
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



	windows_plugin_p -> wp_handle_p = LoadLibraryA (plugin_p -> pl_path_s);

	if (windows_plugin_p -> wp_handle_p)
		{
			success_flag = true;
		}
	else
		{
			DWORD err_code = GetLastError ();
			LPTSTR buffer_s;

			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL, err_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &buffer_s, 0, NULL);

			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "LoadLibraryA () failed for \"%s\", error (%lu) \"%s\" ", plugin_p -> pl_path_s, err_code, buffer_s);


			LocalFree (buffer_s);

//			char s[_MAX_PATH];
//			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
//				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), s, 0, NULL);

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

char *DeterminePluginName (const char * const full_plugin_path_s)
{
	char *name_s = NULL;
	const char * const suffix_s = ".dll";
	const char *start_p = strrchr (full_plugin_path_s, GetFileSeparatorChar ());
	const char* end_p = NULL;

	if (!start_p)
		{
			start_p = full_plugin_path_s;
		}


	end_p = strstr (start_p, suffix_s);

	if (end_p)
		{
			size_t l = end_p - start_p;

			name_s = AllocMemory (l + 1);

			if (name_s)
				{
					strncpy (name_s, start_p, l);
					* (name_s + l) = '\0';
				}
		}

	return name_s;
}




void *GetSymbolFromPlugin (Plugin *plugin_p, const char * const symbol_s)
{
	void *symbol_p = NULL;
	WindowsPlugin *windows_plugin_p = (WindowsPlugin *) plugin_p;

	if (windows_plugin_p -> wp_handle_p)
		{
			symbol_p = GetProcAddress (windows_plugin_p -> wp_handle_p, symbol_s);
		}

	return symbol_p;
}
