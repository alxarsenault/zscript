#include "zspec.h"

ZS_SPEC_SECTION(function_declaration, "Function Declaration", "hjhkj");

ZS_SPEC_TEST(function_declaration,
  ZTITLE("bkbbkbkb"),
  ZDESCRIPTION(""),
  "int a = 56;",
  "return a;")
{
  REQUIRE(value == 56);
}

ZS_SPEC_TEST(function_declaration,
  ZTITLE("Function declaration"),
  ZDESCRIPTION(""),
  ZCODE_A(R"""(
var f = function(k) {
  return k;
};

var a = f(32);)"""))
{
  REQUIRE(value == 32);
}

ZS_SPEC_TEST(function_declaration,
  ZTITLE("Function declaration"),
  ZDESCRIPTION(""),
  ZCODE_A(R"""(
var f = function(int k) {
  return k;
};

var a = f(32);)"""))
{
  REQUIRE(value == 32);
}

ZS_SPEC_TEST(function_declaration,
  ZTITLE("Function declaration"),
  ZDESCRIPTION(""),
  ZCODE_A(R"""(
var f = function(const int k) {
  return k;
};

var a = f(32);)"""))
{
  REQUIRE(value == 32);
}

ZS_SPEC_TEST(function_declaration,
  ZTITLE("Function declaration"),
  ZDESCRIPTION(
    "At compile time, the variable assignment of `k` will generate an error."),
  ZCODE_A(R"""(
var f = function(const var<int> k) {
  k = 32; // Error: trying to assign to a const value.
  return k;
};

var a = f(32);)"""),
  ZBAD)
{
}

ZS_SPEC_TEST(function_declaration,
  ZTITLE("Function declaration"),
  ZDESCRIPTION(""),
  ZCODE_A(R"""(
var f = function(const int k) {
  return k;
};

var a = f(32.2);)"""))
{
  REQUIRE(value == 32.2);
}

ZS_SPEC_TEST(function_declaration,
  ZTITLE("Function declaration"),
  ZDESCRIPTION(""),
  ZCODE_A(R"""(
var f = function(const int k) {
  return k;
};

var a = f(32);)"""))
{
  REQUIRE(value == 32);
}
