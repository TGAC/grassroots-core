/*
** Copyright 2014-2018 The Earlham Institute
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
 * mongodb_tool_test.c
 *
 *  Created on: 23 Sep 2018
 *      Author: billy
 */

#include <stdio.h>

#include "mongodb_tool.h"


static const char * const S_ID_COLUMN_S = "id";
static const char * const S_SURNAME_COLUMN_S = "surname";
static const char * const S_FORENAME_COLUMN_S = "forename";
static const char * const S_HEIGHT_COLUMN_S = "height";
static const char * const S_DESCRIPTION_COLUMN_S = "description";


static bson_t *GetBSONDataToInsert (bson_oid_t *id_p);

static json_t *GetDataToInsert (void);

static bson_t *GetBSONDataToUpdate (bson_oid_t *id_p);

static bson_t *GetBSONCommandToRun (const char *collection_s);


static bool AddData (json_t *values_array_p, const char *id_s, const char *surname_s, const char *forename_s, const double64 *height_p, const char *description_s, json_t *where_values_p);

static void PrintJSON (const json_t *data_p, const char *message_s);

static void PrintBSON (const bson_t *doc_p, const char *message_s);



int main (int argc, char *argv [])
{
	if (argc == 3)
		{
			mongoc_init ();

	    mongoc_client_t *client_p = mongoc_client_new ("mongodb://localhost:27017/?appname=grassroots-mongodb-tool-test");

	    if (client_p)
	    	{
	  			MongoTool *tool_p = AllocateMongoTool (client_p);

	  			if (tool_p)
	  				{
	  					if (SetMongoToolDatabaseAndCollection (tool_p, argv [1], argv [2]))
	  						{
	  							bson_t *bson_doc_p = NULL;
	  							json_t *values_p = GetDataToInsert ();
	  							bson_oid_t id;

	  							bson_t *update_p = BCON_NEW ("$set", "{", "x", BCON_UTF8 ("bar"), "}");
	  							if (update_p)
	  								{
	  									PrintBSON (update_p, "Initial update: ");

	  									bson_destroy (update_p);
	  								}


	  							if (values_p)
	  								{
	  									size_t index;
	  									json_t *doc_p;

	  									json_array_foreach (values_p, index, doc_p)
	  										{
	  											bson_t *reply_p = NULL;

													PrintJSON (doc_p, "Doc before: ");

	  											if (InsertMongoData (tool_p, doc_p, &reply_p))
	  												{
	  													PrintBSON (reply_p, "Reply: ");

	  													bson_destroy (reply_p);
	  												}
	  											else
	  												{
	  													PrintBSON (reply_p, "Failed to insert: ");
	  												}

													PrintJSON (doc_p, "Doc after: ");
	  										}

	  									json_decref (values_p);
	  								}		/* if (values_p) */

	  							if ((bson_doc_p = GetBSONDataToInsert (&id)) != NULL)
	  								{
											bson_t *reply_p = NULL;

											PrintBSON (bson_doc_p, "BSON before insert: ");

											if (InsertMongoDataAsBSON (tool_p, bson_doc_p, &reply_p))
												{
													PrintBSON (reply_p, "Reply: ");

													bson_destroy (reply_p);
												}
											else
												{
													PrintBSON (reply_p, "Failed to insert: ");
												}

											PrintBSON (bson_doc_p, "BSON after: ");
	  								}		/* if ((bson_doc_p = GetBSONDataToInsert (&id)) != NULL) */


	  							if ((bson_doc_p = GetBSONDataToUpdate (&id)) != NULL)
										{
	  									bson_t *reply_p = NULL;
	  									PrintBSON (bson_doc_p, "BSON before update : ");
	  									if (UpdateMongoDataAsBSONForGivenId (tool_p, &id, bson_doc_p, &reply_p))
	  										{
	  											PrintBSON (reply_p, "Reply: ");

	  											bson_destroy (reply_p);
	  										}
	  									else
	  										{
	  											PrintBSON (reply_p, "Failed to update: ");
	  										}
	  									PrintBSON (bson_doc_p, "BSON after update : ");

	  									bson_destroy (bson_doc_p);
										}		/* if ((bson_doc_p = GetBSONDataToUpdate (&id)) != NULL) */


	  							if ((bson_doc_p = GetBSONCommandToRun (argv [2])) != NULL)
	  								{
											bson_t *reply_p = NULL;

											PrintBSON (bson_doc_p, "BSON before: ");
	  									if (RunMongoCommand (tool_p, bson_doc_p, &reply_p))
	  										{
													PrintBSON (reply_p, "Reply: ");

													bson_destroy (reply_p);
	  										}
	  									else
	  										{
													PrintBSON (reply_p, "Failed to run: ");
	  										}

	  									PrintBSON (bson_doc_p, "BSON after: ");

	  									bson_destroy (bson_doc_p);
	  								}

	  						}		/* if ((bson_doc_p = GetBSONCommandToRun (argv [2])) != NULL) */


	  					FreeMongoTool (tool_p);
	  				}		/* if (tool_p) */
	  			else
	  				{
	  					puts ("Failed to allocate MongoTool");
	  				}

	  			mongoc_client_destroy (client_p);
 	    	}		/* if (client_p) */


	    mongoc_cleanup ();
			puts ("exiting");

		}		/* if (argc == 3) */
	else
		{
			printf ("usage: mongodb_test_tool <test_database> <test_collection>, argc = \%d", argc);
		}

	return 0;
}


