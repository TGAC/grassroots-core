#!/bin/sh

gcc -shared -fPIC -o libsqlite3.so sqlite3.c -lc

