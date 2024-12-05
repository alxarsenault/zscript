

function(add_bundle_resources TARGET_NAME RESOURCE_PATH)
  target_sources(${TARGET_NAME} PRIVATE "${RESOURCE_PATH}")
  set_source_files_properties("${RESOURCE_PATH}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
  source_group("Resources" FILES "${RESOURCE_PATH}")
endfunction()

set(APP_NAME zscript-tools)
set(ZSCRIPT_TOOLS_APP_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ZSCRIPT_TOOLS_APP_RESOURCES_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/resources")

file(GLOB_RECURSE ZSCRIPT_TOOLS_APP_SOURCE_FILES
  "${ZSCRIPT_TOOLS_APP_SOURCE_DIRECTORY}/*.h"
  "${ZSCRIPT_TOOLS_APP_SOURCE_DIRECTORY}/*.cpp"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ZSCRIPT_TOOLS_APP_SOURCE_FILES})

# add_executable(${APP_NAME} MACOSX_BUNDLE ${ZSCRIPT_TOOLS_APP_SOURCE_FILES})
add_executable(${APP_NAME} ${ZSCRIPT_TOOLS_APP_SOURCE_FILES})

target_include_directories(${APP_NAME} PUBLIC ${ZSCRIPT_TOOLS_APP_SOURCE_DIRECTORY} ${ZSCRIPT_SOURCE_DIRECTORY})

target_link_libraries(${APP_NAME} PUBLIC zscript)

# set(ZSCRIPT_TOOLS_APP_ICON_PATH "${ZSCRIPT_TOOLS_APP_RESOURCES_DIRECTORY}/Icon.icns")

# add_bundle_resources(${APP_NAME} "${ZSCRIPT_TOOLS_APP_ICON_PATH}")

 # file(READ "${WP_AUDIO_SYNTH_ASSETS_DIRECTORY}/plist-extra.xmsl" DEMO_EXTRA_PLIST_INFO)
  # set(AUDIO_SYNTH_APP_PLIST_EXTRA_FIELDS ${DEMO_EXTRA_PLIST_INFO})



  # set_target_properties(${WP_PLIST_TARGET} PROPERTIES
  #   OUTPUT_NAME ${WP_PLIST_OUTPUT_NAME}
  #   RESOURCE "${RESOURCES}"
  #   XCODE_GENERATE_SCHEME YES
  #   XCODE_ATTRIBUTE_PRODUCT_NAME ${WP_PLIST_PRODUCT_NAME}
  #   XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${WP_PLIST_BUNDLE_IDENTIFIER}
  #   XCODE_ATTRIBUTE_EXECUTABLE_NAME ${WP_PLIST_EXECUTABLE_NAME}
  #   XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO
  #   XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING "YES"
  #   MACOSX_BUNDLE_INFO_PLIST "${WP_PLIST_PLIST_DIRECTORY}/Info.plist.in")




set_target_properties(${APP_NAME} PROPERTIES
  RESOURCE "${RESOURCES}"
  XCODE_GENERATE_SCHEME YES
  # XCODE_ATTRIBUTE_PRODUCT_NAME "zscript"
  # XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "zscript.com.abc"
  # XCODE_ATTRIBUTE_EXECUTABLE_NAME "zscript"
  # XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC NO
  # XCODE_ATTRIBUTE_DEAD_CODE_STRIPPING "YES"
  # MACOSX_BUNDLE_INFO_PLIST "${ZSCRIPT_TOOLS_APP_RESOURCES_DIRECTORY}/Info.plist.in"

  XCODE_SCHEME_ARGUMENTS "${ZSCRIPT_EXAMPLES_DIRECTORY}/main_01.zs"
  RUNTIME_OUTPUT_DIRECTORY "${ZSCRIPT_BIN_DIRECTORY}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${ZSCRIPT_BIN_DIRECTORY}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${ZSCRIPT_BIN_DIRECTORY}"
  FOLDER apps
  # PREFIX "../"
  OUTPUT_NAME "ztools")

# include(CTest)
# enable_testing()
# add_test(NAME MyTestBanana COMMAND ${APP_NAME} "${ZSCRIPT_EXAMPLES_DIRECTORY}/example_01.zs")



# <key>CFBundleDocumentTypes</key>
# 	<array>
# 		<dict>
# 			<key>CFBundleTypeExtensions</key>
# 			<array>
# 				<string>rvp</string>
# 			</array>
# 			<key>CFBundleTypeIconFile</key>
# 			<string>RevoiceProIcon</string>
# 			<key>CFBundleTypeName</key>
# 			<string>Revoice Pro</string>
# 			<key>CFBundleTypeOSTypes</key>
# 			<array>
# 				<string>SyRP</string>
# 			</array>
# 			<key>CFBundleTypeRole</key>
# 			<string>Editor</string>
# 			<key>NSDocumentClass</key>
# 			<string>SessionDocument</string>
# 		</dict>
# 		<dict>
# 			<key>CFBundleTypeExtensions</key>
# 			<array>
# 				<string>vf</string>
# 			</array>
# 			<key>CFBundleTypeIconFile</key>
# 			<string></string>
# 			<key>CFBundleTypeName</key>
# 			<string>Voice Factory</string>
# 			<key>CFBundleTypeOSTypes</key>
# 			<array>
# 				<string>SyVf</string>
# 			</array>
# 			<key>CFBundleTypeRole</key>
# 			<string>Viewer</string>
# 			<key>NSDocumentClass</key>
# 			<string>SessionDocument</string>
# 		</dict>
# 	</array>