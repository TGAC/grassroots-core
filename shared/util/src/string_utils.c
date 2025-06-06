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
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#include "memory_allocations.h"
#include "string_linked_list.h"
#include "string_utils.h"
#include "typedefs.h"
#include "byte_buffer.h"
#include "streams.h"


static LinkedList *ParseStringToLinkedList (const char * const format_p, const char * const delimiters_p, LinkedList *(*allocate_list_fn) (void), ListItem *(*allocate_node_fn) (const char *value_s), const bool treat_whitespace_as_delimiter_flag);

static ListItem *GetStringListNode (const char *value_s);


static const char *ScrollPastSpecificCharacters (const char **value_pp, const char * const delimiters_s, const bool ignore_whitespace_flag, const bool state_flag);


void FreeCopiedString (char *str_p)
{
	if (str_p)
		{
			FreeMemory (str_p);
		}
}


char *EasyCopyToNewString (const char * const src_s)
{
	return CopyToNewString (src_s, 0, false);
}


/**
 * Copy a string to a newly created string.
 *
 * @param src_p The string to copy.
 * @param trim Whether to trim left and right trailing whitespace or not.
 * @return A newly allocated copy of the source string or NULL if there was an error.
 */
char *CopyToNewString (const char * const src_p, const size_t l, bool trim)
{
	char *dest_p = NULL;

	if (src_p)
		{
			size_t len = strlen (src_p);

			if (l > 0)
				{
					if (len > l)
						{
							len = l;
						}
				}

			if (trim)
				{
					const char *start_p = src_p;
					const char *end_p = src_p + len - 1;

					while (isspace (*start_p))
						{
							++ start_p;
						}

					if (*start_p == '\0')
						{
							/* all white space */
							return NULL;
						}

					while ((start_p < end_p) && (isspace (*end_p)))
						{
							-- end_p;
						}

					if (start_p <= end_p)
						{
							ptrdiff_t d = end_p - start_p + 1;
							dest_p = (char *) AllocMemory (d + 1);

							if (dest_p)
								{
									memcpy (dest_p, start_p, d);
									* (dest_p + d) = '\0';

									return dest_p;
								}

						}
				}
			else
				{
					dest_p = (char *) AllocMemory (len + 1);

					if (dest_p)
						{
							strncpy (dest_p, src_p, len);
							* (dest_p + len) = '\0';

							return dest_p;
						}
				}
		}

	return dest_p;
}



bool CloneValidString (const char *src_s, char **dest_ss)
{
	bool success_flag = false;


	if (!IsStringEmpty (src_s))
		{
			char *dest_s = EasyCopyToNewString (src_s);

			if (dest_s)
				{
					*dest_ss = dest_s;
					success_flag = true;
				}
		}
	else
		{
			*dest_ss = NULL;
			success_flag = true;
		}

	return success_flag;
}


/**
 * Read a line from a file with no buffer overflow issues.
 *
 * @param in_f The FILE to read from.
 * @param buffer_pp Pointer to a buffer string where the line will get written to.
 * After being used, *buffer_pp muet be freed.
 * @return true if the line was read successfully, false if there was an error.
 */
bool GetLineFromFile (FILE *in_f, char **buffer_pp)
{
	const size_t buffer_size = 1024;
	size_t last_index = 0;
	size_t old_size = 0;
	size_t buffer_length = 0;
	char *old_buffer_p = NULL;
	char *new_buffer_p = NULL;
	size_t latest_block_length = 0;
	size_t new_size = 0;
	size_t actual_length = 0;

	if (!feof (in_f))
		{
			bool looping = true;

			do
				{
					new_size = old_size + buffer_size;

					new_buffer_p = (char *) ReallocMemory (old_buffer_p, new_size, old_size);

					if (new_buffer_p)
						{
							char * const latest_block_p = new_buffer_p + buffer_length;

							* (new_buffer_p + new_size - 1) = '\0';

							if (new_buffer_p != old_buffer_p)
								{
									old_buffer_p = new_buffer_p;
								}

							if (fgets (latest_block_p, buffer_size, in_f))
								{
									char c;

									latest_block_length = strlen (latest_block_p);

									buffer_length += latest_block_length;
									last_index = buffer_length - 1;
									old_size = new_size;

									c = * (new_buffer_p + last_index);
									looping = (c != '\r') && (c != '\n') && (!feof (in_f));
								}
							else
								{
									*latest_block_p = '\0';
									looping = false;
								}
						}
					else
						{
							return false;
						}
				}
			while (looping);

			/* Remove the carriage return and/or line feed */
			actual_length = (new_buffer_p != NULL) ? strlen (new_buffer_p) : 0;
			if (actual_length > 0)
				{
					char *c_p = new_buffer_p + actual_length - 1;

					looping = true;
					while (looping)
						{
							if ((*c_p == '\r') || (*c_p == '\n'))
								{
									*c_p = '\0';

									if (c_p > new_buffer_p)
										{
											-- c_p;
										}
									else
										{
											looping = false;
										}
								}
							else
								{
									looping = false;
								}

						}		/* while (looping) */

				}		/* if (actual_length > 0) */

			*buffer_pp = new_buffer_p;
			return true;
		}

	*buffer_pp = NULL;
	return false;
}


