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

#include "service_metadata.h"
#include "string_utils.h"

ServiceMetadata *AllocateServiceMetadata (const char *category_s, const char *subcategory_s)
{
	ServiceMetadata *metadata_p = (ServiceMetadata *) AllocMemory (sizeof (ServiceMetadata));

	if (metadata_p)
		{
			if (SetServiceMetadataValues (metadata_p, category_s, subcategory_s))
				{
					return metadata_p;
				}

			FreeMemory (metadata_p);
		}

	return NULL;
}


void ClearServiceMetadata (ServiceMetadata *metadata_p)
{
	if (metadata_p -> sm_application_category_s)
		{
			FreeCopiedString (metadata_p -> sm_application_category_s);
			metadata_p -> sm_application_category_s = NULL;
		}

	if (metadata_p -> sm_application_subcategory_s)
		{
			FreeCopiedString (metadata_p -> sm_application_subcategory_s);
			metadata_p -> sm_application_subcategory_s = NULL;
		}
}


void FreeServiceMetadata (ServiceMetadata *metadata_p)
{
	ClearServiceMetadata (metadata_p);

	FreeMemory (metadata_p);
}


bool SetServiceMetadataValues (ServiceMetadata *metadata_p, const char *category_s, const char *subcategory_s)
{
	bool success_flag = true;
	char *copied_category_s = NULL;

	if (category_s)
		{
			copied_category_s = CopyToNewString (category_s, 0, false);

			success_flag = (copied_category_s != NULL);
		}

	if (success_flag)
		{
			char *copied_subcategory_s = NULL;

			if (subcategory_s)
				{
					copied_subcategory_s = CopyToNewString (subcategory_s, 0, false);

					success_flag = (copied_subcategory_s != NULL);
				}

			if (success_flag)
				{
					ClearServiceMetadata (metadata_p);

					metadata_p -> sm_application_category_s = copied_category_s;
					metadata_p -> sm_application_subcategory_s = copied_subcategory_s;

				}

			if (copied_subcategory_s)
				{
					FreeCopiedString (copied_subcategory_s);
				}
		}

	if (copied_category_s)
		{
			FreeCopiedString (copied_category_s);
		}

	return success_flag;
}


bool AddServiceMetadataToJSON (const ServiceMetadata *metadata_p, json_t *service_json_p)
{
	bool success_flag = true;

	if (metadata_p -> sm_application_category_s)
		{
			if (json_object_set_new (service_json_p, "applicationCategory", json_string (metadata_p -> sm_application_category_s)) != 0)
				{
					success_flag = false;
				}
		}

	if (success_flag)
		{
			if (metadata_p -> sm_application_subcategory_s)
				{
					if (json_object_set_new (service_json_p, "applicationSubCategory", json_string (metadata_p -> sm_application_subcategory_s)) != 0)
						{
							success_flag = false;
						}
				}
		}

	return success_flag;
}


