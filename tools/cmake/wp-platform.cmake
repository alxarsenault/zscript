# wp_scan_platform_definitions
macro(wp_scan_platform_definitions)
  include(CheckSourceCompiles)
  include(CheckIncludeFile)
  include(CheckCXXSourceCompiles)

  # Windows - Windows.h
  check_include_file("Windows.h" WP_PLATFORM_HAS_WINDOWS_H)

  # Unix - unistd.h
  check_include_file("unistd.h" WP_PLATFORM_HAS_UNISTD_H LANGUAGE CXX)

  if(WP_PLATFORM_HAS_UNISTD_H)
    # check_cxx_source_compiles("
    # #include <unistd.h>
    # int main() {
    # #ifdef HAVE_MMAP
    # return 0;
    # #elif _POSIX_VERSION >= 199506L
    # return 0;
    # #else
    # return 0;
    # #endif

    # }
    # " HAVE_PTHREAD_PRIO_INHERIT)
    check_include_file("fcntl.h" WP_PLATFORM_HAS_FCNTL_H)
    check_include_file("sys/mman.h" WP_PLATFORM_HAS_SYS_MMAN_H)
    check_include_file("sys/types.h" WP_PLATFORM_HAS_SYS_TYPES_H)
  endif()

  try_run(WP_STD_STRING_STACK_CAPACITY COMPILE_RESULT_VAR
    ${CMAKE_BINARY_DIR}
    ${WP_CMAKE_TOOLS_DIRECTORY}/std_string_stack_capacity.cpp
    OUTPUT_VARIABLE OUTPUT
  )

endmacro()

# wp_add_platform_definitions
macro(wp_add_platform_definitions TARGET_NAME)
  wp_add_conditional_definition(${TARGET_NAME} WP_PLATFORM_HAS_WINDOWS_H)

  wp_add_conditional_definition(${TARGET_NAME} WP_PLATFORM_HAS_UNISTD_H)
  wp_add_conditional_definition(${TARGET_NAME} WP_PLATFORM_HAS_FCNTL_H)
  wp_add_conditional_definition(${TARGET_NAME} WP_PLATFORM_HAS_SYS_MMAN_H)
  wp_add_conditional_definition(${TARGET_NAME} WP_PLATFORM_HAS_SYS_TYPES_H)

  target_compile_definitions(${TARGET_NAME} PUBLIC -DWP_STD_STRING_STACK_CAPACITY=${WP_STD_STRING_STACK_CAPACITY})  
endmacro()