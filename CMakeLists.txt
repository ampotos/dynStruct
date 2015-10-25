cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "-W -Wall -Wextra -std=gnu99 -nostdlib -fno-builtin -O3")

add_library(dynStruct SHARED src/dynStruct.c src/utils.c src/block_utils.c src/allocs.c src/rw.c src/out.c src/call.c src/sym.c src/tree.c src/elf.c src/args.c src/out_json.c)
find_package(DynamoRIO)
if (NOT DynamoRIO_FOUND)
  message(FATAL_ERROR "DynamoRIO package required to build")
endif(NOT DynamoRIO_FOUND)
use_DynamoRIO_extension(dynStruct drutil)
use_DynamoRIO_extension(dynStruct drwrap)
use_DynamoRIO_extension(dynStruct drmgr)
use_DynamoRIO_extension(dynStruct drsyms)
use_DynamoRIO_extension(dynStruct drcontainers)


set(DynamoRIO_USE_LIBC OFF)
configure_DynamoRIO_client(dynStruct)
