#include "specification/specification.h"
//
// ZS_SPEC_SECTION(fs_lib, "Filesystem Library", R"""(Filesystem library.)""");
//
////
// ZS_SPEC_TEST(fs_lib,
//   "is_directory",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = "/Users";
// var b = fs.is_directory(a);)""",
//     "b"))
//{
//   REQUIRE(value == true);
// }
//
////
// ZS_SPEC_TEST(fs_lib,
//   "get_extension",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = "/Users/bingo.txt";
// var b = fs.get_extension(a);)""",
//     "b"))
//{
//   REQUIRE(value == ".txt");
// }
//
////
// ZS_SPEC_TEST(fs_lib,
//   "get_filename",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = "/Users/bingo.txt";
// var b = fs.get_filename(a);)""",
//     "b"))
//{
//   REQUIRE(value == "bingo.txt");
// }
//
////
// ZS_SPEC_TEST(fs_lib,
//   "get_stem",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = "/Users/bingo.txt";
// var b = fs.get_stem(a);)""",
//     "b"))
//{
//   REQUIRE(value == "bingo");
// }
//
////
// ZS_SPEC_TEST(fs_lib,
//   "get_parent",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = "/Users/bingo.txt";
// var b = fs.get_parent(a);)""",
//     "b"))
//{
//   REQUIRE(value == "/Users");
// }
//
////
// ZS_SPEC_TEST(fs_lib,
//   "is_absolute",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = "/Users/bingo.txt";
// var b = fs.is_absolute(a);)""",
//     "b"))
//{
//   REQUIRE(value == true);
// }
//
////
// ZS_SPEC_TEST(fs_lib,
//   "has_filename",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = "/Users/bingo.txt";
// var b = fs.has_filename(a);)""",
//     "b"))
//{
//   REQUIRE(value == true);
// }
//
////
// ZS_SPEC_TEST(fs_lib,
//   "has_extension",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = "/Users/bingo.txt";
// var b = fs.has_extension(a);)""",
//     "b"))
//{
//   REQUIRE(value == true);
// }
//
////
// ZS_SPEC_TEST(fs_lib,
//   "has_extension",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = "/Users/bingo";
// var b = fs.has_extension(a);)""",
//     "b"))
//{
//   REQUIRE(value == false);
// }
//
////
// ZS_SPEC_TEST(fs_lib,
//   "read",
//   "",
//   ZCODE_R(R"""(const fs = import("fs");
// var a = ")""" ZSCRIPT_SAMPLES_DIRECTORY "/macro_01.zs"
//           R"""(";
// var b = fs.read(a);)""",
//     "b"))
//{
//   REQUIRE(value.is_string());
// }
//
// ZS_SPEC_TEST(fs_lib,
//   "has_extension",
//   "",
//   ZCODE(R"""(const fs = import("fs"), math = import("math");
// var a = "/Users/bingo";
// var b = fs.has_extension(a);
// var c = math.min(4, 2);)""",
//     "return {b = b, c = c};"))
//{
//   REQUIRE(value.is_table());
//   zs::object_unordered_map<zs::object_ptr>& map
//     = *value.get_table_internal_map();
//   REQUIRE(map[zs::_ss("b")] == false);
//   REQUIRE(map[zs::_ss("c")] == 2);
// }
