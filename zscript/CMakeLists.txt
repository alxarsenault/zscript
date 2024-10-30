# ZSCRIPT Programming language.

set(ZSCRIPT_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ZSCRIPT_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)

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

# add_custom_command(
# OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/std/z_std.h"
# COMMAND echo "R\"###(" | cat - "${ZSCRIPT_STD_LIB_DIRECTORY}/std.zs" > "${CMAKE_CURRENT_SOURCE_DIR}/std/z_std.h"
# COMMAND echo ")###\"" >> "${CMAKE_CURRENT_SOURCE_DIR}/std/z_std.h"
# DEPENDS "${ZSCRIPT_STD_LIB_DIRECTORY}/std.zs"
# VERBATIM)

# target_sources(${ZSCRIPT_STATIC_LIB_NAME} PRIVATE
# "${CMAKE_CURRENT_SOURCE_DIR}/std/z_std.h")

# Tests.
add_sub_directories(${CMAKE_CURRENT_SOURCE_DIR}/tests/*)

# Apps.
# add_sub_directories(${CMAKE_CURRENT_SOURCE_DIR}/apps/*)
# add_sub_directories(${CMAKE_CURRENT_SOURCE_DIR}/tools/*)
