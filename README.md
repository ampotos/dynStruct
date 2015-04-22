# dynStruct
dynStruct is a first poc for a tool using dynamoRio to recover internal structure of a binary.

# Setup
Set the environment variable DYNAMORIO_HOME to the absolute path of your DynamoRIO installation
run setup.sh

# Usage
drrun -c dynStruct -- <binary>

# Requirements
CMake >= 2.8
DynamoRIO