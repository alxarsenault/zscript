#include "specification/specification.h"

//
ZS_SPEC_SECTION(variable_declaration,
  "Variable Declaration",
  R"""(The zscript language is an hybrid between **weakly** and **strongly** typed.

Basic types are `null`, `bool`, `integer`, `float`, `string`, `table`, `array`, `function`, `class`, `instance` and `userdata`.

A variable declared with `var` is **weakly** typed and can be assigned or reassigned any type.
)""");

ZS_SPEC_SUBSECTION(variable_declaration, "Empty", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(variable_declaration,
  "Empty variable declaration",
  "`a` is `null`.",
  ZCODE_A("var a;"))
{
  REQUIRE(value.is_null());
}

//
ZS_SPEC_TEST(variable_declaration,
  "Empty variable declaration (without *;*)",
  "`a` is `null`.",
  ZCODE_A("var a"))
{
  REQUIRE(value.is_null());
}

ZS_SPEC_SUBSECTION(variable_declaration, "Null", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(variable_declaration,
  "Null variable declaration",
  "`a` is `null`.",
  ZCODE_A("var a = null;"))
{
  REQUIRE(value.is_null());
}

//
ZS_SPEC_TEST(variable_declaration,
  "Null variable declaration",
  "`a` is `null`.",
  ZCODE_R("var abcdefghijklmnopqrstuvwxyz = 12;", "abcdefghijklmnopqrstuvwxyz"))
{
  REQUIRE(value == 12);
}

ZS_SPEC_SUBSECTION(variable_declaration,
  "Integer",
  R"""(- decimal-constant is a non-zero decimal digit (1, 2, 3, 4, 5, 6, 7, 8, 9), followed by zero or more decimal digits (0, 1, 2, 3, 4, 5, 6, 7, 8, 9)
- hex-constant is the character sequence 0x or the character sequence 0X followed by one or more hexadecimal digits (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, a, A, b, B, c, C, d, D, e, E, f, F)
- binary-constant is the character sequence 0b or the character sequence 0B followed by one or more binary digits (0, 1)
- octal-constant is the character sequence 0h followed by zero or more octal digits (0, 1, 2, 3, 4, 5, 6, 7))""");

//
ZS_SPEC_TEST(variable_declaration,
  "Integer variable declaration",
  "",
  ZCODE_A("var a = 56;"))
{
  REQUIRE(value == 56);
}

ZS_SPEC_TEST(variable_declaration,
  "Integer variable declaration",
  "",
  ZCODE_A("const  int a = 56;"))
{
  REQUIRE(value == 56);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Integer variable declaration",
  "",
  ZCODE_A("var a = -89;"))
{
  REQUIRE(value == -89);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Integer variable declaration",
  "Hexadecimal.",
  ZCODE_A("var a = 0xFFAC;"))
{
  REQUIRE(value == 0xFFAC);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Integer variable declaration",
  "Hexadecimal.",
  ZCODE_A("var a = 0XFFAC;"))
{
  REQUIRE(value == 0xFFAC);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Integer variable declaration",
  "Hexadecimal.",
  ZCODE_A("var a = 0xffa08;"))
{
  REQUIRE(value == 0xffa08);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Integer variable declaration",
  "",
  ZCODE_A("var a = 0x1e5;"))
{
  REQUIRE(value.is_integer());
  REQUIRE(value == 0x1e5);
}

// Octal.
ZS_SPEC_TEST(variable_declaration,
  "Integer variable declaration",
  "Octal number.",
  ZCODE_A("var a = 0h567;"))
{
  REQUIRE(value == 0567);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Integer variable declaration",
  "Binary.",
  ZCODE_A("var a = 0b01001;"))
{
  REQUIRE(value == 0b01001);
}

ZS_SPEC_SUBSECTION(variable_declaration, "Float", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(variable_declaration,
  "Float variable declaration",
  "",
  ZCODE_A("var a = 56.6;"))
{
  REQUIRE(value == 56.6);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Float variable declaration",
  "",
  ZCODE_A("var a = 6.;"))
{
  REQUIRE(value == 6.);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Float variable declaration",
  "",
  ZCODE_A("var a = .8;"))
{
  REQUIRE(value == .8);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Float variable declaration",
  "",
  ZCODE_A("var a = 4e2;"))
{
  REQUIRE(value == 4e2);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Float variable declaration",
  "",
  ZCODE_A("var a = 123.456e-67;"))
{
  REQUIRE(value == 123.456e-67);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Float variable declaration",
  "",
  ZCODE_A("var a = .1E4;"))
{
  REQUIRE(value == .1E4);
}

//
// If the floating literal begins with the character sequence 0x or 0X, the
// floating literal is a hexadecimal floating literal. Otherwise, it is a
// decimal floating literal.
//
// For a hexadecimal floating literal, the significand is interpreted as a
// hexadecimal rational number, and the digit-sequence of the exponent is
// interpreted as the (decimal) integer power of 2 by which the significand has
// to be scaled.
//
// double d = 0x1.4p3;// hex fraction 1.4 (decimal 1.25) scaled by 23, that
// is 10.0
// TODO: ????
// ZS_SPEC_TEST(variable_declaration,
//  "Float variable declaration",
//  "",
//  ZCODE_A("var a = 0x1p5;"),
//  ZBAD)
//{
//  //  zb::print(0x1p5);
//  //    REQUIRE(value.is_float());
//  //    REQUIRE(value == 0x1p5);
//}

ZS_SPEC_SUBSECTION(variable_declaration, "Boolean", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(variable_declaration,
  "Boolean variable declaration",
  "",
  ZCODE_A("var a = true;"))
{
  REQUIRE(value == true);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Boolean variable declaration",
  "",
  ZCODE_A("var a = false;"))
{
  REQUIRE(value == false);
}

ZS_SPEC_SUBSECTION(variable_declaration, "Char", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(variable_declaration,
  "Character variable declaration",
  "",
  ZCODE_A("var a = 'A';"))
{
  REQUIRE(value == 'A');
}

//
ZS_SPEC_TEST(variable_declaration,
  "Character variable declaration",
  "utf8",
  ZCODE_A("var a = 'π';"))
{
  REQUIRE(value == u'π');
}
ZS_SPEC_SUBSECTION(variable_declaration, "String", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(variable_declaration,
  "String variable declaration",
  "",
  ZCODE_A("var a = \"abc\";"))
{
  REQUIRE(value == "abc");
}

//
ZS_SPEC_TEST(variable_declaration,
  "String variable declaration",
  "",
  ZCODE_A("var a = \"π\";"))
{
  REQUIRE(value == "π");
}

//
ZS_SPEC_TEST(variable_declaration,
  "Multiline string variable declaration",
  "",
  ZCODE_A("var a = \"\"\"abc\"\"\";"))
{
  REQUIRE(value == "abc");
}

//
ZS_SPEC_TEST(variable_declaration,
  "Multiline string variable declaration",
  "",
  ZCODE_A("var a = '''abc''';"))
{
  REQUIRE(value == "abc");
}

ZS_SPEC_SUBSECTION(variable_declaration, "Array", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(variable_declaration,
  "Array variable declaration",
  "",
  ZCODE_A("var a = [];"))
{
  REQUIRE(value.is_array());
}

//
ZS_SPEC_TEST(variable_declaration,
  "Array variable declaration",
  "",
  ZCODE_A("var a = [1, 2.89, \"abc\", true];"))
{
  REQUIRE(value.is_array());
}

ZS_SPEC_SUBSECTION(variable_declaration, "Table", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(variable_declaration,
  "Table variable declaration",
  "",
  ZCODE_A("var a = {};"))
{
  REQUIRE(value.is_table());
}

//
ZS_SPEC_TEST(variable_declaration,
  "Table variable declaration",
  "",
  ZCODE_A(R"""(var a = {
  a = 78,
  b = 7.9
};)"""))
{
  REQUIRE(value.is_table());
}

//
ZS_SPEC_TEST(variable_declaration,
  "Table variable declaration",
  "The comma is optional.",
  ZCODE_A(R"""(var a = {
  a = 78
  b = 7.9
};)"""))
{
  REQUIRE(value.is_table());
}

//
ZS_SPEC_TEST(variable_declaration,
  "Table variable declaration",
  "json style.",
  ZCODE_A(R"""(var a = {
  "a": 78,
  "b": 7.9
};)"""))
{
  REQUIRE(value.is_table());
}

//
ZS_SPEC_TEST(variable_declaration,
  "Table variable declaration",
  "Bracket.",
  ZCODE_A(R"""(var a = {
  ["a a"] = 78,
  ["a-a"] = 79
};)"""))
{
  REQUIRE(value.is_table());
}

//
ZS_SPEC_TEST(variable_declaration,
  "Table variable declaration",
  "Mix decl.",
  ZCODE_A(R"""(var a = {
  a = 89,
  ["a a"] = 78,
  ["a-a"] = 79,
  "b": 7.9
};)"""))
{
  REQUIRE(value.is_table());
}

ZS_SPEC_SUBSECTION(variable_declaration, "Multi", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(variable_declaration,
  "Multiple variable declaration on a single line",
  "Mix decl.",
  ZCODE_A(R"""(var a = 32, b = 55, c = "abc";)"""))
{
  REQUIRE(value == 32);
}

//
ZS_SPEC_TEST(variable_declaration,
  "Table variable declaration",
  "",
  ZCODE_A(R"""(var v1 = 5;
var v2 = 6;
var a = v1 == v2;)"""))
{
  REQUIRE(value == false);
}

// Const.
ZS_SPEC_TEST(variable_declaration,
  "Const",
  "",
  ZCODE_A(R"""(const var a = 44;
a = 34;)"""),
  ZBAD)
{
}

// Compare.
ZS_SPEC_TEST(variable_declaration,
  "Table variable declaration",
  "",
  ZCODE_A(R"""(var v1 = 5;
var v2 = 5;
var a = v1 == v2;)"""))
{
  REQUIRE(value == true);
}

// as_string.
ZS_SPEC_TEST(variable_declaration,
  "as_string",
  "",
  ZCODE_A("var a = #as_string(\"" ZSCRIPT_TESTS_RESOURCES_DIRECTORY
          "/data/text_01.txt"
          "\");"))
{
  REQUIRE(value == "bacon-dance");
}

// as_table.
ZS_SPEC_TEST(variable_declaration,
  "as_table",
  "",
  ZCODE_A(
    "var a = #as_table(\"" ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/data/obj_01.json"
    "\");"))
{
  REQUIRE(value.is_table());

  zs::object_unordered_map<zs::object>& map = *value.get_table_internal_map();

  REQUIRE(map[zs::_ss("a")] == 32);
  //  zb::print("TABLE VALUE", value.to_debug_string());
  //  REQUIRE(value == "bacon-dance");
}

// ZS_SPEC_TEST(variable_declaration, "define", "", ZCODE_A(R"""(
// #define k_my_value = 54;
// var a = @@k_my_value;)"""))
//{
//   //  zb::print(value.to_debug_string());
//   REQUIRE(value == 54);
//   //
//   //  zs::object_unordered_map<zs::object_ptr>& map
//   //    = *value.get_table_internal_map();
//   //
//   //  REQUIRE(map[zs::_ss("a")] == 32);
//   //  zb::print("TABLE VALUE", value.to_debug_string());
//   //  REQUIRE(value == "bacon-dance");
// }
//
// ZS_SPEC_TEST(variable_declaration, "define", "", ZCODE_A(R"""(
// #define k_my_value = 54;
// var a = @@k_my_value;)"""))
//{
//   //  zb::print(value.to_debug_string());
//   REQUIRE(value == 54);
//   //
//   //  zs::object_unordered_map<zs::object_ptr>& map
//   //    = *value.get_table_internal_map();
//   //
//   //  REQUIRE(map[zs::_ss("a")] == 32);
//   //  zb::print("TABLE VALUE", value.to_debug_string());
//   //  REQUIRE(value == "bacon-dance");
// }

// as_table.
ZS_SPEC_TEST(variable_declaration,
  "blabalbal",
  "",
  ZCODE_A(

    "var a = #load_json_file(\"" ZSCRIPT_TESTS_RESOURCES_DIRECTORY
    "/data/obj_01.json\");"))
{
  //  zb::print(value.convert_to_string());
  //  REQUIRE(value.is_table());
  //
  //  zs::object_unordered_map<zs::object_ptr>& map
  //    = *value.get_table_internal_map();
  //
  //  REQUIRE(map[zs::_ss("a")] == 32);
  //  zb::print("TABLE VALUE", value.to_debug_string());
  //  REQUIRE(value == "bacon-dance");
}
