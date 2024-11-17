#include "unit_tests.h"

#define ZS_CHECK_TABLE()     \
  REQUIRE(value.is_table()); \
  zs::table_object& tbl = value.as_table();

ZS_CODE_TEST("table_01", "return {};") {
  ZS_CHECK_TABLE();
  REQUIRE(tbl.size() == 0);
}

ZS_CODE_TEST("table_02", "return { k = 1 };") {
  ZS_CHECK_TABLE();
  REQUIRE(tbl.size() == 1);
}

ZS_CODE_TEST("table_03", "return { a = 1, b = 2 };") {
  ZS_CHECK_TABLE();
  REQUIRE(tbl.size() == 2);
  REQUIRE(tbl["a"] == 1);
  REQUIRE(tbl["b"] == 2);
}

ZS_CODE_TEST("table_04", "return { a = 1, b = 2, c = '''asasassasas\nassasasasasa\n''' };") {
  ZS_CHECK_TABLE();
  REQUIRE(tbl.size() == 3);
  REQUIRE(tbl["a"] == 1);
  REQUIRE(tbl["b"] == 2);
  REQUIRE(tbl["c"] == "asasassasas\nassasasasasa\n");
}

ZS_CODE_TEST("table_05", "return { a = 1, b = 2, c = {} };") {
  ZS_CHECK_TABLE();
  REQUIRE(tbl.size() == 3);
  REQUIRE(tbl["a"] == 1);
  REQUIRE(tbl["b"] == 2);
  REQUIRE(tbl["c"].is_table());
}

ZS_CODE_TEST("table_06", R"""(
   var a = { "a": 1, "b": 2 };
   return a;
)""") {
  ZS_CHECK_TABLE();
  REQUIRE(tbl.size() == 2);
  REQUIRE(tbl["a"] == 1);
  REQUIRE(tbl["b"] == 2);
}

ZS_CODE_TEST("table_07", R"""(
   var a = { a = { a = 23, b = {} }, b = [1, 2, 3, {}] };
   return a;
)""") {
  ZS_CHECK_TABLE();
  REQUIRE(tbl.size() == 2);

  {
    REQUIRE(tbl["a"].is_table());
    zs::object_unordered_map<zs::object>& amap = *tbl[zs::_ss("a")].get_table_internal_map();
    REQUIRE(amap.size() == 2);
    REQUIRE(amap[zs::_ss("a")] == 23);
    REQUIRE(amap[zs::_ss("b")].is_table());
  }

  {
    REQUIRE(tbl["b"].is_array());
    zs::vector<zs::object>& vec = *tbl[zs::_ss("b")].get_array_internal_vector();
    REQUIRE(vec.size() == 4);
  }
}

ZS_CODE_TEST("table_08", "return {a = 90, b = 78}.size();") { REQUIRE(value == 2); }
ZS_CODE_TEST("table_09", "return {a = 90, a = 78}.size();") { REQUIRE(value == 1); }

ZS_CODE_TEST("table_10", R"""(
var a = {[0] = 78, [1] = 79};
a[1] = 80;
++a[0];
return [a.size(), a[0], a[1]];
)""") {
  REQUIRE(value == zs::_a(eng, { 2, 79, 80 }));
}
