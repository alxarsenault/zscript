#include "unit_tests.h"

ZS_CODE_TEST("enum_01", R"""(

 enum v1 = {
   a , b ,c, d = "D", e = 8, f = "F", g
 };

 return [v1.a, v1.b, v1.c, v1.d, v1.e, v1.f, v1.g];
)""") {
  REQUIRE(vm.top().as_array()[0] == 0);
  REQUIRE(vm.top().as_array()[1] == 1);
  REQUIRE(vm.top().as_array()[2] == 2);
  REQUIRE(vm.top().as_array()[3] == "D");
  REQUIRE(vm.top().as_array()[4] == 8);
  REQUIRE(vm.top().as_array()[5] == "F");
  REQUIRE(vm.top().as_array()[6] == 9);
}

ZS_CODE_TEST("enum_02", R"""(

 enum v1 {
   a , b ,c, d = "D", e = 8, f = "F", g
 };

 return [v1.a, v1.b, v1.c, v1.d, v1.e, v1.f, v1.g];
)""") {
  REQUIRE(vm.top().as_array()[0] == 0);
  REQUIRE(vm.top().as_array()[1] == 1);
  REQUIRE(vm.top().as_array()[2] == 2);
  REQUIRE(vm.top().as_array()[3] == "D");
  REQUIRE(vm.top().as_array()[4] == 8);
  REQUIRE(vm.top().as_array()[5] == "F");
  REQUIRE(vm.top().as_array()[6] == 9);
}

ZS_CODE_TEST("enum_03", R"""(

var k = 9;
enum v1 {
  a=-1,
  b,
  c = k,
  d = "90",
  e,
};

return {e = v1, a = v1.name_list()};
)""") {

  REQUIRE(!vm.top().is_enum());

  const zs::object& enum_table = vm.top().as_table()["e"];
  REQUIRE(enum_table.is_enum());
  REQUIRE(vm->get(enum_table, zs::_ss("a")) == -1);
  REQUIRE(vm->get(enum_table, zs::_ss("b")) == 0);
  REQUIRE(vm->get(enum_table, zs::_ss("c")) == 9);
  REQUIRE(vm->get(enum_table, zs::_ss("d")) == "90");
  REQUIRE(vm->get(enum_table, zs::_ss("e")) == 10);

  const zs::array_object& name_list = vm.top().as_table()["a"].as_array();
  REQUIRE(name_list[0] == "a");
  REQUIRE(name_list[1] == "b");
  REQUIRE(name_list[2] == "c");
  REQUIRE(name_list[3] == "d");
  REQUIRE(name_list[4] == "e");

  REQUIRE(enum_table.as_table().size() == 0);
  REQUIRE(
      enum_table.as_table().get_delegate().as_table()["__enum_delegate_value_table"].as_table()["a"] == -1);
  REQUIRE(enum_table.as_table().get_delegate().as_table()["__enum_array"].as_array().size() == 5);
}
