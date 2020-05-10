# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.1)
# CMAKE_POLICY(SET CMP0072 NEW)
# Project name
project(ivs)

set(CMAKE_CXX_STANDARD 17)  # enable C++11 standard
set(CMAKE_BUILD_TYPE "Debug")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

file(GLOB all_SRCS
  "${PROJECT_SOURCE_DIR}/src/*.c"
  "${PROJECT_SOURCE_DIR}/src/*.h"
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
  "${PROJECT_SOURCE_DIR}/src/*.hpp"
)

add_executable(ivs ${all_SRCS})

find_package(CGAL REQUIRED COMPONENTS Core)
find_package(Cairo REQUIRED)
find_package(glm REQUIRED)

target_link_libraries(ivs ${CAIRO_LIBRARIES})
target_link_libraries(ivs CGAL::CGAL CGAL::CGAL_Core)

include_directories(
  ${PROJECT_SOURCE_DIR}/src
  ${CAIRO_INCLUDE_DIRS}
  ${GLM_INCLUDE_DIRS}
)



target_precompile_headers(ivs PRIVATE "<cairo.h>" "src/imports.hpp")
