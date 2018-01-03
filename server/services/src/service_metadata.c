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


static bool AddSchemaTermToList (LinkedList *list_p, SchemaTerm *term_p);

static bool AddSchemaTermListToJSON (json_t *root_p, LinkedList *schema_term_list_p, const char * const key_s);



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

