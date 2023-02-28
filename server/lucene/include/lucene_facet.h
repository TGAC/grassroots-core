/*
** Copyright 2014-2019 The Earlham Institute
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
 * lucene_facet.h
 *
 *  Created on: 31 Oct 2019
 *      Author: billy
 */

#ifndef CORE_SERVER_LUCENE_INCLUDE_LUCENE_FACET_H_
#define CORE_SERVER_LUCENE_INCLUDE_LUCENE_FACET_H_

#include "jansson.h"

#include "lucene_library.h"
#include "typedefs.h"
#include "schema_keys.h"
#include "linked_list.h"


typedef struct LuceneFacet
{
	char *lf_name_s;
	uint32 lf_count;
} LuceneFacet;


typedef struct LuceneFacetNode
{
	ListItem lfn_node;
	LuceneFacet *lfn_facet_p;
} LuceneFacetNode;




#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_LUCENE_FACET_CONSTANTS
	#define LUCENE_FACET_PREFIX GRASSROOTS_LUCENE_API
	#define LUCENE_FACET_VAL(x)	= x
	#define LUCENE_CONCAT_VAL(x,y) = x y
#else
	#define LUCENE_FACET_PREFIX GRASSROOTS_LUCENE_API
	#define LUCENE_FACET_VAL(x)
	#define LUCENE_CONCAT_VAL(x,y)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */




LUCENE_FACET_PREFIX const char *LF_NAME_S LUCENE_CONCAT_VAL (CONTEXT_PREFIX_SCHEMA_ORG_S, "name");

LUCENE_FACET_PREFIX const char *LF_COUNT_S LUCENE_FACET_VAL ("count");


#ifdef __cplusplus
extern "C"
{
#endif





/**
 * Allocate a LuceneFacet to store search result metadata
 *
 * @return A newly-allocated LuceneFacet or <code>NULL</code> upon error.
 * @memberof LuceneFacet
 */
GRASSROOTS_LUCENE_API LuceneFacet *AllocateLuceneFacet (const char *name_s, const uint32 count);


/**
 * Free a LuceneFacet.
 *
 * @param tool_p The LuceneFacet to free.
 * @memberof LuceneFacet
 */
GRASSROOTS_LUCENE_API void FreeLuceneFacet (LuceneFacet *facet_p);



/**
 * Allocate a LuceneFacetNode
 *
 * @return A newly-allocated LuceneFacetNode or <code>NULL</code> upon error.
 * @memberof LuceneFacet
 */
GRASSROOTS_LUCENE_API LuceneFacetNode *AllocateLuceneFacetNode (LuceneFacet *facet_p);



/**
 * Allocate a LuceneFacetNode
 *
 * @return A newly-allocated LuceneFacetNode or <code>NULL</code> upon error.
 * @memberof LuceneFacet
 */
GRASSROOTS_LUCENE_API LuceneFacetNode *AllocateLuceneFacetNodeByParts (const char *name_s, const uint32 count);


/**
 * Free a LuceneFacetNode.
 *
 * @param node_p The LuceneFacetNode to free.
 * @memberof LuceneFacet
 */
GRASSROOTS_LUCENE_API void FreeLuceneFacetNode (ListItem *node_p);


/**
 * Get the JSON representation of a LuceneFacet.
 *
 * @param facet_p The LuceneFacet to get the JSON representation of.
 * @return A newly-allocated json_t representing the LuceneFacet or <code>NULL</code> upon error.
 * @memberof LuceneFacet
 */
GRASSROOTS_LUCENE_API json_t *GetLuceneFacetAsJSON (const LuceneFacet *facet_p);



/**
 * Get a LuceneFacet from a JSON representation.
 *
 * @param json_t The json_t representing the LuceneFacet.
 * @return A newly-allocated LuceneFacet or <code>NULL</code> upon error.
 * @memberof LuceneFacet
 */
GRASSROOTS_LUCENE_API LuceneFacet *GetLuceneFacetFromResultsJSON (const json_t *json_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SERVER_LUCENE_INCLUDE_LUCENE_FACET_H_ */
