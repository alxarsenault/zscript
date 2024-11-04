#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("exp", "return 10^2;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 100);
}

ZTEST_CASE("exp", "return 10^2^4;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == std::pow(10, 16));
}

ZTEST_CASE("exp", "return 10^(2 + 3);") {
  REQUIRE(value.is_integer());
  REQUIRE(value == std::pow(10, 5));
}

ZTEST_CASE("exp", "return 10^(2 * 2^2);") {
  REQUIRE(value.is_integer());
  REQUIRE(value == std::pow(10.0, 8.0));
}

ZTEST_CASE("exp", R"""(
  var math = import("math");
  float a = math.pi^2
  return a;
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == std::pow(zb::pi<zs::float_t>, 2.0));
}

ZTEST_CASE("exp", R"""(
  const math = import("math");
  float a = math.pi^2 * 2
  return a;
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == std::pow(zb::pi<zs::float_t>, 2.0) * 2.0);
}

ZTEST_CASE("exp", R"""(
  const math = import("math");
  float a = math.pi;
a^= 2.0;
a*= 2.0;
  return a;
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == std::pow(zb::pi<zs::float_t>, 2.0) * 2.0);
}
