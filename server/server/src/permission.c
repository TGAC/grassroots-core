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
#include "json_util.h"


static const char * const S_ACCESS_RIGHTS_DELETE_S = "delete";
static const char * const S_ACCESS_RIGHTS_WRITE_S = "write";
static const char * const S_ACCESS_RIGHTS_READ_S = "read";


static const char * const S_PERMISSIONS_USERS_S = "users";
static const char * const S_PERMISSIONS_GROUPS_S = "groups";
static const char * const S_PERMISSIONS_MODE_s = "access_mode";


static bool AddPermissionsJSONToGroupJSON (json_t *group_json_p, const Permissions *perms_p, const char * const key_s, const ViewFormat fmt);

static Permissions *GetPermissionsFromCompoundJSON (const json_t *permissions_group_json_p, const char * const key_s, const GrassrootsServer *grassroots_p);



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


Permissions *AllocatePermissions (AccessMode access)
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




json_t *GetPermissionsAsJSON (const Permissions *permissions_p, const ViewFormat vf)
{
	json_t *permissions_json_p = json_object ();
	bool success_flag = false;

	if (permissions_json_p)
		{
			const char *access_s = GetAccessRightsAsString (permissions_p -> pe_access);

			if (access_s)
				{
					if (SetJSONString (permissions_json_p, S_PERMISSIONS_MODE_s, access_s))
						{
							if (permissions_p -> pe_users_p -> ll_size > 0)
								{
									json_t *people_p = json_array ();

									if (people_p)
										{
											if (json_object_set_new (permissions_json_p, S_PERMISSIONS_USERS_S, people_p) == 0)
												{
													UserNode *node_p = (UserNode *) (permissions_p -> pe_users_p -> ll_head_p);
													success_flag = true;

													while (success_flag && node_p)
														{
															json_t *user_json_p = GetUserAsJSON (node_p -> un_user_p, vf);

															if (user_json_p)
																{
																	if (json_array_append_new (people_p, user_json_p) == 0)
																		{
																			node_p = (UserNode *) (node_p -> un_node.ln_next_p);
																		}
																	else
																		{
																			success_flag = false;
																			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "Failed to append entry to array");
																			json_decref (user_json_p);
																		}
																}
															else
																{
																	success_flag = false;
																	json_decref (user_json_p);
																}
														}
												}
											else
												{
													json_decref (people_p);
												}


										}
								}

						}

				}



			if (success_flag)
				{
					return permissions_json_p;
				}
		}		/* if (permissions_json_p) */
	else
		{

		}



	return NULL;
}



bool AddUserToPermissions (Permissions *permissions_p, User *user_p)
{
	bool success_flag = false;
	UserNode *node_p = AllocateUserNode (user_p);

	if (node_p)
		{
			LinkedListAddTail (permissions_p -> pe_users_p, & (node_p -> un_node));
			success_flag = true;
		}

	return success_flag;
}


bool AddGroupToPermissions (Permissions *permissions_p, UserGroup *group_p)
{
	bool success_flag = false;
	UserGroupNode *node_p = AllocateUserGroupNode (group_p);

	if (node_p)
		{
			LinkedListAddTail (permissions_p -> pe_groups_p, & (node_p -> ugn_node));
			success_flag = true;
		}

	return success_flag;
}


bool AddUserToGroupInPermissions (Permissions *permissions_p, const char * const group_s, User *user_p)
{
	bool success_flag = false;

	return success_flag;
}


void ClearPermissions (Permissions *permissions_p)
{
	ClearLinkedList (permissions_p -> pe_users_p);
	ClearLinkedList (permissions_p -> pe_groups_p);
}



PermissionsGroup *GetPermissionsGroupFromJSON (const json_t *permissions_group_json_p, const GrassrootsServer *grassroots_p)
{
	Permissions *read_perms_p = GetPermissionsFromCompoundJSON (permissions_group_json_p, S_ACCESS_RIGHTS_READ_S, grassroots_p);

	if (read_perms_p)
		{
			Permissions *write_perms_p = GetPermissionsFromCompoundJSON (permissions_group_json_p, S_ACCESS_RIGHTS_WRITE_S, grassroots_p);

			if (write_perms_p)
				{
					Permissions *delete_perms_p = GetPermissionsFromCompoundJSON (permissions_group_json_p, S_ACCESS_RIGHTS_DELETE_S, grassroots_p);

					if (delete_perms_p)
						{
							PermissionsGroup *perms_group_p = (PermissionsGroup *) AllocMemory (sizeof (PermissionsGroup));

							if (perms_group_p)
								{
									perms_group_p -> pg_read_access_p = read_perms_p;
									perms_group_p -> pg_write_access_p = write_perms_p;
									perms_group_p -> pg_delete_access_p = delete_perms_p;

									return perms_group_p;
								}
						}

				}

		}

	return NULL;
}



