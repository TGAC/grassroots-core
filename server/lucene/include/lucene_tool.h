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

#include "uuid/uuid.h"

#include "lucene_library.h"
#include "typedefs.h"
#include "string_hash_table.h"
#include "linked_list.h"
#include "lucene_document.h"



/**
 * @struct LuceneTool
 * @brief A Tool for running Lucene jobs.
 *
 * @ingroup lucene_group
 */
typedef struct LuceneTool
{
	const char *lt_search_class_s;

	const char *lt_classpath_s;

	const char *lt_index_s;

	const char *lt_taxonomy_s;

	const char *lt_working_directory_s;

	char *lt_output_file_s;

	uuid_t lt_id;

} LuceneTool;





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
GRASSROOTS_LUCENE_API LuceneTool *AllocateLuceneTool (uuid_t id);


/**
 * Free a LuceneTool.
 *
 * @param tool_p The LuceneTool to free.
 * @memberof LuceneTool
 */
GRASSROOTS_LUCENE_API void FreeLuceneTool (LuceneTool *tool_p);


/**
 * Run a LuceneTool.
 *
 * @param tool_p The LuceneTool to run.
 * @param query_s The query to run.
 * @param facets_p An optional list of KeyValuePairNodes for facets to run with. This can be
 * <code>NULL</code>
 * @return <code>true</code> if the LuceneTool ran successfully, <code>false</code> otherwise.
 * @memberof LuceneTool
 */
GRASSROOTS_LUCENE_API bool RunLuceneTool (LuceneTool *tool_p, const char *query_s, LinkedList *facets_p, const char *search_type_s);



GRASSROOTS_LUCENE_API bool ParseLuceneResults (LuceneTool *tool_p, bool (*lucene_results_callback_fn) (LuceneDocument *document_p, const uint32 index, void *data_p), void *data_p);


GRASSROOTS_LUCENE_API void SetLuceneToolOutput (LuceneTool *tool_p, char *output_s);


#ifdef __cplusplus
}
#endif

#endif /* LUCENE_TOOL_H_ */
