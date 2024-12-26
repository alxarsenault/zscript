#include "unit_tests.h"

using namespace utest;

//
// MARK: Unary Arithmetic.
//

ZTEST_CASE("unary_arithmetic", R"""(
var t = { a = 0 };
var a0 = t.a++;
return [a0, t.a];
)""") {
  REQUIRE(value == zs::_a(vm, { 0, 1 }));
}

ZTEST_CASE("unary_arithmetic", R"""(
var t = { a = 0 };
var a0 = ++t.a;
return [a0, t.a];
)""") {
  REQUIRE(value == zs::_a(vm, { 1, 1 }));
}

ZTEST_CASE("unary_arithmetic", R"""(
var t = {
 value = 10,

 function __pre_incr() {
   this.value = this.value + 1;
   return this.value;
 }
};

var k = ++t;
return [t, k];
)""") {
  REQUIRE(value == zs::_a(vm, { 11, 11 }));
}

ZTEST_CASE("unary_arithmetic", R"""(
var t = {
 value = 10,

 function __pre_incr() {
   ++this.value;
   return this;
 }

 function __incr() {
   return this.value++;
 }
};

var k = t++;
++t;
return [k, t.value];
)""") {
  REQUIRE(value == zs::_a(vm, { 10, 12 }));
}

ZTEST_CASE("unary_arithmetic", R"""(
var arr = [0, 1, 2, 3, 4, 5];

var it = arr.begin();
var i = it;
var itt = it++;
itt = it++;
return [i.get(), it.get(), itt.get()];
)""") {

  REQUIRE(value == zs::_a(vm, { 0, 2, 1 }));
}

ZTEST_CASE("unary_arithmetic", R"""(
var arr = [0, 1, 2, 3, 4, 5];
var it = arr.begin();

var i = it;
var itt = ++it;
itt = ++it;
return [i.get(), it.get(), itt.get()];
)""") {
  REQUIRE(value == zs::_a(vm, { 0, 2, 2 }));
}
