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
 * service_metadata.c
 *
 *  Created on: 10 Nov 2017
 *      Author: billy
 */

#include <string.h>

#include "service_metadata.h"
#include "string_utils.h"
#include "memory_allocations.h"
#include "schema_keys.h"
#include "streams.h"
#include "json_util.h"


static bool AddSchemaTermToList (LinkedList *list_p, SchemaTerm *term_p);

static bool AddSchemaTermListToJSON (json_t *root_p, LinkedList *schema_term_list_p, const char * const key_s);

static SchemaTerm *GetSchemaTerm (const json_t *service_json_p, const char * const key_s);

static bool AddSchemaTermsToLinkedListFromJSON (const json_t *metadata_json_p, const char * const key_s, LinkedList *list_p);



ServiceMetadata *AllocateServiceMetadata (SchemaTerm *category_p, SchemaTerm *subcategory_p)
{
	ServiceMetadata *metadata_p = (ServiceMetadata *) AllocMemory (sizeof (ServiceMetadata));

	if (metadata_p)
		{
			LinkedList *input_types_p = AllocateLinkedList (FreeSchemaTermNode);

			if (input_types_p)
				{
					LinkedList *output_types_p = AllocateLinkedList (FreeSchemaTermNode);

					if (output_types_p)
						{
							memset (metadata_p, 0, sizeof (ServiceMetadata));

							SetServiceMetadataValues (metadata_p, category_p, subcategory_p);

							metadata_p -> sm_input_types_p = input_types_p;
							metadata_p -> sm_output_types_p = output_types_p;

							return metadata_p;
						}		/* if () */

					FreeLinkedList (input_types_p);
				}		/* if (input_types_p) */


			FreeMemory (metadata_p);
		}

	return NULL;
}


void ClearServiceMetadata (ServiceMetadata *metadata_p)
{
	if (metadata_p -> sm_application_category_p)
		{
			FreeSchemaTerm (metadata_p -> sm_application_category_p);
			metadata_p -> sm_application_category_p = NULL;
		}

	if (metadata_p -> sm_application_subcategory_p)
		{
			FreeSchemaTerm (metadata_p -> sm_application_subcategory_p);
			metadata_p -> sm_application_subcategory_p = NULL;
		}

	ClearLinkedList (metadata_p -> sm_input_types_p);
	ClearLinkedList (metadata_p -> sm_output_types_p);
}


void FreeServiceMetadata (ServiceMetadata *metadata_p)
{
	ClearServiceMetadata (metadata_p);

	FreeLinkedList (metadata_p -> sm_input_types_p);
	FreeLinkedList (metadata_p -> sm_output_types_p);

	FreeMemory (metadata_p);
}


void SetServiceMetadataValues (ServiceMetadata *metadata_p, SchemaTerm *category_p, SchemaTerm *subcategory_p)
{
	if (metadata_p -> sm_application_category_p)
		{
			FreeSchemaTerm (metadata_p -> sm_application_category_p);
		}

	metadata_p -> sm_application_category_p = category_p;


	if (metadata_p -> sm_application_subcategory_p)
		{
			FreeSchemaTerm (metadata_p -> sm_application_subcategory_p);
		}

	metadata_p -> sm_application_subcategory_p = subcategory_p;
}


