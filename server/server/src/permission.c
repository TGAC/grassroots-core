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
#include "streams.h"

static const char * const S_ACCESS_RIGHTS_FULL = "full";
static const char * const S_ACCESS_RIGHTS_READ_WRITE = "read/write";
static const char * const S_ACCESS_RIGHTS_READ_ONLY = "read only";
static const char * const S_ACCESS_RIGHTS_NONE = "none";


static bool AddPermissionsJSONToGroupJSON (json_t *group_json_p, const Permissions *perms_p, const char * const key_s, const bool full_user_flag);



PermissionsManager *AllocatePermissionsManager (GrassrootsServer *grassroots_p, const char *database_s, const char *collection_s)
{
	char *copied_database_s = EasyCopyToNewString (database_s);

	if (copied_database_s)
		{
			char *copied_collection_s = EasyCopyToNewString (collection_s);

			if (copied_collection_s)
				{
					MongoTool *mongo_p = AllocateMongoTool (NULL, grassroots_p -> gs_mongo_manager_p);

					if (mongo_p)
						{
							PermissionsGroup *pg_p = AllocatePermissionsGroup ();

							if (pg_p)
								{
									PermissionsManager *manager_p = (PermissionsManager *) AllocMemory (sizeof (PermissionsManager));

									if (manager_p)
										{
											manager_p -> pm_collection_s = copied_collection_s;
											manager_p -> pm_database_s = copied_database_s;
											manager_p -> pm_mongo_p = mongo_p;
											manager_p -> pm_permissions_p = pg_p;

											return manager_p;
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for PermissionsManager");
										}

									FreePermissionsGroup (pg_p);
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocatePermissionsGroup () failed");
								}

							FreeMongoTool (mongo_p);
						}		/* if (mongo_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AllocateMongoTool () failed");
						}

					FreeCopiedString (copied_collection_s);
				}		/* if (copied_collection_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy collection name \"%s\"", collection_s);
				}

			FreeCopiedString (copied_database_s);
		}		/* if (copied_database_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy database name \"%s\"", database_s);
		}

	return NULL;
}


void FreePermissionsManager (PermissionsManager *manager_p)
{
	FreePermissionsGroup (manager_p -> pm_permissions_p);

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



json_t *GetPermissionsAsJSON (const Permissions *permissions_p, const bool full_flag)
{
	if (permissions_p -> pe_users_p -> ll_size > 0)
		{
			json_t *people_p = json_array ();

			if (people_p)
				{
					UserNode *node_p = (UserNode *) (permissions_p -> pe_users_p -> ll_head_p);
					bool success_flag = true;

					while (success_flag && node_p)
						{

						}

					if (success_flag)
						{
							return people_p;
						}

					json_decref (people_p);
				}
		}

	return NULL;
}


Permissions *GetPermissionsFromJSON (const json_t *json_p)
{
	return NULL;
}



json_t *GetPermissionsGroupAsJSON (const PermissionsGroup *permissions_group_p, const bool full_user_flag)
{
	json_t *group_json_p = json_object ();

	if (group_json_p)
		{
			if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_full_access_p, S_ACCESS_RIGHTS_FULL, full_user_flag))
				{
					if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_read_write_access_p, S_ACCESS_RIGHTS_READ_WRITE, full_user_flag))
						{
							if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_read_only_access_p, S_ACCESS_RIGHTS_READ_ONLY, full_user_flag))
								{
									if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_no_access_p, S_ACCESS_RIGHTS_NONE, full_user_flag))
										{
											return group_json_p;
										}		/* if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_no_access_p, S_ACCESS_RIGHTS_NONE)) */

								}		/* if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_read_only_access_p, S_ACCESS_RIGHTS_READ_ONLY)) */

						}		/* if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_read_write_access_p, S_ACCESS_RIGHTS_READ_WRITE)) */

				}		/* if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_full_access_p, S_ACCESS_RIGHTS_FULL)) */

			json_decref (group_json_p);
		}		/* if (group_json_p) */

	return NULL;
}


bool HasPermissionsSet (const Permissions * const permissions_p)
{
	return ((permissions_p -> pe_users_p -> ll_size > 0) || (permissions_p -> pe_groups_p -> ll_size > 0));
}


