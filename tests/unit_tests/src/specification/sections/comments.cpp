#include "specification/specification.h"

ZS_SPEC_SECTION(comments,
  "Comments",
  R"""(A comment is text that the compiler ignores but that is useful for programmers. Comments are normally used to embed annotations in the code. The compiler treats them as white space.)""");

//
ZS_SPEC_TEST(comments,
  "Multi-line comment",
  R"""(A comment can be `/*` (slash, asterisk) characters, followed by any sequence of characters (including new lines), followed by the `*/` characters.
  This syntax is the same as ANSI C.)""",
  ZCODE_R(R"""(/*
 * This is a multi-line comment.
 * Keeps going.
 */)""",
    "null"))
{
  REQUIRE(value.is_null());
}

//
ZS_SPEC_TEST(comments,
  "Multi-line comment",
  "",
  ZCODE_R(R"""(/*
   This is a multi-line comment.
   Keeps going.
*/)""",
    "null"))
{
  REQUIRE(value.is_null());
}

//
ZS_SPEC_TEST(
  comments, "Inplace comment", "", ZCODE_A(R"""(var a = /* comment */ 55;)"""))
{
  REQUIRE(value == 55);
}

//
ZS_SPEC_TEST(comments,
  "Single line comment",
  "A comment can also be `//` (two slashes) characters, followed by any "
  "sequence of characters.",
  ZCODE_R(R"""(// This is a single line comment.)""", "null"))
{
  REQUIRE(value.is_null());
}
