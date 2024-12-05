#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("fs::path", R"""(
var dir = fs::path("/Users");
var dir2 = dir / "alexarse";
return dir2;
)""") {
  REQUIRE(value.as_mutable_string().get_string() == "/Users/alexarse");
}

ZTEST_CASE("fs::path", R"""(
var dir = fs::path("/Users");
var dir2 = dir / "alexarse";
return dir;
)""") {
  REQUIRE(value.as_mutable_string().get_string() == "/Users");
}

ZTEST_CASE("fs::path", R"""(
var dir1 = fs::path("/Users");
var dir2 = fs::path("alexarse/bingo");
var dir3 = dir1 / dir2;
return dir3;
)""") {
  REQUIRE(value.as_mutable_string().get_string() == "/Users/alexarse/bingo");
}

ZTEST_CASE("fs::path", R"""(
/////
var dir1 = fs::path("/Users");
var dir2 = fs::path("alexarse/bingo");
var dir3 = dir1 / dir2 / "Banana";
return dir3;
)""") {
  REQUIRE(value.as_mutable_string().get_string() == "/Users/alexarse/bingo/Banana");
}

ZTEST_CASE("fs::path", R"""(
var dir1 = fs::path("/Users");
var dir2 = fs::path("alexarse/bingo");
var dir3 = dir1 / "Banana" / dir2;
return dir3;
)""") {
  REQUIRE(value.as_mutable_string().get_string() == "/Users/Banana/alexarse/bingo");
}

ZTEST_CASE("fs::path", R"""(
var dir1 = fs::path("/Users");
var dir2 = fs::path("/alexarse/bingo");
var dir3 = dir1 / dir2;
return dir3;
)""") {
  REQUIRE(value.as_mutable_string().get_string() == "/Users/alexarse/bingo");
}

ZTEST_CASE("fs::path", R"""(
var dir1 = fs::path("/Users");
var dir2 = fs::path("/alexarse/");
var dir3 = dir1 / dir2 / "pogo" / "john.txt";
return dir3;
)""") {
  REQUIRE(value.as_mutable_string().get_string() == "/Users/alexarse/pogo/john.txt");
}

ZTEST_CASE("fs::path", R"""(
var dir2 = fs::path("/alexarse/");
var dir3 = fs::path("/Users") / dir2 / "pogo" / "john.txt";
return dir3;
)""") {
  REQUIRE(value.as_mutable_string().get_string() == "/Users/alexarse/pogo/john.txt");
}
