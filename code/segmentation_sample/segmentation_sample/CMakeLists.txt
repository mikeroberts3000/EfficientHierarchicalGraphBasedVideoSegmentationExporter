cmake_minimum_required(VERSION 2.6)

project(segmentation_sample)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
include(${CMAKE_MODULE_PATH}/common.cmake)
include("${CMAKE_SOURCE_DIR}/depend.cmake")

set(SOURCES main.cpp)
headers_from_sources_cpp(HEADERS "${SOURCES}")
set(SOURCES "${SOURCES}" "${HEADERS}")

add_executable(segmentation_sample ${SOURCES})

apply_dependencies(segmentation_sample)
