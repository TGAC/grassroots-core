/*
 * permission.h
 *
 *  Created on: 6 Dec 2023
 *      Author: billy
 */

#ifndef CORE_SHARED_UTIL_INCLUDE_PERMISSION_H_
#define CORE_SHARED_UTIL_INCLUDE_PERMISSION_H_

#include "typedefs.h"
#include "grassroots_util_library.h"
#include "linked_list.h"
#include "user_details.h"
#include "user_group.h"
#include "mongodb_tool.h"
#include "grassroots_server.h"




typedef enum
{
	AR_READ,
	AR_WRITE,
	AR_DELETE
} AccessRights;


typedef struct Permisssions
{
	AccessRights pe_access;

	LinkedList *pe_users_p;

	LinkedList *pe_groups_p;

} Permissions;

typedef struct PermissionsGroup
{
	Permissions *pg_read_access_p;

	Permissions *pg_write_access_p;

	Permissions *pg_delete_access_p;

} PermissionsGroup;


typedef struct PermissionsManager
{
	char *pm_database_s;

	char *pm_collection_s;

	MongoTool *pm_mongo_p;

	PermissionsGroup *pm_permissions_p;

} PermissionsManager;

#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_UTIL_API PermissionsManager *AllocatePermissionsManager (GrassrootsServer *grassroots_p, const char *database_s, const char *collection_s);


GRASSROOTS_UTIL_API void FreePermissionsManager (PermissionsManager *manager_p);


GRASSROOTS_UTIL_API Permissions *AllocatePermissions (AccessRights access);


GRASSROOTS_UTIL_API void FreePermissions (Permissions *permissions_p);


GRASSROOTS_UTIL_API PermissionsGroup *AllocatePermissionsGroup (void);


GRASSROOTS_UTIL_API void FreePermissionsGroup (PermissionsGroup *permissions_group_p);


GRASSROOTS_UTIL_API Permissions *GetPermissionsFromJSON (const json_t *json_p);


GRASSROOTS_UTIL_API json_t *GetPermissionsGroupAsJSON (const PermissionsGroup *permissions_group_p, const bool full_user_flag);


GRASSROOTS_UTIL_API json_t *GetPermissionsAsJSON (const Permissions *permissions_p, bool full_flag);


GRASSROOTS_UTIL_API Permissions *GetPermissionsFromJSON (const json_t *json_p);



GRASSROOTS_UTIL_API AccessRights CheckPermissionsManagerForUser (const PermissionsManager * const permissions_manager_p, const User * const user_p);


GRASSROOTS_UTIL_API bool CheckPermissionsForUser (const Permissions * const permissions_p, const User * const user_p, AccessRights *ar_p);


GRASSROOTS_UTIL_API bool HasPermissionsSet (const Permissions * const permissions_p);


#ifdef __cplusplus
}
#endif

#endif /* CORE_SHARED_UTIL_INCLUDE_PERMISSION_H_ */
