# SQLitelibrary {#sqlite_library}

The MongoDB library allows the Grassroots Server to use the functionality provided by [SQLite](https://sqlite.org/). 

## Installation

To build this library, you need the [grassroots core](https://github.com/TGAC/grassroots-core) and [grassroots build config](https://github.com/TGAC/grassroots-build-config) installed and configured. 

The files to build the Grassroots SQLite library are in the ```build/<platform>``` directory. 

### Linux

Enter the build directory 

```
cd build/linux
```

and

```
make all
```

and then 

```
make install
```

to install the library into the Grassroots system where it will be available on the server application after it has been restarted.
