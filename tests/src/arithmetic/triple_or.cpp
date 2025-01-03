#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("triple_or", R"""(
var a = null;
var b = a ||| 0 ||| 32;
return b;
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("triple_or", R"""(
var a = 32;
var b = a ||| 0 ||| 55;
return b;
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("triple_or", R"""(
var a = "";
var b = a ||| 45 ||| 55;
return b;
)""") {
  REQUIRE(value == 45);
}

ZTEST_CASE("triple_or", R"""(
var a = "";
var b = a ||| "A" ||| 55;
return b;
)""") {
  REQUIRE(value == "A");
}
