#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("ARRAY", R"""(
var a = [1, 2, 3];
var b = [];

for(var v : a) {
  b.push(v);
}

return b;
)""") {
  REQUIRE(value == zs::_a(vm, { 1, 2, 3 }));
}
