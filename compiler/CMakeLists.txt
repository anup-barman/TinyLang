cmake_minimum_required(VERSION 3.10)
project(TinyLangCompiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")

add_executable(tinylang-compiler ${SOURCES})

# Ensure we can include headers from src
target_include_directories(tinylang-compiler PRIVATE src)

# Add -O2 by default for release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
