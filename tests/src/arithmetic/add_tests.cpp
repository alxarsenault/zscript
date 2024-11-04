#include "unit_tests.h"

using namespace utest;

//
// MARK: Int + Int
//

ZTEST_CASE("add_int_int", "return 0 + 0;") {
  REQUIRE(value.is_integer());
  //  REQUIRE(value == 0);
}

ZTEST_CASE("add_int_int", "return -0 + -0;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 0);
}

ZTEST_CASE("add_int_int", "return 1 + 0;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 1);
}

ZTEST_CASE("add_int_int", "return -1 + 0;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == -1);
}

ZTEST_CASE("add_int_int", "return -1 + -0;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == -1);
}

ZTEST_CASE("add_int_int", "return 1 + 1;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 2);
}

ZTEST_CASE("add_int_int", "return 5 + 7;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 12);
}

ZTEST_CASE("add_int_int", "return -5 + 7;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 2);
}

ZTEST_CASE("add_int_int", "return -5 + -7;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == -12);
}

ZTEST_CASE("add_int_int", "return 5 + -7;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == -2);
}

//
// MARK: Float + Float
//

ZTEST_CASE("add_float_float", "return 0.0 + 0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 0.0);
}

ZTEST_CASE("add_float_float", "return -0.0 + -0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 0.0);
}

ZTEST_CASE("add_float_float", "return 1.0 + 0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 1.0);
}

ZTEST_CASE("add_float_float", "return -1.0 + 0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -1.0);
}

ZTEST_CASE("add_float_float", "return -1.0 + -0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -1.0);
}

ZTEST_CASE("add_float_float", "return 1.0 + 1.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.0);
}

ZTEST_CASE("add_float_float", "return 5.0 + 7.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 12.0);
}

ZTEST_CASE("add_float_float", "return -5.0 + 7.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.0);
}

ZTEST_CASE("add_float_float", "return -5.0 + -7.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -12.0);
}

ZTEST_CASE("add_float_float", "return 5.0 + -7.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -2.0);
}

//
// MARK: Int + Float
//

ZTEST_CASE("add_int_float", "return 0 + 0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 0.0);
}

ZTEST_CASE("add_int_float", "return -0 + -0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 0.0);
}

ZTEST_CASE("add_int_float", "return 1 + 0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 1.0);
}

ZTEST_CASE("add_int_float", "return -1 + 0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -1.0);
}

ZTEST_CASE("add_int_float", "return -1 + -0.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -1.0);
}

ZTEST_CASE("add_int_float", "return 1 + 1.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.0);
}

ZTEST_CASE("add_int_float", "return 5 + 7.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 12.0);
}

ZTEST_CASE("add_int_float", "return -5 + 7.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.0);
}

ZTEST_CASE("add_int_float", "return -5 + -7.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -12.0);
}

ZTEST_CASE("add_int_float", "return 5 + -7.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -2.0);
}

//
// MARK: Float + Int
//

ZTEST_CASE("add_float_int", "return 0.0 + 0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 0.0);
}

ZTEST_CASE("add_float_int", "return -0.0 + -0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 0.0);
}

ZTEST_CASE("add_float_int", "return 1.0 + 0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 1.0);
}

ZTEST_CASE("add_float_int", "return -1.0 + 0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -1.0);
}

ZTEST_CASE("add_float_int", "return -1.0 + -0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -1.0);
}

ZTEST_CASE("add_float_int", "return 1.0 + 1;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.0);
}

ZTEST_CASE("add_float_int", "return 5.0 + 7;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 12.0);
}

ZTEST_CASE("add_float_int", "return -5.0 + 7;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.0);
}

ZTEST_CASE("add_float_int", "return -5.0 + -7;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -12.0);
}

ZTEST_CASE("add_float_int", "return 5.0 + -7;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -2.0);
}

//
// MARK: Int + Bool
//

ZTEST_CASE("add_int_bool", "return 5 + true;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 6);
}

ZTEST_CASE("add", "return true + true;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 2);
}

ZTEST_CASE("add", "return true + false;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 1);
}

//
// MARK: Float + Bool
//

ZTEST_CASE("add", "return 2.2 + false;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.2);
}

ZTEST_CASE("add", "return false + 2.2;") {
  REQUIRE(value.is_float());
  REQUIRE(value == 2.2);
}

//
// MARK: String + String
//

