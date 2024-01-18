/*
 * view_format.h
 *
 *  Created on: 15 Jan 2024
 *      Author: billy
 */

#ifndef CORE_SHARED_UTIL_INCLUDE_VIEW_FORMAT_H_
#define CORE_SHARED_UTIL_INCLUDE_VIEW_FORMAT_H_


/**
 * An indicator of what the output destination
 * is for the JSON data values. This lets us
 * know whether we need to e.g. expand fields
 * to full objects from their ids, or omit
 * certain fields entirely
 */
typedef enum
{
	/**
	 * This is for generating JSON to be stored in the
	 * server-side mongo db.
	 */
	VF_STORAGE,

	/**
	 * This is for generating a full data set for displaying
	 * within a client.
	 */
	VF_CLIENT_FULL,

	/**
	 * This is for generating a minimal data set for displaying
	 * within a client. This is used when doing LinkedService
	 * calls to get subsequent child data.
	 */
	VF_CLIENT_MINIMAL,


	/**
	 * Get the object with all of the relevant data
	 * needed for indexing into the Lucene component
	 */
	VF_INDEXING,


	/**
	 * Store just a reference to the object such as a bson_oid
	 */
	VF_REFERENCE,

	/**
	 * The number of available formats
	 */
	VF_NUM_FORMATS
} ViewFormat;



#endif /* CORE_SHARED_UTIL_INCLUDE_VIEW_FORMAT_H_ */
