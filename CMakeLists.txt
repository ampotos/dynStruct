cmake_minimum_required(VERSION 2.8)

add_library(dynStruct SHARED dynStruct.c utils.c block_utils.c)
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)
use_DynamoRIO_extension(dynStruct drwrap)
configure_DynamoRIO_client(dynStruct)