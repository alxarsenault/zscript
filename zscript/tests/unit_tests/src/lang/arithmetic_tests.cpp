#include "ztests.h"

//
// MARK: Add
//

ZS_CODE_TEST("add_01", R"""(
return 5 + 7.0;
)""") {
  REQUIRE(vm.top() == 12.0);
  REQUIRE(vm.top().is_float());
}

ZS_CODE_TEST("add_02", R"""(
return 5 + 7;
)""") {
  REQUIRE(vm.top() == 12);
  REQUIRE(vm.top().is_integer());
}

ZS_CODE_TEST("add_03", R"""(
return 5 + true;
)""") {
  REQUIRE(vm.top() == 6);
  REQUIRE(vm.top().is_integer());
}

//
// MARK: Sub
//

ZS_CODE_TEST("sub_01", "return 5 - 7.0;") {
  REQUIRE(vm.top() == -2.0);
  REQUIRE(vm.top().is_float());
}

ZS_CODE_TEST("sub_02", "return 5 - 7;") {
  REQUIRE(vm.top() == -2);
  REQUIRE(vm.top().is_integer());
}

ZS_CODE_TEST("sub_str_01", "return '''alex''' - '''le''';") { REQUIRE(vm.top() == "ax"); }
ZS_CODE_TEST("sub_str_02", "return '''Abc''' - 2;") { REQUIRE(vm.top() == "c"); }
ZS_CODE_TEST("sub_str_03", "return '''Abc''' - 1;") { REQUIRE(vm.top() == "bc"); }
ZS_CODE_TEST("sub_str_04", "return '''Abc''' - (-1);") { REQUIRE(vm.top() == "Ab"); }
ZS_CODE_TEST("sub_str_05", "return '''Abcdef''' - -3;") { REQUIRE(vm.top() == "Abc"); }

ZS_CODE_TEST("sub_str_06", "return '''alex--bingo''' - '''--''';") { REQUIRE(vm.top() == "alexbingo"); }
ZS_CODE_TEST("sub_str_07", "return '''a b c d e''' - ''' ''';") { REQUIRE(vm.top() == "abcde"); }
ZS_CODE_TEST("sub_str_08", "return ''' a b c    d e ''' - ''' ''';") { REQUIRE(vm.top() == "abcde"); }

//
// MARK: rshit
//

ZS_CODE_TEST("rshift_01", "return 16 >> 2;") { REQUIRE(vm.top() == 4); }
ZS_CODE_TEST("rshift_02", "return 16 >> 2.0;") { REQUIRE(vm.top() == 4); }

ZS_CODE_TEST("rshift_str_01", "return '''alex''' >> 0;") { REQUIRE(vm.top() == "alex"); }
ZS_CODE_TEST("rshift_str_02", "return '''alex''' >> 1;") { REQUIRE(vm.top() == "ale"); }
ZS_CODE_TEST("rshift_str_03", "return '''alex''' >> 2;") { REQUIRE(vm.top() == "al"); }
ZS_CODE_TEST("rshift_str_04", "return '''alex''' >> 3;") { REQUIRE(vm.top() == "a"); }
ZS_CODE_TEST("rshift_str_05", "return '''alex''' >> 4;") { REQUIRE(vm.top() == ""); }
ZS_CODE_TEST("rshift_str_06", "return '''alex''' >> 5;") { REQUIRE(vm.top() == ""); }

//
// MARK: lshit
//

ZS_CODE_TEST("lshift_01", "return 8 << 2;") { REQUIRE(vm.top() == 32); }
ZS_CODE_TEST("lshift_02", "return 8 << 2.0;") { REQUIRE(vm.top() == 32); }

ZS_CODE_TEST("lshift_str_01", "return '''alex''' << 0;") { REQUIRE(vm.top() == "alex"); }
ZS_CODE_TEST("lshift_str_02", "return '''alex''' << 1;") { REQUIRE(vm.top() == "lex"); }
ZS_CODE_TEST("lshift_str_03", "return '''alex''' << 2;") { REQUIRE(vm.top() == "ex"); }
ZS_CODE_TEST("lshift_str_04", "return '''alex''' << 3;") { REQUIRE(vm.top() == "x"); }
ZS_CODE_TEST("lshift_str_05", "return '''alex''' << 4;") { REQUIRE(vm.top() == ""); }
ZS_CODE_TEST("lshift_str_06", "return '''alex''' << 5;") { REQUIRE(vm.top() == ""); }

//
// MARK: Mul
//

ZS_CODE_TEST("mul_01", "return 32 * 8;") { REQUIRE(vm.top() == 256); }
ZS_CODE_TEST("mul_02", "return 32 * 8.2;") { REQUIRE(vm.top() == 262.4); }
ZS_CODE_TEST("mul_03", "return 32 * true;") { REQUIRE(vm.top() == 32); }
ZS_CODE_TEST("mul_04", "return 32 * false;") { REQUIRE(vm.top() == 0); }

