#include "unit_tests.h"

using namespace utest;
#include <zbase/sys/path.h>

ZTEST_CASE("struct", R"""(
return struct{};
)""") {
  REQUIRE(value.is_struct());
}

ZTEST_CASE("Scope", R"""(
@module dev-uio
@author "Alexandre Arsenault"
@version 1.10.1
@date 2024-08-23
@copyright BananananP

const m1 = import("module_04");
//local var p1 = 23;
//
//
//var t = {
//  A = 32,
//  B = function() {
//
//  }
//};
//
//t.B();
//m1.a = 222;

export function main(var args = []) {
//  print(args, p1);
  //p1 = 12;
  return 0;
}

)""") {
  //  zb::print(value);

  //  auto vv = value.as_table()["main"];
  //  zs::object ret;
  //  REQUIRE(!vm->call(vv, { value, zs::_a(vm.get_engine(), { zs::_ss("john"), 123, 12.12 }) }, ret));
  //  zb::print(ret);
  //
  //  REQUIRE(!vm->call(vv, { value, zs::_a(vm.get_engine(), { zs::_ss("john") }) }, ret));
  //  zb::print(ret);
}

ZTEST_CASE("ImportDev2", R"""(
@module dev-uio
@author "Alexandre Arsenault"
@author "Anghj" jk
@brief '''"BananananP"


jhjhjhjkh

'''
@version 1.10.1
@date 2024-08-23
@copyright BananananP

const m1 = import("module_04");

//local const john = 908;
//local var abcde = 12312312;
//__locals__.abcd = 123;

//export john;
//export.ABC = {};
var aa = 32;
//var aa = 333;

//export function BDSJHD() {
//  var abcdss = 232;
//  print("KKKK",  local.abcde, __locals__.abcde, local.abcd, __locals__.abcd);
//}

//print(abcd);

//BDSJHD();
//__exports__.ABC.g = 7787;
export m1;
)""") {
  //  zb::print(value);

  //  auto vv = value.as_table()["BDSJHD"];
  //  zs::object ret;
  //  vm->call(vv, vm->get_root(), ret);
}

ZTEST_CASE("ImportDev", R"""(
@module dev
@author Alexandre Arsenault
@brief Banananan

const geo = import("geo");
const math = import("math");
const base64 = import("base64");
//const fs = import("fs");

var r1 = geo.Rect(10, 20, 30, 40);
 
var gino = base64.encode("ALexndre");
var gino2 = base64.decode(gino);

var s1 = r1.size();
var s2 = r1.size();

function make_it(a, b) {
  return geo.Point(a, b);
}

var m1 = make_it(1, 2);

function make_it_again(a, b) {
  var mi = make_it(a, b);
  mi.x = s1.width + s2.width;
  return mi;
}

var pts = [geo.Point(0, 0), geo.Point(0, 1), geo.Point(0, 2), geo.Point(1, 0)];


var m2 = make_it_again(100, 200);

var t = {
  function bingo(n) {
    return 8 * n + s1.width;
  }
};
 
s1.width = 1000;

const Alex = "Alex";

struct God {
  var a = 32;
};

var gggg = {
  bacon = "hotdog"
};

//var gggg = "BANANA";

var abc = 123;

export {"r1":r1, "s1":s1};
)""") {
  REQUIRE(value.is_table());
  // REQUIRE(value.as_table()["a"] == 678);
  //   zb::print("N", value);
  //   zb::print(vm->get_root());
}

ZTEST_CASE("JESUS", R"""(
const m1 = import("module_01.zs");
 
var f = 32;
f = 234;
m1.a = 92;
 

export var K = 323;
K = 888;

var ttt = {
bingo = 234,

gg = function() {
  bingo = 32;
//  johnsonsh = 234;
}
};
function banana() {
  K = 234;
//  bagel = 32;
}

banana();

ttt.gg();

__this__.john = 32;

//john = 234;
 

__exports__.somesome = "Some";
 
)""") {
  REQUIRE(value.is_table());
  // REQUIRE(value.as_table()["a"] == 678);
  //   zb::print("N", value);
  //   zb::print(vm->get_root());
}

