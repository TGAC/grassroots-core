#!/bin/sh

gcc -shared -fPIC -Wl,-soname,libsqlite.so -o libsqlite3.so sqlite3.c -lc

