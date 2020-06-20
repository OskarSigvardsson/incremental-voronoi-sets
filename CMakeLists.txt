cmake_minimum_required(VERSION 3.9.4)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_BUILD_TYPE "Release")

project(ivs)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

if(NOT MSVC) 
    add_compile_options(-Wall -Werror)
endif()

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

if (CMAKE_BUILD_TYPE MATCHES RELEASE)
  set_property(TARGET ivs PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

#target_precompile_headers(ivs PRIVATE "<cairo.h>" "src/main.hpp")
