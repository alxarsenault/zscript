#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("math::sin", R"""(
return math::sin(2.2);
)""") {
  REQUIRE(value == std::sin(2.2));
}

ZTEST_CASE("math::min", R"""(
return math::min(3, 2);
)""") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 2);
}

ZTEST_CASE("math::min", R"""(
return math::min(3, 1.5, 2);
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 1.5);
}

ZTEST_CASE("math::min", R"""(
return math::min(-32, 1.5, 2);
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == -32);
}

ZTEST_CASE("math::min", R"""(
return math::min(2.2);
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.2);
}

ZTEST_CASE("math::max", R"""(
return math::max(3, 2);
)""") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 3);
}

ZTEST_CASE("math::max", R"""(
return math::max(3, 1.5, 2);
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 3);
}

ZTEST_CASE("math::max", R"""(
return math::max(-32, 1.5, 2);
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2);
}

ZTEST_CASE("math::max", R"""(
return math::max(2.2);
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.2);
}

ZTEST_CASE("math::rand_uniform", R"""(
return math::rand_uniform(2, 8);
)""") {
  REQUIRE(value.is_integer());
  REQUIRE((value._int >= 2 and value._int <= 8));
}

ZTEST_CASE("math::rand_uniform", R"""(
return math::rand_uniform(2.0, 8.0);
)""") {
  REQUIRE(value.is_float());
  REQUIRE((value._float >= 2 and value._float <= 8));
}

ZTEST_CASE("math::rand_normal", R"""(
return math::rand_normal(2.0, 1.0);
)""") {
  REQUIRE(value.is_float());
  REQUIRE((value._float >= 2.0 - 10.0 and value._float <= 2.0 + 10.0));
}
