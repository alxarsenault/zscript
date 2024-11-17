cmake_minimum_required(VERSION 3.22.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_XCODE_GENERATE_SCHEME ON)

set(ZSCRIPT_BIN_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(ZSCRIPT_DOC_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc")
set(ZSCRIPT_TOOLS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tools")
set(ZSCRIPT_CMAKE_TOOLS_DIRECTORY "${ZSCRIPT_TOOLS_DIRECTORY}/cmake")
set(ZSCRIPT_SAMPLES_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/samples")
set(ZSCRIPT_EXAMPLES_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples")
set(ZSCRIPT_INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(ZSCRIPT_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")

list(APPEND CMAKE_MODULE_PATH "${ZSCRIPT_CMAKE_TOOLS_DIRECTORY}")
include(zscript-utils)
include(zscript-copyright)

# # Parse version file.
# set(WP_VERSION_FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/version.txt")
# wp_parse_version(${WP_VERSION_FILE_PATH})
project(zs
  VERSION 0.0.1
  LANGUAGES C CXX)

# message(STATUS "wp version: ${WP_VERSION_MAJOR}.${WP_VERSION_MINOR}.${WP_VERSION_PATCH}")
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(IS_MULTI_CONFIG)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)
else()
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  endif()
endif()

# Check if ZScript is included as a subproject (via add_subdirectory)
# or if it is the main/primary project.
if(NOT DEFINED ZSCRIPT_MASTER_PROJECT)
  set(ZSCRIPT_MASTER_PROJECT OFF)

  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(ZSCRIPT_MASTER_PROJECT ON)
  endif()
endif()


add_sub_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/*)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/zbase)





set(ZSCRIPT_STATIC_LIB_NAME zscript)

file(GLOB_RECURSE ZSCRIPT_HEADER_FILES
  "${ZSCRIPT_INCLUDE_DIRECTORY}/zscript/*.h")

file(GLOB_RECURSE ZSCRIPT_SOURCE_FILES
  "${ZSCRIPT_SOURCE_DIRECTORY}/*.cpp"
  "${ZSCRIPT_SOURCE_DIRECTORY}/*.c"
  "${ZSCRIPT_SOURCE_DIRECTORY}/*.h"
  "${ZSCRIPT_SOURCE_DIRECTORY}/*.ipp")

file(GLOB_RECURSE ZSCRIPT_SOURCE_CC_FILES
  "${ZSCRIPT_SOURCE_DIRECTORY}/*.cc")

source_group(TREE "${ZSCRIPT_INCLUDE_DIRECTORY}/zscript"
  PREFIX include
  FILES ${ZSCRIPT_HEADER_FILES})

source_group(TREE ${ZSCRIPT_SOURCE_DIRECTORY}
  PREFIX src
  FILES ${ZSCRIPT_SOURCE_FILES} ${ZSCRIPT_SOURCE_CC_FILES})

add_library(${ZSCRIPT_STATIC_LIB_NAME} STATIC
  ${ZSCRIPT_HEADER_FILES}
  ${ZSCRIPT_SOURCE_FILES}
  ${ZSCRIPT_SOURCE_CC_FILES})

set_target_properties(${ZSCRIPT_STATIC_LIB_NAME} PROPERTIES
  FOLDER zs
  XCODE_GENERATE_SCHEME NO)

# wp_set_compile_options(${ZSCRIPT_STATIC_LIB_NAME} PUBLIC)
target_compile_definitions(${ZSCRIPT_STATIC_LIB_NAME} PUBLIC
  -DZSCRIPT_STD_LIB_DIRECTORY="${ZSCRIPT_STD_LIB_DIRECTORY}")

target_include_directories(${ZSCRIPT_STATIC_LIB_NAME}
  PUBLIC
  "${ZSCRIPT_INCLUDE_DIRECTORY}"
  PRIVATE
  "${ZSCRIPT_SOURCE_DIRECTORY}"
)

set(ZSCRIPT_CC_FILE_CONTENT "${ZSCRIPT_COPYRIGHT_STR}
/// @file zinclude.h
/// @author Alexandre Arsenault
/// @date October 6 2024
/// @brief Contains all the `.cc` files to include in `zscript.cpp`.
/// @warning This file is auto-generated.
///
/// File naming convention:
/// * Any `*.cc` files will be compiled as part of `zscript.cpp`.
/// * Any `*.cpp` files are compiled normally.
///
/// @note Even if auto-generated, any changes to this file should be commited.
")

foreach(ZFILE_PATH IN LISTS ZSCRIPT_SOURCE_CC_FILES)
  file(RELATIVE_PATH ZREL_FILE_PATH "${ZSCRIPT_SOURCE_DIRECTORY}" ${ZFILE_PATH})
  set(ZSCRIPT_CC_FILE_CONTENT "${ZSCRIPT_CC_FILE_CONTENT}\n#include \"${ZREL_FILE_PATH}\"")
endforeach()

set(CMAKE_CONFIGURABLE_FILE_CONTENT "${ZSCRIPT_CC_FILE_CONTENT}")
configure_file("${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
  "${ZSCRIPT_SOURCE_DIRECTORY}/zinclude.h"
  @ONLY
)
unset(CMAKE_CONFIGURABLE_FILE_CONTENT)

set_source_files_properties(${ZSCRIPT_SOURCE_CC_FILES}
  PROPERTIES HEADER_FILE_ONLY TRUE)

target_link_libraries(${ZSCRIPT_STATIC_LIB_NAME} PUBLIC
  zbase
)

target_link_libraries(${ZSCRIPT_STATIC_LIB_NAME} PRIVATE
  bitsery
)

# add_custom_command(
# OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/std/z_std.h"
# COMMAND echo "R\"###(" | cat - "${ZSCRIPT_STD_LIB_DIRECTORY}/std.zs" > "${CMAKE_CURRENT_SOURCE_DIR}/std/z_std.h"
# COMMAND echo ")###\"" >> "${CMAKE_CURRENT_SOURCE_DIR}/std/z_std.h"
# DEPENDS "${ZSCRIPT_STD_LIB_DIRECTORY}/std.zs"
# VERBATIM)

# target_sources(${ZSCRIPT_STATIC_LIB_NAME} PRIVATE
# "${CMAKE_CURRENT_SOURCE_DIR}/std/z_std.h")
add_sub_directories(${CMAKE_CURRENT_SOURCE_DIR}/modules/*)

# Tests.
add_sub_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/*)

# Apps.
# add_sub_directories(${CMAKE_CURRENT_SOURCE_DIR}/apps/*)
# add_sub_directories(${CMAKE_CURRENT_SOURCE_DIR}/tools/*)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/zscript)

add_sub_directories(${CMAKE_CURRENT_SOURCE_DIR}/apps/*)

