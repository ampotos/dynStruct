# dynStruct
dynStruct is a in development tool using dynamoRio to recover internal
structure of a binary.

## Setup
Set the environment variable DYNAMORIO_HOME to the absolute path of your
DynamoRIO installation

execute `setup.sh`

## Usage
`drrun -c dynStruct -- <binary> <arg 1> <arg 2> ...`

## Requirements
* CMake >= 2.8
* [DynamoRIO](https://github.com/DynamoRIO/dynamorio=

## TODO
* monitoring all read/write on the heap
* recover struct members
* create en html page
* display struct
* add alloc/free fonction for the structs
* display acess read/write for each elem