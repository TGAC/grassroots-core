/*
 ** Copyright 2014-2017 The Earlham Institute
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
/*
 * schema_term.h
 *
 *  Created on: 14 Nov 2017
 *      Author: billy
 */

#ifndef CORE_SERVER_SERVICES_INCLUDE_SCHEMA_TERM_H_
#define CORE_SERVER_SERVICES_INCLUDE_SCHEMA_TERM_H_


#include "grassroots_service_library.h"
#include "linked_list.h"

#include "jansson.h"

/**
 * A datatype for specifying ontological terms
 * as a way to describe the data that each of the
 * Services consumes and produces. They are also used
 * to categorise each of the Services.
 */
typedef struct SchemaTerm
{
	/**
	 * The URL for this SchemaTerm.
	 */
	char *st_url_s;

	/**
	 * The user-friendly name for the term that should
	 * be displayed to the user.
	 */
	char *st_name_s;

	/**
	 * A more-verbose description of this SchemaTerm to
	 * display to the user.
	 */
	char *st_description_s;


	/**
	 * An optional abbreviation of this term
	 */
	char *st_abbreviation_s;

} SchemaTerm;





/**
 * A datatype for storing SchemaTerms in a LinkedList.
 */
typedef struct SchemaTermNode
{
	/**
	 * The base list node.
	 */
	ListItem stn_node;

	/**
	 * The SchemaTerm to store on the node.
	 */
	SchemaTerm *stn_term_p;
} SchemaTermNode;



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a SchemaTerm with an abbreviation.
 *
 * @param url_s The url for the SchemaTerm.
 * @param name_s The name to give to the SchemaTerm.
 * @param description_s The description to give to the SchemaTerm.
 * @param abbreviation_s The abbreviation to give to the SchemaTerm.
 * @return The new SchemaTerm or <code>NULL</code> upon error.
 * @memberof SchemaTerm
 */
GRASSROOTS_SERVICE_API SchemaTerm *AllocateExtendedSchemaTerm (const char *url_s, const char *name_s, const char *description_s, const char *abbreviation_s);


/**
 * Allocate a SchemaTerm.
 *
 * @param url_s The url for the SchemaTerm.
 * @param name_s The name to give to the SchemaTerm.
 * @param description_s The description to give to the SchemaTerm.
 * @return The new SchemaTerm or <code>NULL</code> upon error.
 * @memberof SchemaTerm
 */
GRASSROOTS_SERVICE_API SchemaTerm *AllocateSchemaTerm (const char *url_s, const char *name_s, const char *description_s);


GRASSROOTS_SERVICE_API bool SetSchemaTermValues (SchemaTerm *term_p, const char *url_s, const char *name_s, const char *description_s);


/**
 * Free a SchemaTerm.
 *
 * @param term_p The SchemaTerm to free.
 * @memberof SchemaTerm
 */
GRASSROOTS_SERVICE_API void FreeSchemaTerm (SchemaTerm *term_p);


/**
 * Clear all of the data within a SchemaTerm.
 *
 * @param term_p The SchemaTerm to clear.
 * @memberof SchemaTerm
 */
GRASSROOTS_SERVICE_API void ClearSchemaTerm (SchemaTerm *term_p);


/**
 * Allocate a SchemaTermNode.
 *
 * This will create a new SchemaTerm and store it on the
 * newly-created SchemaTermNode.
 *
 * @param url_s The url for the SchemaTerm.
 * @param name_s The name to give to the SchemaTerm.
 * @param description_s The description to give to the SchemaTerm.
 * @return The new SchemaTermNode or <code>NULL</code> upon error.
 * @memberof SchemaTermNode
 */
GRASSROOTS_SERVICE_API SchemaTermNode *AllocateSchemaTermNodeByParts (const char *url_s, const char *name_s, const char *description_s);


/**
 * Allocate a SchemaTermNode to store the given SchemaTerm.
 *
 * @param term_p The SchemaTerm.
 * @return The new SchemaTermNode or <code>NULL</code> upon error.
 * @memberof SchemaTermNode
 */
GRASSROOTS_SERVICE_API SchemaTermNode *AllocateSchemaTermNode (SchemaTerm *term_p);


/**
 * Free a SchemaTermNode.
 *
 * The associated SchemaTerm stored on the given SchemaTermNode will also be freed.
 *
 * @param node_p The SchemaTermNode to free.
 * @memberof SchemaTermNode
 */
GRASSROOTS_SERVICE_API void FreeSchemaTermNode (ListItem *node_p);


/**
 * Get the JSON fragment for a given SchemaTerm.
 *
 * @param term_p The SchemaTerm to get the JSON for.
 * @return The JSON fragment or <code>NULL</code> upon error.
 * @memberof SchemaTerm
 */
GRASSROOTS_SERVICE_API json_t *GetSchemaTermAsJSON (const SchemaTerm *term_p);


/**
 * Populate a JSON fragment with the details for a given SchemaTerm.
 *
 * @param term_p The SchemaTerm to get the JSON for.
 * @param The JSON fragment to populate.
 * @return <code>true</code> upon success, <code>false</code> upon failure.
 * @memberof SchemaTerm
 */
GRASSROOTS_SERVICE_API bool AddSchemaTermToJSON (const SchemaTerm *term_p, json_t *root_p);


/**
 * Get the JSON fragment for a given SchemaTerm.
 *
 * @param term_json_p The JSON fragment for a given SchemaTerm.
 * @return The new SchemaTerm or <code>NULL</code> upon error.
 * @memberof SchemaTerm
 */
GRASSROOTS_SERVICE_API SchemaTerm *GetSchemaTermFromJSON (const json_t *term_json_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_SERVICES_INCLUDE_SCHEMA_TERM_H_ */
