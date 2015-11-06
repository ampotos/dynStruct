#!/bin/sh

rm -rf build

mkdir build
cd build

if [ $# -eq 1 ] && [ $1 -eq 32 ]
then
    CFLAGS="-W -Wall -Wextra -std=gnu99 -nostdlib -fno-builtin -O3 -m32 -DBUILD_32" CXXFLAGS=-m32 cmake -DDynamoRIO_DIR=$DYNAMORIO_HOME/cmake ../ && make && cp libdynStruct.so ../dynStruct
else
    CFLAGS="-W -Wall -Wextra -std=gnu99 -nostdlib -fno-builtin -O3 -DBUILD_64" cmake -DDynamoRIO_DIR=$DYNAMORIO_HOME/cmake ../ && make && cp libdynStruct.so ../dynStruct
fi
