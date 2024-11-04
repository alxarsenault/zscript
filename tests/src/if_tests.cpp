#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("ternary-if", R"""(
var a = true;
var b = a ? 21 : 12;
return b;
)""") {
  REQUIRE(value == 21);
}

ZTEST_CASE("ternary-if", R"""(
var a = false;
var b = a ? 21 : 12;
return b;
)""") {
  REQUIRE(value == 12);
}

ZTEST_CASE("ternary-if", R"""(
return "a" == "a" ? 33 : 123;
)""") {
  REQUIRE(value == 33);
}
ZTEST_CASE("ternary-if", R"""(
return "a" == "c" ? 33 : false ? 123 : 124;
)""") {
  REQUIRE(value == 124);
}

ZTEST_CASE("ternary-if", R"""(
return "a" == "c" ? 33 : true ? 123 : 124;
)""") {
  REQUIRE(value == 123);
}

ZTEST_CASE("ternary-if", R"""(
return true ? (true ? 125 : 126): 127;
)""") {
  REQUIRE(value == 125);
}

ZTEST_CASE("ternary-if", R"""(
return true ? (false ? 125 : 126): 127;
)""") {
  REQUIRE(value == 126);
}

ZTEST_CASE("ternary-if", R"""(
return false ? (true ? 125 : 126): 127;
)""") {
  REQUIRE(value == 127);
}

ZTEST_CASE("ternary-if", R"""(
return false ? (false ? 125 : 126): 127;
)""") {
  REQUIRE(value == 127);
}

ZTEST_CASE("if_null", R"""(
var a = 21;
var b = a !> 55;
return b;
)""") {
  REQUIRE(value == 21);
}

ZTEST_CASE("if_null", R"""(
var a = null;
return a !> 89;
)""") {
  REQUIRE(value == 89);
}

ZTEST_CASE("if_null", R"""(
var b = true;
var a = b !> "Alex";
return a;
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("if_null", R"""(
var b = false;
var a = b !> "Alex";
return a;
)""") {
  REQUIRE(value == "Alex");
}

ZTEST_CASE("if_null", R"""(
var a = false !> null !> "Alex";
return a;
)""") {
  REQUIRE(value == "Alex");
}

ZTEST_CASE("if_null", R"""(
var a = false !> "bacon" !> "Alex";
return a;
)""") {
  REQUIRE(value == "bacon");
}

ZTEST_CASE("if_null", R"""(
var a = false !> "" !> "Alex";
return a;
)""") {
  REQUIRE(value == "Alex");
}

ZTEST_CASE("if_null", R"""(
var a = false !> [] !> "Alex";
return a;
)""") {
  REQUIRE(value == "Alex");
}

//
ZTEST_CASE("or", R"""(
var a = false;
var b = "Alex";
if(a or b) {
  return "A";
}

return "B";
)""") {
  REQUIRE(value == "A");
}

ZTEST_CASE("or", R"""(
var a = false;
var b = "";

var aa = a or "KL";// or "Banana";

return a or "L:"; //a ||| b ||| "Banana";
)""") {

  REQUIRE(value == true);
}

ZTEST_CASE("or", R"""(
return false || "" || [];
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("triple_or", R"""(
var a = false;
var b = "";
return a ||| b ||| "Banana";
)""") {

  REQUIRE(value == "Banana");
}

ZTEST_CASE("triple_or", R"""(
var a = false;
var b = "Alex";
a = a ||| b;

return a;
)""") {

  REQUIRE(value == "Alex");
}

ZTEST_CASE("triple_or", R"""(
var a = false;
var b = "Alex";
a = a or b;

return a;
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("triple_or", R"""(
var b = "";
return false ||| b ||| "Alex";
)""") {
  REQUIRE(value == "Alex");
}

ZTEST_CASE("triple_or", R"""(
var b = "";
return "Johnson" ||| b ||| "Alex";
)""") {
  REQUIRE(value == "Johnson");
}
