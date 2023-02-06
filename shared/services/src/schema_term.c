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
 * schema_term.c
 *
 *  Created on: 14 Nov 2017
 *      Author: billy
 */

#include "../../../shared/services/include/schema_term.h"
#include "memory_allocations.h"
#include "string_utils.h"
#include "schema_keys.h"
#include "streams.h"
#include "json_util.h"


static bool AddTermToJSON (json_t *root_p, const char *root_key_s, const char *type_s, const char *key_s, const char *value_s);


SchemaTerm *AllocateExtendedSchemaTerm (const char *url_s, const char *name_s, const char *description_s, const char *abbreviation_s)
{
	SchemaTerm *term_p = AllocateSchemaTerm (url_s, name_s, description_s);

	if (term_p)
		{
			if (abbreviation_s)
				{
					term_p -> st_abbreviation_s = EasyCopyToNewString (abbreviation_s);

					if (! (term_p -> st_abbreviation_s))
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSchemaTermFromJSON failed failed to set abbreviation to \"%s\"", abbreviation_s);
							FreeSchemaTerm (term_p);
							term_p = NULL;
						}
				}
		}

	return term_p;
}



bool SetSchemaTermValues (SchemaTerm *term_p, const char *url_s, const char *name_s, const char *description_s)
{
	char *copied_url_s = EasyCopyToNewString (url_s);

	if (copied_url_s)
		{
			bool success_flag = true;
			char *copied_name_s = NULL;

			if (name_s)
				{
					copied_name_s = EasyCopyToNewString (name_s);
					success_flag = (copied_name_s != NULL);
				}


			if (success_flag)
				{
					char *copied_description_s = NULL;

					if (description_s)
						{
							copied_description_s = EasyCopyToNewString (description_s);
							success_flag = (copied_description_s != NULL);
						}


					if (success_flag)
						{
							term_p -> st_url_s = copied_url_s;
							term_p -> st_name_s = copied_name_s;
							term_p -> st_description_s = copied_description_s;
							term_p -> st_abbreviation_s = NULL;

							return true;
						}

					if (copied_description_s)
						{
							FreeCopiedString (copied_description_s);
						}
				}

			if (copied_name_s)
				{
					FreeCopiedString (copied_name_s);
				}

			FreeCopiedString (copied_url_s);
		}

	return false;
}


SchemaTerm *AllocateSchemaTerm (const char *url_s, const char *name_s, const char *description_s)
{
	SchemaTerm *term_p = (SchemaTerm *) AllocMemory (sizeof (SchemaTerm));

	if (term_p)
		{
			if (SetSchemaTermValues (term_p, url_s, name_s, description_s))
				{
					return term_p;
				}

			FreeMemory (term_p);
		}

	return NULL;
}


void ClearSchemaTerm (SchemaTerm *term_p)
{
	FreeCopiedString (term_p -> st_url_s);

	if (term_p -> st_name_s)
		{
			FreeCopiedString (term_p -> st_name_s);
		}

	if (term_p -> st_description_s)
		{
			FreeCopiedString (term_p -> st_description_s);
		}

	if (term_p -> st_abbreviation_s)
		{
			FreeCopiedString (term_p -> st_abbreviation_s);
		}

}

void FreeSchemaTerm (SchemaTerm *term_p)
{
	ClearSchemaTerm (term_p);

	FreeMemory (term_p);
}


SchemaTermNode *AllocateSchemaTermNodeByParts (const char *url_s, const char *name_s, const char *description_s)
{
	SchemaTerm *term_p = AllocateSchemaTerm (url_s, name_s, description_s);

	if (term_p)
		{
			SchemaTermNode *node_p = AllocateSchemaTermNode (term_p);

			if (node_p)
				{
					return node_p;
				}

			FreeSchemaTerm (term_p);
		}

	return NULL;
}


SchemaTermNode *AllocateSchemaTermNode (SchemaTerm *term_p)
{
	SchemaTermNode *node_p = (SchemaTermNode *) AllocMemory (sizeof (SchemaTermNode));

	if (node_p)
		{
			node_p -> stn_term_p = term_p;

			node_p -> stn_node.ln_prev_p = NULL;
			node_p -> stn_node.ln_next_p = NULL;
		}

	return node_p;
}


