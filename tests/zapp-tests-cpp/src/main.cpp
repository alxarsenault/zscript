

#include <zapp.h>
#include <string.h>

static const char* file_content = R"######(
var a = 678;

print(a);
return 0;
)######";

int main(int argc, char* argv[]) {

  zapp_options options;
  memset(&options, 0, sizeof(zapp_options));
  options.code = file_content;
  options.main_function = "main";
  options.args.size = argc;
  options.args.items = (const char**)argv;

  return zapp_call_main(&options);
}

// #include <zapp.h>
// #include <string.h>
//
// int main(int argc, char* argv[]) {
//
//   zapp_options options;
//   memset(&options, 0, sizeof(zapp_options));
//   options.filepath = "/Users/alexarse/Develop/zscript/examples/main_02.zs";
//
//   const char* include_directories[] = {"/Users/alexarse/Develop/zscript/examples"};
//
//   options.include_directories.size = sizeof(include_directories) / sizeof(const char*);
//   options.include_directories.items = include_directories;
//
//
//   zapp_item d2;
//   d2.name = "MY_VALUE";
//   d2.value = "Alex";
//
//   options.defines.size = 1;
//   options.defines.items = &d2;
//   //  options.code = "print(\"John\"); export function main() {return 0;}\n";
//   //  options.code = "print(\"John\");\n" "export function main(args) {print(args);return 0;}\n";
////  options.code = "print(\"John\", MY_VALUE, __FILE__);\n"
////                 "export function main(...) {\n"
////                 "  print(vargs);\n"
////                 "  return 0;\n"
////                 "}\n";
//
//  options.main_function = "main";
//
//  options.args.size = argc;
//  options.args.items = (const char**)argv;
//
//  return zapp_call_main(&options);
//}
//
//
