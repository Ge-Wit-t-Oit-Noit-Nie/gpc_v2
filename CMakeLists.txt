cmake_minimum_required(VERSION 3.22)

project(GPC
    VERSION 1.0
    DESCRIPTION "GWTONN Program Compiler"
    LANGUAGES C
) 

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_BUILD_TYPE Debug)

IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Build type: Debug")
    #set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -Werror -g3 -O0")
ELSEIF(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Build type: RELEASE")
    set(CMAKE_C_FLAGS "-O3")
ENDIF()

# Add external libraries
add_subdirectory("${PROJECT_SOURCE_DIR}/lib/clogger")

add_subdirectory(src)