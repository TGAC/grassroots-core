/*
 * usergroup.c
 *
 *  Created on: 7 Dec 2023
 *      Author: billy
 */

#include <string.h>

#define ALLOCATE_USER_GROUP_TAGS (1)
#include "user_group.h"
#include "string_utils.h"
#include "memory_allocations.h"
#include "streams.h"
#include "json_util.h"


UserGroup *AllocateUserGroup (const char *name_s)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			LinkedList *users_p = AllocateLinkedList (FreeUserNode);

			if (users_p)
				{
					UserGroup *group_p = (UserGroup *) AllocMemory (sizeof (UserGroup));

					if (group_p)
						{
							group_p -> ug_name_s = copied_name_s;
							group_p -> ug_users_p = users_p;

							return group_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate User Group \"%s\"", name_s);
						}

					FreeLinkedList (users_p);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate users list for User Group \"%s\"", name_s);
				}

			FreeCopiedString (copied_name_s);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy User Group name \"%s\"", name_s);
		}

	return NULL;
}


void FreeUserGroup (UserGroup *group_p)
{
	FreeCopiedString (group_p -> ug_name_s);
	FreeLinkedList (group_p -> ug_users_p);

	FreeMemory (group_p);
}



bool AddUserToGroup (UserGroup *group_p, User *user_p)
{
	bool success_flag = false;
	UserNode *node_p = AllocateUserNode (user_p);

	if (user_p)
		{
			LinkedListAddTail (group_p -> ug_users_p, & (node_p -> un_node));
			success_flag = true;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate UsersNode for User \"%s\" to be added to Group \"%s\"",
									 user_p -> us_email_s, group_p -> ug_name_s);
		}

	return success_flag;
}



json_t *GetUserGroupAsJSON (UserGroup *group_p)
{
	json_t *res_p = json_object ();

	if (res_p)
		{
			bool success_flag = true;

			if (group_p -> ug_users_p -> ll_size > 0)
				{
					json_t *users_p = json_array ();

					if (users_p)
						{
							if (json_object_set_new (res_p, UG_USERS_S, users_p) == 0)
								{
									UserNode *node_p = (UserNode *) (group_p -> ug_users_p -> ll_head_p);

									while (node_p && success_flag)
										{
											const User *user_p = node_p -> un_user_p;
											json_t *user_json_p = GetUserAsJSON (user_p);

											if (user_json_p)
												{
													if (json_array_append_new (users_p, user_json_p) == 0)
														{
															node_p = (UserNode *) (node_p -> un_node.ln_next_p);
														}
													else
														{
															success_flag = false;
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "Failed to add user to UserGroup \"%s\"",
																								 group_p -> ug_name_s);

															json_decref (user_json_p);
														}
												}
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get user \"%s\" as json for UserGroup \"%s\"",
																			 user_p -> us_email_s, group_p -> ug_name_s);
													success_flag = false;
												}
										}		/* while (node_p && success_flag) */

								}		/* if (json_object_set_new (res_p, UG_USERS_S, users_p) == 0) */
							else
								{
									json_decref (users_p);
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "Failed to add users array to UserGroup \"%s\"",
																		 group_p -> ug_name_s);

								}

						}		/* if (users_p) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate json array for Users in User Group \"%s\"", group_p -> ug_name_s);
						}

				}		/* if (group_p -> ug_users_p -> ll_size > 0) */

			if (success_flag)
				{
					if (SetJSONString (res_p, UG_NAME_S, group_p -> ug_name_s))
						{
							return res_p;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, res_p, "SetJSONString failed for \"%s\": \"%s\"",
																 UG_NAME_S, group_p -> ug_name_s);
						}
				}

			json_decref (res_p);
		}		/* if (res_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate json object for User Group \"%s\"", group_p -> ug_name_s);
		}

	return NULL;
}



UserGroup *GetUserGroupFromJSON (const json_t *group_json_p)
{
	const char *name_s = GetJSONString (group_json_p, UG_NAME_S);

	if (name_s)
		{
			UserGroup *group_p = AllocateUserGroup (name_s);

			if (group_p)
				{
					bool success_flag = true;
					json_t *users_json_p = json_object_get (group_json_p, UG_USERS_S);

					if (users_json_p)
						{
							if (json_is_array (users_json_p))
								{
									const size_t num_users = json_array_size (users_json_p);
									size_t i = 0;

									while ((i < num_users) && success_flag)
										{
											json_t *entry_p = json_array_get (users_json_p, i);
											User *user_p = GetUserFromJSON (entry_p);

											if (user_p)
												{
													if (AddUserToGroup (group_p, user_p))
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
													success_flag = false;
												}

										}		/* while ((i < num_users) && success_flag) */

								}
							else
								{
									success_flag = false;
								}
						}

					if (success_flag)
						{
							return group_p;
						}

					FreeUserGroup (group_p);
				}		/* if (group_p) */


		}		/* if (name_s) */

	return NULL;
}


bool IsUserInGroup (const UserGroup * const group_p, const User * const user_p)
{
	UserNode *user_node_p = (UserNode *) (group_p -> ug_users_p -> ll_head_p);

	while (user_node_p)
		{
			if (strcmp (user_p -> us_email_s, user_node_p -> un_user_p -> us_email_s) == 0)
				{
					return true;
				}
			else
				{
					user_node_p = (UserNode *) (user_node_p -> un_node.ln_next_p);
				}
		}

	return false;
}



UserGroupNode *AllocateUserGroupNode (UserGroup *group_p)
{
	UserGroupNode *node_p = (UserGroupNode *) AllocMemory (sizeof (UserGroupNode));

	if (node_p)
		{
			InitListItem (& (node_p -> ugn_node));
			node_p -> ugn_group_p = group_p;

			return node_p;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for UserGroupNode for \"%s\"", group_p -> ug_name_s);
		}

	return NULL;
}



void FreeUserGroupNode (ListItem *node_p)
{
	UserGroupNode *ug_node_p = (UserGroupNode *) node_p;

	FreeUserGroup (ug_node_p -> ugn_group_p);
	FreeMemory (ug_node_p);
}


