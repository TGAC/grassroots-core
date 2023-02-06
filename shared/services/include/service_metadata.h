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
 * service_metadata.h
 *
 *  Created on: 10 Nov 2017
 *      Author: billy
 */

#ifndef CORE_SHARED_SERVICES_INCLUDE_SERVICE_METADATA_H_
#define CORE_SHARED_SERVICES_INCLUDE_SERVICE_METADATA_H_

#include "../../../shared/services/include/grassroots_service_library.h"
#include "typedefs.h"
#include "../../../shared/services/include/schema_term.h"

#include "jansson.h"




/**
 * A datatype for describing the type of application
 * that a Service provides.
 */
typedef struct ServiceMetadata
{
	/**
	 * This is the value used for defining the category of
	 * the Service.
	 */
	SchemaTerm *sm_application_category_p;

	/**
	 * This is the value used for defining the sub-category of
	 * the Service.
	 */
	SchemaTerm *sm_application_subcategory_p;

	/**
	 * A LinkedList of SchemaTermNodes defining the types of input
	 * that the Service can take.
	 */
	LinkedList *sm_input_types_p;

	/**
	 * A LinkedList of SchemaTermNodes defining the types of output
	 * that the Service can generate.
	 */
	LinkedList *sm_output_types_p;


} ServiceMetadata;



#ifdef __cplusplus
extern "C"
{
#endif


/**
 * Allocate a ServiceMetadata object.
 *
 * @param category_p The top-level application category to use.
 * @param subcategory_p The application subcategory to use.
 * @return The new ServiceMetadata or <code>NULL</code> upon error.
 * @memberof ServiceMetadata
 */
GRASSROOTS_SERVICE_API ServiceMetadata *AllocateServiceMetadata (SchemaTerm *category_p, SchemaTerm *subcategory_p);


/**
 * Free a ServiceMetadata object.
 *
 * @param metadata_p The ServiceMetadata to free.
 * @memberof ServiceMetadata
 */
GRASSROOTS_SERVICE_API void FreeServiceMetadata (ServiceMetadata *metadata_p);


/**
 * Clear any data within a ServiceMetadata object.
 *
 * @param metadata_p The ServiceMetadata to clear.
 * @memberof ServiceMetadata
 */
GRASSROOTS_SERVICE_API void ClearServiceMetadata (ServiceMetadata *metadata_p);


/**
 * Set the values of a ServiceMetadata object.
 *
 * @param metadata_p The ServiceMetadata to set.
 * @param category_p The top-level application category to use.
 * @param subcategory_p The application subcategory to use.
 * @memberof ServiceMetadata
 */
GRASSROOTS_SERVICE_API void SetServiceMetadataValues (ServiceMetadata *metadata_p, SchemaTerm *category_p, SchemaTerm *subcategory_p);



/**
 * Add the information from a ServiceMetadata object to a JSON fragment describing a Service.
 *
 * @param metadata_p The ServiceMetadata to get the values from.
 * @param service_json_p The JSON fragment where the metadata information will be added.
 * @return <code>true</code> if the ServiceMetadata information was added successfully, <code>false</code> if it was not.
 * @memberof ServiceMetadata
 */
GRASSROOTS_SERVICE_API bool AddServiceMetadataToJSON (const ServiceMetadata *metadata_p, json_t *service_json_p);


/**
 * Add a SchemaTerm to the list of input types stored in this ServiceMetadata.
 *
 * @param metadata_p The ServiceMetadata to add the input term to.
 * @param term_p The SchemaTerm to add
 * @return <code>true</code> if the ServiceMetadata input list was updated successfully, <code>false</code> if it was not.
 * @return ServiceMetadata
 */
GRASSROOTS_SERVICE_API bool AddSchemaTermToServiceMetadataInput (ServiceMetadata *metadata_p, SchemaTerm *term_p);


/**
 * Add a SchemaTerm to the list of output types stored in this ServiceMetadata.
 *
 * @param metadata_p The ServiceMetadata to add the output term to.
 * @param term_p The SchemaTerm to add
 * @return <code>true</code> if the ServiceMetadata output list was updated successfully, <code>false</code> if it was not.
 * @return ServiceMetadata
 */
GRASSROOTS_SERVICE_API bool AddSchemaTermToServiceMetadataOutput (ServiceMetadata *metadata_p, SchemaTerm *term_p);


/**
 * Get a ServiceMetadata object from a JSON fragment.
 *
 * @param service_json_p The JSON fragment for a Service to get the ServiceMetadata from.
 * @return The new ServiceMetadata or <code>NULL</code> upon error.
 * @memberof ServiceMetadata
 */
GRASSROOTS_SERVICE_API ServiceMetadata *GetServiceMetadataFromJSON (const json_t *service_json_p);


#ifdef __cplusplus
}
#endif


#endif /* CORE_SHARED_SERVICES_INCLUDE_SERVICE_METADATA_H_ */
