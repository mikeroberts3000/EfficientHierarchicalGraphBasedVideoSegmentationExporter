cmake_minimum_required(VERSION 2.6)

project(segment_util)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../cmake")
include(${CMAKE_MODULE_PATH}/common.cmake)
include("${CMAKE_SOURCE_DIR}/depend.cmake")

set(SOURCES segmentation_io.cpp
	    segmentation_util.cpp)

headers_from_sources_cpp(HEADERS "${SOURCES}")
set(SOURCES "${SOURCES}" "${HEADERS}")

# Custom build steps.
WRAP_PROTO(PROTO_SOURCES segmentation.proto)

add_library(segment_util ${SOURCES} ${PROTO_SOURCES})

include_directories(${PROTOBUF_INCLUDE_DIR})

apply_dependencies(segment_util)