static bool AddPermissionsJSONToGroupJSON (json_t *group_json_p, const Permissions *perms_p, const char * const key_s, const bool full_user_flag)
{
	bool success_flag = false;

	if (HasPermissionsSet (perms_p))
		{
			json_t *perms_json_p = GetPermissionsAsJSON (perms_p, full_user_flag);

			if (perms_json_p)
				{
					if (json_object_set_new (group_json_p, key_s, perms_json_p) == 0)
						{
							success_flag = true;
						}
					else
						{
							json_decref (perms_json_p);
						}
				}
		}
	else
		{
			success_flag = true;
		}

	return success_flag;
}



PermissionsGroup *AllocatePermissionsGroup (void)
{
	Permissions *full_permissions_p = AllocatePermissions (AR_FULL);

	if (full_permissions_p)
		{
			Permissions *read_write_permissions_p = AllocatePermissions (AR_READ_WRITE);

			if (read_write_permissions_p)
				{
					Permissions *read_only_permissions_p = AllocatePermissions (AR_READ_ONLY);

					if (read_only_permissions_p)
						{
							Permissions *no_permissions_p = AllocatePermissions (AR_NONE);

							if (no_permissions_p)
								{
									PermissionsGroup *group_p = (PermissionsGroup *) AllocMemory (sizeof (PermissionsGroup));

									if (group_p)
										{
											group_p -> pg_full_access_p = full_permissions_p;
											group_p -> pg_read_write_access_p = read_write_permissions_p;
											group_p -> pg_read_only_access_p = read_only_permissions_p;
											group_p -> pg_no_access_p = no_permissions_p;

											return group_p;
										}
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate PermissionsGroup");
										}

									FreePermissions (no_permissions_p);
								}		/* if (no_permissions_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate none permissions");
								}

							FreePermissions (read_only_permissions_p);
						}		/* if (read_only_permissions_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate read only permissions");
						}

					FreePermissions (read_write_permissions_p);
				}		/* if (read_write_permissions_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate read/write permissions");
				}

			FreePermissions (full_permissions_p);
		}		/* if (full_permissions_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate full permissions");
		}

	return NULL;
}


void FreePermissionsGroup (PermissionsGroup *permissions_group_p)
{
	FreePermissions (permissions_group_p -> pg_full_access_p);
	FreePermissions (permissions_group_p -> pg_read_write_access_p);
	FreePermissions (permissions_group_p -> pg_read_only_access_p);
	FreePermissions (permissions_group_p -> pg_no_access_p);

	FreeMemory (permissions_group_p);
}




const char *GetAccessRightsAsString (const AccessRights ar)
{
	const char *res_s = NULL;

	switch (ar)
		{
			case AR_FULL:
				res_s = S_ACCESS_RIGHTS_FULL;
				break;

			case AR_READ_WRITE:
				res_s = S_ACCESS_RIGHTS_READ_WRITE;
				break;

			case AR_READ_ONLY:
				res_s = S_ACCESS_RIGHTS_READ_ONLY;
				break;

			case AR_NONE:
				res_s = S_ACCESS_RIGHTS_NONE;
				break;

			default:
				break;
		}

	return res_s;
}


bool SetAccessRightsFromString (AccessRights *ar_p, const char * const ar_s)
{
	bool success_flag = false;

	if (ar_s)
		{
			if (strcmp (ar_s, S_ACCESS_RIGHTS_FULL) == 0)
				{
					*ar_p = AR_FULL;
					success_flag = true;
				}
			else if (strcmp (ar_s, S_ACCESS_RIGHTS_READ_WRITE) == 0)
				{
					*ar_p = AR_READ_WRITE;
					success_flag = true;
				}
			else if (strcmp (ar_s, S_ACCESS_RIGHTS_READ_ONLY) == 0)
				{
					*ar_p = AR_READ_ONLY;
					success_flag = true;
				}
			else if (strcmp (ar_s, S_ACCESS_RIGHTS_NONE) == 0)
				{
					*ar_p = AR_NONE;
					success_flag = true;
				}
		}

	return success_flag;
}





AccessRights CheckPermissionsManagerForUser (const PermissionsManager * const permissions_manager_p, const User * const user_p)
{
	AccessRights rights = AR_FULL;

	if (permissions_manager_p)
		{
			PermissionsGroup *group_p = permissions_manager_p -> pm_permissions_p;

			if (!CheckPermissionsForUser (group_p -> pg_full_access_p, user_p, &rights))
				{
					if (!CheckPermissionsForUser (group_p -> pg_read_write_access_p, user_p, &rights))
						{
							if (!CheckPermissionsForUser (group_p -> pg_read_only_access_p, user_p, &rights))
								{
									CheckPermissionsForUser (group_p -> pg_no_access_p, user_p, &rights);
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

