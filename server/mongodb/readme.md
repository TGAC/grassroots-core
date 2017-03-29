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


