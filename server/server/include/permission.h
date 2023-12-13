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


typedef enum
{
	AR_NONE,
	AR_READ_ONLY,
	AR_READ_WRITE,
	AR_FULL
} AccessRights;


typedef struct Permisssions
{
	AccessRights pe_access;

	LinkedList *pe_users_p;

	LinkedList *pe_groups_p;

} Permissions;


typedef struct PermissionsManager
{
	char *pm_database_s;

	char *pm_collection_s;

	Permissions *pm_full_access_p;

	Permissions *pm_read_write_access_p;

	Permissions *pm_read_only_access_p;

	Permissions *pm_no_access_p;

	MongoTool *pm_mongo_p;

} PermissionsManager;

#ifdef __cplusplus
extern "C"
{
#endif


GRASSROOTS_UTIL_API PermissionsManager *AllocatePermissionsManager (const char *database_s, const char *collection_s);


GRASSROOTS_UTIL_API void FreePermissionsManager (PermissionsManager *manager_p);


GRASSROOTS_UTIL_API Permissions *AllocatePermissions (AccessRights access);


GRASSROOTS_UTIL_API void FreePermissions (Permissions *permissions_p);



GRASSROOTS_UTIL_API AccessRights CheckPermissionsManagerForUser (const PermissionsManager * const permissions_manager_p, const User * const user_p);


GRASSROOTS_UTIL_API bool CheckPermissionsForUser (const Permissions * const permissions_p, const User * const user_p, AccessRights *ar_p);



#ifdef __cplusplus
}
#endif

#endif /* CORE_SHARED_UTIL_INCLUDE_PERMISSION_H_ */
