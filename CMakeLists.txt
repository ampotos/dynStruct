cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "-W -Wall -Wextra -std=gnu99")

add_library(dynStruct SHARED src/dynStruct.c src/utils.c src/block_utils.c src/allocs.c)
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)
use_DynamoRIO_extension(dynStruct drutil)
use_DynamoRIO_extension(dynStruct drwrap)
use_DynamoRIO_extension(dynStruct drmgr)
configure_DynamoRIO_client(dynStruct)