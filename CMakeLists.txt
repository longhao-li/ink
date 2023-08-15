cmake_minimum_required(VERSION 3.16)

project(ink VERSION 0.0.1 LANGUAGES CXX)

# Use C++17 if this is not built as a subproject.
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "C++17 or newer standard is required.")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

option(INK_BUILD_STATIC_LIBS "Build static libraries." ON)
option(INK_BUILD_SHARED_LIBS "Build shared libraries." ON)
option(INK_BUILD_EXAMPLES "Build examples." OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory("ink")
if(INK_BUILD_EXAMPLES)
    add_subdirectory("examples")
endif()