void FreeLineBuffer (char *buffer_p)
{
	FreeMemory (buffer_p);
}


char *ConcatenateStrings (const char * const first_s, const char * const second_s)
{
	const size_t len1 = (first_s != NULL) ? strlen (first_s) : 0;
	const size_t len2 = (second_s != NULL) ? strlen (second_s) : 0;

	char *result_s = (char *) AllocMemory (sizeof (char) * (len1 + len2 + 1));

	if (result_s)
		{
			if (len1 > 0)
				{
					strncpy (result_s, first_s, len1);
				}

			if (len2 > 0)
				{
					strcpy (result_s + len1, second_s);
				}

			* (result_s + len1 + len2) = '\0';
		}

	return result_s;
}



/**
 * Parse the format string and sort out into a linked
 * list of tokens.
 *
 * @param format_p The format string to use.
 * @param treat_whitespace_as_delimiter_flag If true whitespace will be treated as a delimiter.
 * @return A pointer to a StringLinkedList where each node
 * refers to a token or NULL upon error.
 */
LinkedList *ParseStringToStringLinkedList (const char * const format_p, const char * const delimiters_p, const bool treat_whitespace_as_delimiter_flag)
{
	return (ParseStringToLinkedList (format_p, delimiters_p, AllocateStringLinkedList, GetStringListNode, treat_whitespace_as_delimiter_flag));
}


/**
 * Scroll through a string until we reach a charcter that satisfies some input constraints.
 *
 * @param state_flag If this is true then keep scrolling until we find some non-whitespace/delimiters, if false keep
 * scrolling until we find some whitespace/delimiters.
 */
static const char *ScrollPastSpecificCharacters (const char **value_pp, const char * const delimiters_s, const bool check_for_whitespace_flag, const bool state_flag)
{
	const char *value_s = *value_pp;
	const size_t delimiters_length = (delimiters_s != NULL) ? strlen (delimiters_s) : 0;
	bool is_delimiter_flag;
	bool loop_flag = (*value_s != '\0');

	while (loop_flag)
		{
			const char c = *value_s;

			is_delimiter_flag = false;

			/* Are we're checking for whitespace? */
			if (check_for_whitespace_flag)
				{
					is_delimiter_flag = (isspace (c) != 0);
				}		/* if (check_for_whitespace_flag) */

			/* if the whitespace check failed, test against our delimiters */
			if (!is_delimiter_flag)
				{
					/* If necessary, check for any delimiters */
					if (delimiters_length > 0)
						{
							size_t i = delimiters_length;
							const char *delim_p = delimiters_s;

							while (i > 0)
								{
									is_delimiter_flag = (*delim_p == c);

									if (is_delimiter_flag)
										{
											i = 0;
										}
									else
										{
											-- i;
											++ delim_p;
										}

								}		/* while (i > 0) */

						}		/* if (delimiters_length > 0) */

				}		/* if (!is_delimter_flag) */


			/* Are we continuing? */
			if (is_delimiter_flag == state_flag)
				{
					++ value_s;

					loop_flag = (*value_s != '\0');
				}
			else
				{
					loop_flag = false;
				}

		}		/* while (loop_flag) */

	return value_s;
}


/**
 * Parse the format string and sort out into a linked
 * list of items.
 *
 * @param format_p The format string to use.
 * @param allocate_list_fn The function used to create the LinkedList.

 * @return A pointer to a LinkedList where each node
 * refers to a token or NULL upon error.
 */
