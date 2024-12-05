#include "unit_tests.h"

// ZS_CODE_TEST("div_str_01", "return '''alex--bingo''' / '''--''';") {
//   REQUIRE(vm.top().is_array());
//   REQUIRE(vm.top().as_array()[0] == "alex");
//   REQUIRE(vm.top().as_array()[1] == "bingo");
// }
//
// ZS_CODE_TEST("div_str_02", "return '''alex--bingo''' / '''---''';") {
//   REQUIRE(vm.top().is_array());
//   REQUIRE(vm.top().as_array()[0] == "alex--bingo");
// }

// ZS_CODE_TEST("div_02", R"""(
// var a = "A--BC--";
// var b = "--";
// return a / b;
//)""") {
//   REQUIRE(vm.top().is_array());
// }

ZS_CODE_TEST("add_eq_01", R"""(
  var a = 2;
  a += 3;
  return a;
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 5);
}

ZS_CODE_TEST("add_eq_02", R"""(
  var a = 2;
  var b = 5;
  a += b;
  return a;
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 7);
}

ZS_CODE_TEST("add_eq_03", R"""(
  var a = 2;
  var b = 5;
  a += b + 2;
  return a;
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 9);
}

ZS_CODE_TEST("add_eq_04", R"""(
  var a = 2;
  var b = 5;
  a += b * 2;
  return a;
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 12);
}

ZS_CODE_TEST("add_mul_01", R"""(
  var a = 2;
  a *= 3;
  return a;
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 6);
}

ZS_CODE_TEST("add_meta_01", R"""(
  var a = {
    k = 21
  };

  return a;
)""") {}

inline static void include_test_lib(zs::virtual_machine& vm) {
  vm.get_root().as_table()[zs::_s(vm.get_engine(), "CHECK")]
      = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
          CHECK(vm[1].is_if_true());
          return 0;
        });
}

static inline void add_meta_01(zs::virtual_machine& vm) {

  include_test_lib(vm);

  vm.get_root().as_table()[zs::_s(vm.get_engine(), "CreateValueObject")]
      = zs::_nc(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
          zs::object del = zs::_t(vm.get_engine());

          del.as_table()[zs::constants::get<zs::meta_method::mt_add>()]
              = zs::_nc(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
                  const zs::object& rhs = vm[1];

                  if (rhs.is_integer()) {
                    vm.push(vm[0].as_table()["value"]._int + rhs._int);
                    return 1;
                  }
                  else if (rhs.is_string()) {
                    vm.push_string("Bingo");
                    return 1;
                  }
                  else if (rhs.is_table() and rhs.as_table().contains("value")) {
                    vm.push(vm[0].as_table()["value"]._int + rhs.as_table()["value"]._int);
                    return 1;
                  }

                  return -1;
                });

          zs::object obj = zs::_t(vm.get_engine());
          obj.as_table().set_delegate(del);
          obj.as_table()["value"] = vm[1];

          vm.push(obj);
          return 1;
        });
}

ZS_CODE_TEST("add_meta_02", R"""(
var obj = CreateValueObject(10);
var obj2 = CreateValueObject(100);

var a = obj + 2;
CHECK(a == 12);

var b = obj.value;
CHECK(b == 10);

obj.value = 32;
CHECK(obj.value == 32);

CHECK(obj + "Steeve" == "Bingo");

var d = obj + obj2;
CHECK(d == 132);

var c = obj.value;
return c;
)""",
    add_meta_01) {}

static inline void add_meta_02(zs::virtual_machine& vm) { include_test_lib(vm); }

ZS_CODE_TEST("add_meta_03", R"""(
var obj = {};
var del = {};
//set_metadata(obj, obj);
return 56;
)""",
    add_meta_02) {

  REQUIRE(vm.top() == 56);
}

ZS_CODE_TEST("incr_01", R"""(
var i = 1;
var i2 = i++;
return i2;
)""") {
  REQUIRE(value == 1);
}

ZS_CODE_TEST("incr_02", R"""(
var i = 1;
i++;
return i;
)""") {
  REQUIRE(value == 2);
}

ZS_CODE_TEST("incr_03", R"""(
var i = 1;
i = i + 1;
return i;
)""") {
  REQUIRE(value == 2);
}

ZS_CODE_TEST("incr_04", R"""(
var k = 1;
++k;
return k++;
)""") {
  REQUIRE(value == 2);
}

ZS_CODE_TEST("incr_05", R"""(
var k = 1;
return (++k, ++k, k++);
)""") {
  REQUIRE(value == 3);
}

ZS_CODE_TEST("incr_06", R"""(
var k = 1;
var z = 21;
var a = k++;
z = k;
var zz = z;
z++;
return [k, z, zz];
)""") {
  REQUIRE(value == zs::_a(eng, { 2, 3, 2 }));
}
