#include "unit_tests.h"

using namespace utest;

#define IMPORT_VEC3 \
  R"""(const vec3 = import(fs::join(ZSCRIPT_EXAMPLES_DIRECTORY, "vec3"));
)"""

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 2, 3)[0];") { REQUIRE(value == 1.0); }
ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 2, 3)[1];") { REQUIRE(value == 2.0); }
ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 2, 3)[2];") { REQUIRE(value == 3.0); }

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 0, 0).norm();") { REQUIRE(value == 1.0); }
ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(0, 1, 0).norm();") { REQUIRE(value == 1.0); }
ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(0, 0, 1).norm();") { REQUIRE(value == 1.0); }

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 2, 3).dot(vec3(4, 5, 6));") {
  REQUIRE(value == 1.0 * 4.0 + 2.0 * 5.0 + 3.0 * 6.0);
}

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 2, 3).norm();") {
  zs::float_t norm = std::sqrt(1.0 + 2.0 * 2.0 + 3.0 * 3.0);
  REQUIRE(value == norm);
}

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 2, 3).normalized();") {
  zs::float_t norm = std::sqrt(1.0 + 2.0 * 2.0 + 3.0 * 3.0);
  REQUIRE(value.as_struct_instance()[0] == 1.0 / norm);
  REQUIRE(value.as_struct_instance()[1] == 2.0 / norm);
  REQUIRE(value.as_struct_instance()[2] == 3.0 / norm);
}

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 2, 3).proj(vec3(4, 5, 6));") {
  zs::float_t norm = std::sqrt(1.0 + 2.0 * 2.0 + 3.0 * 3.0);
  zs::float_t x = 1.0 / norm;
  zs::float_t y = 2.0 / norm;
  zs::float_t z = 3.0 / norm;
  zs::float_t dot = x * 4.0 + y * 5.0 + z * 6.0;

  REQUIRE(value.as_struct_instance()[0] == x * dot);
  REQUIRE(value.as_struct_instance()[1] == y * dot);
  REQUIRE(value.as_struct_instance()[2] == z * dot);
}

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 0, 0).normalized();") {
  const zs::struct_instance_object& vec3 = value.as_struct_instance();
  REQUIRE(vec3[0] == 1.0);
  REQUIRE(vec3[1] == 0.0);
  REQUIRE(vec3[2] == 0.0);
}

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(0, 1, 0).normalized();") {
  const zs::struct_instance_object& vec3 = value.as_struct_instance();
  REQUIRE(vec3[0] == 0.0);
  REQUIRE(vec3[1] == 1.0);
  REQUIRE(vec3[2] == 0.0);
}

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(0, 0, 1).normalized();") {
  const zs::struct_instance_object& vec3 = value.as_struct_instance();
  REQUIRE(vec3[0] == 0.0);
  REQUIRE(vec3[1] == 0.0);
  REQUIRE(vec3[2] == 1.0);
}

ZTEST_CASE("vec3", IMPORT_VEC3 "return vec3(1, 1, 1).reflect(vec3(0, 1, 0));") {
  const zs::struct_instance_object& vec3 = value.as_struct_instance();
  REQUIRE(vec3[0] == -1.0);
  REQUIRE(vec3[1] == 1.0);
  REQUIRE(vec3[2] == -1.0);
}

ZTEST_CASE("vec3", IMPORT_VEC3 R"""(
var v1 = vec3(1, 2, 3);
v1 *= vec3(10, 100, 1000);
return v1;
)""") {

  //  REQUIRE(value.is_struct_instance());
  //  const zs::struct_instance_object& vec3 = value.as_struct_instance();
  //  REQUIRE(vec3[0] == 10.0);
  //  REQUIRE(vec3[1] == 200.0);
  //  REQUIRE(vec3[2] == 3000.0);
}

ZTEST_CASE("vec3", IMPORT_VEC3 R"""(
var v1 = vec3(1, 2, 3);
v1 = v1 * (vec3(10, 100, 1000) + 2);
return v1;
)""") {
  const zs::struct_instance_object& vec3 = value.as_struct_instance();
  REQUIRE(vec3[0] == 12.0);
  REQUIRE(vec3[1] == 204.0);
  REQUIRE(vec3[2] == 3006.0);
}

ZTEST_CASE("vec3", IMPORT_VEC3 R"""(
var v1 = vec3(1, 2, 3);
v1 *= vec3(10, 100, 1000) + 2;
return v1;
)""") {
  const zs::struct_instance_object& vec3 = value.as_struct_instance();
  REQUIRE(vec3[0] == 12.0);
  REQUIRE(vec3[1] == 204.0);
  REQUIRE(vec3[2] == 3006.0);
}

ZTEST_CASE("vec3", IMPORT_VEC3 R"""(
var v1 = vec3(1, 2, 3);
v1 *= 2;
return v1;
)""") {
  const zs::struct_instance_object& vec3 = value.as_struct_instance();
  REQUIRE(vec3[0] == 2.0);
  REQUIRE(vec3[1] == 4.0);
  REQUIRE(vec3[2] == 6.0);
}

ZTEST_CASE("vec3", IMPORT_VEC3 R"""(
var v1 = vec3(1, 2, 3);
v1 *= 0.5;
return v1;
)""") {
  const zs::struct_instance_object& vec3 = value.as_struct_instance();
  REQUIRE(vec3[0] == 0.5);
  REQUIRE(vec3[1] == 1.0);
  REQUIRE(vec3[2] == 1.5);
}

ZTEST_CASE("vec3", R"""(
zs::add_import_directory(ZSCRIPT_EXAMPLES_DIRECTORY);
const vec3 = import("vec3");
//const vec3 = veclib.vec3;

var v1 = vec3(1, 2, 3);
//var v2 = vec3({x = 10, y = 20, z = 30 });
var v3 = vec3(v1);
var v4 = vec3(23.2);

//zs::print(typeof(v2));
//var v3 = vec3(v1);
//var v5 = vec3(32.2);


var r1 = v1 + v4;
//zs::print(r1);


var r2 = v1 + vec3(100.0);
//zs::print(r2);

var r3 = v1 + 1000.0;
//zs::print(r3);

var r4 = v1 * 2;

var r5 = v1 * 2;
//zs::print("R5", r5);
//v1.x = 3.2;

//var gg = v1.cross(vec3(4, 5, 6));


return v3;
)""") {
  //  zb::print(value);
  //  zb::print("-----------------------",value.as_struct_instance().get_base().as_struct().get_doc());
}
