cmake_minimum_required(VERSION 2.6)

project(assert_log)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
include(${CMAKE_MODULE_PATH}/common.cmake)
include("${CMAKE_SOURCE_DIR}/depend.cmake")

set(SOURCES assert_log.cpp)
headers_from_sources_cpp(HEADERS "${SOURCES}")

set(SOURCES "${SOURCES}" "${HEADERS}")

add_library(assert_log ${SOURCES})

apply_dependencies(assert_log)
