#include "unit_tests.h"

using namespace utest;

//
// MARK: Int + Int
//

ZTEST_CASE("add_int_int", "return 0 + 0;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 0);
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

ZTEST_CASE("add_str", "return '''Abc''' + zs::to_string(32);") { REQUIRE(value == "Abc32"); }
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
// MARK: substr
//

ZTEST_CASE("substr", "return '''alex''' - '''le''';") { REQUIRE(value == "ax"); }
ZTEST_CASE("substr", "return '''Abc''' - 2;") { REQUIRE(value == "c"); }
ZTEST_CASE("substr", "return '''Abc''' - 1;") { REQUIRE(value == "bc"); }
ZTEST_CASE("substr", "return '''Abc''' - (-1);") { REQUIRE(value == "Ab"); }
ZTEST_CASE("substr", "return '''Abcdef''' - -3;") { REQUIRE(value == "Abc"); }
ZTEST_CASE("substr", "return '''alex--bingo''' - '''--''';") { REQUIRE(value == "alexbingo"); }
ZTEST_CASE("substr", "return '''a b c d e''' - ''' ''';") { REQUIRE(value == "abcde"); }
ZTEST_CASE("substr", "return ''' a b c    d e ''' - ''' ''';") { REQUIRE(value == "abcde"); }

//
// MARK: lshit
//

ZTEST_CASE("lshift", "return 8 << 2;") { REQUIRE(value == 32); }
ZTEST_CASE("lshift", "return 8 << 2.0;") { REQUIRE(value == 32); }

ZTEST_CASE("lshift_str", "return '''alex''' << 0;") { REQUIRE(value == "alex"); }
ZTEST_CASE("lshift_str", "return '''alex''' << 1;") { REQUIRE(value == "lex"); }
ZTEST_CASE("lshift_str", "return '''alex''' << 2;") { REQUIRE(value == "ex"); }
ZTEST_CASE("lshift_str", "return '''alex''' << 3;") { REQUIRE(value == "x"); }
ZTEST_CASE("lshift_str", "return '''alex''' << 4;") { REQUIRE(value == ""); }
ZTEST_CASE("lshift_str", "return '''alex''' << 5;") { REQUIRE(value == ""); }

//
// MARK: rshit
//

ZTEST_CASE("rshift_01", "return 16 >> 2;") { REQUIRE(value == 4); }
ZTEST_CASE("rshift_02", "return 16 >> 2.0;") { REQUIRE(value == 4); }

ZTEST_CASE("rshift_str_01", "return '''alex''' >> 0;") { REQUIRE(value == "alex"); }
ZTEST_CASE("rshift_str_02", "return '''alex''' >> 1;") { REQUIRE(value == "ale"); }
ZTEST_CASE("rshift_str_03", "return '''alex''' >> 2;") { REQUIRE(value == "al"); }
ZTEST_CASE("rshift_str_04", "return '''alex''' >> 3;") { REQUIRE(value == "a"); }
ZTEST_CASE("rshift_str_05", "return '''alex''' >> 4;") { REQUIRE(value == ""); }
ZTEST_CASE("rshift_str_06", "return '''alex''' >> 5;") { REQUIRE(value == ""); }

//
// MARK: bitwise
//

ZTEST_CASE("bitwise_or", " return 2 | 4;") { REQUIRE(value == 6); }
ZTEST_CASE("bitwise_or", " return 2 | 4.0;") { REQUIRE(value == 6); }

ZTEST_CASE("bitwise_and", "return 5 & 7;") { REQUIRE(value == (5 & 7)); }

ZTEST_CASE("bitwise_and", "return 5 & 7.0;") {
  REQUIRE(value.is_integer());
  REQUIRE(value == (5 & 7));
}

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
var b = 7.0;
return a % b;
)""") {
  REQUIRE(value.is_float());
  REQUIRE(value == 5);
}

ZTEST_CASE("mod", R"""(
var a = 5;
var b = 7;
return a % b;
)""") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 5);
}

ZTEST_CASE("mod", R"""(
var a = 5;
var b = true;
return a % b;
)""") {
  REQUIRE(value.is_integer());
  REQUIRE(value == 0);
}

//
// MARK: mod_str
//

ZTEST_CASE("mod_str", R"""(
var a = "Abc";
var b = "Abcdef";
return a % b;
)""") {
  REQUIRE(value == "Abc");
}

ZTEST_CASE("mod_str", R"""(
var a = "Abc";
var b = "A";
return a % b;
)""") {
  REQUIRE(value == "A");
}

ZTEST_CASE("mod_str", R"""(
var a = "Abc";
var b = "D";
return a % b;
)""") {
  REQUIRE(value == "");
}

//
// MARK: Mul
//

ZTEST_CASE("mul_01", "return 32 * 8;") { REQUIRE(value == 256); }
ZTEST_CASE("mul_02", "return 32 * 8.2;") { REQUIRE(value == 262.4); }
ZTEST_CASE("mul_03", "return 32 * true;") { REQUIRE(value == 32); }
ZTEST_CASE("mul_04", "return 32 * false;") { REQUIRE(value == 0); }

ZTEST_CASE("mul_str_01", "return '''alex''' * 2;") { REQUIRE(value == "alexalex"); }
ZTEST_CASE("mul_str_02", "return '''alex''' * 3;") { REQUIRE(value == "alexalexalex"); }

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
// MARK: Exp
//

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