static void PrintJSON (const json_t *data_p, const char *message_s)
{
	char *dump_s = json_dumps (data_p, 0);

	if (dump_s)
		{
			printf ("%s \"%s\"\n", message_s, dump_s);
			free (dump_s);
		}
	else
		{
			printf ("%s NULL\n", message_s);
		}
}


static void PrintBSON (const bson_t *doc_p, const char *message_s)
{
  char *dump_s = bson_as_canonical_extended_json (doc_p, NULL);

	if (dump_s)
		{
			printf ("%s \"%s\"\n", message_s, dump_s);
		  bson_free (dump_s);
		}
	else
		{
			printf ("%s NULL\n", message_s);
		}
}


static bson_t *GetBSONCommandToRun (const char *collection_s)
{
	bson_t *bson_command_p = NULL;
	json_t *command_p = json_object ();

	if (command_p)
		{
			if (json_object_set_new (command_p, "insert", json_string (collection_s)) == 0)
				{
					json_t *docs_p = json_array ();

					if (docs_p)
						{
							if (json_object_set_new (command_p, "documents", docs_p) == 0)
								{
									json_t *doc_p = json_object ();

									if (doc_p)
										{
											if (json_array_append_new (docs_p, doc_p) == 0)
												{
													if (json_object_set_new (doc_p, "foo", json_string ("bar")) == 0)
														{
															bson_command_p = ConvertJSONToBSON (command_p);

														}
												}
											else
												{
													json_decref (doc_p);
												}
										}

								}
							else
								{
									json_decref (docs_p);
								}
						}

				}


			json_decref (command_p);
		}

	return bson_command_p;
}


static bson_t *GetBSONDataToInsert (bson_oid_t *id_p)
{
	bson_t *doc_p = bson_new ();

	if (doc_p)
		{
			if (BSON_APPEND_UTF8 (doc_p, "hello", "world"))
				{
					bson_oid_init (id_p, NULL);

					if (BSON_APPEND_OID (doc_p, "_id", id_p))
						{
							return doc_p;
						}
				}

			bson_destroy (doc_p);
		}

	return NULL;
}


static bson_t *GetBSONDataToUpdate (bson_oid_t *id_p)
{
	bson_t *doc_p = bson_new ();

	if (doc_p)
		{
			if (BSON_APPEND_UTF8 (doc_p, "hello", "you"))
				{
					return doc_p;
				}

			bson_destroy (doc_p);
		}

	return NULL;
}


static json_t *GetDataToInsert (void)
{
	json_t *values_p = json_array ();

	if (values_p)
		{
			double64 height = 1.80;

			if (AddData (values_p, NULL, "Fish", "Billy", &height, NULL, NULL))
				{
					height = 2.860;

					if (AddData (values_p, NULL, "Donkey", "Wonky", &height, "Oh to be a horse", NULL))
						{
							height = 1.230;

							if (AddData (values_p, NULL, "Bag", "Bean", &height, "Woof!", NULL))
								{
									return values_p;
								}
						}
				}

			json_decref (values_p);
		}

	return NULL;
}



static bool AddData (json_t *values_array_p, const char *id_s, const char *surname_s, const char *forename_s, const double64 *height_p, const char *description_s, json_t *where_values_p)
{
	json_t *entry_p = json_object ();

	if (entry_p)
		{
			if ((id_s == NULL) || (json_object_set_new (entry_p, S_ID_COLUMN_S, json_string (id_s)) == 0))
				{
					if ((surname_s == NULL) || (json_object_set_new (entry_p, S_SURNAME_COLUMN_S, json_string (surname_s)) == 0))
						{
							if ((forename_s == NULL) || (json_object_set_new (entry_p, S_FORENAME_COLUMN_S, json_string (forename_s)) == 0))
								{
									if ((height_p == NULL) || (json_object_set_new (entry_p, S_HEIGHT_COLUMN_S, json_real (*height_p)) == 0))
										{
											if ((description_s == NULL) || (json_object_set_new (entry_p, S_DESCRIPTION_COLUMN_S, json_string (description_s)) == 0))
												{
													if (json_array_append_new (values_array_p, entry_p) == 0)
														{
															return true;
														}
												}
										}
								}
						}
				}

			json_decref (entry_p);
		}

	return false;
}
