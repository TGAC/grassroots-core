/*
** Copyright 2014-2018 The Earlham Institute
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
 * lucene_document.c
 *
 *  Created on: 12 Feb 2019
 *      Author: billy
 */


#include "lucene_document.h"
#include "memory_allocations.h"
#include "json_util.h"


LuceneDocument *AllocateLuceneDocument (void)
{
	json_t *store_p = json_object ();

	if (store_p)
		{
			LuceneDocument *doc_p = (LuceneDocument *) AllocMemory (sizeof (LuceneDocument));

			if (doc_p)
				{
					doc_p -> ld_store_p = store_p;

					return doc_p;
				}

			json_decref (store_p);
		}

	return NULL;
}


void ClearLuceneDocument (LuceneDocument *doc_p)
{
	json_object_clear (doc_p -> ld_store_p);
}


void FreeLuceneDocument (LuceneDocument *doc_p)
{
	json_decref (doc_p -> ld_store_p);
	FreeMemory (doc_p);
}


bool AddFieldToLuceneDocument (LuceneDocument *doc_p, char *key_s, char *value_s)
{
	bool success_flag = true;
	json_t *field_p = json_object_get (doc_p -> ld_store_p, key_s);

	if (field_p)
		{
			if (json_is_array (field_p))
				{
					if (json_array_append_new (field_p, json_string (value_s)) == 0)
						{
							success_flag = true;
						}
				}
			else if (json_is_string (field_p))
				{
					/*
					 * As we now have multiple values, remove the string and replace it with an
					 * array of values
					 */
					const char *old_value_s = json_string_value (field_p);
					json_t *new_field_p = json_array ();

					if (new_field_p)
						{
							if (json_array_append (new_field_p, field_p) == 0)
								{
									if (json_array_append_new (new_field_p, json_string (value_s)) == 0)
										{
											if (json_object_set_new (doc_p -> ld_store_p, key_s, new_field_p) == 0)
												{
													success_flag = true;
												}
										}
								}

							if (!success_flag)
								{
									json_decref (new_field_p);
								}
						}

				}
		}
	else
		{
			 if (SetJSONString (doc_p -> ld_store_p, key_s, value_s))
				 {
					 success_flag = true;
				 }
		}

	return success_flag;
}


const char *GetDocumentFieldValue (const LuceneDocument *doc_p, const char *key_s)
{
	return GetJSONString (doc_p -> ld_store_p, key_s);
}


bool IsDocumentFieldMultiValued (const LuceneDocument *doc_p, const char *key_s, size_t *num_values_p)
{
	return (GetDocumentFieldSize (doc_p, key_s) > 1);
}


size_t GetDocumentFieldSize (const LuceneDocument *doc_p, const char *key_s)
{
	size_t res = 0;
	json_t *value_p = json_object_get (doc_p -> ld_store_p, key_s);

	if (value_p)
		{
			if (json_is_array (value_p))
				{
					res = json_array_size (value_p);
				}
			else
				{
					res = 1;
				}
		}

	return res;
}


const char *GetDocumentFieldMultiValue (const LuceneDocument *doc_p, const char *key_s, size_t index)
{
	const char *value_s;
	json_t *value_p = json_object_get (doc_p -> ld_store_p, key_s);

	if (value_p)
		{
			if (json_is_array (value_p))
				{
					size_t size = json_array_size (value_p);

					if (index < size)
						{
							json_t *indexed_value_p = json_array_get (value_p, index);

							if (json_is_string (indexed_value_p))
								{
									value_s = json_string_value (indexed_value_p);
								}
						}
				}
		}

	return value_s;
}