PermissionsGroup *GetPermissionsGroupFromChildJSON (const json_t *parent_json_p, const char * const key_s, const GrassrootsServer *grassroots_p)
{
	PermissionsGroup *perms_group_p = NULL;
	json_t *perms_group_json_p = json_object_get (parent_json_p, key_s);

	if (perms_group_json_p)
		{
			perms_group_p = GetPermissionsGroupFromJSON (perms_group_json_p, grassroots_p);
		}

	return perms_group_p;
}



Permissions *GetPermissionsFromJSON (const json_t *permissions_json_p, const GrassrootsServer *grassroots_p)
{
	const char *access_s = GetJSONString (permissions_json_p, S_PERMISSIONS_MODE_s);

	if (access_s)
		{
			AccessMode mode = AM_NONE;

			if (SetAccessModeFromString (&mode, access_s))
				{
					Permissions *permissions_p = AllocatePermissions (mode);

					if (permissions_p)
						{
							bool success_flag = true;
							const json_t *users_p = json_object_get (permissions_json_p, S_PERMISSIONS_USERS_S);

							if (users_p)
								{
									if (json_is_array (users_p))
										{
											size_t i = 0;
											const size_t num_entries = json_array_size (users_p);

											while ((i < num_entries) && success_flag)
												{
													const json_t *user_json_p = json_array_get (users_p, i);
													bson_oid_t id;

													if (GetMongoIdFromJSON (user_json_p, &id))
														{
															User *user_p = GetUserById (grassroots_p, &id);

															if (user_p)
																{
																	if (AddUserToPermissions (permissions_p, user_p))
																		{
																			++ i;
																		}
																	else
																		{
																			success_flag = false;
																			FreeUser (user_p);
																		}
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "GetUserById () failed ");
																	success_flag = false;
																}
														}
													else
														{
															success_flag = false;
														}

												}

										}
								}


							if (success_flag)
								{
									return permissions_p;
								}
						}




				}

		}


	return NULL;
}


bool AddPermissionsGroupToJSON (const PermissionsGroup *permissions_group_p, json_t *json_p, const char * const key_s, const ViewFormat vf)
{
	bool success_flag = false;
	json_t *perms_group_json_p = GetPermissionsGroupAsJSON (permissions_group_p, vf);

	if (perms_group_json_p)
		{
			if (json_object_set_new (json_p, key_s, perms_group_json_p) == 0)
				{
					success_flag = true;
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, perms_group_json_p, "Failed to add PermissionsGroup ...");
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "... to this json");

					json_decref (perms_group_json_p);
				}
		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "GetPermissionsGroupAsJSON () failed");
		}

	return success_flag;
}



json_t *GetPermissionsGroupAsJSON (const PermissionsGroup *permissions_group_p, const ViewFormat vf)
{
	json_t *group_json_p = json_object ();

	if (group_json_p)
		{
			if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_delete_access_p, S_ACCESS_RIGHTS_DELETE_S, vf))
				{
					if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_write_access_p, S_ACCESS_RIGHTS_WRITE_S, vf))
						{
							if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_read_access_p, S_ACCESS_RIGHTS_READ_S, vf))
								{
									return group_json_p;
								}		/* if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_read_access_p, S_ACCESS_RIGHTS_READ_S, vf)) */

						}		/* if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_read_access_p, S_ACCESS_RIGHTS_WRITE_S, vf)) */

				}		/* if (AddPermissionsJSONToGroupJSON (group_json_p, permissions_group_p -> pg_read_access_p, S_ACCESS_RIGHTS_DELETE_S, vf)) */

			json_decref (group_json_p);
		}		/* if (group_json_p) */

	return NULL;
}


bool HasPermissionsSet (const Permissions * const permissions_p)
{
	return ((permissions_p -> pe_users_p -> ll_size > 0) || (permissions_p -> pe_groups_p -> ll_size > 0));
}


