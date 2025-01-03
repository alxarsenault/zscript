#include "unit_tests.h"
#include <zscript/std/zfs.h>

using namespace utest;

ZTEST_CASE("fs::path", R"""(
var a = fs::path("/");
return typeof(a);
)""") {
  REQUIRE(value == "path");
}

ZTEST_CASE("fs::path", R"""(
var dir = fs::path("/Users");
return dir;
)""") {
  REQUIRE(zs::is_path(value));
  REQUIRE(zs::get_path(value) == "/Users");
}

ZTEST_CASE("fs::path", R"""(
var dir = fs::path("/Users");
var dir2 = dir / "alexarse";
return dir2;
)""") {
  REQUIRE(zs::get_path(value) == "/Users/alexarse");
}

ZTEST_CASE("fs::path", R"""(
var dir = fs::path("/Users");
var dir2 = dir / "alexarse";
return dir;
)""") {
  REQUIRE(zs::get_path(value) == "/Users");
}
//
ZTEST_CASE("fs::path", R"""(
var dir1 = fs::path("/Users");
var dir2 = fs::path("alexarse/bingo");
var dir3 = dir1 / dir2;
return dir3;
)""") {
  REQUIRE(zs::get_path(value) == "/Users/alexarse/bingo");
}
//
ZTEST_CASE("fs::path", R"""(
/////
var dir1 = fs::path("/Users");
var dir2 = fs::path("alexarse/bingo");
var dir3 = dir1 / dir2 / "Banana";
return dir3;
)""") {
  REQUIRE(zs::get_path(value) == "/Users/alexarse/bingo/Banana");
}

ZTEST_CASE("fs::path", R"""(
var dir1 = fs::path("/Users");
var dir2 = fs::path("alexarse/bingo");
var dir3 = dir1 / "Banana" / dir2;
return dir3;
)""") {
  REQUIRE(zs::get_path(value) == "/Users/Banana/alexarse/bingo");
}

ZTEST_CASE("fs::path", R"""(
var dir1 = fs::path("/Users");
var dir2 = fs::path("/alexarse/bingo");
var dir3 = dir1 / dir2;
return dir3;
)""") {
  REQUIRE(zs::get_path(value) == "/Users/alexarse/bingo");
}

ZTEST_CASE("fs::path", R"""(
var dir1 = fs::path("/Users");
var dir2 = fs::path("/alexarse/");
var dir3 = dir1 / dir2 / "pogo" / "john.txt";
return dir3;
)""") {
  REQUIRE(zs::get_path(value) == "/Users/alexarse/pogo/john.txt");
}

ZTEST_CASE("fs::path", R"""(
var dir2 = fs::path("/alexarse/");
var dir3 = fs::path("/Users") / dir2 / "pogo" / "john.txt";
return dir3;
)""") {
  REQUIRE(zs::get_path(value) == "/Users/alexarse/pogo/john.txt");
}

ZTEST_CASE("fs::path", R"""(
var dir2 = fs::path("/alexarse/");
return dir2.to_string();
)""") {
  REQUIRE(value == "/alexarse/");
}

ZTEST_CASE("fs::path", R"""(
var dir2 = fs::path(ZSCRIPT_TESTS_RESOURCES_DIRECTORY);
return dir2.list_recursive();
)""") {
  //  zs::print(value);
}

ZTEST_CASE("fs::path", R"""(
return fs::path("/A/BN/Johnson")[0];
)""") {
  REQUIRE(value == "/");
}

ZTEST_CASE("fs::path", R"""(
return fs::path("/A/BN/Johnson")[1];
)""") {
  REQUIRE(value == "A");
}

ZTEST_CASE("fs::path", R"""(
return fs::path("/A/BN/Johnson")[2];
)""") {
  REQUIRE(value == "BN");
}

ZTEST_CASE("fs::path", R"""(
return fs::path("/A/BN/Johnson")[3];
)""") {
  REQUIRE(value == "Johnson");
}

ZTEST_CASE("fs::path", R"""(
return fs::path("/A/BN/Johnson").get_component(3);
)""") {
  REQUIRE(value == "Johnson");
}

ZTEST_CASE("fs::path", R"""(
return fs::path("/A/BN/Johnson").get_char(3);
)""") {
  REQUIRE(value == 'B');
}

ZTEST_CASE("fs::path", R"""(
var p = fs::path("/A/BN/Johnson");
var s = "";
for(var v : p) {
  s += v;
}

return s;
)""") {
  REQUIRE(value == "/ABNJohnson");
}
