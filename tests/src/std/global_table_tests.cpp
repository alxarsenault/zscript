#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("global_table", R"""(
var g = global;
global.K = 32;
return "123";
)""") {
  REQUIRE(value == "123");
}

ZTEST_CASE("global_table", R"""(
var a = 123;
return string(a);
)""") {

  REQUIRE(value == "123");
}

ZTEST_CASE("global_table", R"""(
var a = 2.24;
return string(a);
)""") {
  REQUIRE(value == "2.24");
}

ZTEST_CASE("global_table", R"""(
var a = 2.24;
return int(a);
)""") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 2);
}

ZTEST_CASE("global_table", R"""(
return ::int(2.24);
)""") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 2);
}

ZTEST_CASE("global_table", R"""(
var a = 2;
return float(a);
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2);
}

ZTEST_CASE("global_table", R"""(
return ::float(2);
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2);
}

ZTEST_CASE("global_table", R"""(
return to_json([1, 2, 3]);
)""") {
  REQUIRE(value == "[1,2,3]");
}

ZTEST_CASE("global_table", R"""(
return is_float(2.2);
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("global_table", R"""(
return is_float(22);
)""") {
  REQUIRE(value == false);
}

ZTEST_CASE("global_table", R"""(
return is_empty([]);
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("global_table", R"""(
return is_empty([1]);
)""") {
  REQUIRE(value == false);
}