bool AddServiceMetadataToJSON (const ServiceMetadata *metadata_p, json_t *service_json_p)
{
	bool success_flag = false;

	json_t *metadata_json_p = json_object ();

	if (metadata_json_p)
		{
			if (metadata_p -> sm_application_category_p)
				{
					json_t *category_p = GetSchemaTermAsJSON (metadata_p -> sm_application_category_p);

					if (category_p)
						{
							if (json_object_set_new (metadata_json_p, SERVICE_METADATA_APPLICATION_CATEGORY_S, category_p) == 0)
								{
									success_flag = true;

									if (metadata_p -> sm_application_subcategory_p)
										{
											json_t *subcategory_p = GetSchemaTermAsJSON (metadata_p -> sm_application_subcategory_p);

											if (subcategory_p)
												{
													if (json_object_set_new (metadata_json_p, SERVICE_METADATA_APPLICATION_SUBCATEGORY_S, subcategory_p) != 0)
														{
															success_flag = false;
														}
												}
											else
												{
													success_flag = false;
												}

										}		/* if (metadata_p -> sm_application_subcategory_p) */

									if (success_flag)
										{
											/* Now add the input parameters */
											if (metadata_p -> sm_input_types_p)
												{
													success_flag = AddSchemaTermListToJSON (metadata_json_p, metadata_p -> sm_input_types_p, SERVICE_METADATA_APPLICATION_INPUT_S);
												}

											if (success_flag)
												{
													/* Now add the output parameters */
													if (metadata_p -> sm_output_types_p)
														{
															success_flag = AddSchemaTermListToJSON (metadata_json_p, metadata_p -> sm_output_types_p, SERVICE_METADATA_APPLICATION_OUTPUT_S);
														}
												}

											if (success_flag)
												{
													if (json_object_set_new (service_json_p, SERVICE_CATEGORY_S, metadata_json_p) != 0)
														{
															success_flag = false;
														}
												}

										}		/* if (success_flag) */

								}		/* if (json_object_set_new (service_json_p, SERVICE_METADATA_APPLICATION_CATEGORY_S, category_p) == 0) */

						}		/* if (category_p) */

				}		/* if (metadata_p -> sm_application_category_p) */

			if (!success_flag)
				{
					json_decref (metadata_json_p);
				}
		}		/* if (metadata_json_p) */

	return success_flag;
}


ServiceMetadata *GetServiceMetadataFromJSON (const json_t *service_json_p)
{
	const json_t *category_json_p = json_object_get (service_json_p, SERVICE_CATEGORY_S);

	if (category_json_p)
		{
			SchemaTerm *category_p = GetSchemaTerm (category_json_p, SERVICE_METADATA_APPLICATION_CATEGORY_S);

			if (category_p)
				{
					bool success_flag = true;
					SchemaTerm *subcategory_p = NULL;
					const json_t *subcategory_json_p = GetJSONString (service_json_p, SERVICE_METADATA_APPLICATION_SUBCATEGORY_S);

					if (subcategory_json_p)
						{
							subcategory_p = GetSchemaTermFromJSON (subcategory_json_p);

							if (!subcategory_p)
								{
									success_flag = false;
								}
						}

					if (success_flag)
						{
							ServiceMetadata *service_metadata_p = AllocateServiceMetadata (category_p, subcategory_p);

							if (service_metadata_p)
								{
									if (AddSchemaTermsToLinkedListFromJSON (category_json_p, SERVICE_METADATA_APPLICATION_INPUT_S, service_metadata_p -> sm_input_types_p))
										{
											if (AddSchemaTermsToLinkedListFromJSON (category_json_p, SERVICE_METADATA_APPLICATION_OUTPUT_S, service_metadata_p -> sm_output_types_p))
												{
													return service_metadata_p;
												}		/* if (AddSchemaTermsToLinkedListFromJSON (category_json_p, SERVICE_METADATA_APPLICATION_INPUT_S, service_metadata_p -> sm_input_types_p)) */
											else
												{
													PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, category_json_p, "Failed to add input terms to ServiceMetadata");
												}

										}		/* if (AddSchemaTermsToLinkedListFromJSON (category_json_p, SERVICE_METADATA_APPLICATION_INPUT_S, service_metadata_p -> sm_input_types_p)) */
									else
										{
											PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, category_json_p, "Failed to add input terms to ServiceMetadata");
										}

									FreeServiceMetadata (service_metadata_p);
								}		/* if (service_metadata_p) */
							else
								{
									PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate ServiceMetadata for \"%s\" and \"%s\"", category_p -> st_name_s, subcategory_p ? subcategory_p -> st_name_s : "");
								}

						}		/* if (success_flag) */
					else
						{
							FreeSchemaTerm (category_p);
						}

				}		/* if (category_p) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, category_json_p, "GetSchemaTerm failed for \"%s\"", SERVICE_METADATA_APPLICATION_CATEGORY_S);
				}

		}		/* if (category_json_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, service_json_p, "Failed to find \"%s\"", SERVICE_CATEGORY_S);
		}

	return NULL;
}


bool AddSchemaTermToServiceMetadataInput (ServiceMetadata *metadata_p, SchemaTerm *term_p)
{
	return AddSchemaTermToList (metadata_p -> sm_input_types_p, term_p);
}


bool AddSchemaTermToServiceMetadataOutput (ServiceMetadata *metadata_p, SchemaTerm *term_p)
{
	return AddSchemaTermToList (metadata_p -> sm_output_types_p, term_p);
}


static bool AddSchemaTermListToJSON (json_t *root_p, LinkedList *schema_term_list_p, const char * const key_s)
{
	bool success_flag = true;
	json_t *terms_array_p = json_array ();

	if (terms_array_p)
		{
			SchemaTermNode *node_p = (SchemaTermNode *) (schema_term_list_p -> ll_head_p);

			while (node_p && success_flag)
				{
					json_t *param_metadata_p = GetSchemaTermAsJSON (node_p -> stn_term_p);

					if (param_metadata_p)
						{
							if (json_array_append_new (terms_array_p, param_metadata_p) == 0)
								{
									node_p = (SchemaTermNode *) (node_p -> stn_node.ln_next_p);
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							success_flag = false;
						}

				}		/* while (node_p && success_flag) */

			if (success_flag)
				{
					if (json_object_set_new (root_p, key_s, terms_array_p) != 0)
						{
							success_flag = false;
						}
				}
		}

	return success_flag;
}



