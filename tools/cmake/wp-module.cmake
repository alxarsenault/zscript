function(wp_get_module_name OUTPUT_VAR MODULE_NAME)
  if(${WP_MASTER_PROJECT})
    set(${OUTPUT_VAR} ${MODULE_NAME} PARENT_SCOPE)
  else()
  set(${OUTPUT_VAR} wp-${MODULE_NAME} PARENT_SCOPE)
  endif()
endfunction()

# wp_setup_module.
function(wp_setup_module TARGET_NAME MODULE_NAME)

  set_target_properties(${TARGET_NAME} PROPERTIES
    FOLDER modules
    XCODE_GENERATE_SCHEME NO)

  add_library(zb::${MODULE_NAME} ALIAS ${TARGET_NAME})
endfunction()


# wp_create_module.
function(wp_add_module MODULE_NAME)
  set(options OPTIONAL FAST)

  set(oneValueArgs "")

  set(multiValueArgs SOURCES)

  cmake_parse_arguments(WP_MOD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_library(${MODULE_NAME} STATIC ${WP_MOD_SOURCES})
endfunction()


