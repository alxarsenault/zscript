#!/usr/local/bin/zscript

const argparser = import("argparser");

var create_cmake_str;
var create_output_str;

// Main.
export function main(args) {
  var parg = argparser("packager", "description");

  parg.add_option("input", ["--input", "-f"], "DBABAB", true);

  parg.add_option("output", ["--output", "-o"], "DBABAB", true);

  parg.add_option("include", "-I", "Include directories", false, true);
  parg.add_option("main", ["--main", "-m"], "Main function", false, false);

  parg.add_flag("version", "-v", "Print version");

  var res = parg.parse(args);

  if(res.has_error()) {
    res.print_error();
    res.print_help();
    return -1;
  }
  //print(res);

  var fpath = fs::path(res.input);
  //print(fpath, fpath.parent);
  var file_content = fpath.read_all();
  //print(pp);

  var output_dir = fs::path(fpath.parent.tostring() + "/" + fpath.stem + "_package");
  var output_src_dir = fs::path(output_dir.tostring() + "/src");
  output_src_dir.mkdir();

  var output_str = create_output_str(res, file_content);
  var output_file_path = output_src_dir.tostring() + "/" + fpath.stem + ".cpp";
  var output_file = fs::file(output_file_path, fs::mode.create | fs::mode.write);
  output_file.write(output_str);
  output_file.close();

  var library_dir_path = "/Users/alexarse/Develop/zscript/bin";

  var output_cmake_str = create_cmake_str(fpath.stem.to_snake_case(), fpath.stem, library_dir_path);
  var cmake_output_file_path = output_dir.tostring() + "/CMakeLists.txt";
  var cmake_output_file = fs::file(cmake_output_file_path, fs::mode.create | fs::mode.write);
  cmake_output_file.write(output_cmake_str);
  cmake_output_file.close();



  //fs::path(library_dir_path + "/libzbase.a").copy(output_dir.tostring() + "/libzbase.a");
  //fs::path(library_dir_path + "/libzscript.a").copy(output_dir.tostring() + "/libzscript.a");
  //fs::path(library_dir_path + "/libzapplib.a").copy(output_dir.tostring() + "/libzapplib.a");

  return 0;
}

create_cmake_str = function(string project_name, string app_name, string library_dir_path) {
  return '''cmake_minimum_required(VERSION 3.22.0)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(''' + project_name + ''' VERSION 0.0.1 LANGUAGES C CXX)
set(APP_NAME ''' + app_name + ''')

file(GLOB_RECURSE APP_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${APP_NAME} ${APP_SOURCE_FILES})
target_include_directories(${APP_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_directories(${APP_NAME} PUBLIC "''' + library_dir_path + '''")
target_link_libraries(${APP_NAME} PUBLIC zapplib zbase zscript)
''';
}


create_output_str = function(options, string file_content) {
  var output_str = '''//
// MIT License
//
// Copyright (c) 2024 Alexandre Arsenault
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char** items;
  int size;
} zapp_string_list;

typedef struct {
  const char* name;
  const char* value;
} zapp_item;

typedef struct {
  const zapp_item* items;
  int size;
} zapp_item_list;

typedef struct {
  const char* filepath;
  const char* code;
  const char* main_function;
  zapp_string_list args;
  zapp_string_list include_directories;
  zapp_item_list defines;
} zapp_options;

int zapp_call_main(const zapp_options* opts);

#ifdef __cplusplus
} // extern "C".
#endif

static const char* file_content = R"######(\n''' + file_content + '''\n)######";

int main(int argc, char* argv[]) {

  zapp_options options;
  memset(&options, 0, sizeof(zapp_options));
  options.code = file_content;
''';

  if(options.contains("main")) {
    output_str = output_str + '''  options.main_function = "''' + options.main + '''";''';
  }

  return output_str + '''
  options.args.size = argc;
  options.args.items = (const char**)argv;

  return zapp_call_main(&options);
}''';
}