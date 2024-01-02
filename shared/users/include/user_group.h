/*
 * usergroup.h
 *
 *  Created on: 6 Dec 2023
 *      Author: billy
 */

#ifndef CORE_SHARED_UTIL_INCLUDE_USER_GROUP_H_
#define CORE_SHARED_UTIL_INCLUDE_USER_GROUP_H_

#include "jansson.h"

#include "typedefs.h"
#include "grassroots_users_library.h"
#include "linked_list.h"
#include "user_details.h"


/**
 * @brief  A datatype to store user credentials
 * @ingroup utility_group
 */
typedef struct UserGroup
{
	char *ug_name_s;
	LinkedList *ug_users_p;
} UserGroup;


typedef struct UserGroupNode
{
	ListItem ugn_node;
	UserGroup *ugn_group_p;
} UserGroupNode;


#ifndef DOXYGEN_SHOULD_SKIP_THIS


#ifdef ALLOCATE_USER_GROUP_TAGS
	#define USER_GROUP_PREFIX  GRASSROOTS_USERS_API
	#define USER_GROUP_VAL(x) = x
	#define USER_GROUP_CONCAT_VAL(x,y) = x y
#else
	#define USER_GROUP_PREFIX extern GRASSROOTS_USERS_API
	#define USER_GROUP_VAL(x)
	#define USER_GROUP_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


USER_GROUP_PREFIX const char * const UG_NAME_S USER_GROUP_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

USER_GROUP_PREFIX const char * const UG_USERS_S USER_GROUP_VAL ("users");




#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a UserGroup with the given name.
 *
 * @param name_s The name of the group.
 *
 * @return A newly-allocated UserGroup or <code>NULL</code> upon error.
 * @memberof UserGroup
 */
GRASSROOTS_USERS_API UserGroup *AllocateUserGroup (const char *name_s);


/**
 * Free a UserGroup.
 *
 * @param group_p The UserGroup to free.
 * @memberof UserGroup
 */
GRASSROOTS_USERS_API void FreeUserGroup (UserGroup *group_p);


/**
 * Get the json for a UserGroup.
 *
 * @param user_p The UserGroup to get the json for.
 * @return The json representation or <code>NULL</code> upon error.
 * @memberof UserGroup
 */
GRASSROOTS_USERS_API json_t *GetUserGroupAsJSON (UserGroup *group_p, const bool full_user_flag);



/**
 * Get a UserGroup from a json representation.
 *
 * @param group_json_p The The json representation.
 * @return The UserGroup or <code>NULL</code> upon error.
 * @memberof UserGroup
 */
GRASSROOTS_USERS_API UserGroup *GetUserGroupFromJSON (const json_t *group_json_p);


/**
 * Add a given User to a UserGroup.
 *
 * @param group_p The UserGroup to add the User to.
 * @param user_p The User to add.
 * @return <code>true</code> if the User was added successfully, <code>false</code> otherwise
 * @memberof UserGroup
 */
GRASSROOTS_USERS_API bool AddUserToGroup (UserGroup *group_p, User *user_p);


/**
 * Check whether a given User is a member of a UserGroup.
 *
 * @param group_p The UserGroup to check.
 * @param user_p The User to check for.
 * @return <code>true</code> if the User is a member of the UserGroup, <code>false</code> otherwise
 * @memberof UserGroup
 */
GRASSROOTS_USERS_API bool IsUserInGroup (const UserGroup * const group_p, const User * const user_p);


/**
 * Allocate a UserGroupNode for the given UserGroup.
 *
 * @param group_p The UserGroupNode that will store the UserGroup.
 *
 * @return A newly-allocated UserGroupNode or <code>NULL</code> upon error.
 * @memberof UserGroupNode
 */
GRASSROOTS_USERS_API UserGroupNode *AllocateUserGroupNode (UserGroup *group_p);


/**
 * Free a UserGroupNode.
 *
 * @param node_p The UserGroupNode to free.
 * @memberof UserGroupNode
 */
GRASSROOTS_USERS_API void FreeUserGroupNode (ListItem *node_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SHARED_UTIL_INCLUDE_USER_GROUP_H_ */
