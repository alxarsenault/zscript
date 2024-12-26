#include "unit_tests.h"

using namespace utest;

#include <zbase/strings/parse_utils.h>

#define VDECL_INT(...) ZTEST_CASE("variable_declaration_integer", __VA_ARGS__)
VDECL_INT("int v; return v;") { REQUIRE(value.is_null()); }
VDECL_INT("int v = 0; return v;") { REQUIRE(value == 0); }
VDECL_INT("int v = -0; return v;") { REQUIRE(value == -0); }
VDECL_INT("int v = 32; return v;") { REQUIRE(value == 32); }
VDECL_INT("var<int> v = 33; return v;") { REQUIRE(value == 33); }
VDECL_INT("int v = 0xFF0000FF; return v;") { REQUIRE(value == 0xFF0000FF); }
VDECL_INT("int v = 0b110011; return v;") { REQUIRE(value == 0b110011); }
VDECL_INT("int v = 0h564; return v;") { REQUIRE(value == 0564); }
VDECL_INT("int v = -1233; return v;") { REQUIRE(value == -1233); }
VDECL_INT("int v = 1234567891234; return v;") { REQUIRE(value == 1234567891234); }

#define VDECL_FLOAT(...) ZTEST_CASE("variable_declaration_float", __VA_ARGS__)
VDECL_FLOAT("float v; return v;") { REQUIRE(value.is_null()); }
VDECL_FLOAT("float v = 0.0; return v;") { REQUIRE(value == 0.0); }
VDECL_FLOAT("float v = -0.0; return v;") { REQUIRE(value == -0.0); }
VDECL_FLOAT("var<float> v = 0.2; return v;") { REQUIRE(value == 0.2); }
VDECL_FLOAT("float v = -0.2; return v;") { REQUIRE(value == -0.2); }
VDECL_FLOAT("float v = .34; return v;") { REQUIRE(value == .34); }
VDECL_FLOAT("float v = -.34; return v;") { REQUIRE(value == -.34); }
VDECL_FLOAT("float v = 32.2; return v;") { REQUIRE(value == 32.2); }
VDECL_FLOAT("float v = -32.2; return v;") { REQUIRE(value == -32.2); }
VDECL_FLOAT("float v = 12345.67821; return v;") { REQUIRE(value == 12345.67821); }
VDECL_FLOAT("float v = -12345.67821; return v;") { REQUIRE(value == -12345.67821); }
VDECL_FLOAT("float v = 1e5; return v;") { REQUIRE(value == 1e5); }
VDECL_FLOAT("float v = -1e5; return v;") { REQUIRE(value == -1e5); }
VDECL_FLOAT("float v = 123.456e-3; return v;") { REQUIRE(value == 123.456e-3); }
VDECL_FLOAT("float v = -123.456e-3; return v;") { REQUIRE(value == -123.456e-3); }
VDECL_FLOAT("float v = 1.18e-49; return v;") { REQUIRE(value == 1.18e-49); }
VDECL_FLOAT("float v = -1.18e-49; return v;") { REQUIRE(value == -1.18e-49); }

ZTEST_CASE("variable_declaration", R"""(
int v1 = 32;
var<int> v2 = 33;
float v3 = 0.0;
var<float> v4 = 0.0;
var<int | float> v5 = -890;
var<int | float> v6 = 890.32;
number v7 = 5.92;
var<number> v8 = -890;

return [v1, v2, v3, v4, v5, v6, v7, v8];
)""") {
  REQUIRE(value == zs::_a(vm, { 32, 33, 0.0, 0.0, -890, 890.32, 5.92, -890 }));
}
