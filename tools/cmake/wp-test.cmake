# wp_add_test.
function(wp_add_test TARGET_NAME)
  target_link_libraries(${TARGET_NAME} PUBLIC catch2)

  set_target_properties(${TARGET_NAME} PROPERTIES
    FOLDER tests
    XCODE_GENERATE_SCHEME YES)

  get_property(tmp GLOBAL PROPERTY WP_TEST_LIST)
  list(APPEND tmp ${TARGET_NAME})
  set_property(GLOBAL PROPERTY WP_TEST_LIST "${tmp}")
endfunction()

# wp_add_test_executable
function(wp_add_test_executable)
  set(options OPTIONAL FAST)

  set(oneValueArgs
    TARGET_NAME
    RESOURCES_DIRECTORY)

  set(multiValueArgs SOURCES)

  cmake_parse_arguments(WP_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${WP_TEST_TARGET_NAME} ${WP_TEST_SOURCES})

  wp_add_test(${WP_TEST_TARGET_NAME})

  target_compile_definitions(${WP_TEST_TARGET_NAME} PUBLIC -DTESTS_RESOURCES_DIRECTORY="${WP_TEST_RESOURCES_DIRECTORY}")
endfunction()

# wp_add_all_tests_target
function(wp_add_all_tests_target)
  get_property(test_list GLOBAL PROPERTY WP_TEST_LIST)

  set(test_list_custom_target_args)

  foreach(i IN LISTS test_list)
    list(APPEND test_list_custom_target_args COMMAND $<TARGET_FILE:${i}>)
  endforeach()

  add_custom_target(wp-test-all ${test_list_custom_target_args})
  add_dependencies(wp-test-all ${test_list})

  set_target_properties(wp-test-all PROPERTIES
    # FOLDER tests
    XCODE_GENERATE_SCHEME YES)
endfunction()

# wp_add_test_subdirectory
macro(wp_add_test_subdirectory DIR_PATH)
  if(${WP_BUILD_TESTS})
    add_subdirectory(${DIR_PATH})
  endif()
endmacro()
