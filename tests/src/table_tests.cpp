#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("TABLE", R"""(
var t1 = { a = 32, b = 44 };
var t2 = {};

for(var k, v : t1) {
 t2[k] = v;
}

return t2;
)""") {
  REQUIRE(value.as_table()["a"] == 32);
  REQUIRE(value.as_table()["b"] == 44);
}
