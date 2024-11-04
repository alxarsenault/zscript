#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("base64", R"""(
const base64 = import("base64");
var e = base64.encode("ZScript");
return e;
)""") {
  REQUIRE(value == "WlNjcmlwdA==");
}

ZTEST_CASE("base64", R"""(
const base64 = import("base64");
var e = base64.encode("ZScript");
var d = base64.decode(e);
return d;
)""") {
  REQUIRE(value == "ZScript");
}
