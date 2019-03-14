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
/*
 * version.c
 *
 *  Created on: 6 May 2016
 *      Author: tyrrells
 */

/**
 * This define will allocate the variables
 * CURRENT_SCHEMA_VERSION_MAJOR and
 * CURRENT_SCHEMA_VERSION_MINOR
 * in schema_version.h
 */
#define ALLOCATE_SCHEMA_VERSION_TAGS

#include <string.h>

#include "schema_version.h"
#include "json_util.h"
#include "streams.h"
#include "string_utils.h"
#include "math_utils.h"


json_t *GetSchemaVersionAsJSON (const SchemaVersion * const sv_p)
{
	json_t *res_p = json_object ();

	if (res_p)
		{
			if (json_object_set_new (res_p, VERSION_S, json_string (sv_p -> sv_version_s)) == 0)
				{
					return res_p;
				}
			else
				{
					json_decref (res_p);
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to add \"%s\": \"%s\" to schema version json", VERSION_S, sv_p -> sv_version_s);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create schema version json");
		}

	return NULL;
}


SchemaVersion *GetSchemaVersionFromJSON (const json_t * const json_p)
{
	SchemaVersion *sv_p = NULL;
	const char *version_s = GetJSONString (json_p, VERSION_S);

	if (version_s)
		{
			const char *dot_s = strchr (version_s, '.');

			/* move past the dot */
			if (dot_s)
				{
					++ dot_s;

					if (*dot_s != '\0')
						{
							int major;

							if (GetValidInteger (&version_s, &major))
								{
									int minor;

									if (GetValidInteger (&dot_s, &minor))
										{
											sv_p = AllocateSchemaVersion (major, minor);
										}
									else
										{
											PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, json_p, "Failed to get %s from \"%s\"", VERSION_MINOR_S, version_s);
										}
								}
							else
								{
									PrintJSONToErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, json_p, "Failed to get %s from \"%s\"", VERSION_MINOR_S, version_s);
								}
						}

				}

		}		/* if (version_s) */



	return sv_p;
}



bool SetSchemaVersionDetails (SchemaVersion *sv_p, const int major, const int minor)
{
	bool success_flag = false;
	char *major_s = ConvertIntegerToString (major);

	if (major_s)
		{
			char *minor_s = ConvertIntegerToString (minor);

			if (minor_s)
				{
					char *version_s = ConcatenateVarargsStrings (major_s, ".", minor_s, NULL);

					if (version_s)
						{
							if (sv_p -> sv_version_s)
								{
									FreeCopiedString (sv_p -> sv_version_s);
								}

							sv_p -> sv_major = major;
							sv_p -> sv_minor = minor;
							sv_p -> sv_version_s = version_s;

							success_flag = true;
						}		/* if (version_s) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create version identifier \"%s.%s\"", major_s, minor_s);
						}

					FreeCopiedString (minor_s);
				}		/* if (minor_s) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create minor version identifier for \"%d", minor);
				}

			FreeCopiedString (major_s);
		}		/* if (major_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to create major version identifier for \"%d", major);
		}

	return success_flag;
}


SchemaVersion *AllocateSchemaVersion (const int major, const int minor)
{
	SchemaVersion *sv_p = (SchemaVersion *) AllocMemory (sizeof (SchemaVersion));

	if (sv_p)
		{
			memset (sv_p, 0, sizeof (SchemaVersion));

			if (SetSchemaVersionDetails (sv_p, major, minor))
				{
					return sv_p;
				}

			FreeSchemaVersion (sv_p);
		}

	return NULL;
}


void FreeSchemaVersion (SchemaVersion *sv_p)
{
	ClearSchemaVersion (sv_p);
	FreeMemory (sv_p);
}


void ClearSchemaVersion (SchemaVersion *sv_p)
{
	if (sv_p -> sv_version_s)
		{
			FreeCopiedString (sv_p -> sv_version_s);
			sv_p -> sv_version_s = NULL;
		}

	sv_p -> sv_major = CURRENT_SCHEMA_VERSION_MAJOR;
	sv_p -> sv_minor = CURRENT_SCHEMA_VERSION_MINOR;
}
