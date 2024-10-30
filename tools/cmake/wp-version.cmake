# wp_parse_version
macro(wp_parse_version VERSION_FILE_PATH)
  file(READ ${VERSION_FILE_PATH} VERSION_STRING)
  string(REPLACE "." ";" VERSION_LIST ${VERSION_STRING})
  list(GET VERSION_LIST 0 WP_VERSION_MAJOR)
  list(GET VERSION_LIST 1 WP_VERSION_MINOR)
  list(GET VERSION_LIST 2 WP_VERSION_PATCH)
  list(GET VERSION_LIST 3 WP_VERSION_BUILD)
endmacro()

# wp_add_version_definitions
macro(wp_add_version_definitions TARGET_NAME)
  target_compile_definitions(
    ${TARGET_NAME} PUBLIC
    WP_VERSION_MAJOR=${WP_VERSION_MAJOR}
    WP_VERSION_MINOR=${WP_VERSION_MINOR}
    WP_VERSION_PATCH=${WP_VERSION_PATCH}
    WP_VERSION_BUILD=${WP_VERSION_BUILD}
  )
endmacro()
