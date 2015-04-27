cmake_minimum_required(VERSION 2.8)

add_library(dynStruct SHARED src/dynStruct.c src/utils.c src/block_utils.c)
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)
use_DynamoRIO_extension(dynStruct drwrap drmgr)
configure_DynamoRIO_client(dynStruct)