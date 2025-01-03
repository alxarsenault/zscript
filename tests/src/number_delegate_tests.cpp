#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("number", "return (32).to_string();") { REQUIRE(value == "32"); }

ZTEST_CASE("number", "return (32.23).to_string();") { REQUIRE(value == "32.23"); }

ZTEST_CASE("number", "return (0).to_string();") { REQUIRE(value == "0"); }

ZTEST_CASE("number.to_float", "return (0).to_float();") { REQUIRE(value == 0); }

ZTEST_CASE("number.to_float", "return (128).to_float();") { REQUIRE(value == 128.0); }

ZTEST_CASE("number.to_float", "return (12.12).to_float();") { REQUIRE(value == 12.12); }

ZTEST_CASE("number.to_int", "return (12.12).to_int();") { REQUIRE(value == 12); }

ZTEST_CASE("number.to_int", "return (12.99).to_int();") { REQUIRE(value == 12); }

ZTEST_CASE("number.to_int", "return (12.99999).to_int();") { REQUIRE(value == 12); }

ZTEST_CASE("number.to_int", "return (-12.12).to_int();") { REQUIRE(value == -12); }

ZTEST_CASE("number.to_int", "return (-12.99).to_int();") { REQUIRE(value == -12); }

ZTEST_CASE("number.to_int", "return (-128).to_int();") { REQUIRE(value == -128); }

ZTEST_CASE("number.to_char", "return ('A').to_char();") { REQUIRE(value == "A"); }

ZTEST_CASE("number.to_char", "return ('π').to_char();") { REQUIRE(value == "π"); }

ZTEST_CASE("number.to_char", "return (65).to_char();") { REQUIRE(value == "A"); }

ZTEST_CASE("number.is_neg", "return (0).is_neg();") { REQUIRE(value == false); }

ZTEST_CASE("number.is_neg", "return (1).is_neg();") { REQUIRE(value == false); }

ZTEST_CASE("number.is_neg", "return (-0.00001).is_neg();") { REQUIRE(value == true); }

ZTEST_CASE("ghjghgjhgj", R"""(
var delegate = {
  operator(>>=) = function(rhs, del) {
    this.a >>= rhs;
    return this;
  }
};

var t = {
  a = 56
};

zs::set_delegate(t, delegate);
t >>= 1;

return t.a;
)""") {
  //  REQUIRE(value == (56 >> 1));
}

ZTEST_CASE("tttt", R"""(
var t = zs::set_delegate({a = 56}, {
  operator(>>) = function(rhs, del) {
    return 78;
  }});

return t.a >> 1;
)""") {
  REQUIRE(value == 28);
}

ZTEST_CASE("zslib", R"""(
var a = 0;
var b = a ? a : 55;
var c = a !> 55;
return zs::all_equals(b, c, 55);
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("zslib", R"""(
return zs::in_range(9.9, 9.9, 90);
)""") {
  REQUIRE(value);
}

ZTEST_CASE("zslib", R"""(
return zs::in_range(9.9, 9.9, 90, false);
)""") {
  REQUIRE(!value);
}

ZTEST_CASE("v", R"""(
return zs::all_true(1, 3);
)""") {
  REQUIRE(value);
}

ZTEST_CASE("zslib", R"""(
return zs::all_true(1, 3, false);
)""") {
  REQUIRE(!value);
}

ZTEST_CASE("gkloo", R"""(

global.set("k", 90);
global.k = 89;
//io::print(global);
return 445;
)""") {
  REQUIRE(value == 445);
}