static LinkedList *ParseStringToLinkedList (const char * const format_p, const char * const delimiters_p, LinkedList *(*allocate_list_fn) (void), ListItem *(*allocate_node_fn) (const char *value_s), const bool treat_whitespace_as_delimiter_flag)
{
	LinkedList *tokens_list_p = NULL;
	const char *index_p = format_p;

	if (index_p)
		{
			tokens_list_p = allocate_list_fn ();

			if (tokens_list_p)
				{
					const char *ptr = format_p;
					bool loop_flag = true;
					bool error_flag = false;
					char *copy_p = NULL;

					while (loop_flag)
						{
							/* scroll past any delimiters */
							char *value_s = GetNextToken (&ptr, delimiters_p, treat_whitespace_as_delimiter_flag, true);

							if (value_s)
								{
									ListItem *node_p = allocate_node_fn (value_s);

									if (node_p)
										{
											LinkedListAddTail (tokens_list_p, node_p);

											/*
											ptr = strtok (NULL, delimiters_p);
											loop_flag = (ptr != NULL);
											 */
										}
									else
										{
											loop_flag = false;
											error_flag = true;
										}

									FreeCopiedString (value_s);
								}		/* if (value_s) */
							else
								{
									loop_flag = false;
								}

						}		/* while (loop_flag) */

					if (error_flag)
						{
							FreeLinkedList (tokens_list_p);
							tokens_list_p = NULL;
						}

					FreeMemory (copy_p);


					/* If the list is empty, don't return it */
					if (tokens_list_p && (tokens_list_p-> ll_size == 0))
						{
							FreeLinkedList (tokens_list_p);
							tokens_list_p = NULL;
						}

				}		/* if (tokens_list_p) */

		}		/* if (index_p) */

	return tokens_list_p;
}


static ListItem *GetStringListNode (const char *value_s)
{
	ListItem *node_p = (ListItem *) AllocateStringListNode (value_s, MF_DEEP_COPY);

	return node_p;
}



void ReplaceCharacter (char *value_s, const char char_to_replace, const char replacement_char)
{
	char *ptr = strchr (value_s, char_to_replace);

	while (ptr)
		{
			*ptr = replacement_char;
			ptr = strchr (ptr + 1, char_to_replace);
		}
}


/*
 * Taken from
 *
 * https://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
 *
 * You must free the result if result is non-NULL.
 */
bool SearchAndReplaceInString (const char *src_s, char **dest_ss, const char *to_replace_s, const char *with_s)
{
	bool success_flag = true;
	char *result_s = NULL; // the return string
//	int len_rep;  // length of rep (the string to remove)
//	int len_with; // length of with (the string to replace rep with)
	int len_front; // distance between rep and end of last rep

	// sanity checks and initialization
	if (!IsStringEmpty (src_s))
		{
			if (!IsStringEmpty (to_replace_s))
				{
					size_t to_replace_length = strlen (to_replace_s);
					size_t with_length;
					char *tmp_p;
					char *insert_p;    // the next insert point
					size_t count;    // number of replacements

					if (with_s)
						{
							with_length = strlen (with_s);
						}
					else
						{
							with_s = "";
							with_length = 0;
						}

					// count the number of replacements needed
					insert_p = (char *) src_s;
					for (count = 0; (tmp_p = strstr (insert_p, to_replace_s)) != NULL; ++ count)
						{
							insert_p = tmp_p + to_replace_length;
						}

					if (count > 0)
						{
							result_s = (char *) AllocMemory (strlen (src_s) + ((with_length - to_replace_length) * count) + 1);

							if (result_s)
								{
									tmp_p = result_s;

									// first time through the loop, all the variable are set correctly
									// from here on,
									//    tmp points to the end of the result string
									//    insert_p points to the next occurrence of rep in src_s
									//    src_s points to the remainder of src_s after "end of rep"
									while (count --)
										{
											insert_p = strstr (src_s, to_replace_s);
											len_front = insert_p - src_s;
											tmp_p = strncpy (tmp_p, src_s, len_front) + len_front;
											tmp_p = strcpy (tmp_p, with_s) + with_length;
											src_s += len_front + to_replace_length; // move to next "end of rep"
										}
									strcpy (tmp_p, src_s);

									*dest_ss = result_s;
								}
							else
								{
									success_flag = false;
								}

						}		/* if (count > 0) */

				}		/* if (!IsStringEmpty (to_replace_s)) */

		}		/* if (!IsStringEmpty (src_s)) */

	return success_flag;
}



