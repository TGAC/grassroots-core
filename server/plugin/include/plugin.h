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
/**@file plugin.h
*/

#ifndef PLUGIN_H
#define PLUGIN_H

#include "typedefs.h"
#include "grassroots_plugin_library.h"
#include "linked_list.h"
#include "memory_allocations.h"


#ifdef __cplusplus
	extern "C" {
#endif


/*********** FORWARD DECLARATION ****************/
struct ServicesArray;
struct Client;
struct Handler;
struct JobsManager;
struct GrassrootsServer;

/**
 * The current status of a Plugin
 *
 * @ingroup utility_group
 */
typedef enum PluginStatus
{
	/** No attempt to load a plugin of this type yet. */
	PS_UNSET,

	/** Plugin of this type has been loaded successfully */
	PS_LOADED,

	/** Plugin of this type failed to previously load */
	PS_UNAVAILABLE,

	/** Plugin is in the process of closing */
	PS_CLOSING

} PluginStatus;


/**
 * The type of module that the Plugin contains
 *
 * @ingroup utility_group
 */
typedef enum PluginNature
{
	/** Unknown type of Plugin */
	PN_UNKNOWN = -1,

	/** The Plugin creates a Service. */
	PN_SERVICE,

	/** The Plugin creates a Client. */
	PN_CLIENT,

	/** The Plugin creates a Handler. */
	PN_HANDLER,

	/** The Plugin creates a JobsManager. */
	PN_JOBS_MANAGER,

	/** The Plugin creates a ServersManager. */
	PN_SERVERS_MANAGER,

	/** The number of different PluginTypes */
	PN_NUM_TYPES
} PluginNature;


/**
 * A union datatype that abstracts the different types
 * of plugins that are avaialble.
 */
typedef union PluginValue
{
	/**
	 * If pl_type is PN_SERVICE then this will point
	 * to a ServicesArray of Services available.
	 */
	struct ServicesArray *pv_services_p;

	/**
	 * If pl_type is PN_CLIENT then this will point
	 * to a valid Client.
	 */
	struct Client *pv_client_p;

	/**
	 * If pl_type is PN_HANDLER then this will point
	 * to a valid Handler.
	 */
	struct Handler *pv_handler_p;


	/**
	 * If pl_type is PN_JOBS_MANAGER then this will point
	 * to a valid JobsManager.
	 */
	struct JobsManager *pv_jobs_manager_p;


	/**
	 * If pl_type is PN_SERVERS_MANAGER then this will point
	 * to a valid JobsManager.
	 */
	struct ServersManager *pv_servers_manager_p;

} PluginValue;




/**
 * @brief
 *
 * A Plugin is a dynamically-loaded component to provide
 * a piece of functionality. The plugin is stored in a
 * platform-specific container e.g. a .dll on Windows,
 * a .so on Unix, etc.
 *
 * @ingroup utility_group
 */
typedef struct Plugin
{
	/**
	 * The path of the plugin
	 */
	char *pl_path_s;

	/**
	 * Memory ownership of the path variable.
	 * @see pl_path_s
	 */
	MEM_FLAG pl_path_mem;

	/**
	 * A multi-platform name for the Plugin.
	 */
	char *pl_name_s;

	/**
	 * The current status of the Plugin.
	 */
	PluginStatus pl_status;

	/**
	 * The type of this Plugin e.g. Client, Service, etc.
	 */
	PluginNature pl_type;

	/**
	 * The data specific for the PluginNature of this Plugin.
	 */
	PluginValue pl_value;

	/**
	 * The count of how many current tasks have this Plugin open.
	 */
	int32 pl_open_count;


	struct GrassrootsServer *pl_server_p;
} Plugin;



/**
 * A ListNode for LinkedLists that also stores
 * a plugin value.
 *
 * @extends ListItem
 *
 * @ingroup utility_group
 */
typedef struct PluginListNode
{
	/** The ListNode. */
	ListItem pln_node;

	/** The plugin. */
	Plugin *pln_plugin_p;

	/** How the memory for the Plugin is stored and freed. */
	MEM_FLAG pln_plugin_flag;
} PluginListNode;


/***********************************/
/**********  API METHODS  **********/
/***********************************/



/**
 * Get the string with the platform-specific plugin parts
 * e.g. *.dll for windows, lib*.so for unix. for use in the
 * configuration hash table.
 *
 * @param plugin_p The Plugin to get the config string for.
 * @result A read-only string containing the config
 * string or NULL upon error.
 * @memberof Plugin
 */
GRASSROOTS_PLUGIN_API const char *GetPluginConfigName (const Plugin * const plugin_p);

/**
 * The following functions are platform-specific
 */

/**
 * @brief Allocate a new Plugin.
 *
 * @param path_s The path to the file where the Plugin will be loaded from.
 * @return A newly-allocated Plugin or <code>NULL</code> upon error.
 * @memberof Plugin
 * @see FreePlugin
 */
GRASSROOTS_PLUGIN_API Plugin *AllocatePlugin (const char * const path_s, struct GrassrootsServer *server_p);


/**
 * @brief Free a Plugin.
 *
 * @param plugin_p The plugin to free.
 * @memberof Plugin
 * @see FreePlugin
 */
GRASSROOTS_PLUGIN_API void FreePlugin (Plugin * const plugin_p);


/**
 * @brief Open a Plugin in preparation for getting the actual
 * component.
 *
 * This checks that the Plugin is a valid platfrom-specific
 * module.
 *
 * @param plugin_p The Plugin to open.
 * @return <code>true</code> if the Plugin was opened succesfully,
 * <code>false</code> otherwise.
 * @memberof Plugin
 * @see ClosePlugin
 */
GRASSROOTS_PLUGIN_API bool OpenPlugin (Plugin * const plugin_p);


/**
 * @brief Close a Plugin
 *
 * @param plugin_p The Plugin to close.
 * @return <code>true</code> if the Plugin was closed succesfully,
 * <code>false</code> otherwise.
 * @memberof Plugin
 * @see OpenPlugin
 */
GRASSROOTS_PLUGIN_API void ClosePlugin (Plugin * const plugin_p);


/**
 * @brief Get a symbol from a Plugin
 *
 * This is used to get a given function name from a Plugin.
 *
 * @param plugin_p The Plugin to get the symbol from.
 * @param symbol_s The symbol to get.
 * @return A pointer to the symbol's code or <code>NULL</code> upon error.
 * @memberof Plugin
 */
GRASSROOTS_PLUGIN_API void *GetSymbolFromPlugin (Plugin *plugin_p, const char * const symbol_s);


/**
 * Create a new PluginListNode.
 *
 * @param plugin_p The Plugin to store in the newly-created PluginListNode.
 * @return The new PluginListNode or <code>NULL</code> upon error.
 * @memberof PluginListNode
 */
GRASSROOTS_PLUGIN_API PluginListNode *AllocatePluginListNode (Plugin * const plugin_p);


/**
 * Free a PluginListNode.
 *
 * @param node_p The ListNode to free.
 * @memberof PluginListNode
 */
GRASSROOTS_PLUGIN_API void FreePluginListNode (ListItem * const node_p);


/**
 * @brief Get the platform-independent Plugin name from a platform-specific filename
 * possibly spanning multiple directories.
 *
 * For example, on Unix foo/bar/libmy_module.xso would return my_module
 *
 * @param full_plugin_path_s The name to use to get the platform-specific plugin name.
 * @return The platform-independent Plugin name which will be need to be freed with <code>
 * FreeCopiedString</code> when finished with to avoid a memory leak.
 * Upon error, this will return <code>NULL</code>
 * @memberof Plugin
 * @see FreeCopiedString
 */
GRASSROOTS_PLUGIN_API char *DeterminePluginName (const char * const full_plugin_path_s);


/**
 * @brief Make the platform-specific filename for a given Plugin name
 *
 * @param name_s The name to use to get the platform-specific plugin name.
 * @return The filename which will be need to be freed with <code>
 * FreeCopiedString</code> when finished with to avoid a memory leak.
 * Upon error, this will return <code>NULL</code>
 * @memberof Plugin
 * @see FreeCopiedString
 */
GRASSROOTS_PLUGIN_API char *MakePluginName (const char * const name_s);



/**
 * @brief Increment the number of objects that this Plugin has open.
 *
 * @param plugin_p The Plugin to alter.
 * @memberof Plugin
 */
GRASSROOTS_PLUGIN_API void IncrementPluginOpenCount (Plugin *plugin_p);


/**
 * @brief Decrement the number of objects that this Plugin has open.
 * If the open count falls to zero then the Plugin will be freed.
 *
 * @param plugin_p The Plugin to alter.
 * @memberof Plugin
 */
GRASSROOTS_PLUGIN_API void DecrementPluginOpenCount (Plugin *plugin_p);


/**
 * Clear all values in a given Plugin ready for it to be reused.
 *
 * @param plugin_p The Plugin to clear.
 * @memberof Plugin
 */
GRASSROOTS_PLUGIN_API void ClearPluginValue (Plugin * const plugin_p);


/***********************************/
/*********  LOCAL METHODS  *********/
/***********************************/


GRASSROOTS_PLUGIN_LOCAL bool InitBasePlugin (Plugin * const plugin_p, const char * const path_p, struct GrassrootsServer *server_p);

GRASSROOTS_PLUGIN_LOCAL void ClearBasePlugin (Plugin * const plugin_p);


GRASSROOTS_PLUGIN_LOCAL void ClearPluginPath (Plugin * const plugin_p);


#ifdef __cplusplus
}
#endif

#endif	/* #ifndef PLUGIN_H */
