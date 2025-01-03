#include "unit_tests.h"
#include <zscript/std/zfloat_array.h>

using namespace utest;

ZTEST_CASE("ARRAY", R"""(
var a = [1, 2, 3];
var b = [];

for(var v : a) {
  b.push(v);
}

return b;
)""") {
  REQUIRE(value == zs::_a(vm, { 1, 2, 3 }));
}

ZTEST_CASE("array::visit", R"""(
var arr = ["A", "B", "C"];
var arr2 = [];
arr.visit((item) => arr2.push(item));
return arr2;
)""") {
  REQUIRE(value == zs::_a(vm, { zs::_ss("A"), zs::_ss("B"), zs::_ss("C") }));
}

ZTEST_CASE("array::visit", R"""(
var arr = ["A", "B", "C"];
var arr2 = [];
var arr3 = [];
arr.visit((idx, item) => {
  arr2.push(idx);
  arr3.push(item);
});

return [arr2, arr3];
)""") {
  REQUIRE(value
      == zs::_a(vm, { zs::_a(vm, { 0, 1, 2 }), zs::_a(vm, { zs::_ss("A"), zs::_ss("B"), zs::_ss("C") }) }));
}

ZTEST_CASE("float_array", R"""(
var arr = np.ramp(0, 5);

//arr.ramp(0, 0.01);

return arr;
)""") {

  //  zs::print(zs::float_array::as_float_array(value));
}

ZTEST_CASE("float_array", R"""(
var arr = np.array([1, 2, 3, -4, 5]);
var s  = np.ramp(0, 5);

arr[2] = 12.12;
arr[2] = 123.12;

function john() {
}

john();

//var a = arr;
//a[2] = 89.12;

//a.push(1232.2);

//io::print(89, arr.size(), 78, arr[2], 99, arr, 777, typeof(arr), 88, arr.min(),   789);

//for(int i = 0; i < s.size(); i++) {
//    io::print(s[i]);
//}

return arr;
)""") {

  zs::object f = vm->global().as_table()["np"].as_table()["ramp"];
  //  zs::print(f);

  REQUIRE(!vm->call(f, { vm->global(), zs::object(0), 10, 10 }, value));
}

ZTEST_CASE("array", ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/tests/array_test_01.zs") {
  REQUIRE(value.is_table());

  zs::table_object& tbl = value.as_table();

  REQUIRE(tbl["capacity_01"] == 3);
  REQUIRE(tbl["capacity_02"] == 10);
  REQUIRE(tbl["get0_00"] == 1);
  REQUIRE(tbl["get0_02"] == 1);
  REQUIRE(tbl["get1_01"] == 2);
  REQUIRE(tbl["get1_02"] == 3);
  REQUIRE(tbl["get2_02"] == 10);
  REQUIRE(tbl["get3_02"] == 11);
  REQUIRE(tbl["get4_02"] == 12);
  REQUIRE(tbl["is_empty_01"] == false);
  REQUIRE(tbl["is_empty_02"] == false);
  REQUIRE(tbl["is_number_array"] == true);
  REQUIRE(tbl["length_01"] == 3);
  REQUIRE(tbl["length_02"] == 10);
  REQUIRE(tbl["size_01"] == 3);
  REQUIRE(tbl["size_02"] == 10);
  REQUIRE(tbl["size_03"] == 2);
  REQUIRE(tbl["size_04"] == 5);
}
