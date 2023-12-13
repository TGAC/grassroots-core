/*
 * permission.c
 *
 *  Created on: 7 Dec 2023
 *      Author: billy
 */

#include "permission.h"
#include "string_utils.h"
#include "memory_allocations.h"
#include "user_group.h"

PermissionsManager *AllocatePermissionsManager (const char *database_s, const char *collection_s)
{
	PermissionsManager *manager_p = NULL;

	return manager_p;
}


void FreePermissionsManager (PermissionsManager *manager_p)
{
	FreePermissions (manager_p -> pm_full_access_p);
	FreePermissions (manager_p -> pm_read_write_access_p);
	FreePermissions (manager_p -> pm_read_only_access_p);
	FreePermissions (manager_p -> pm_no_access_p);

	FreeMongoTool (manager_p -> pm_mongo_p);
	FreeCopiedString (manager_p -> pm_database_s);
	FreeCopiedString (manager_p -> pm_collection_s);

	FreeMemory (manager_p);
}


Permissions *AllocatePermissions (AccessRights access)
{
	LinkedList *groups_p = AllocateLinkedList (FreeUserGroupNode);

	if (groups_p)
		{
			LinkedList *users_p = AllocateLinkedList (FreeUserNode);

			if (users_p)
				{
					Permissions *permissions_p = (Permissions *) AllocMemory (sizeof (Permissions));

					if (permissions_p)
						{
							permissions_p -> pe_access = access;
							permissions_p -> pe_groups_p = groups_p;
							permissions_p -> pe_users_p = users_p;

							return permissions_p;
						}

					FreeLinkedList (users_p);
				}

			FreeLinkedList (groups_p);
		}

	return NULL;
}


void FreePermissions (Permissions *permissions_p)
{
	FreeLinkedList (permissions_p -> pe_groups_p);
	FreeLinkedList (permissions_p -> pe_users_p);
	FreeMemory (permissions_p);
}






AccessRights CheckPermissionsManagerForUser (const PermissionsManager * const permissions_manager_p, const User * const user_p)
{
	AccessRights rights = AR_FULL;

	if (permissions_manager_p)
		{
			if (!CheckPermissionsForUser (permissions_manager_p -> pm_full_access_p, user_p, &rights))
				{
					if (!CheckPermissionsForUser (permissions_manager_p -> pm_read_write_access_p, user_p, &rights))
						{
							if (!CheckPermissionsForUser (permissions_manager_p -> pm_read_only_access_p, user_p, &rights))
								{
									CheckPermissionsForUser (permissions_manager_p -> pm_no_access_p, user_p, &rights);
								}

						}
				}
		}

	return rights;
}


bool CheckPermissionsForUser (const Permissions * const permissions_p, const User * const user_p, AccessRights *ar_p)
{
	if (permissions_p -> pe_users_p)
		{
			UserNode *user_node_p = (UserNode *) (permissions_p -> pe_users_p -> ll_head_p);

			while (user_node_p)
				{
					if (strcmp (user_p -> us_email_s, user_node_p -> un_user_p -> us_email_s) == 0)
						{
							*ar_p = permissions_p -> pe_access;
							return true;
						}
					else
						{
							user_node_p = (UserNode *) (user_node_p -> un_node.ln_next_p);
						}
				}
		}

	if (permissions_p -> pe_groups_p)
		{
			UserGroupNode *group_node_p = (UserGroupNode *) (permissions_p -> pe_groups_p -> ll_head_p);

			while (group_node_p)
				{
					if (IsUserInGroup (group_node_p -> ugn_group_p, user_p))
						{
							*ar_p = permissions_p -> pe_access;
							return true;
						}
					else
						{
							group_node_p = (UserGroupNode *) (group_node_p -> ugn_node.ln_next_p);
						}
				}
		}

	return false;
}

