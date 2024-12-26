#include "unit_tests.h"

using namespace utest;

#define COMPARE_TEST \
  R"""(
var a = {
  value = 10,

  // Same as function __compare(...).
  function operator(<=>)(rhs) {
    return this.value <=> rhs.value;
  }
};
)"""

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a < { value = 9 };
)""") {
  REQUIRE(value == false);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a < { value = 10 };
)""") {
  REQUIRE(value == false);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a < { value = 11 };
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a <= { value = 9 };
)""") {
  REQUIRE(value == false);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a <= { value = 10 };
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a <= { value = 11 };
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a > { value = 9 };
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a > { value = 10 };
)""") {
  REQUIRE(value == false);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a > { value = 11 };
)""") {
  REQUIRE(value == false);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a >= { value = 9 };
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a >= { value = 10 };
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("compare", COMPARE_TEST R"""(
return a >= { value = 11 };
)""") {
  REQUIRE(value == false);
}
