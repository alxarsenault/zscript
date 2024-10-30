
function(wp_set_compile_options TARGET EXPOSURE)
  if(WIN32)
    target_compile_options(${TARGET} ${EXPOSURE} -D_CRT_SECURE_NO_WARNINGS=1 -D_ITERATOR_DEBUG_LEVEL=0)
  endif()

  # Clang
  if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    message(STATUS "${TARGET} : Detected Clang compiler.")

    set(WP_CLANG_COMMON_OPTIONS
      -Wall
      -Wextra
      -Wpedantic

      # -Werror
      -Wno-unused-parameter
      -Wzero-as-null-pointer-constant
      -Wno-reserved-id-macro
      -Wno-c++98-compat-pedantic
      -Wno-gnu-zero-variadic-macro-arguments
      -Wno-c++98-compat
      -Wno-old-style-cast
      -Wno-global-constructors
      -Wno-switch-enum
      -Wno-format-nonliteral
      -Wno-missing-prototypes
      -Wno-covered-switch-default
      -Wno-float-equal
      -Wno-unused-member-function
    )

    set(WP_CLANG_DEBUG_OPTIONS ${WP_CLANG_COMMON_OPTIONS})
    set(WP_CLANG_RELEASE_OPTIONS ${WP_CLANG_COMMON_OPTIONS})

    target_compile_options(${TARGET} ${EXPOSURE} "$<$<CONFIG:DEBUG>:${WP_CLANG_DEBUG_OPTIONS}>")
    target_compile_options(${TARGET} ${EXPOSURE} "$<$<CONFIG:RELEASE>:${WP_CLANG_RELEASE_OPTIONS}>")

  # GNU
  elseif(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    message(STATUS "${TARGET} : Detected GNU compiler.")

    set(WP_GCC_COMMON_OPTIONS -Wall -Wextra -Wpedantic -Wno-parentheses -Wno-unused-but-set-variable)

    set(WP_GCC_DEBUG_OPTIONS ${WP_GCC_COMMON_OPTIONS})
    set(WP_GCC_RELEASE_OPTIONS ${WP_GCC_COMMON_OPTIONS})

    target_compile_options(${TARGET} ${EXPOSURE} "$<$<CONFIG:DEBUG>:${WP_GCC_DEBUG_OPTIONS}>")
    target_compile_options(${TARGET} ${EXPOSURE} "$<$<CONFIG:RELEASE>:${WP_GCC_RELEASE_OPTIONS}>")

  # MSVC
  elseif(CMAKE_CXX_COMPILER_ID MATCHES MSVC)
    message(STATUS "${TARGET} : Detected MSVC compiler.")

    set(WP_MSVC_COMMON_OPTIONS
      /Zc:__cplusplus # compiler option enables the __cplusplus preprocessor macro to report an updated value for

      # recent C++ language standards support
      /Zc:alignedNew # Enable support for C++17 over-aligned new
      /permissive- # Standards conformance
      /W4 #

      # /WX # Treat linker warnings as errors
      /utf-8 # Set source and execution character sets to UTF-8
      /MP # Build with multiple processes
      /Zc:hiddenFriend- # Enforce Standard C++ hidden friend rules
      /FAs
      /GR- # Disable Run-Time Type Information
      /EHs-c- # Disable exceptions

      # /we26800
      /wd4623
      /wd4625
      /wd4626 # derived class : assignment operator was implicitly defined as deleted because a base class assignment operator is inaccessible or deleted
      /wd5025
      /wd5026
      /wd5027
    )
    set(WP_MSVC_COMMON_OPTIONS_CONG ${WP_MSVC_COMMON_OPTIONS} CACHE INTERNAL "")
    set(WP_MSVC_DEBUG_OPTIONS ${WP_MSVC_COMMON_OPTIONS})

    # file(STRINGS  "${WP_TRACE_DIRECTORY}/dskl.log" ${WP_MSVC_COMMON_OPTIONS})
    set(WP_MSVC_RELEASE_OPTIONS
      ${WP_MSVC_COMMON_OPTIONS}
      /MT # Causes the application to use the multithread, static version of the run-time library
      /Zp16 # Packs structures on 16-byte boundaries
      /GS- # Remove Buffer Security Check
      /fp:fast # Specify floating-point behavior
      /fp:except- # Disable floating-point exceptions
      /Ox # Enable Most Speed Optimizations
      /Ot # Favor Fast Code

      # /Zl # Omit Default Library Name
      /Zc:threadSafeInit-
      /Oy # Frame-Pointer Omission
    )

    set(WP_MSVC_PRIVATE_COMMON_OPTIONS
      /Wall #
      /wd4514 # Unreferenced inline function has been
      /wd4577 # noexcept used with no exception handling mode specified
      /wd4355 # this used in base member initializer list
      /wd5045 # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
      /wd4820 # bytes padding added after struct
      /wd4868 # compiler may not enforce left-to-right evaluation order in braced initializer list
      /wd4866 # compiler may not enforce left-to-right evaluation order for call to operator_name
      /wd4062 # enumerator in switch of enum is not handled
      /wd26812 # The enum type is unscoped. Prefer enum class over 'enum

      /wd4061

      /wd4244
      /wd4242 # conversion from 'int' to 'unsigned short',
      /wd5262
      /wd4365
      /wd4310 # cast truncates constant value

      /wd4702 # unreachable
    )

    set(WP_MSVC_PRIVATE_DEBUG_OPTIONS ${WP_MSVC_PRIVATE_COMMON_OPTIONS})
    set(WP_MSVC_PRIVATE_RELEASE_OPTIONS ${WP_MSVC_PRIVATE_COMMON_OPTIONS})

    target_compile_options(${TARGET} ${EXPOSURE} "$<$<CONFIG:DEBUG>:${WP_MSVC_DEBUG_OPTIONS}>")
    target_compile_options(${TARGET} ${EXPOSURE} "$<$<CONFIG:RELEASE>:${WP_MSVC_RELEASE_OPTIONS}>")

    target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:DEBUG>:${WP_MSVC_PRIVATE_DEBUG_OPTIONS}>")
    target_compile_options(${TARGET} PRIVATE "$<$<CONFIG:RELEASE>:${WP_MSVC_PRIVATE_RELEASE_OPTIONS}>")

  else()
    message(FATAL_ERROR "${TARGET} : Compiler unsupported, aborting.\n")
  endif()

  # target_compile_features(${TARGET} PRIVATE cxx_std_20)
  target_compile_definitions(${TARGET} PUBLIC -D_HAS_EXCEPTIONS=0)
endfunction()

#
# macro(fst_generate_config_file)
# set(WP_CONFIG_FILE_INPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tools/config.h.in")
# set(WP_CONFIG_FILE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fst/config")
# set(WP_CONFIG_FILE_OUTPUT_PATH "${WP_CONFIG_FILE_OUTPUT_DIRECTORY}/config.h")
# configure_file(${WP_CONFIG_FILE_INPUT_PATH} ${WP_CONFIG_FILE_OUTPUT_PATH})
# endmacro()

#
# macro(fst_include_config TARGET_NAME)
# if(WP_DEFAULT_CONFIG_DEFAULT)
# target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
# target_sources(${TARGET_NAME} PUBLIC ${WP_CONFIG_FILE_OUTPUT_PATH})
# source_group(TREE ${WP_CONFIG_FILE_OUTPUT_DIRECTORY} PREFIX config FILES ${WP_CONFIG_FILE_OUTPUT_PATH})
# endif()
# endmacro()

# macro(fst_link_dependencies TARGET_NAME)
# if(WIN32)
# target_link_libraries(${TARGET_NAME} PUBLIC winhttp)
# endif()
# endmacro()