bool GetKeyValuePair (char *line_p, char **key_pp, char **value_pp, const char comment_identifier)
{
	char *marker_p = strchr (line_p, comment_identifier);
	size_t l;
	bool success_flag = false;

	/* treat the # as a comment delimiter for the rest of the line */
	if (marker_p)
		{
			*marker_p = '\0';
		}

	*key_pp = NULL;
	*value_pp = NULL;

	l = strlen (line_p);
	if (l > 2)
		{
			marker_p = strtok (line_p, "=");

			if (marker_p)
				{
					char *key_p = CopyToNewString (marker_p, 0, true);

					if (key_p)
						{
							marker_p = strtok (NULL, "=");

							if (marker_p)
								{
									char *value_p = CopyToNewString (marker_p, 0, true);

									if (value_p)
										{
											*key_pp = key_p;
											*value_pp = value_p;
											success_flag = true;
										}
								}

							if (!success_flag)
								{
									FreeCopiedString (key_p);
								}
						}
				}
		}

	return success_flag;
}



/**
 * Get a copy of the next word from a buffer.
 *
 * @param buffer_p The buffer to read from.
 * @param end_pp If the word is successfully read, then this will
 * be where the orginal word  in the buffer ends.
 * @return A copied version of the string.
 */
char *GetNextToken (const char **start_pp, const char *delimiters_s, const bool check_for_whitespace_flag, const bool update_position_flag)
{
	char *word_s = NULL;
	const char *value_p = *start_pp;

	const char *start_p = ScrollPastSpecificCharacters (&value_p, delimiters_s, check_for_whitespace_flag, true);

	if (start_p && *start_p)
		{
			const char *end_p = NULL;
			bool quoted_token_flag = false;

			value_p = start_p;

			/* does it start with a " in which case we have a quoted string? */
			if (*start_p == '\"')
				{
					quoted_token_flag = true;

					++ value_p;

					end_p = ScrollPastSpecificCharacters (&value_p, "\"", false, false);
				}
			else
				{
					end_p = ScrollPastSpecificCharacters (&value_p, delimiters_s, check_for_whitespace_flag, false);
				}

			if (end_p)
				{

					/* scroll past the quotes  */
					if (quoted_token_flag)
						{
							++ start_p;
						}

					if (start_p != end_p)
						{
							const ptrdiff_t d = end_p - start_p;

							word_s = CopyToNewString (start_p, d, false);

							if (*end_p)
								{
									if (update_position_flag)
										{
											*start_pp = end_p + 1;
										}
								}
							else
								{
									*start_pp = end_p;
								}

						}		/* if (start_p != end_p) */

				}		/* if (end_p) */

		}		/* if (start_p) */

	return word_s;
}


void FreeToken (char *token_s)
{
	FreeMemory (token_s);
}



void NullifyTrailingZeroes (char *numeric_string_p)
{
	size_t l = strlen (numeric_string_p);
	char *c_p = numeric_string_p + (l - 1);

	bool loop_flag = (l > 0);

	while (loop_flag)
		{
			switch (*c_p)
			{
				case '0':
					*c_p = '\0';
					-- l;
					-- c_p;
					break;

				case '.':
					*c_p = '\0';
					loop_flag = false;
					break;

				default:
					loop_flag = false;
					break;

			}		/* switch (*c_p) */

		}		/* while (loop_flag) */
}


bool IsStringEmpty (const char *value_s)
{
	if (value_s)
		{
			while (*value_s)
				{
					if (isspace (*value_s))
						{
							++ value_s;
						}
					else
						{
							return false;
						}
				}
		}

	return true;
}




int Stricmp (const char *c0_p, const char *c1_p)
{
	int res = 0;

	if (c0_p != c1_p)
		{
			bool loop_flag = true;
			int i0;
			int i1;
			char c0;
			char c1;

			while (loop_flag)
				{
					c0 = tolower (*c0_p);
					c1 = tolower (*c1_p);

					if ((c0 == c1) && (c0 != '\0'))
						{
							++ c0_p;
							++ c1_p;
						}
					else
						{
							loop_flag = false;
						}
				}

			/* The comparison must be performed as if the characters were unsigned characters. */
			i0 = (unsigned char) c0;
			i1 = (unsigned char) c1;

			res = i0 - i1;
		}

	return res;
}


