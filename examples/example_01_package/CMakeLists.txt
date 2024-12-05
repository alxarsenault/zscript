cmake_minimum_required(VERSION 3.22.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(example_01 VERSION 0.0.1 LANGUAGES C CXX)
set(APP_NAME example_01)

file(GLOB_RECURSE APP_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${APP_NAME} ${APP_SOURCE_FILES})
target_include_directories(${APP_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_directories(${APP_NAME} PUBLIC "/Users/alexarse/Develop/zscript/bin")
target_link_libraries(${APP_NAME} PUBLIC zapplib zbase zscript)
