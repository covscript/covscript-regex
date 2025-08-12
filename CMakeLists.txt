cmake_minimum_required(VERSION 3.10)

project(covscript-regex)

if(DEFINED ENV{CS_DEV_PATH})
    include_directories($ENV{CS_DEV_PATH}/include)
    link_directories($ENV{CS_DEV_PATH}/lib)
endif()

if(DEFINED ENV{CS_DEV_OUTPUT})
    set(LIBRARY_OUTPUT_PATH $ENV{CS_DEV_OUTPUT})
    set(EXECUTABLE_OUTPUT_PATH $ENV{CS_DEV_OUTPUT})
endif()

# Compiler Options
set(CMAKE_CXX_STANDARD 14)

if (MSVC)
    set(CMAKE_CXX_FLAGS "/O2 /EHsc /utf-8 /w")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif (CMAKE_COMPILER_IS_GNUCXX)
    if (WIN32)
        set(CMAKE_C_FLAGS "--static -fPIC -s -O3")
        set(CMAKE_CXX_FLAGS "--static -fPIC -s -O3")
    else ()
        set(CMAKE_C_FLAGS "-fPIC -s -O3")
        set(CMAKE_CXX_FLAGS "-fPIC -s -O3")
    endif ()
else ()
    set(CMAKE_C_FLAGS "-fPIC -O3")
    set(CMAKE_CXX_FLAGS "-fPIC -O3")
endif ()

add_library(regex SHARED regex.cpp)
add_library(unicode SHARED unicode.cpp)

target_link_libraries(regex covscript)
target_link_libraries(unicode covscript)

set_target_properties(regex PROPERTIES OUTPUT_NAME regex)
set_target_properties(regex PROPERTIES PREFIX "")
set_target_properties(regex PROPERTIES SUFFIX ".cse")

set_target_properties(unicode PROPERTIES OUTPUT_NAME unicode)
set_target_properties(unicode PROPERTIES PREFIX "")
set_target_properties(unicode PROPERTIES SUFFIX ".cse")