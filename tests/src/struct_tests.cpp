#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("struct", R"""(
return struct{};
)""") {
  REQUIRE(value.is_struct());
}

ZTEST_CASE("struct-range", ZSCRIPT_TESTS_RESOURCES_DIRECTORY "/tests/range_test_01.zs") {
  REQUIRE(value.is_table());

  zs::table_object& tbl = value.as_table();

  REQUIRE(tbl["contains_01"] == true);
  REQUIRE(tbl["contains_02"] == false);
  REQUIRE(tbl["contains_03"] == false);
  REQUIRE(tbl["cvalue_01"] == 10);
  REQUIRE(tbl["cvalue_02"] == 7);
  REQUIRE(tbl["cvalue_03"] == 5);

  REQUIRE(tbl["r_middle"] == 17.5);

  REQUIRE(tbl["r"].is_struct_instance());
  zs::struct_instance_object& r = tbl["r"].as_struct_instance();
  REQUIRE(r.key(0) == "start");
  REQUIRE(r.key(1) == "end");
  REQUIRE(r[0] == 10);
  REQUIRE(r[1] == 25);
  zs::object rvalue;
  REQUIRE(!r.get(zs::_ss("start"), rvalue));
  REQUIRE(rvalue == 10);

  REQUIRE(tbl["r10"].is_struct_instance());

  // "r": [{"start" : 10},{"end" : 25}],
  // "r10": [{"start" : 0},{"end" : 21}],
  // "r2": [{"start" : 20},{"end" : 25}],
  // "r3": [{"start" : -10},{"end" : 10}],
  // "r4": [{"start" : -10},{"end" : 10}],
  // "r5": [{"start" : -25.5},{"end" : 25.5}],
  // "r6": [{"start" : 0},{"end" : 25.5}],
  // "r7": [{"start" : 120.12},{"end" : 145.62}],
  // "r8": [{"start" : 5},{"end" : 10}],
}

ZTEST_CASE("struct-geo", R"""(
const geo = import("geo");

var r1 = geo.Rect(10, 20, 30, 40);
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

return {"r1":r1, "s1":s1};
)""") {
  REQUIRE(value.is_table());
  // REQUIRE(value.as_table()["a"] == 678);
  //   zb::print("N", value);
  //   zb::print(vm->get_root());
}
