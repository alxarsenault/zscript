
# wp_add_bundle_resources
function(wp_add_bundle_resources TARGET_NAME RESOURCE_PATH)
  target_sources(${TARGET_NAME} PRIVATE "${RESOURCE_PATH}")
  set_source_files_properties("${RESOURCE_PATH}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  source_group("Resources" FILES "${RESOURCE_PATH}")
endfunction()

# wp_add_bundle
macro(wp_add_bundle)
  set(options OPTIONAL FAST)

  set(oneValueArgs
    TARGET
    PLIST_DIRECTORY
    ICON_PATH
    OUTPUT_NAME
    PRODUCT_NAME
    EXECUTABLE_NAME
    SHORT_VERSION
    VERSION
    ICON_FILE
    BUNDLE_IDENTIFIER
    COPYRIGHT
    BUNDLE_INFO_STRING)

  set(multiValueArgs SOURCES)

  cmake_parse_arguments(WP_PLIST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  #
  add_executable(${WP_PLIST_TARGET} MACOSX_BUNDLE ${WP_PLIST_SOURCES})

  #
  wp_add_bundle_resources(${WP_PLIST_TARGET} "${WP_PLIST_ICON_PATH}")

  #
  set(WP_PLIST_NS_PRINCIPAL_CLASS "NSApplication")

  # file(READ "${WP_VIRTUAL_MIDI_KEYBOARD_ASSETS_DIRECTORY}/plist-extra.xmsl" DEMO_EXTRA_PLIST_INFO)
  # set(VIRTUAL_MIDI_KEYBOARD_APP_PLIST_EXTRA_FIELDS ${DEMO_EXTRA_PLIST_INFO})

  #
  set_target_properties(${WP_PLIST_TARGET} PROPERTIES
    OUTPUT_NAME ${WP_PLIST_OUTPUT_NAME}
    RESOURCE "${RESOURCES}"
    XCODE_GENERATE_SCHEME YES
    XCODE_ATTRIBUTE_PRODUCT_NAME ${WP_PLIST_PRODUCT_NAME}
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${WP_PLIST_BUNDLE_IDENTIFIER}
    XCODE_ATTRIBUTE_EXECUTABLE_NAME ${WP_PLIST_EXECUTABLE_NAME}
    XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO
    XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING "YES"
    MACOSX_BUNDLE_INFO_PLIST "${WP_PLIST_PLIST_DIRECTORY}/Info.plist.in")

  # add_custom_command(
  # TARGET foobar
  # POST_BUILD
  # COMMAND plutil -replace NSHighResolutionCapable -bool true foobar.app/Contents/Info.plist
  # )
endmacro()