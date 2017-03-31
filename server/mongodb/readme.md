# MongoDB library

The MongoDB library allows the Grassroots Server to use the functionality provided by [MongoDB](https://www.mongodb.com/). The library uses the [C language driver](https://github.com/mongodb/mongo-c-driver).

## Installation

To build this service, you need the [grassroots core](https://github.com/TGAC/grassroots-core) and [grassroots build config](https://github.com/TGAC/grassroots-build-config) installed and configured. 

The files to build the MongoDB library are in the ```build/<platform>``` directory. 

### Linux

Enter the build directory 

```cd build/linux```

and create a *user.prefs* file.

```cp example-user.prefs user.prefs```

You will need to edit this file to configure where the MongoDB library dependencies are stored. The file content is similar to the following

``` 
#
# mongodb dependencies
#

# Set this to where you have the mongo-c-driver directory 
# containing "include" and "lib" subdirectories.
export MONGODB_HOME := $(DIR_GRASSROOTS_EXTRAS)/mongodb-c
export BSON_HOME := $(DIR_GRASSROOTS_EXTRAS)/mongodb-c
```

Adjust the ```MONGODB_HOME``` and ```BSON_HOME``` variables to where you have the mongo-c-driver installed. You can then build the handler by typing

```make all```

and then 

```make install```

to install the library into the Grassroots system where it will be available on the server application after it has been restarted.


﻿## Usage

One of the facilities that Grassroots system provides is an API to access a [MongoDB](https://www.mongodb.org/) instance through JSON-based messages. All of the MongoDB messages are accessed using a **mongodb** key. 


### Inserting data

The client would send a child **insert** key with a list of child key-value pairs. These can be in a single object if you just want to insert a single document or as an array if you want to insert multiple documents. For example, both of the following requests are valid:

~~~{.json}
{
	"mongodb": {
		"collection": "bands",
		"insert": [{
			"band": "spinal tap",
			"volume": 11,
			"recorded in": "dobly"		
		}, {
			"band": "bad news",
			"label": "frilly pink"
		}]	
	}
}
~~~

~~~{.json}
{
	"mongodb": {
		"collection": "bands",
		"insert": {
			"band": "spinal tap",
			"volume": 11,
			"recorded in": "dobly"		
		}
	}
}
~~~

The response from the Server will be a list of **success** values along with the document ids in the same order as they were specified in the incoming request 

~~~{.json}
{
	"mongodb": {
		"response": [{
			"success": true,
			"id": "123456789012345678901234"
		}, {
			"success": false
		}]
	}
}
~~~

### Deleting documents

To delete documents from a collection, a Client would send a list of the queries that would specify which documents to remove.

~~~{.json}
{
	"mongodb": {
		"collection": "bands",
		"delete": [{
			"id": "123456789012345678901234"
		}, {
			"id": "1112223334445556667778889"
		}, {
			"crop": "wheat"
		}]
	}
}
~~~

### Updating documents

Documents that were previously added can be subsequently updated.

