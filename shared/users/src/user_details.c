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

#include <string.h>

#define ALLOCATE_USER_TAGS (1)
#include "user_details.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "json_util.h"
#include "streams.h"
#include "mongodb_tool.h"


User *AllocateUser (bson_oid_t *id_p, const char *email_s, const char *forename_s, const char *surname_s, const char *org_s, const char *orcid_s)
{
	if (email_s)
		{
			char *copied_email_s = EasyCopyToNewString (email_s);

			if (copied_email_s)
				{
					char *copied_surname_s = NULL;

					if ((surname_s == NULL) || ((copied_surname_s = EasyCopyToNewString (surname_s)) != NULL))
						{
							char *copied_forename_s = NULL;

							if ((forename_s == NULL) || ((copied_forename_s = EasyCopyToNewString (forename_s)) != NULL))
								{
									char *copied_org_s = NULL;


									if ((org_s == NULL) || ((copied_org_s = EasyCopyToNewString (org_s)) != NULL))
										{
											char *copied_orcid_s = NULL;

											if ((orcid_s == NULL) || ((copied_orcid_s = EasyCopyToNewString (orcid_s)) != NULL))
												{
													User *user_p = (User *) AllocMemory (sizeof (User));

													if (user_p)
														{
															user_p -> us_id_p = id_p;
															user_p -> us_email_s = copied_email_s;
															user_p -> us_forename_s = copied_forename_s;
															user_p -> us_surname_s = copied_surname_s;
															user_p -> us_org_s = copied_org_s;
															user_p -> us_orcid_s = copied_orcid_s;

															return user_p;
														}

												}		/* if ((orcid_s == NULL) || ((copied_orcid_s = EasyCopyToNewString (orcid_s)) != NULL)) */
											else
												{
													PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy orcid \"%s\"", orcid_s);
												}

										}		/* if ((org_s == NULL) || ((copied_org_s = EasyCopyToNewString (org_s)) != NULL)) */
									else
										{
											PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy org \"%s\"", org_s);
										}

								}
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy org \"%s\"", org_s);
								}



						}		/* if ((username_s == NULL) || ((copied_username_s = EasyCopyToNewString (username_s)) != NULL)) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy surname \"%s\"", surname_s);
						}
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy email address \"%s\"", email_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "User's must have a valid email address");
		}

	return NULL;
}


void FreeUser (User *user_p)
{
	if (user_p -> us_email_s)
		{
			FreeCopiedString (user_p -> us_email_s);
		}

	if (user_p -> us_surname_s)
		{
			FreeCopiedString (user_p -> us_surname_s);
		}

	if (user_p -> us_forename_s)
		{
			FreeCopiedString (user_p -> us_forename_s);
		}


	if (user_p -> us_org_s)
		{
			FreeCopiedString (user_p -> us_org_s);
		}


	if (user_p -> us_orcid_s)
		{
			FreeCopiedString (user_p -> us_orcid_s);
		}

	FreeMemory (user_p);
}


json_t *GetUserAsJSON (const User *user_p, const bool full_flag)
{
	json_t *user_json_p = json_object ();

	if (user_json_p)
		{
			if ((! (user_p -> us_id_p)) || (AddCompoundIdToJSON (user_json_p, user_p -> us_id_p)))
				{
					bool success_flag = false;

					if (full_flag)
						{
							if (SetJSONString (user_json_p, US_EMAIL_S, user_p -> us_email_s))
								{
									if ((user_p -> us_forename_s == NULL) || (SetJSONString (user_json_p, US_FORENAME_S, user_p -> us_forename_s)))
										{
											if ((user_p -> us_surname_s == NULL) || (SetJSONString (user_json_p, US_SURNAME_S, user_p -> us_surname_s)))
												{
													if ((user_p -> us_org_s == NULL) || (SetJSONString (user_json_p, US_AFFILATION_S, user_p -> us_org_s)))
														{
															if ((user_p -> us_orcid_s == NULL) || (SetJSONString (user_json_p, US_ORCID_S, user_p -> us_orcid_s)))
																{
																	success_flag = true;
																}
															else
																{
																	PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "SetJSONString () failed for \"%s\": \"%s\"",
																										 US_ORCID_S, user_p -> us_orcid_s);
																}

														}
													else
														{
															PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "SetJSONString () failed for \"%s\": \"%s\"",
																								 US_AFFILATION_S, user_p -> us_org_s);
														}
												}
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "SetJSONString () failed for \"%s\": \"%s\"",
																						 US_SURNAME_S, user_p -> us_surname_s);
												}

										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "SetJSONString () failed for \"%s\": \"%s\"",
																				 US_FORENAME_S, user_p -> us_forename_s);
										}

								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "SetJSONString () failed for \"%s\": \"%s\"",
																		 US_EMAIL_S, user_p -> us_email_s);
								}

						}
					else
						{
							success_flag = true;
						}

					if (success_flag)
						{
							return user_json_p;
						}

				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "AddCompoundIdToJSON () failed for \"%s\"",
														 user_p -> us_email_s);
				}

			json_decref (user_json_p);
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to get json object");
		}

	return NULL;
}


User *GetUserFromNamedJSON (const json_t *json_p, const char *user_key_s)
{
	User *user_p = NULL;
	const json_t *user_json_p = json_object_get (json_p, user_key_s);

	if (user_json_p)
		{
			user_p = GetUserFromJSON (user_json_p);
		}

	return user_p;
}



User *GetUserFromJSON (const json_t *user_json_p)
{
	User *user_p = NULL;
	bson_oid_t *id_p = GetNewUnitialisedBSONOid ();

	if (id_p)
		{
			if (GetMongoIdFromJSON (user_json_p, id_p))
				{
					const char *email_s = GetJSONString (user_json_p, US_EMAIL_S);

					if (email_s)
						{
							const char *forename_s = GetJSONString (user_json_p, US_FORENAME_S);

							if (forename_s)
								{
									const char *surname_s = GetJSONString (user_json_p, US_SURNAME_S);

									if (surname_s)
										{
											const char *org_s = GetJSONString (user_json_p, US_AFFILATION_S);
											const char *orcid_s = GetJSONString (user_json_p, US_ORCID_S);

											user_p = AllocateUser (id_p, email_s, forename_s, surname_s, org_s, orcid_s);

											if (!user_p)
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "AllocateUser () failed");
												}

										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "No surname specified in user json");
										}

								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "No forename specified in user json");
								}

						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, user_json_p, "No email specified in user json");
						}

				}
		}


	return user_p;
}


bool AddUserToJSON (const User *user_p, json_t *json_p, const char * const user_key_s, const bool full_user_flag)
{
	json_t *user_json_p = GetUserAsJSON (user_p, full_user_flag);

	if (user_json_p)
		{
			if (json_object_set_new (json_p, user_key_s, user_json_p) == 0)
				{
					return true;
				}

			json_decref (user_json_p);
		}

	return false;
}



char *GetFullUsername (const User *user_p)
{
	char *name_s = NULL;

	if ((user_p -> us_forename_s) && (user_p -> us_forename_s))
		{
			name_s = ConcatenateVarargsStrings (user_p -> us_forename_s, " ", user_p -> us_surname_s, NULL);
		}

	return name_s;
}


void FreeFullUsername (char *name_s)
{
	FreeCopiedString (name_s);
}


UserNode *AllocateUserNode (User *user_p)
{
	UserNode *node_p = (UserNode *) AllocMemory (sizeof (UserNode));

	if (node_p)
		{
			InitListItem (& (node_p -> un_node));
			node_p -> un_user_p = user_p;

			return node_p;
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate memory for UserNode for \"%s\"", user_p -> us_email_s);
		}

	return NULL;
}


void FreeUserNode (ListItem *node_p)
{
	UserNode *user_node_p = (UserNode *) node_p;

	FreeUser (user_node_p -> un_user_p);
	FreeMemory (user_node_p);
}



