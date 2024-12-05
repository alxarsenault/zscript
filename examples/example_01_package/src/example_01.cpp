//
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

static const char* file_content = R"######(
var a = 678;

print(a);
return 0;
)######";

int main(int argc, char* argv[]) {

  zapp_options options;
  memset(&options, 0, sizeof(zapp_options));
  options.code = file_content;

  options.args.size = argc;
  options.args.items = (const char**)argv;

  return zapp_call_main(&options);
}