ZTEST_CASE("exp", R"""(
  const math = import("math");
//  float a = math.pi^2 * 2;
  return 32;
)""") {
  //  REQUIRE(value.is_float());
  //  REQUIRE(value == std::pow(zb::pi<zs::float_t>, 2.0) * 2.0);
}

ZTEST_CASE("JESUS", R"""(
const m1 = import("module_01.zs");

return m1;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 678);
}

ZTEST_CASE("JESUS", R"""(
const m1 = import("module_01");
return m1;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 678);
}
ZTEST_CASE("JESUS", R"""(
const m1 = import("subfolder/module_01");
return m1;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 234);
}

ZTEST_CASE("JESUS", R"""(
const m1 = import("subfolder.module_01");
return m1;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 234);
}

ZTEST_CASE("zscript", R"""(
const m1 = import("module_01");
const m2 = import("module_02");
@import("module_03.zs")

//print(m1, m2);
 
var @keyword("tom") = 55;
//print(tom);
 
@macro peter_maker(name, v) { var @keyword("peter_", name) = v }
 
$peter_maker("alex", 90);

//print(peter_alex);

return struct{};
)""") {
  //  zb::print(get_test_name());
  REQUIRE(value.is_struct());
  //  zb::print(vm->get_imported_modules());
  //  zb::print(vm->get_module_loaders());
}

ZTEST_CASE("zscript", R"""(
const m1 = import("module_02");

//print(m1);

return struct{};
)""") {
  //  zb::print(get_test_name());
  REQUIRE(value.is_struct());
}

ZTEST_CASE("zscript", ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/test_01.zs") { /*zb::print(get_test_name());*/ }

ZTEST_CASE("import", R"""(
return struct{};
)""") {
  REQUIRE(value.is_struct());
}

ZTEST_CASE("import", R"""(
return struct{};
)""") {
  REQUIRE(value.is_struct());
}

ZTEST_CASE("bingo", R"""(
//print("DBDN");
return struct{};
)""") {
  REQUIRE(value.is_struct());
}

// TEST_CASE("proto-serialize") {
//   const char* filepath = ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/module_01.zs";
//   zs::vm vm;
//   zs::engine* eng = vm.get_engine();
//   zb::byte_vector data_buffer;
//
//   {
//     zb::file_view file;
//     REQUIRE(!file.open(filepath));
//
//     zs::object fpo;
//     zs::jit_compiler compiler(eng);
//     if (auto err = compiler.compile(file.str(), filepath, fpo, nullptr, nullptr, false, false)) {
//       FAIL(compiler.get_error());
//     }
//
//     REQUIRE(fpo.is_function_prototype());
//     //    fpo.as_proto().debug_print();
//
//     if (auto err = fpo.as_proto().save(data_buffer)) {
//       FAIL(err.message());
//     }
//
//     std::ofstream output_file;
//     output_file.open(
//         ZSCRIPT_TESTS_OUTPUT_DIRECTORY "/module_01.zsc", std::ios_base::out | std::ios_base::binary);
//     REQUIRE(output_file.is_open());
//
//     output_file.write((const char*)data_buffer.data(), data_buffer.size());
//     output_file.close();
//   }
//
////  {
////    zb::file_view file;
////    REQUIRE(!file.open(ZSCRIPT_TESTS_OUTPUT_DIRECTORY "/compiler_03.zsc"));
////
////    zs::function_prototype_object* fpo_ptr = zs::function_prototype_object::create(eng);
////    if (auto err = fpo_ptr->load(file.content())) {
////      FAIL(err.message());
////    }
////
////    zs::object fpo(fpo_ptr, false);
////
////    //    zb::print("--------------------------------");
////    //    fpo.as_proto().debug_print();
////
////    zs::object closure = zs::object::create_closure(eng, fpo, vm->get_root());
////
////    zs::object result;
////    if (auto err = vm->call(closure, { vm->get_root() }, result)) {
////      FAIL(vm.get_error());
////    }
////
////    //    zb::print(result);
////  }
//}
