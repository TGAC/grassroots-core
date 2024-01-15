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

#ifndef USER_DETAILS_H
#define USER_DETAILS_H

#include "bson.h"

#include "jansson.h"

#include "typedefs.h"
#include "grassroots_users_library.h"
#include "linked_list.h"
#include "schema_keys.h"
#include "view_format.h"


/**
 * @brief  A datatype to store user credentials
 * @ingroup utility_group
 */
typedef struct User
{
	bson_oid_t *us_id_p;

	/** The user's email address */
	char *us_email_s;

	/** The user's surname(s) */
	char *us_surname_s;

	/** The user's forename(s) */
	char *us_forename_s;

	/**
	 * The organisation that the user is associated with. This can be <code>NULL</code>
	 */
	char *us_org_s;

	/** The user's orcid */
	char *us_orcid_s;


} User;


typedef struct UserNode
{
	ListItem un_node;
	User *un_user_p;
} UserNode;



#ifndef DOXYGEN_SHOULD_SKIP_THIS


#ifdef ALLOCATE_USER_TAGS
	#define USER_PREFIX  GRASSROOTS_USERS_API
	#define USER_VAL(x) = x
	#define USER_CONCAT_VAL(x,y) = x y
#else
	#define USER_PREFIX extern GRASSROOTS_USERS_API
	#define USER_VAL(x)
	#define USER_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */


USER_PREFIX const char *US_FORENAME_S USER_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "givenName");

USER_PREFIX const char *US_SURNAME_S USER_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "familyName");

USER_PREFIX const char *US_EMAIL_S USER_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "email");

USER_PREFIX const char *US_AFFILATION_S USER_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "affiliation");

USER_PREFIX const char *US_ORCID_S USER_VAL ("orcid");



#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a User with the given details.
 *
 * @param email_s The email address of the user. This is required.
 * @param forename_s The first name of this user. This is required.
 * @param forename_s The last name of this user. This is required.
 * @param org_s The organisation that this User belongs to. This can be <code>NULL</code>.
 * @param orcid_s The ORCID for this User. This can be <code>NULL</code>.
 * .
 * @return A newly-allocated User or <code>NULL</code> upon error.
 * @memberof User
 */
GRASSROOTS_USERS_API User *AllocateUser (bson_oid_t *id_p, const char *email_s, const char *forename_s, const char *surname_s, const char *org_s, const char *orcid_s);


/**
 * Free a User.
 *
 * @param user_p The User to free.
 * @memberof User
 */
GRASSROOTS_USERS_API void FreeUser (User *user_p);


/**
 * Get the json for a User.
 *
 * @param user_p The User to get the json for.
 * @return The json representation or <code>NULL</code> upon error.
 * @memberof User
 */
GRASSROOTS_USERS_API json_t *GetUserAsJSON (const User *user_p, const ViewFormat vf);



/**
 * Get a User from a json representation.
 *
 * @param user_json_p The json representation.
 * @return The User or <code>NULL</code> upon error.
 * @memberof User
 */
GRASSROOTS_USERS_API User *GetUserFromJSON (const json_t *user_json_p);



/**
 * Get a User from a json representation which is the named child object of the given json.
 *
 * @param json_p The json object containing the user json representation as a child object.
 * @param user_key_s The key for the child object.
 * @return The User or <code>NULL</code> upon error.
 * @memberof User
 */
GRASSROOTS_USERS_API User *GetUserFromNamedJSON (const json_t *json_p, const char *user_key_s);



/**
 * Add a compound User json to another piece of json
 *
 * @param user_p The User to add the json for.
 * @param json_p The json object to add the user json to.
 * @param user_key_s The key to use to add the user's json value to json_p.
 * @return <code>true</code> if the User's json was added successfully, code>false</code> otherwise.
 * @memberof User
 */
GRASSROOTS_USERS_API bool AddUserToJSON (const User *user_p, json_t *json_p, const char * const user_key_s, const ViewFormat vf);




/**
 * Get the full name of a a User.
 *
 * @param user_p The User to get the name for
 * @return The newly-allocated string containing the User's name or <code>NULL</code> upon error.
 * This should be freed using FreeFullUsername()
 * @see FreeFullUsername
 * @memberof User
 */
GRASSROOTS_USERS_API char *GetFullUsername (const User *user_p);


/**
 * Free a username that was previously generated by a call to GetFullUsername().
 *
 * @param name_s The name to free
 * @see GetFullUsername
 * @memberof User
 */
GRASSROOTS_USERS_API void FreeFullUsername (char *name_s);


/**
 * Allocate a UserNode.
 *
 * @param user_p The User to store in the UserNode.
 * @return The UserNode or <code>NULL</code> upon error.
 * @memberof UserNode
 */
GRASSROOTS_USERS_API UserNode *AllocateUserNode (User *user_p);



/**
 * Free a UserNode and its associated User.
 *
 * @param user_p The UserNode to free.
 * @memberof UserNode
 */
GRASSROOTS_USERS_API void FreeUserNode (ListItem *node_p);



#ifdef __cplusplus
}
#endif


#endif		/* #ifndef USER_DETAILS_H */
