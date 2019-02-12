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
 */


#include "jansson.h"

#include "lucene_library.h"
#include "typedefs.h"




/**
 * @struct LuceneDocument
 * @brief A datatype for holding Lucene Documents
 *
 * @ingroup lucene_group
 */
typedef struct LuceneDocument
{
	json_t *ld_store_p;

} LuceneDocument;





#ifdef __cplusplus
extern "C"
{
#endif

/** @publicsection */



/**
 * Allocate a LuceneDocument.
 *
 * @return A newly-allocated LuceneDocument or <code>NULL</code> upon error.
 * @memberof LuceneDocument
 */
GRASSROOTS_LUCENE_API LuceneDocument *AllocateLuceneDocument (void);


/**
 * Free a LuceneDocument.
 *
 * @param doc_p The LuceneDocument to free.
 * @memberof LuceneDocument
 */
GRASSROOTS_LUCENE_API void FreeLuceneDocument (LuceneDocument *doc_p);


GRASSROOTS_LUCENE_API void ClearLuceneDocument (LuceneDocument *doc_p);


/**
 * Add a field to a LuceneDocument.
 *
 * @param doc_p The LuceneDocument to amend.
 * @memberof LuceneDocument
 */
GRASSROOTS_LUCENE_API bool AddFieldToLuceneDocument (LuceneDocument *doc_p, char *key_s, char *value_s);



GRASSROOTS_LUCENE_API size_t GetDocumentFieldSize (const LuceneDocument *doc_p, const char *key_s);


GRASSROOTS_LUCENE_API bool IsDocumentFieldMultiValued (const LuceneDocument *doc_p, const char *key_s, size_t *num_values_p);


GRASSROOTS_LUCENE_API const char *GetDocumentFieldValue (const LuceneDocument *doc_p, const char *key_s);


GRASSROOTS_LUCENE_API const char *GetDocumentFieldMultiValue (const LuceneDocument *doc_p, const char *key_s, size_t index);