int Strnicmp (const char *c0_p, const char *c1_p, size_t length)
{
	int res = 0;

	if (c0_p != c1_p)
		{
			bool loop_flag = (length > 0);
			int i0;
			int i1;
			char c0 = 0;
			char c1 = 0;

			while (loop_flag)
				{
					c0 = tolower (*c0_p);
					c1 = tolower (*c1_p);

					if ((c0 == c1) && (c0 != '\0'))
						{
							++ c0_p;
							++ c1_p;

							-- length;
							if (length == 0)
								{
									loop_flag = false;
								}
						}
					else
						{
							loop_flag = false;
						}
				}

			/* The comparison must be performed as if the characters were unsigned characters. */
			i0 = (unsigned char) c0;
			i1 = (unsigned char) c1;

			res = i0 - i1;
		}

	return res;
}


const char *Stristr (const char *value_s, const char *substring_s)
{
	const char *res_p = NULL;

	if (value_s != substring_s)
		{
			const char *value_p = value_s;
			const char *substring_p = substring_s;

			while (*value_p && *substring_p)
				{
					/* Do they match? */
					if (tolower (*value_p) == tolower (*substring_p))
						{
							if (!res_p)
								{
									res_p = value_p;
								}

							++ substring_p;
						}
					else
						{
							substring_p = substring_s;

							if (tolower (*value_p) == tolower (*substring_p))
								{
									res_p = value_p;
									++ substring_p;
								}
							else
								{
									res_p = NULL;
								}
						}

					++ value_p;
				}		/* while (*value_p && *substring_p) */
		}
	else
		{
			res_p = value_s;
		}

	return res_p;
}



bool DoStringsMatch (const char * const val_0_s, const char * const val_1_s)
{
	bool match_flag = false;

	if (val_0_s)
		{
			if (val_1_s)
				{
					if (strcmp (val_0_s, val_1_s) == 0)
						{
							match_flag = true;
						}
				}
		}
	else
		{
			if (!val_1_s)
				{
					match_flag = true;
				}
		}

	return match_flag;
}


char *Strrstr (const char *haystack_s, const char *needle_s)
{
	char *res_s = NULL;
	const size_t needle_length = strlen (needle_s);
	const size_t haystack_length = strlen (haystack_s);

	if (needle_length <= haystack_length)
		{
			const char *search_s = haystack_s + haystack_length - needle_length;

			while ((search_s >= haystack_s) && (res_s == NULL))
				{
					if (strncmp (search_s, needle_s, needle_length) == 0)
						{
							res_s = (char *) search_s;
						}
					else
						{
							-- search_s;
						}
				}
		}

	return res_s;
}



bool ReplaceStringValue (char **dest_ss, const char * const value_s)
{
	bool success_flag = false;
	char *new_value_s = EasyCopyToNewString (value_s);

	if (new_value_s)
		{
			if (*dest_ss)
				{
					FreeCopiedString (*dest_ss);
				}

			*dest_ss = new_value_s;
			success_flag = true;
		}

	return success_flag;
}



void ReplaceChars (char *value_s, char old_data, char new_data)
{
	char *value_p = value_s;

	while (*value_p != '\0')
		{
			if (*value_p == old_data)
				{
					*value_p = new_data;
				}

			++ value_p;
		}
}


uint32 HashString (const void * const key_p)
{
	uint32 res = 0;

	if (key_p)
		{
			const char *c_p = (const char *) key_p;

			while (*c_p)
				{
					res = (res << 1) ^ *c_p ++;
				}
		}

	return res;
}


char *ConcatenateVarargsStrings (const char *value_s, ...)
{
	char *result_s = NULL;
	ByteBuffer *buffer_p = AllocateByteBuffer (1024);

	if (buffer_p)
		{
			va_list args;

			va_start (args, value_s);

			if (AppendVarArgsToByteBuffer (buffer_p, value_s, args))
				{
					result_s = DetachByteBufferData (buffer_p);
				}		/* if (AppendVarArgsToByteBuffer (buffer_p, value_s, args)) */

			va_end (args);
		}		/* if (buffer_p) */

	return result_s;
}


char *GetFileContentsAsString (FILE *input_f)
{
	char *data_s = NULL;
	long int current_pos = ftell (input_f);

	// Determine file size
	fseek (input_f, 0, SEEK_END);
	long int size = ftell (input_f);

	if (size != -1)
		{
			data_s = (char *) AllocMemory ((size + 1) * sizeof (char));

			if (data_s)
				{
					size_t read_size = 0;

					rewind (input_f);

					read_size = fread (data_s, sizeof (char), size, input_f);

					if (read_size == (size_t) size)
						{
							* (data_s + size) = '\0';

							return data_s;
						}		/* if (read_size == size) */
					else
						{
							PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to read all of file " SIZET_FMT " / " SIZET_FMT " bytes ", read_size, (size_t) size);
						}

					fseek (input_f, current_pos, SEEK_SET);

					FreeMemory (data_s);
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to allocate " SIZET_FMT " bytes ", (size_t) size);
				}
		}
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to determine file size");
		}

	return NULL;
}



