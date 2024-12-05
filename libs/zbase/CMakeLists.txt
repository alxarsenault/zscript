set(ZBASE_STATIC_LIB_NAME zbase)
set(ZBASE_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ZBASE_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE ZBASE_HEADER_FILES
  "${ZBASE_INCLUDE_DIRECTORY}/*.h")

file(GLOB_RECURSE ZBASE_SOURCE_FILES
  "${ZBASE_SOURCE_DIRECTORY}/*.cpp"
  "${ZBASE_SOURCE_DIRECTORY}/*.h")

source_group(TREE "${ZBASE_INCLUDE_DIRECTORY}/zbase"
  PREFIX include
  FILES ${ZBASE_HEADER_FILES})

source_group(TREE ${ZBASE_SOURCE_DIRECTORY}
  PREFIX src
  FILES ${ZBASE_SOURCE_FILES})

add_library(${ZBASE_STATIC_LIB_NAME} STATIC
  ${ZBASE_HEADER_FILES}
  ${ZBASE_SOURCE_FILES})

set_target_properties(${ZBASE_STATIC_LIB_NAME} PROPERTIES
  FOLDER zs
  XCODE_GENERATE_SCHEME NO)

# wp_add_version_definitions(${ZBASE_STATIC_LIB_NAME})
# wp_add_platform_definitions(${ZBASE_STATIC_LIB_NAME})
# wp_setup_module(${ZBASE_STATIC_LIB_NAME} core)

# wp_set_compile_options(${ZBASE_STATIC_LIB_NAME} PUBLIC)
if(APPLE)
  target_link_libraries(${ZBASE_STATIC_LIB_NAME}
    PUBLIC
    "-framework CoreFoundation"
    "-framework Foundation"
  )

  target_compile_options(${ZBASE_STATIC_LIB_NAME}
    PUBLIC
    "-fno-objc-arc")
endif()

target_include_directories(${ZBASE_STATIC_LIB_NAME}
  PUBLIC
  ${ZBASE_INCLUDE_DIRECTORY})

# Tests.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