ZS_CODE_TEST("mul_str_01", "return '''alex''' * 2;") { REQUIRE(vm.top() == "alexalex"); }
ZS_CODE_TEST("mul_str_02", "return '''alex''' * 3;") { REQUIRE(vm.top() == "alexalexalex"); }

ZS_CODE_TEST("xor_01", R"""(
return 5 xor 7;
)""") {
  REQUIRE(vm.top() == 2);
}

ZS_CODE_TEST("div_str_01", "return '''alex--bingo''' / '''--''';") {
  REQUIRE(vm.top().is_array());
  REQUIRE(vm.top().as_array()[0] == "alex");
  REQUIRE(vm.top().as_array()[1] == "bingo");
}

ZS_CODE_TEST("div_str_02", "return '''alex--bingo''' / '''---''';") {
  REQUIRE(vm.top().is_array());
  REQUIRE(vm.top().as_array()[0] == "alex--bingo");
}

ZS_CODE_TEST("bitwise_or_01", " return 2 | 4;") { REQUIRE(vm.top() == 6); }

ZS_CODE_TEST("bitwise_or_02", " return 2 | 4.0;") { REQUIRE(vm.top() == 6); }

ZS_CODE_TEST("bitwise_and_01", "return 5 & 7;") { REQUIRE(vm.top() == (5 & 7)); }

ZS_CODE_TEST("bitwise_and_02", "return 5 & 7.0;") {
  REQUIRE(vm.top() == (5 & 7));
  REQUIRE(vm.top().is_integer());
}

ZS_CODE_TEST("mod_01", R"""(
var a = 5;
var b = 7.0;
return a % b;
)""") {
  REQUIRE(vm.top() == 5);
  REQUIRE(vm.top().is_float());
}

ZS_CODE_TEST("mod_02", R"""(
var a = 5;
var b = 7;
return a % b;
)""") {
  REQUIRE(vm.top() == 5);
  REQUIRE(vm.top().is_integer());
}

ZS_CODE_TEST("mod_03", R"""(
var a = 5;
var b = true;
return a % b;
)""") {
  REQUIRE(vm.top() == 0);
  REQUIRE(vm.top().is_integer());
}

ZS_CODE_TEST("div_01", R"""(
var a = 32;
var b = 8;
return a / b;
)""") {
  REQUIRE(vm.top() == 4);
}

ZS_CODE_TEST("div_02", R"""(
var a = "A--BC--";
var b = "--";
return a / b;
)""") {
  REQUIRE(vm.top().is_array());
}

ZS_CODE_TEST("add_str_01", "return '''Abc''' + tostring(32);") { REQUIRE(vm.top() == "Abc32"); }

ZS_CODE_TEST("add_str_02", "return '''Abc''' + tostring(32.2);") { REQUIRE(vm.top() == "Abc32.20"); }

ZS_CODE_TEST("add_str_04", "return '''Abc''' + tostring(true);") { REQUIRE(vm.top() == "Abctrue"); }

ZS_CODE_TEST("add_str_05", "return '''Abc''' + tostring(false);") { REQUIRE(vm.top() == "Abcfalse"); }

ZS_CODE_TEST("mod_str_01", R"""(
var a = "Abc";
var b = "Abcdef";
return a % b;
)""") {
  REQUIRE(vm.top() == "Abc");
}

ZS_CODE_TEST("mod_str_02", R"""(
var a = "Abc";
var b = "A";
return a % b;
)""") {
  REQUIRE(vm.top() == "A");
}

ZS_CODE_TEST("mod_str_03", R"""(
var a = "Abc";
var b = "D";
return a % b;
)""") {
  REQUIRE(vm.top() == "");
}

ZS_CODE_TEST("exp_01", "return 10^2;") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 100);
}

ZS_CODE_TEST("exp_02", "return 10^2^4;") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == std::pow(10, 16));
}

ZS_CODE_TEST("exp_03", "return 10^(2 + 3);") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == std::pow(10, 5));
}

ZS_CODE_TEST("exp_04", "return 10^(2 * 2^2);") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == std::pow(10.0, 8.0));
}

ZS_CODE_TEST("exp_05", R"""(
  const math = import("math");
  float a = math.pi^2
  return a;
)""") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == std::pow(zb::pi<zs::float_t>, 2.0));
}

ZS_CODE_TEST("exp_06", R"""(
  const math = import("math");
  float a = math.pi^2 * 2
  return a;
)""") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == std::pow(zb::pi<zs::float_t>, 2.0) * 2.0);
}

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
//set_delegate(obj, obj);
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