static bool AddPermissionsJSONToGroupJSON (json_t *group_json_p, const Permissions *perms_p, const char * const key_s, const ViewFormat fmt)
{
	bool success_flag = false;

	if (HasPermissionsSet (perms_p))
		{
			json_t *perms_json_p = GetPermissionsAsJSON (perms_p, fmt);

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
	Permissions *delete_permissions_p = AllocatePermissions (AM_DELETE);

	if (delete_permissions_p)
		{
			Permissions *write_permissions_p = AllocatePermissions (AM_WRITE);

			if (write_permissions_p)
				{
					Permissions *read_only_permissions_p = AllocatePermissions (AM_READ);

					if (read_only_permissions_p)
						{
							PermissionsGroup *group_p = (PermissionsGroup *) AllocMemory (sizeof (PermissionsGroup));

							if (group_p)
								{
									group_p -> pg_delete_access_p = delete_permissions_p;
									group_p -> pg_write_access_p = write_permissions_p;
									group_p -> pg_read_access_p = read_only_permissions_p;

									return group_p;
								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate PermissionsGroup");
								}

							FreePermissions (read_only_permissions_p);
						}		/* if (read_only_permissions_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate read only permissions");
						}

					FreePermissions (write_permissions_p);
				}		/* if (read_write_permissions_p) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate read/write permissions");
				}

			FreePermissions (delete_permissions_p);
		}		/* if (delete_permissions_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate delete permissions");
		}

	return NULL;
}


void FreePermissionsGroup (PermissionsGroup *permissions_group_p)
{
	FreePermissions (permissions_group_p -> pg_delete_access_p);
	FreePermissions (permissions_group_p -> pg_write_access_p);
	FreePermissions (permissions_group_p -> pg_read_access_p);

	FreeMemory (permissions_group_p);
}




const char *GetAccessRightsAsString (const AccessMode ar)
{
	const char *res_s = NULL;

	switch (ar)
		{
			case AM_DELETE:
				res_s = S_ACCESS_RIGHTS_DELETE_S;
				break;

			case AM_WRITE:
				res_s = S_ACCESS_RIGHTS_WRITE_S;
				break;

			case AM_READ:
				res_s = S_ACCESS_RIGHTS_READ_S;
				break;

			default:
				break;
		}

	return res_s;
}


bool SetAccessModeFromString (AccessMode *ar_p, const char * const ar_s)
{
	bool success_flag = false;

	if (ar_s)
		{
			if (strcmp (ar_s, S_ACCESS_RIGHTS_DELETE_S) == 0)
				{
					*ar_p = AM_DELETE;
					success_flag = true;
				}
			else if (strcmp (ar_s, S_ACCESS_RIGHTS_WRITE_S) == 0)
				{
					*ar_p = AM_WRITE;
					success_flag = true;
				}
			else if (strcmp (ar_s, S_ACCESS_RIGHTS_READ_S) == 0)
				{
					*ar_p = AM_READ;
					success_flag = true;
				}
		}

	return success_flag;
}


bool CheckPermissionsGroupForUser (const PermissionsGroup * const permissions_group_p, const User * const user_p, const AccessMode mode)
{
	bool has_access_flag = false;

	if (permissions_group_p)
		{
			Permissions *permissions_p = NULL;

			switch (mode)
			{
				case AM_READ:
					permissions_p = permissions_group_p -> pg_read_access_p;
					break;

				case AM_WRITE:
					permissions_p = permissions_group_p -> pg_write_access_p;
					break;

				case AM_DELETE:
					permissions_p = permissions_group_p -> pg_delete_access_p;
					break;

				default:
					break;
			}

			if (permissions_p)
				{
					has_access_flag = CheckPermissionsForUser (permissions_p, user_p);
				}
		}
	else
		{
			has_access_flag = true;
		}

	return has_access_flag;
}




bool CheckPermissionsManagerForUser (const PermissionsManager * const permissions_manager_p, const User * const user_p, const AccessMode mode)
{
	bool has_access_flag = false;

	if (permissions_manager_p)
		{
			PermissionsGroup *permissions_group_p = permissions_manager_p -> pm_permissions_p;
			has_access_flag = CheckPermissionsGroupForUser (permissions_group_p, user_p, mode);

		}
	else
		{
			has_access_flag = true;
		}

	return has_access_flag;
}


bool CheckPermissionsForUser (const Permissions * const permissions_p, const User * const user_p)
{
	bool user_access = false;
	bool no_users_flag = true;
	bool no_groups_flag = true;

	if (permissions_p -> pe_users_p)
		{
			if (permissions_p -> pe_users_p -> ll_size > 0)
				{
					UserNode *user_node_p = (UserNode *) (permissions_p -> pe_users_p -> ll_head_p);

					no_users_flag = false;

					while (user_node_p && (!user_access))
						{
							if (strcmp (user_p -> us_email_s, user_node_p -> un_user_p -> us_email_s) == 0)
								{
									user_access = true;
								}
							else
								{
									user_node_p = (UserNode *) (user_node_p -> un_node.ln_next_p);
								}
						}
				}
		}

	if (!user_access)
		{
			if (permissions_p -> pe_groups_p)
				{
					if (permissions_p -> pe_groups_p -> ll_size > 0)
						{
							UserGroupNode *group_node_p = (UserGroupNode *) (permissions_p -> pe_groups_p -> ll_head_p);

							no_groups_flag = false;

							while (group_node_p && (!user_access))
								{
									if (IsUserInGroup (group_node_p -> ugn_group_p, user_p))
										{
											user_access = true;
										}
									else
										{
											group_node_p = (UserGroupNode *) (group_node_p -> ugn_node.ln_next_p);
										}
								}

						}

				}
		}

	if (!user_access)
		{
			if (no_users_flag && no_groups_flag)
				{
					user_access = true;
				}
		}

	return user_access;
}




static Permissions *GetPermissionsFromCompoundJSON (const json_t *permissions_group_json_p, const char * const key_s, const GrassrootsServer *grassroots_p)
{
	const json_t *permissions_json_p = json_object_get (permissions_group_json_p, key_s);

	if (permissions_json_p)
		{
			Permissions *perms_p = GetPermissionsFromJSON (permissions_json_p, grassroots_p);

			if (perms_p)
				{
					return perms_p;
				}
		}

	return NULL;
}