static bool AddSchemaTermToList (LinkedList *list_p, SchemaTerm *term_p)
{
	bool success_flag = false;
	SchemaTermNode *node_p = AllocateSchemaTermNode (term_p);

	if (node_p)
		{
			LinkedListAddTail (list_p, (ListItem *) node_p);
			success_flag = true;
		}

	return success_flag;
}


static SchemaTerm *GetSchemaTerm (const json_t *service_json_p, const char * const key_s)
{
	const json_t *term_json_p = json_object_get (service_json_p, key_s);

	if (term_json_p)
		{
			SchemaTerm *term_p = GetSchemaTermFromJSON (term_json_p);

			if (term_p)
				{
					return term_p;
				}		/* if (term_p) */
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, term_json_p, "GetSchemaTermFromJSON failed");
				}

		}		/* if (term_json_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, service_json_p, "Failed to find \"%s\" key", key_s);
		}

	return NULL;
}



static bool AddSchemaTermsToLinkedListFromJSON (const json_t *metadata_json_p, const char * const key_s, LinkedList *list_p)
{
	bool success_flag = false;
	const json_t *terms_json_p = json_object_get (metadata_json_p, key_s);

	if (terms_json_p)
		{
			if (json_is_array (terms_json_p))
				{
					const size_t size = json_array_size (terms_json_p);
					size_t i;
					size_t num_added = 0;

					for (i = 0; i < size; ++ i)
						{
							const json_t *term_json_p = json_array_get (terms_json_p, i);
							SchemaTerm *term_p = GetSchemaTermFromJSON (term_json_p);

							if (term_p)
								{
									 if (AddSchemaTermToList (list_p, term_p))
										 {
											 ++ num_added;
										 }
									 else
										 {
											 PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "AddSchemaTermToList failed for \"%s\"", term_p -> st_name_s);
											 FreeSchemaTerm (term_p);
										 }

								}		/* if (term_p) */
							else
								{
									PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, term_json_p, "GetSchemaTermFromJSON failed");
								}

						}		/* for (i = 0; i < size; ++ i) */


					if (num_added == size)
						{
							success_flag = true;
						}
				}

		}		/* if (term_json_p) */
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, metadata_json_p, "Failed to find \"%s\" key", key_s);
		}

	return success_flag;
}

