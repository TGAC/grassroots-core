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

#ifndef LUCENE_TOOL_H_
#define LUCENE_TOOL_H_

#include "uuid_defs.h"

#include "lucene_library.h"
#include "typedefs.h"
#include "string_hash_table.h"
#include "linked_list.h"
#include "lucene_document.h"
#include "grassroots_server.h"



/**
 * @struct LuceneTool
 * @brief A Tool for running Lucene jobs.
 *
 * @ingroup lucene_group
 */
typedef struct LuceneTool
{
	char *lt_name_s;

	const char *lt_index_class_s;

	const char *lt_search_class_s;

	const char *lt_classpath_s;

	const char *lt_index_s;

	const char *lt_taxonomy_s;

	const char *lt_working_directory_s;

	const char *lt_facet_key_s;

	char *lt_output_file_s;

	uint32 lt_num_total_hits;

	uint32 lt_hits_from_index;

	uint32 lt_hits_to_index;

	uuid_t lt_id;

	LinkedList *lt_facet_results_p;
} LuceneTool;



#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifdef ALLOCATE_LUCENE_TOOL_CONSTANTS
	#define LUCENE_TOOL_PREFIX GRASSROOTS_LUCENE_API
	#define LUCENE_TOOL_VAL(x)	= x
#else
	#define LUCENE_TOOL_PREFIX extern
	#define LUCENE_TOOL_VAL(x)
#endif

#endif 		/* #ifndef DOXYGEN_SHOULD_SKIP_THIS */




LUCENE_TOOL_PREFIX const char *LT_NUM_TOTAL_HITS_S LUCENE_TOOL_VAL ("total_hits");

LUCENE_TOOL_PREFIX const char *LT_HITS_START_INDEX_S LUCENE_TOOL_VAL ("from");

LUCENE_TOOL_PREFIX const char *LT_HITS_END_INDEX_S LUCENE_TOOL_VAL ("to");

LUCENE_TOOL_PREFIX const char *LT_FACETS_S LUCENE_TOOL_VAL ("facets");


/** The identifier used to uniquely specify a Lucene document.
 *
 * @memberof LuceneTool
 */
LUCENE_TOOL_PREFIX const char *LUCENE_ID_S LUCENE_TOOL_VAL("id");


#ifdef __cplusplus
extern "C"
{
#endif

/** @publicsection */



/**
 * Allocate a LuceneTool to run searches with.
 *
 * @return A newly-allocated LuceneTool or <code>NULL</code> upon error.
 * @memberof LuceneTool
 */
GRASSROOTS_LUCENE_API LuceneTool *AllocateLuceneTool (GrassrootsServer *grassroots_p, uuid_t id);


/**
 * Free a LuceneTool.
 *
 * @param tool_p The LuceneTool to free.
 * @memberof LuceneTool
 */
GRASSROOTS_LUCENE_API void FreeLuceneTool (LuceneTool *tool_p);


/**
 * Run a Lucene Search
 *
 * @param tool_p The LuceneTool to run.
 * @param query_s The query to run.
 * @param facets_p An optional list of KeyValuePairNodes for facets to run with. This can be
 * <code>NULL</code>
 * @return <code>true</code> if the LuceneTool ran successfully, <code>false</code> otherwise.
 * @memberof LuceneTool
 */
GRASSROOTS_LUCENE_API bool SearchLucene (LuceneTool *tool_p, const char *query_s, LinkedList *facets_p, const char *search_type_s, const uint32 page_index, const uint32 page_size);


/**
 * Index data in Lucene.
 *
 * @param tool_p The LuceneTool to index with.
 * @param data_p The data to index
 * @param update_flag If this is <code>true</code> then add to the current index.
 * If this <code>false</code>, then the index will be cleared before indexing the given data.
 * @return The OperationStatus from running the indexing.
 * @memberof LuceneTool
 */
GRASSROOTS_LUCENE_API OperationStatus IndexLucene (LuceneTool *tool_p, const json_t *data_p, bool update_flag);


/**
 * Delete data in Lucene.
 *
 * @param tool_p The LuceneTool to index with.
 * @param query_s The query to run for deleting matching documents
 * @return The OperationStatus from running the deletion.
 * @memberof LuceneTool
 */
GRASSROOTS_LUCENE_API OperationStatus DeleteLucene (LuceneTool *tool_p, const char *query_s);



GRASSROOTS_LUCENE_API OperationStatus ParseLuceneResults (LuceneTool *tool_p, const uint32 from, const uint32 to, bool (*lucene_results_callback_fn) (json_t *document_p, const uint32 index, void *data_p), void *data_p);

GRASSROOTS_LUCENE_API bool SetLuceneToolOutput (LuceneTool *tool_p, char *output_s);


GRASSROOTS_LUCENE_API bool SetLuceneToolName (LuceneTool *tool_p, const char *name_s);


GRASSROOTS_LUCENE_API bool AddLuceneFacetResultsToJSON (LuceneTool *tool_p, json_t *metadata_p);



GRASSROOTS_LUCENE_API bool AddFacetResultToLucene (LuceneTool *tool_p, const char *name_s, const uint32 count);



#ifdef __cplusplus
}
#endif

#endif /* LUCENE_TOOL_H_ */
