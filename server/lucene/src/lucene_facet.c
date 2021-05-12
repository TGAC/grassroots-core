/*
 * lucene_facet.c
 *
 *  Created on: 31 Oct 2019
 *      Author: billy
 */

#define ALLOCATE_LUCENE_FACET_CONSTANTS (1)
#include "lucene_facet.h"
#include "memory_allocations.h"
#include "json_util.h"
#include "streams.h"
#include "string_utils.h"


LuceneFacet *AllocateLuceneFacet (const char *name_s, const uint32 count)
{
	char *copied_name_s = EasyCopyToNewString (name_s);

	if (copied_name_s)
		{
			LuceneFacet *facet_p = (LuceneFacet *) AllocMemory (sizeof (LuceneFacet));

			if (facet_p)
				{
					facet_p -> lf_name_s = copied_name_s;
					facet_p -> lf_count = count;

					return facet_p;
				}		/* if (facet_p) */

			FreeCopiedString (copied_name_s);
		}		/* if (copied_name_s) */

	return NULL;
}


void FreeLuceneFacet (LuceneFacet *facet_p)
{
	FreeCopiedString (facet_p -> lf_name_s);

	FreeMemory (facet_p);
}


LuceneFacetNode *AllocateLuceneFacetNode (LuceneFacet *facet_p)
{
	LuceneFacetNode *node_p = (LuceneFacetNode *) AllocMemory (sizeof (LuceneFacetNode));

	if (node_p)
		{
			InitListItem (& (node_p -> lfn_node));
			node_p -> lfn_facet_p = facet_p;

			return node_p;
		}

	return node_p;
}


LuceneFacetNode *AllocateLuceneFacetNodeByParts (const char *name_s, const uint32 count)
{
	LuceneFacet *facet_p = AllocateLuceneFacet (name_s, count);

	if (facet_p)
		{
			LuceneFacetNode *node_p = AllocateLuceneFacetNode (facet_p);

			if (node_p)
				{
					return node_p;
				}

			FreeLuceneFacet (facet_p);
		}

	return NULL;
}


void FreeLuceneFacetNode (ListItem *node_p)
{
	LuceneFacetNode *facet_node_p = (LuceneFacetNode *) node_p;

	FreeLuceneFacet (facet_node_p -> lfn_facet_p);
	FreeMemory (facet_node_p);
}


json_t *GetLuceneFacetAsJSON (const LuceneFacet *facet_p)
{
	json_t *json_p = json_object ();

	if (json_p)
		{
			if (SetJSONString (json_p, LF_NAME_S, facet_p -> lf_name_s))
				{
					if (SetJSONInteger (json_p, LF_COUNT_S, facet_p -> lf_count))
						{
							return json_p;
						}
					else
						{
							PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to add \"%s\": " UINT32_FMT " to json", LF_COUNT_S, facet_p -> lf_count);
						}

				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to add \"%s\": \"%s\" to json", LF_NAME_S, facet_p -> lf_name_s);
				}

		}		/* if (json_p) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate json for LuceneFacet");
		}

	return NULL;
}


LuceneFacet *GetLuceneFacetFromResultsJSON (const json_t *json_p)
{
	const char *name_s = GetJSONString (json_p, "label");

	if (name_s)
		{
			uint32 count = 0;

			if (GetJSONUnsignedInteger (json_p, "value", &count))
				{
					LuceneFacet *facet_p = AllocateLuceneFacet (name_s, count);

					if (facet_p)
						{
							return facet_p;
						}
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate LuceneFacet for \"%s\" = " UINT32_FMT, name_s, count);
						}
				}
			else
				{
					PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get \"value\" from json");
				}

		}
	else
		{
			PrintJSONToErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, json_p, "Failed to get \"label\" from json");
		}

	return NULL;
}