void FreeSchemaTermNode (ListItem *node_p)
{
	SchemaTermNode *term_node_p = (SchemaTermNode *) node_p;

	FreeSchemaTerm (term_node_p -> stn_term_p);
	FreeMemory (node_p);
}



bool AddSchemaTermToJSON (const SchemaTerm *term_p, json_t *root_p)
{
	bool success_flag = false;

	if (SetJSONString (root_p, SCHEMA_TERM_URL_S, term_p -> st_url_s))
		{
			if (SetJSONString (root_p, SCHEMA_TERM_NAME_S, term_p -> st_name_s))
				{
					if ((IsStringEmpty (term_p -> st_description_s)) || (SetJSONString (root_p, SCHEMA_TERM_DESCRIPTION_S, term_p -> st_description_s)))
						{
							if ((IsStringEmpty (term_p -> st_abbreviation_s)) || (SetJSONString (root_p, SCHEMA_TERM_ABBREVIATION_S, term_p -> st_abbreviation_s)))
								{
									success_flag = true;
								}
						}
				}
		}

	return success_flag;
}


json_t *GetSchemaTermAsJSON (const SchemaTerm *term_p)
{
	json_t *root_p = json_object ();

	if (root_p)
		{
			if (AddSchemaTermToJSON (term_p, root_p))
				{
					return root_p;
				}

			json_decref (root_p);
		}		/* if (root_p) */

	return NULL;
}


bool PopulateSchemaTermFromJSON (const json_t *term_json_p, SchemaTerm *term_p)
{
	const char *url_s = GetJSONString (term_json_p, SCHEMA_TERM_URL_S);
	const char *name_s = GetJSONString (term_json_p, SCHEMA_TERM_NAME_S);
	const char *description_s = GetJSONString (term_json_p, SCHEMA_TERM_DESCRIPTION_S);

	return SetSchemaTermValues (term_p, url_s, name_s, description_s);
}



SchemaTerm *GetSchemaTermFromJSON (const json_t *term_json_p)
{
	const char *url_s = GetJSONString (term_json_p, SCHEMA_TERM_URL_S);

	if (url_s)
		{
			const char *name_s = GetJSONString (term_json_p, SCHEMA_TERM_NAME_S);

			if (name_s)
				{
					const char *description_s = GetJSONString (term_json_p, SCHEMA_TERM_DESCRIPTION_S);
					SchemaTerm *term_p = AllocateSchemaTerm (url_s, name_s, description_s);

					if (term_p)
						{
							const char *abbreviation_s = GetJSONString (term_json_p, SCHEMA_TERM_ABBREVIATION_S);

							if (abbreviation_s)
								{
									term_p -> st_abbreviation_s = EasyCopyToNewString (abbreviation_s);

									if (!term_p -> st_abbreviation_s)
										{
											PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "GetSchemaTermFromJSON failed failed to set abbreviation to \"%s\"", abbreviation_s);
										}
								}

							return term_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "GetSchemaTermFromJSON failed for url \"%s\", name \"%s\", description \"%s\"", url_s, name_s, description_s);
						}

				}		/* if (name_s) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, term_json_p, "GetSchemaTermFromJSON failed to get \"%s\"", SCHEMA_TERM_NAME_S);
				}

		}		/* if (url_s) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, term_json_p, "GetSchemaTermFromJSON failed to get \"%s\"", SCHEMA_TERM_URL_S);
		}

	return NULL;
}



static bool AddTermToJSON (json_t *root_p, const char *root_key_s, const char *type_s, const char *key_s, const char *value_s)
{
	json_t *data_p = json_object ();

	if (data_p)
		{
			if (json_object_set_new (data_p, "@type", json_string (type_s)) == 0)
				{
					if (json_object_set_new (data_p, key_s, json_string (value_s)) == 0)
						{
							if (json_object_set_new (root_p, root_key_s, data_p) == 0)
								{
									return true;
								}		/* if (json_object_set_new (data_p "@type", json_string (type_s)) == 0) */

						}		/* if (json_object_set_new (data_p "@type", json_string (type_s)) == 0) */

				}		/* if (json_object_set_new (data_p "@type", json_string (type_s)) == 0) */

			json_decref (data_p);
		}		/* if (data_p) */

	return false;
}
