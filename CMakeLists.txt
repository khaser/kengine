cmake_minimum_required(VERSION 3.6)
project(kengine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Ofast")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O0")
endif()

set(TARGET_NAME "${PROJECT_NAME}")

file(GLOB_RECURSE SRCS "src/*.cpp")

find_package(OpenMP)

add_executable(${TARGET_NAME} ${SRCS})
target_include_directories(${TARGET_NAME} PUBLIC "include")
target_link_libraries(${TARGET_NAME} PUBLIC)
if (OpenMP_CXX_FOUND)
    target_link_libraries(${TARGET_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif()
