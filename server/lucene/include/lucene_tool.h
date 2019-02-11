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

#include "lucene_library.h"
#include "typedefs.h"
#include "linked_list.h"
#include "operation.h"
#include "json_util.h"
#include "uuid/uuid.h"
//#include "lucene.h"



/**
 * @struct LuceneTool
 * @brief A Tool for running Lucene jobs.
 *
 * @ingroup lucene_group
 */
typedef struct LuceneTool
{
	char *lt_environment_s;
} LuceneTool;



#ifdef __cplusplus
extern "C"
{
#endif

/** @publicsection */



/**
 * Allocate a LuceneTool to run the given program.
 *
 * @param program_name_s The program that this LuceneTool will run.
 * @param id The id to give to this LuceneTool.
 * @return A newly-allocated LuceneTool or <code>NULL</code> upon error.
 * @memberof LuceneTool
 */
GRASSROOTS_LUCENE_API LuceneTool *AllocateLuceneTool (const char *program_name_s, const uuid_t id);


/**
 * Free a LuceneTool.
 *
 * The LuceneTool will be cleared and then the memory for the tool will be freed.
 *
 * @param tool_p The LuceneTool to free.
 * @memberof LuceneTool
 * @see ClearLuceneTool
 */
GRASSROOTS_LUCENE_API void FreeLuceneTool (LuceneTool *tool_p);


/**
 * Run a LuceneTool.
 *
 * @param tool_p The LuceneTool to add the argument for.
 * @param async_flag If this is <code>true</code> then the method will return straight away and
 * the job will continue to run asynchrnously or in the background. If this is <code>false</code>
 * then this method will not return until the job has completed.
 * @param log_s Optional filename for where to store the id of the Lucene job that this LuceneTool runs.
 * If this is <code>NULL</code>, then the id will not be written to a file.
 * @return <code>true</code> if the job was started successfully, <code>false</code> otherwise. To get the
 * status of whether the job completed successfully, use <code>GetLuceneToolStatus</code> @see GetLuceneToolStatus.
 * @memberof LuceneTool
 */
GRASSROOTS_LUCENE_API bool RunLuceneTool (LuceneTool *tool_p, const bool async_flag, const char * const log_s);



#ifdef __cplusplus
}
#endif

#endif /* LUCENE_TOOL_H_ */
