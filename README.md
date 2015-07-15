# dynStruct
dynStruct is a tool using dynamoRio to recover data
structure of a ELF binary at runtime.

It will be made of 2 different tools :
* a client for dynamoRio (the data collector)
* a webui to visualize the data

The collector will create a file with raw data.
The webui will get this file and produce a database with guessing structure on it.

This database may be modified via the webui.

## Requirements
* CMake >= 2.8
* [DynamoRIO](https://github.com/DynamoRIO/dynamorio)

## Setup
Set the environment variable DYNAMORIO_HOME to the absolute path of your
DynamoRIO installation

execute `setup.sh`

## Usage
* collector :
`drrun -c dynStruct -- <binary> <arg 1> <arg 2> ...`
