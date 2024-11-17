#include "specification/specification.h"

//
ZS_SPEC_SECTION(typed_variable_declaration,
  "Typed Variable Declaration",
  "The type restriction gets lost when it is assigned to an **array**, a "
  "**table** or passed as a function parameter.");

ZS_SPEC_SUBSECTION(typed_variable_declaration, "Integer", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Empty variable declaration",
  "`a` is `null`.",
  ZCODE_A("int a;"))
{
  REQUIRE(value.is_null());
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Integer variable declaration",
  "",
  ZCODE_A("int a = 21;"))
{
  REQUIRE(value == 21);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Empty variable declaration",
  "",
  ZCODE_A(R"""(int a;
a = 78;)"""))
{
  REQUIRE(value == 78);
}

//<style>p{color:red;}</style>

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Wrong variable declaration",
  "🔴 <span style=\"color:#DE5030\">**Runtime Error**</span> `b` is not an "
  "integer.",
  ZCODE_A(R"""(int a;
var b = 32.2;
a = b; // Error 'b' is not an integer.)"""),
  ZGOOD,
  ZBAD)
{
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Wrong type variable declaration",
  "🔴 <span style=\"color:#DE5030\">**Compiler Error**</span> not an integer.",
  ZCODE_A(R"""(int a = 33.2; // Error not an integer.)"""),
  ZBAD)
{
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Empty variable declaration",
  "🔴 <span style=\"color:#DE5030\">**Compiler Error**</span> not an integer.",
  ZCODE_A(R"""(int a;
a = 78.8; // Error not an integer.)"""),
  ZBAD)
{
  //  REQUIRE(false);
}

ZS_SPEC_SUBSECTION(typed_variable_declaration, "Float", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Float variable declaration",
  "",
  ZCODE_A("float a = 21.2;"))
{
  REQUIRE(value == 21.2);
}

//

ZS_SPEC_TEST(typed_variable_declaration,
  "Float variable declaration",
  "<span style=\"color:#DE5030\">**Compiler Error**</span> not a float.",
  ZCODE_A("float a = 21; // Error not a float."),
  ZBAD)
{
}

ZS_SPEC_SUBSECTION(typed_variable_declaration, "Boolean", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Boolean variable declaration",
  "",
  ZCODE_A("bool a = false;"))
{
  REQUIRE(value == false);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Boolean variable declaration",
  "",
  ZCODE_A("bool a = true;"))
{
  REQUIRE(value == true);
}

ZS_SPEC_SUBSECTION(typed_variable_declaration, "Character", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Character variable declaration",
  "",
  ZCODE_A("char a = 'A';"))
{
  REQUIRE(value == 'A');
}

ZS_SPEC_SUBSECTION(typed_variable_declaration, "String", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(typed_variable_declaration,
  "String variable declaration",
  "",
  ZCODE_A("string a = \"abc\";"))
{
  REQUIRE(value == "abc");
}

ZS_SPEC_SUBSECTION(typed_variable_declaration, "Array", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Array variable declaration",
  "",
  ZCODE_A("array a = [];"))
{
  REQUIRE(value.is_array());
}

ZS_SPEC_SUBSECTION(typed_variable_declaration, "Table", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Table variable declaration",
  "",
  ZCODE_A("table a = {};"))
{
  REQUIRE(value.is_table());
}

ZS_SPEC_SUBSECTION(typed_variable_declaration, "Multiple", "jkjkjjlkjlk");

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Typed variable declaration",
  "Same as `int a = 32;`.",
  ZCODE_A("var<int> a = 32;"))
{
  REQUIRE(value == 32);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Empty variable declaration",
  "`a` is `null`.",
  ZCODE_A(R"""(var<int> a = 88;
a = 78.6;)"""),
  ZBAD)
{
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Empty variable declaration",
  "`a` is `null`.",
  ZCODE_A(R"""(var<int, float> a = 88;
a = 78.6;)"""))
{
  REQUIRE(value == 78.6);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Typed variable declaration",
  "Same as `float a = 32.2;`.",
  ZCODE_A("var<float> a = 32.2;"))
{
  REQUIRE(value == 32.2);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Typed variable declaration",
  "Same as `bool a = true;`.",
  ZCODE_A("var<bool> a = true;"))
{
  REQUIRE(value == true);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Typed variable declaration",
  "Same as `char a = 'A';`.",
  ZCODE_A("var<char> a = 'A';"))
{
  REQUIRE(value == 'A');
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Typed variable declaration",
  "Same as `string a = \"abc\";`.",
  ZCODE_A("var<string> a = \"abc\";"))
{
  REQUIRE(value == "abc");
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Typed variable declaration",
  "Same as `array a = [];`.",
  ZCODE_A("var<array> a = [];"))
{
  REQUIRE(value.is_array());
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Typed variable declaration",
  "Same as `table a = {};`.",
  ZCODE_A("var<table> a = {};"))
{
  REQUIRE(value.is_table());
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Integer variable declaration",
  "`a` is `null`.",
  ZCODE_A(R"""(var<int> a = 88;
var b = 77;
a = b;)"""))
{
  REQUIRE(value == 77);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Integer variable declaration",
  "",
  ZCODE_A(R"""(var<int> a = 88;
var b = 77.7;
a = b;)"""),
  ZGOOD,
  ZBAD)
{
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Integer or float variable declaration",
  "",
  ZCODE_A(R"""(var<int, float, string> a = 88;
var b = 77.9;
a = b;
a = false;)"""),
  ZBAD)
{
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Integer or float variable declaration",
  "",
  ZCODE_A(R"""(var<int, float> a = 177;)"""))
{
  REQUIRE(value == 177);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Integer or float variable declaration",
  "",
  ZCODE_A(R"""(var<int> a = 177.8;)"""),
  ZBAD)
{
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Integer or float variable declaration",
  "",
  ZCODE_A(R"""(var<int, float> a = 177.8;
a = false;)"""),
  ZBAD)
{
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Typed variable declaration",
  "",
  ZCODE_A("var<int, float> a = 32.2;"))
{
  REQUIRE(value == 32.2);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Typed variable declaration",
  "",
  ZCODE_A("var<string, float> a = \"bacon\";"))
{
  REQUIRE(value == "bacon");
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Multiple variable declaration on a single line",
  "Mix decl.",
  ZCODE_A(R"""(int a = 32, b = 55, c = 324;)"""))
{
  REQUIRE(value == 32);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Multiple variable declaration on a single line",
  "Mix decl.",
  ZCODE_A(R"""(var<int, float> a = 32, b = 55.5, c = 324;)"""))
{
  REQUIRE(value == 32);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Multiple variable declaration on a single line",
  "Mix decl.",
  ZCODE_A(R"""(var<int, MyClass> a = 32;)"""))
{
  REQUIRE(value == 32);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Multiple variable declaration on a single line",
  "Mix decl.",
  ZCODE_A(R"""(var<MyClass> a;)"""))
{
  //  REQUIRE(value == 32);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Multiple variable declaration on a single line",
  "Mix decl.",
  ZCODE_A(R"""(var<int> a = 55, b = 45;)"""))
{
  //  REQUIRE(value == 32);
}

//
ZS_SPEC_TEST(typed_variable_declaration,
  "Multiple variable declaration on a single line",
  "🔴 <span style=\"color:#DE5030\">**Compiler Error**</span> `b` is not an "
  "integer or a float.",
  ZCODE_A(R"""(var<int, float> a = 55, b = "abc";)"""),
  ZBAD)
{
  //  REQUIRE(value == 32);
}
