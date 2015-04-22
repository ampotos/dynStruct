#!/bin/sh

rm -rf build

mkdir build
cd build
cmake -DDynamoRIO_DIR=$DYNAMORIO_HOME/cmake ../ && make && cp libdynStruct.so ../dynStruct

