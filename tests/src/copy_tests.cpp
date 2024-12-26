#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("copy", R"""(
var t1 = { a = 89};
var t2 = zs::copy(t1);
t2.a = 32;
return [t1.a, t2.a];
)""") {
  REQUIRE(value == zs::_a(vm, { 89, 32 }));
}

ZTEST_CASE("copy", R"""(
var t1 = {
  a = 89,

  function __copy(delegate) {
    return this;
  }
};

var t2 = zs::copy(t1);
t2.a = 32;
return [t1.a, t2.a];
)""") {
  REQUIRE(value == zs::_a(vm, { 32, 32 }));
}

ZTEST_CASE("copy", R"""(
var t1 = {
  a = 89,

  function operator(copy)(delegate) {
    return this;
  }
};

var t2 = zs::copy(t1);
t2.a = 32;
return [t1.a, t2.a];
)""") {
  REQUIRE(value == zs::_a(vm, { 32, 32 }));
}

ZTEST_CASE("copy", R"""(
var a = mutable_string("DSLKDJS");
var p = fs::path("/Alex/sa");
var b = zs::copy(a);
return [ b == a, p == "/Alex/sa", fs::is_path(p), fs::is_path(a), fs::is_path("/Alex/sa")];
)""") {
  REQUIRE(value == zs::_a(vm, { true, true, true, false, false }));
}