char *GetFileContentsAsStringByFilename (const char *filename_s)
{
	char *data_s = NULL;

	if (filename_s)
		{
			FILE *in_f = fopen (filename_s, "r");

			if (in_f)
				{
					int res = 0;

					data_s = GetFileContentsAsString (in_f);

					res = fclose (in_f);

					if (res)
						{
							PrintErrors (STM_LEVEL_WARNING, __FILE__, __LINE__, "Error %d closing %s", res, filename_s);
						}

				}		/* if (in_f) */
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to open file %s", filename_s);
				}

		}		/* if (filename_s) */
	else
		{
			PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Filename is NULL");
		}

	return data_s;
}



bool DoesStringContainWhitespace (const char *value_s)
{
	while (*value_s != '\0')
		{
			if (isspace (*value_s))
				{
					return true;
				}
			else
				{
					++ value_s;
				}
		}

	return false;
}


bool DoesStringEndWith (const char *value_s, const char *ending_s)
{
	bool matching_ends_flag = false;
	const size_t value_length = strlen (value_s);
	const size_t ending_length = strlen (ending_s);

	if (value_length >= ending_length)
		{
			const size_t offset = value_length - ending_length;

			value_s += offset;

			if (strcmp (value_s, ending_s) == 0)
				{
					matching_ends_flag = true;
				}
		}

	return matching_ends_flag;
}



bool DoesStringStartWith (const char *value_s, const char *start_s)
{
	bool matching_ends_flag = false;
	const size_t value_length = strlen (value_s);
	const size_t start_length = strlen (start_s);

	if (value_length >= start_length)
		{
			if (strncmp (value_s, start_s, start_length) == 0)
				{
					matching_ends_flag = true;
				}
		}

	return matching_ends_flag;
}


char *GetStringAsLowerCase (const char *value_s)
{
	char *lower_s = NULL;

	if (value_s)
		{
			lower_s = EasyCopyToNewString (value_s);

			if (lower_s)
				{
					char *c_p = lower_s;

					while (*c_p != '\0')
						{
							*c_p = tolower (*c_p);
							++ c_p;
						}
				}
		}

	return lower_s;

}


void FreeStringArray (char **values_ss, const size_t num_values)
{
	size_t i;
	char **value_ss = values_ss;

	if (num_values > 0)
		{
			for (i = num_values; i > 0; -- i, ++ value_ss)
				{
					if (*value_ss)
						{
							FreeCopiedString (*value_ss);
						}
				}
		}
	else
		{
			while (*value_ss)
				{
					FreeCopiedString (*value_ss);
					++ value_ss;
				}
		}

	FreeMemory (values_ss);
}


bool CopyAndAddStringValue (const char * const src_s, char **dest_ss)
{
	bool success_flag = false;

	if (src_s)
		{
			char *copied_value_s = EasyCopyToNewString (src_s);

			if (copied_value_s)
				{
					*dest_ss = copied_value_s;
					success_flag = true;
				}
			else
				{
					PrintErrors (STM_LEVEL_SEVERE, __FILE__, __LINE__, "Failed to copy \"%s\"", src_s);
				}
		}
	else
		{
			*dest_ss = NULL;
			success_flag = true;
		}

	return success_flag;
}


char **CopyStringArray (const char **src_array_ss, const size_t num_values)
{
	char **dest_array_ss = (char **) AllocMemoryArray (num_values, sizeof (char *));

	if (dest_array_ss)
		{
			char **dest_ss = dest_array_ss;
			bool success_flag = true;
			const char **src_ss = src_array_ss;
			size_t i = num_values;

			while (success_flag && (i > 0))
				{
					if (*src_ss)
						{
							char *dest_s = EasyCopyToNewString (*src_ss);

							if (dest_s)
								{
									*dest_ss = dest_s;
								}
							else
								{
									success_flag = false;
								}
						}
					else
						{
							*dest_ss = NULL;
						}

					if (success_flag)
						{
							++ dest_ss;
							++ src_ss;
							-- i;
						}
				}

			if (success_flag)
				{
					return dest_array_ss;
				}
			else
				{
					FreeStringArray (dest_array_ss, num_values);
				}

		}		/* if (dest_ss) */

	return NULL;
}