ZTEST_CASE("add_strkkk", "return '''Abc''' + zs::to_string(32);") { REQUIRE(value == "Abc32"); }
ZTEST_CASE("add_str", "return '''Abc''' + zs::to_string(32.2);") { REQUIRE(value == "Abc32.20"); }
ZTEST_CASE("add_str", "return '''Abc''' + zs::to_string(true);") { REQUIRE(value == "Abctrue"); }
ZTEST_CASE("add_str", "return '''Abc''' + zs::to_string(false);") { REQUIRE(value == "Abcfalse"); }

//
// MARK: Sub
//

ZTEST_CASE("sub", "return 5 - 7.0;") {
  REQUIRE(value.is_float());
  REQUIRE(value == -2.0);
}

ZTEST_CASE("sub", "return 5 - 7;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == -2);
}

//
// MARK: lshit
//

ZTEST_CASE("lshift", "return 8 << 2;") { REQUIRE(value == 32); }
ZTEST_CASE("lshift", "return 8 << 2.0;", compile_good | call_fail) {}

//
// MARK: rshit
//

ZTEST_CASE("rshift_01", "return 16 >> 2;") { REQUIRE(value == 4); }
ZTEST_CASE("rshift_02", "return 16 >> 2.0;", compile_good | call_fail) {}

//
// MARK: bitwise
//

ZTEST_CASE("bitwise_or", " return 2 | 4;") { REQUIRE(value == 6); }
ZTEST_CASE("bitwise_or", " return 2 | 4.0;", compile_good | call_fail) {}

ZTEST_CASE("bitwise_and", "return 5 & 7;") { REQUIRE(value == (5 & 7)); }

ZTEST_CASE("bitwise_and", "return 5 & 7.0;", compile_good | call_fail) {}

ZTEST_CASE("xor", R"""(
return 5 xor 7;
)""") {
  REQUIRE(value == 2);
}

//
// MARK: mod
//

ZTEST_CASE("mod", R"""(
var a = 5;
var b = 7;
return a % b;
)""") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 5);
}

ZTEST_CASE("mod", R"""(
return 5 % true;
)""") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 0);
}

ZTEST_CASE("mod", R"""(
return 5 % 7.0;
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 5);
}

ZTEST_CASE("mod", R"""(
return 5.0 % 7;
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 5);
}

ZTEST_CASE("mod", R"""(
return 7.25 % 5.67;
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 1.58);
}

ZTEST_CASE("mod", R"""(
const math = import("math");
var p = 2.0 * math.pi;
return p % math.pi;
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 0);
}

ZTEST_CASE("mod", R"""(
var a = 7.25;
a %= 5.67;
return a;
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 1.58);
}

//
// MARK: Mul
//

ZTEST_CASE("mul_01", "return 32 * 8;") { REQUIRE(value == 256); }
ZTEST_CASE("mul_02", "return 32 * 8.2;") { REQUIRE(value == 262.4); }
ZTEST_CASE("mul_03", "return 32 * true;") { REQUIRE(value == 32); }
ZTEST_CASE("mul_04", "return 32 * false;") { REQUIRE(value == 0); }

//
// MARK: Div
//

ZTEST_CASE("div", R"""(
var a = 32;
var b = 8;
return a / b;
)""") {
  REQUIRE(value == 4);
}

//
// +=
//

ZTEST_CASE("add_eq", R"""(
var a = 2;
a += 2;
return a;
)""") {
  REQUIRE(value == 4);
}

ZTEST_CASE("add_eq", R"""(
var t = { a = 32 };
t.a += 2;
return t.a;
)""") {
  REQUIRE(value == 34);
}

//
// *=
//

ZTEST_CASE("mul_eq", R"""(
var a = 4;
a *= 2;
return a;
)""") {
  REQUIRE(value == 8);
}

ZTEST_CASE("mul_eq", R"""(
var t = { a = 32 };
t.a *= 2;
return t.a;
)""") {
  REQUIRE(value == 64);
}

//
// /=
//

ZTEST_CASE("mul_eq", R"""(
var a = 4;
a /= 2;
return a;
)""") {
  REQUIRE(value == 2);
}

ZTEST_CASE("mul_eq", R"""(
var a = 4.0;
a /= 2.0;
return a;
)""") {
  REQUIRE(value == 2.0);
}

ZTEST_CASE("mul_eq", R"""(
var t = { a = 32 };
t.a /= 2;
return t.a;
)""") {
  REQUIRE(value == 16);
}

ZTEST_CASE("mul_eq", R"""(
var t = { a = 32.0 };
t.a /= 2.0;
return t.a;
)""") {
  REQUIRE(value == 16.0);
}
