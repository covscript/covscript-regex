cmake_minimum_required(VERSION 3.16)

project(covscript-regex)

if (DEFINED ENV{CS_DEV_PATH})
    message("-- CovScript SDK detected at $ENV{CS_DEV_PATH}")
    include("$ENV{CS_DEV_PATH}/csbuild.cmake")
    include_directories($ENV{CS_DEV_PATH}/include)
    link_directories($ENV{CS_DEV_PATH}/lib)
else ()
    message(FATAL_ERROR "-- CovScript SDK not detected. Please set environment variable CS_DEV_PATH")
endif ()

set(PCRE2_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_STATIC ON CACHE BOOL "" FORCE)

set(PCRE2_SUPPORT_JIT ON CACHE BOOL "" FORCE)

set(PCRE2_BUILD_PCRE2_8 ON CACHE BOOL "" FORCE)
set(PCRE2_BUILD_PCRE2_16 OFF CACHE BOOL "" FORCE)
set(PCRE2_BUILD_PCRE2_32 ON  CACHE BOOL "" FORCE)

add_subdirectory(pcre2 EXCLUDE_FROM_ALL)

add_library(regex SHARED regex.cpp)
add_library(unicode SHARED unicode.cpp)

target_link_libraries(regex covscript pcre2-8)
target_link_libraries(unicode covscript pcre2-32)

set_target_properties(regex PROPERTIES OUTPUT_NAME regex)
set_target_properties(regex PROPERTIES PREFIX "")
set_target_properties(regex PROPERTIES SUFFIX ".cse")

set_target_properties(unicode PROPERTIES OUTPUT_NAME unicode)
set_target_properties(unicode PROPERTIES PREFIX "")
set_target_properties(unicode PROPERTIES SUFFIX ".cse")