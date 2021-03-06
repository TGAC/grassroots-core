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
 * @brief
 */
#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>

#include "typedefs.h"
#include "memory_allocations.h"
#include "grassroots_util_library.h"
#include "streams.h"

#ifdef __cplusplus
	extern "C" {
#endif


/**
 * A datatype for holding a key-value pair along with
 * the hashed value of the key.
 *
 * @ingroup utility_group
 */
typedef struct HashBucket
{
	/** The hashed value of this HashBucket key */
	uint32 hb_hashed_key;

	/** The key */
	const void *hb_key_p;

	/** The value */
	const void *hb_value_p;

	/** Can the HashBucket free its key or is the memory owned by another process? */
	MEM_FLAG hb_owns_key;

	/** Can the HashBucket free its value or is the memory owned by another process? */
	MEM_FLAG hb_owns_value;
} HashBucket;


/**
 * Function for freeing a HashBucket.
 *
 * @param bucket_p The HashBucket to free.
 * @memberof HashBucket
 */
GRASSROOTS_UTIL_API void FreeHashBucket (HashBucket * const bucket_p);


/**
 * Function for creating the HashBuckets. The allocated
 * memory is all set to 0/<code>NULL</code>.
 *
 * @param num_buckets The number of HashBuckets to create.
 * @param key_mem_flag How the HashBuckets will deal with memory allocation of the keys.
 * @param value_mem_flag How the HashBuckets will deal with memory allocation of the values.
 * @return Pointer to the HashBuckets or <code>NULL</code> upon error.
 * @memberof HashBucket
 */
GRASSROOTS_UTIL_API HashBucket *CreateHashBuckets (const uint32 num_buckets, const MEM_FLAG key_mem_flag, const MEM_FLAG value_mem_flag);


/**
 * Create an array of HashBuckets where each one will make a deep copy of the key and value when they are
 * put into the HashTable.
 *
 * @param num_buckets The number of HashBuckets to allocate.
 * @return The array of HashBuckets or <code>NULL</code> upon error.
 * @memberof HashBucket
 * @see MF_DEEP_COPY
 */
GRASSROOTS_UTIL_API HashBucket *CreateDeepCopyHashBuckets (const uint32 num_buckets);


/**
 * Create an array of HashBuckets where each one will make a shallow copy of the key and value when they are
 * put into the HashTable.
 *
 * @param num_buckets The number of HashBuckets to allocate.
 * @return The array of HashBuckets or <code>NULL</code> upon error.
 * @memberof HashBucket
 * @see MF_SHALLOW_COPY
 */
GRASSROOTS_UTIL_API HashBucket *CreateShallowCopyHashBuckets (const uint32 num_buckets);


/**
 * Create an array of HashBuckets where each one will make shadow use the key and value when they are
 * put into the HashTable.
 *
 * @param num_buckets The number of HashBuckets to allocate.
 * @return The array of HashBuckets or <code>NULL</code> upon error.
 * @memberof HashBucket
 * @see MF_SHADOW_USE
 */
GRASSROOTS_UTIL_API HashBucket *CreateShadowUseHashBuckets (const uint32 num_buckets);


/**
 * Create an array of HashBuckets where each one will make a deep copy of the key and a shallow
 * copy of the value when they are put into the HashTable.
 *
 * @param num_buckets The number of HashBuckets to allocate.
 * @return The array of HashBuckets or <code>NULL</code> upon error.
 * @memberof HashBucket
 * @see MF_SHALLOW_COPY
 * @see MF_DEEP_COPY
 */
GRASSROOTS_UTIL_API HashBucket *CreateDeepCopyKeysShallowCopyValueHashBuckets (const uint32 num_buckets);



/**
 * A container using HashBuckets to allow for fast lookup
 * of key-value pairs.
 *
 * @ingroup utility_group
 */
typedef struct HashTable
{
	/** The number of valid HashBuckets in the HashTable */
	uint32 ht_size;

	/** The actual number of HashBucket slots in the HashTable */
	uint32 ht_capacity;

	/** How full to let the hash table get before rehashing, between 0 and 100 */
	uint8 ht_load;

	/** The ht_capacity * ht_load */
	uint32 ht_load_limit;

	/** The HashBuckets */
	HashBucket *ht_buckets_p;

	/** Function for calculating the hashed value of a HashBucket key */
	uint32 (*ht_hash_fn) (const void *);

	/** Function for creating the HashBuckets */
	HashBucket *(*ht_create_buckets_fn) (const uint32 num_buckets);

	/** Function for feeing a HashBucket */
	void (*ht_free_bucket_fn) (HashBucket * const bucket_p);

	/** Function for filling a HashBucket */
	bool (*ht_fill_bucket_fn) (HashBucket * const bucket_p, const void * const key_p, const void * const value_p);

	/** Function for comparing the keys of two HashBuckets */
	bool (*ht_compare_keys_fn) (const void * const bucket_key_p, const void * const key_p);

	/** Function for printing out the HashBuckets in this HashTable */
	void (*ht_print_bucket_fn) (const HashBucket * const bucket_p, OutputStream * const stream_p);

	/** Function for saving the HashBuckets in this HashTable to a file. */
	bool (*ht_save_bucket_fn) (const HashBucket * const bucket_p, FILE *out_f);

} HashTable;


/**
 * Allocate a new HashTable.
 *
 * @param initial_size The initial number of HashBuckets for this HashTable.
 * @param load_percentage The percentage value for how full the HashTable should
 * be allowed to become before it is extended.
 * @param hash_fn The callback function to use for generating the hashed_key value
 * for each HashBucket in the HashTable from its key.
 * @param create_buckets_fn The callback function used to create the given number
 * of HashBuckets.
 * @param free_bucket_fn The callback function used to free each given HashBucket
 * when the HashTable is freed.
 * @param fill_bucket_fn The callback function to use when adding a key-value pair
 * to the HashTable.
 * @param compare_keys_fn The callback function that takes 2 HashBuckets and
 * determines whether they have the same key.
 * @param print_bucket_fn The callback function that prints out the contents of
 * the given HashBucket to the given stream.
 * @param save_bucket_fn The callback function to use for saving each HashBucket when
 * saving the HashTable to a file.
 * @return The new HashTable or <code>NULL</code> if there was an error.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API HashTable *AllocateHashTable (const uint32 initial_size,
	const uint8 load_percentage,
	uint32 (*hash_fn) (const void * const key_p),
	HashBucket *(*create_buckets_fn) (const uint32 num_buckets),
	void (*free_bucket_fn) (HashBucket * const bucket_p),
	bool (*fill_bucket_fn) (HashBucket * const bucket_p, const void * const key_p, const void * const value_p),
	bool (*compare_keys_fn) (const void * const bucket_key_p, const void * const key_p),
	void (*print_bucket_fn) (const HashBucket * const bucket_p, OutputStream * const stream_p),
	bool (*save_bucket_fn) (const HashBucket * const bucket_p, FILE *out_f));


/**
 * Free a HashTable
 *
 * @param hash_table_p The HashTable to free.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API void FreeHashTable (HashTable * const hash_table_p);


/** 
 * Clear a HashTable.
 *
 * @param hash_table_p The HashTable to clear.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API void ClearHashTable (HashTable * const hash_table_p);


/**
 * Does the HashBucket contain a valid key-value pair.
 *
 * @param bucket_p The HashBucket.
 * @return TRUE if the bucket does contain a valid key-value pair, FALSE if it is empty.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API bool IsValidHashBucket (const HashBucket * const bucket_p);


/**
 * Put a key-value pair in the HashTable. If a bucket already contains this key,
 * its value will be overwritten. If the HashTable is getting beyond its
 * designated load, its capacity will be extended.
 *
 * @param hash_table_p The HashTable to put for the key-value pair in.
 * @param key_p The key.
 * @param value_p The value.
 * @return <code>true</code> if the key-value pair were added successfully,
 * <code>false</code> otherwise.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API bool PutInHashTable (HashTable * const hash_table_p, const void * const key_p, const void * const value_p);


/**
 * For a given key, get the value from a HashTable. If the key
 * is not in the HashTable, this will return NULL
 *
 * @param hash_table_p The HashTable to search for the value in.
 * @param key_p The key.
 * @return The value or <code>NULL</code> if there is not a matching value for the
 * given key.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API const void *GetFromHashTable (const HashTable * const hash_table_p, const void * const key_p);

/**
 * Get a new array with pointers to each of the keys in a given HashTable.
 *
 * @param hash_table_p The HashTable to get the keys from.
 * @memberof HashTable
 * @see FreeKeysIndex
 */
GRASSROOTS_UTIL_API void **GetKeysIndexFromHashTable (const HashTable * const hash_table_p);

/**
 * Free an index array previously returned by a call to GetKeysIndexFromHashTable.
 *
 * @param index_pp The array of index pointers to free.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API void FreeKeysIndex (void **index_pp);

/**
 * For a given key, empty the corresponding bucket in a HashTable. If the key
 * is not in the HashTable, this will do nothing
 *
 * @param hash_table_p The HashTable to search for the in.
 * @param key_p The key.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API void RemoveFromHashTable (HashTable * const hash_table_p, const void * const key_p);


/**
 * Print out a HashTable to a given stream.
 *
 * @param hash_table_p The HashTable to print out.
 * @param stream_p The OutputStream to print the HashTable to.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API void PrintHashTable (const HashTable * const hash_table_p, OutputStream * const stream_p);


/**
 * Copy a HashTable.
 *
 * @param src_table_p The HashTable to copy.
 * @param deep_copy_if_necessary How to avoid potential memory corruption.
 * Check the source table's memory settings.
 *
 * MF_DEEP_COPY: our copied table will work correctly
 * MF_SHADOW_USE: our copied table will work correctly
 * MF_SHALLOW_COPY: This will cause memory corruption since
 * both the cource and destination tables will try to free
 * the same memory. So switch to either MF_DEEP_COPY or
 * MF_SHADOW_USE depending upon deep_copy_if_necessary.
 *
 * @return The copied HashTable or <code>NULL</code> upon error.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API HashTable *CopyHashTable (const HashTable * const src_table_p, const bool deep_copy_if_necessary);


/**
 * Save a HashTable to a file.
 *
 * @param table_p The HashTable to save.
 * @param filename_s The filename where the HashTable will be saved to.
 * @param compare_fn An optional callback function allowing the contents of the HashTable
 * to be saved in a user-defined order. If this is <code>NULL</code> then the contents
 * will be saved in an undetermined order.
 * @return <code>true</code> if the data was saved successfully, <code>false</code>
 * otherwise.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API bool SaveHashTable (const HashTable * const table_p, const char * const filename_s, int (*compare_fn) (const void *v0_p, const void *v1_p));


/**
 * Load the contents of a previously-saved HashTable file into a HashTable.
 *
 * @param table_p The HastTable to load the data into.
 * @param current_path_s The directory containing the saved file.
 * @param filename_s The filename to load.
 * @return <code>true</code> if the data was loaded successfully, <code>false</code>
 * otherwise.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API bool LoadHashTable (HashTable * const table_p, char *current_path_s, const char * const filename_s);


/**
 * Get the number of entries in a HashTable.
 *
 * @param table_p The HashTable to get the size of.
 * @return The number of entries in the HashTable.
 * @memberof HashTable
 */
GRASSROOTS_UTIL_API uint32 GetHashTableSize (const HashTable * const table_p);

#ifdef __cplusplus
}
#endif

#endif 	/* #ifndef HASH_TABLE_H */
