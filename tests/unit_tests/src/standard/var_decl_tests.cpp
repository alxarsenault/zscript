#include "unit_tests.h"
#include "lex/ztoken.h"
#include "lex/zlexer.h"
#include "lang/jit/zjit_compiler.h"

#include "zvirtual_machine.h"

// static inline void print_stack(zs::virtual_machine* vm) {
//
//   for (zs::int_t i = 0; i < vm->stack_size(); i++) {
//     zb::print("value", i, vm->stack_get(i).to_debug_string());
//   }
// }

//  zs::instruction_vector& ivector
//      =
//      zs::object_proxy::as_function_prototype(zs::object_proxy::as_closure(closure)->_function)
//            ->_instructions;
//  zb::print(ivector._data.size());

//  for (auto it = ivector.begin(); it != ivector.end(); ++it) {
//
//    switch (it.get_opcode()) {
// #define ZS_DECL_OPCODE(name) \
//  case zs::opcode::op_##name: \
//    zb::print(it.get_ref<zs::opcode::op_##name>()); \
// break;
//
// #include "lang/zopcode_def.h"
// #undef ZS_DECL_OPCODE
//
//    default:
//      zb::print(it.get_opcode());
//    }
//  }

//
// MARK: var_decl_int
//

// var a = 55;
ZS_FILE_TEST("var_decl_int_01.zs") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 55);
}

// var a = 0xFF;
ZS_FILE_TEST("var_decl_int_02.zs") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 0xFF);
}

// var<int> a = 32;
ZS_FILE_TEST("var_decl_int_03.zs") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 32);
}

// int a = 32;
ZS_FILE_TEST("var_decl_int_04.zs") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 32);
}

// int π = 32;
ZS_FILE_TEST("var_decl_int_05.zs") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 32);
}

//
// MARK: var_decl_float
//

// var a = 55.5;
ZS_FILE_TEST("var_decl_float_01.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == 55.5);
}

// var a = 4e2;
ZS_FILE_TEST("var_decl_float_02.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == 4e2);
}

// var a = 123.456e-2;
ZS_FILE_TEST("var_decl_float_03.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == 123.456e-2);
}

// var a = 58. + 2;
ZS_FILE_TEST("var_decl_float_04.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == 60.0);
}

//
// MARK: var_decl_string
//

// var a = "peter";
ZS_FILE_TEST("var_decl_string_01.zs") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "peter");
}

// var a = """peter""";
ZS_FILE_TEST("var_decl_string_02.zs") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "peter");
}

// var a = '''peter''';
ZS_FILE_TEST("var_decl_string_03.zs") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "peter");
}

//
// MARK: var_decl_bool
//

// var a = true;
ZS_FILE_TEST("var_decl_bool_01.zs") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == true);
}

// var a = false;
ZS_FILE_TEST("var_decl_bool_02.zs") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == false);
}

//
// MARK: var_decl_empty
//

// var a;
ZS_FILE_TEST("var_decl_empty_01.zs") { REQUIRE(vm.top().is_null()); }

//
// MARK: var_decl_char
//

// char a = 'a';
ZS_FILE_TEST("var_decl_char_01.zs") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 'a');
}

// char a = 'π';
ZS_FILE_TEST("var_decl_char_02.zs") {
  std::string_view value = "π";
  uint32_t ivalue = zb::unicode::next_u8_to_u32_s(value.data());
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == ivalue);
}

//
// MARK: var_decl_null
//

// var a = null;
ZS_FILE_TEST("var_decl_null_01.zs") { REQUIRE(vm[-1].is_null()); }

//
// MARK: var_decl_multi
//

//
// var<int, float> a = 32;
ZS_FILE_TEST("var_decl_multi_01.zs") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 32);
}

// var<int, float, MyClass> a = 32;
ZS_FILE_TEST("var_decl_multi_02.zs") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 32);
}

ZS_FILE_TEST("var_decl_no_return_01.zs") {
  REQUIRE(vm.stack_size() == 2);
  REQUIRE(vm.top().is_null());
}

//
// MARK: include
//

// #include "common.zs"
// var a = 32;
// var c = a + z;
// return c;
// ZS_FILE_TEST("include_01.zs") {
//  REQUIRE(vm.top().is_integer());
//  REQUIRE(vm.top() == 67 + 32);
//}

// #include <common>
// var a = 32;
// var c = a + z;
// return c;
// ZS_FILE_TEST("include_02.zs") {
//  REQUIRE(vm[-1].is_integer());
//  REQUIRE(vm[-1] == 67 + 32);
//}

// TODO: Remove print.
// #include <random_invalid_file>
// var a = 32;
// return a;
// ZS_FILE_TEST("include_03.zs") { REQUIRE(error); }

//
// MARK: import
//

// #import "common.zs"
// var a = 32;
// var c = a + z;
// return c;
// ZS_FILE_TEST("import_01.zs") {
//  REQUIRE(vm.top().is_integer());
//  REQUIRE(vm.top() == 67 + 32);
//}

// #import <common>
// var a = 32;
// var c = a + z;
// return c;
// ZS_FILE_TEST("import_02.zs") {
//  REQUIRE(vm.top().is_integer());
//  REQUIRE(vm.top() == 67 + 32);
//}

// #import "common.zs"
// #import "common.zs"
// var a = 32;
// var c = a + z;
// return c;
// ZS_FILE_TEST("import_03.zs") {
//  REQUIRE(vm.top().is_integer());
//  REQUIRE(vm.top() == 67 + 32);
//}

static inline void native_closure_01(zs::virtual_machine& vm) {
  zs::object bingo = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) {
    REQUIRE(vm.stack_size() == 3);
    //    print_stack(vm);
    REQUIRE(vm->stack()[-1] == "john");
    REQUIRE(vm->stack()[-2] == 5);
    REQUIRE(vm->stack()[-3].is_table());

    vm.push_string("test");
    return 1;
  });

  zs::object& root = vm.get_root();
  zs::table_object* tbl = root._table;
  tbl->set(zs::object::create_small_string("bingo"), bingo);
}

// var a = bingo(5, "john");
// return a;
ZS_FILE_TEST("native_closure_01.zs", native_closure_01) {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "test");
}

// var a = math.sin(5.0);
// return a;
ZS_FILE_TEST("native_closure_02.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == std::sin(5.0));
}

// var a = math.min(5.0, 2.2, 78);
// return a;
ZS_FILE_TEST("native_closure_03.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == 2.2);
}

// var a = math.min(5, 2, 78);
// return a;
ZS_FILE_TEST("native_closure_04.zs") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 2);
}

// var a = math.max(5, 2.2, 78);
// return a;
ZS_FILE_TEST("native_closure_05.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == 78.0);
}

// var a = math.tan(5.0);
// return a;
ZS_FILE_TEST("native_closure_06.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == std::tan(5.0));
}

// var a = math.tan(5);
// var b = math.tan(2.0);
// return b;
ZS_FILE_TEST("native_closure_07.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == std::tan(2.0));
}

static inline void native_closure_08(zs::virtual_machine& vm) {
  zs::object bingo = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) {
    REQUIRE(vm.stack_size() == 2);

    vm->push(vm->stack()[-1]);
    return 1;
  });

  zs::object& root = vm.get_root();
  zs::table_object* tbl = root._table;
  tbl->set(zs::object::create_small_string("bingo"), bingo);
}

// var a = bingo(5);
// var b = bingo(2.0);
// return b;
ZS_FILE_TEST("native_closure_08.zs", native_closure_08) {
  //  REQUIRE(vm.top().is_float());
  //  zb::print(vm.top().to_debug_string());
  REQUIRE(vm.top() == 2.0);
}

// var a = math.ceil(5.2);
// var b = math.sin(1.8);
// var c = a * b + 3;
// return c;
ZS_FILE_TEST("native_closure_09.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top() == (std::ceil(5.2) * std::sin(1.8) + 3));
}

// var a = math.pi;
// return a;
ZS_FILE_TEST("native_closure_10.zs") { REQUIRE(vm.top() == zb::pi<zs::float_t>); }

// var p = math.pi;
// var e = math.e;
// var k = p * e;
// var z = k + math.pi;
// var z2 = z * e + math.pi * math.e;
// return z2 - 23.62;
ZS_FILE_TEST("native_closure_11.zs") {
  zs::float_t k = zb::pi<zs::float_t> * zb::e<zs::float_t>;
  zs::float_t z = k + zb::pi<zs::float_t>;
  zs::float_t z2 = z * zb::e<zs::float_t> + zb::pi<zs::float_t> * zb::e<zs::float_t>;
  zs::float_t r = z2 - 23.62;
  REQUIRE_THAT(vm.top()._float, Catch::Matchers::WithinAbs(r, 0.01));
}

// var a = math.pi - 51.28;
// var b = 2.0 / a;
// return b;
ZS_FILE_TEST("native_closure_12.zs") {
  zs::float_t a = zb::pi<zs::float_t> - 51.28;
  zs::float_t r = 2.0 / a;
  REQUIRE_THAT(vm.top()._float, Catch::Matchers::WithinAbs(r, 0.01));
}

//
// MARK: user_data
//

// var a = math.normal_dist(1.0, 2.0);
// return a;
ZS_FILE_TEST("user_data_01.zs") { REQUIRE(vm.top().is_user_data()); }

// var a = math.normal_dist(1.0, 2.0);
// return a.gen();
ZS_FILE_TEST("user_data_02.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE_THAT(vm.top()._float, Catch::Matchers::WithinAbs(1.0, 8.0));
}

// var a = math.normal_dist(1.0, 2.0);
// return a();
// ZS_FILE_TEST("user_data_03.zs") {
//   REQUIRE(vm.top().is_float());
//   REQUIRE_THAT(vm.top()._float,
//   Catch::Matchers::WithinAbs(1.0, 8.0));
// }

// var a = math.normal_dist(1.0, 2.0);
// return a.get_mean();
ZS_FILE_TEST("user_data_04.zs") {
  REQUIRE(vm.top().is_float());
  REQUIRE(vm.top()._float == 1.0);
}

// var a = math.normal_dist(1.0, 2.0);
// return a.mean;
// ZS_FILE_TEST("user_data_05.zs") {
//   REQUIRE(vm.top().is_float());
//   REQUIRE(vm.top()._float == 1.0);
// }

// var a = math.normal_dist(1.0, 2.0);
// return a.stddev;
// ZS_FILE_TEST("user_data_06.zs") {
//   REQUIRE(vm.top().is_float());
//   REQUIRE(vm.top()._float == 2.0);
// }

// var a = math.normal_dist(1.0, 2.0);
// a.mean = 3.0;
// return a.mean + a.get_mean();
// ZS_FILE_TEST("user_data_07.zs") {
//   REQUIRE(vm.top().is_float());
//   REQUIRE(vm.top()._float == 6.0);
// }

ZS_FILE_TEST("typeid_01.zs") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "array");
}

ZS_FILE_TEST("typeid_02.zs") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "integer");
}

ZS_FILE_TEST("typeid_03.zs") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "float");
}

ZS_FILE_TEST("typeid_04.zs") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "string");
}

ZS_FILE_TEST("typeid_05.zs") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "null");
}

ZS_FILE_TEST("typeof_01.zs") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "integer");
}

static inline void typeof_02(zs::virtual_machine& vm) {
  zs::object t = zs::object::create_table(vm.get_engine());
  zs::object delegate = zs::object::create_table(vm.get_engine());
  delegate._table->set(zs::constants::get<zs::meta_method::mt_typeof>(),
      zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
        vm->push(zs::object::create_small_string("john"));
        return 1;
      }));

  t.set_delegate(delegate);

  zs::object& root = vm.get_root();
  zs::table_object* tbl = root._table;
  tbl->set(zs::object::create_small_string("bingo"), t);
}

ZS_FILE_TEST("typeof_02.zs", typeof_02) {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "john");
}

static inline void typeof_03(zs::virtual_machine& vm) {
  zs::object t = zs::object::create_table(vm.get_engine());
  zs::object& root = vm.get_root();

  REQUIRE(!vm.set(root, zs::object::create_small_string("bingo"), t));
}

ZS_FILE_TEST("typeof_03.zs", typeof_03) {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "table");
}

//
// MARK: function
//

ZS_CODE_TEST("function_01", R"""(
  var a = function() {};
  return a;
)""") {
  REQUIRE(vm.top().is_closure());
}

ZS_CODE_TEST("function_02", R"""(
  var a = function() {
    return 55;
  };

  return a;
)""") {
  REQUIRE(vm.top().is_closure());

  zs::object ret;
  REQUIRE(!vm->call(vm.top(), 1, vm.stack_size() - 1, ret));
  REQUIRE(ret == 55);
}

ZS_CODE_TEST("function_03", R"""(
  var a = function() {
    return "john";
  };

  return a;
)""") {
  REQUIRE(vm.top().is_closure());

  zs::object ret;
  REQUIRE(!vm->call(vm.top(), 1, vm.stack_size() - 1, ret));
  REQUIRE(ret == "john");
}

ZS_CODE_TEST("function_04", R"""(
  var a = {
    f = function() {
      return "peter";
    },

    f2 = function() {
      return 192;
    }
  };

  return a;
)""") {
  REQUIRE(vm.top().is_table());

  zs::object_unordered_map<zs::object>& map = *vm.top().get_table_internal_map();

  REQUIRE(map.size() == 2);
  REQUIRE(map[zs::_ss("f")].is_function());
  REQUIRE(map[zs::_ss("f2")].is_function());

  zs::object ret;
  REQUIRE(!vm->call(map[zs::_ss("f")], 1, vm.stack_size() - 1, ret));
  REQUIRE(ret == "peter");

  REQUIRE(!vm->call(map[zs::_ss("f2")], 1, vm.stack_size() - 1, ret));
  REQUIRE(ret == 192);
}

ZS_CODE_TEST("function_05", R"""(
  var a = function() {
    return 55;
  };

  var b = a();

  return b;
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 55);
}

ZS_CODE_TEST("function_06", R"""(
  var a = function(k) {
    return k + 1;
  }

  return a(34);
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 35);
}

ZS_CODE_TEST("function_07", R"""(
  var a = function(k1, k2) {
    return k1 + k2;
  }

  return a(34, 26);
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 60);
}

ZS_CODE_TEST("function_08", R"""(
  return function() { return 55; }();
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 55);
}

// ZS_CODE_TEST("function_08", R"""(
//
//   var a = function() {
//     var my_fct = function() {
//       return 10;
//     };
//
//     return my_fct();
//   }
//
//   var b = a();
//
//   return b;
//)""") {
////  REQUIRE(vm.top().is_integer());
////  REQUIRE(vm.top() == 10);
//}

ZS_CODE_TEST("operator_01", R"""(
  var a = {};
  a.operator(typeof) = "john";
  return a;
)""") {
  REQUIRE(vm.top().is_table());
  zs::object_unordered_map<zs::object>& map = *vm.top().get_table_internal_map();

  REQUIRE(map[zs::constants::get<zs::meta_method::mt_typeof>()].is_string());
  REQUIRE(map[zs::constants::get<zs::meta_method::mt_typeof>()] == "john");
}

ZS_CODE_TEST("operator_02", R"""(
  var a = {};
  a.operator(typeof) = "john";
  return typeof(a);
)""") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "john");
}

ZS_CODE_TEST("operator_03", R"""(
  var a = {};
  a.operator(typeof) = function() {return "john";};
  return typeof(a);
)""") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "john");
}

ZS_CODE_TEST("operator_04", R"""(
  var a = {};
  var b = {
    operator(typeof) = function() { return "john"; }
  };

  // Set 'b' as delegate.
  zs::set_metadata(a, b);
  return typeof(a);
)""") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "john");
}

ZS_CODE_TEST("operator_05", R"""(
  var a = {};
  var b = {
    operator(typeof) = function() { return "john"; }
  };
   zs::set_metadata(b, null);
  // Set 'b' as delegate.
  zs::set_metadata(a, b);

 return typeof(a);
)""") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "john");
}

ZS_CODE_TEST("operator_06", R"""(
var a = { k1 = 56};
var b = {
  operator(get) = function(key, value) { return "john"; }
};

// Set 'b' as delegate.
zs::set_metadata(a, b);


return a.bababa;
)""") {
  REQUIRE(vm.top().is_string());
  REQUIRE(vm.top() == "john");
}

static inline void operator_07(zs::virtual_machine& vm) {
  zs::object t = zs::object::create_table(vm.get_engine());
  zs::object delegate = zs::object::create_table(vm.get_engine());

  zs::table_object* tbl = delegate._table;
  tbl->set(zs::constants::get<zs::meta_method::mt_typeof>(),
      zs::_nc(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
        vm->push(zs::_ss("john"));
        return 1;
      }));

  t.set_delegate(delegate);

  zs::object& root = vm.get_root();
  zs::table_object* root_tbl = root._table;
  root_tbl->set(zs::_ss("bingo"), t);
}
ZS_CODE_TEST("operator_07", R"""(
var a = {};
a.operator(typeof) = "john";
return typeof(a);
)""",
    operator_07) {
  REQUIRE(vm.top().is_string());
}

ZS_CODE_TEST("operator_08", R"""(
var a = { k1 = 56};

var b = {
  operator(get) = function(obj, key) {
    return none;
  }
};

zs::set_metadata(b, null);

// Set 'b' as delegate.
zs::set_metadata(a, b);


return a.k1;
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 56);
}

ZS_CODE_TEST("operator_09", R"""(
var a = { k1 = 56};

var b = {
  operator(get) = function(obj, key) {
    return "john";
  }
};

// Set 'b' as delegate.
zs::set_metadata(a, b);


return a.k1;
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 56);
}

// ZS_CODE_TEST("operator_10", R"""(
//  var a = { k1 = 56};
//
//  var b = {
//    operator(get) = function(obj, key) {
//      return key;
//    }
//  };
//
//  // Set 'b' as delegate.
//  set_delegate(a, b);
//
//
// return a.k2;
//)""") {
//  REQUIRE(vm.top().is_string());
//  REQUIRE(vm.top() == "k2");
//}

// ZS_CODE_TEST("operator_11", R"""(
//  var a = { k1 = 56};
//
//  var b = {
//    operator(get) = function(obj, key) {
//      return obj.k1;
//    }
//  };
//
//  // Set 'b' as delegate.
//  set_delegate(a, b);
//
//
// return a.k2;
//)""") {
//  REQUIRE(vm.top().is_integer());
//  REQUIRE(vm.top() == 56);
//}

#if ZS_TEST_PRINT_METHODS
ZS_CODE_TEST("print_01", R"""(
print(1);
)""") {}

ZS_CODE_TEST("print_02", R"""(
print(1, "bacon", "ballon", 23);
)""") {}
#endif // ZS_TEST_PRINT_METHODS

static inline void global_table_01(zs::virtual_machine& vm) {
  zs::object t = zs::object::create_table(vm.get_engine());
  zs::object& root = vm.get_root();
  zs::table_object* root_tbl = root._table;
  root_tbl->set(zs::_ss("john"), t);
}
ZS_CODE_TEST("global_table_01", R"""(
var a = {};

john.k = 32;
return a;
)""",
    global_table_01) {
  REQUIRE(vm.top().is_table());
}

static inline void function_test_01(zs::virtual_machine& vm) {

  //  zs::table_object* tbl = zs::object_proxy::as_table(t);

  // Create sub_table.
  zs::object sub_table = zs::object::create_table(vm.get_engine());

  zs::object fct = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
    zs::int_t nags = vm.stack_size();

    if (nags != 1) {
      zb::print("INVALID CALL");
      return -1;
    }

    {
      zs::object& o = vm[0];
      REQUIRE(o.is_table());

      zs::table_object* tbl = o._table;
      REQUIRE(tbl->contains(zs::_ss("fct")));
    }

    vm.push_string("Banana");
    return 1;
  });

  // Add fct to sub_table.
  if (auto err = vm.set(sub_table, zs::_ss("fct"), fct)) {
    zb::print(err);
    return;
  }

  // Create main_table.
  zs::object main_table = zs::object::create_table(vm.get_engine());

  // Add sub_table to main_table.
  if (auto err = vm.set(main_table, zs::_ss("sub_table"), sub_table)) {
    zb::print(err);
    return;
  }

  // Add main_table to root.
  if (auto err = vm.set(vm.get_root(), zs::_ss("main_table"), main_table)) {
    zb::print(err);
    return;
  }
}

ZS_CODE_TEST("function_test_01", R"""(
return main_table.sub_table.fct();
)""",
    function_test_01) {
  REQUIRE(vm.top() == "Banana");
}

namespace function_test_02 {
// static inline void print_stack(zs::vm_ref vm) {
//
//   for (zs::int_t i = 0; i < vm->stack_size(); i++) {
//     zb::print("value", i, vm->stack_get(i).to_debug_string());
//   }
// }

struct vec3 {
  float x, y, z;
};

static inline zs::int_t function_vec3_get_x(zs::vm_ref vm) {
  zs::int_t nags = vm.stack_size();

  //  zb::print("--------------------------");
  //  print_stack(vm);

  //  zb::print("nags", nags);
  if (nags != 1) {
    zb::print("INVALID CALL");
    return -1;
  }

  zs::var& o = vm[0];

  if (!o.is_user_data()) {
    zb::print("Not a user data");
    return -1;
  }

  vec3& v3 = o._udata->data_ref<vec3>();
  //  zb::print(std::span<const float>((const float*)&v3, 3));
  vm.push_float(v3.x);
  //  vm.push(o._udata.data_ref<vec3>().x);
  return 1;
}

static inline zs::int_t function_vec3_get_y(zs::vm_ref vm) {
  zs::int_t nags = vm.stack_size();

  //  zb::print("--------------------------");
  //  print_stack(vm);

  //  zb::print("nags", nags);
  if (nags != 1) {
    zb::print("INVALID CALL");
    return -1;
  }

  zs::var& o = vm[0];

  if (!o.is_user_data()) {
    zb::print("Not a user data");
    return -1;
  }

  vec3& v3 = o._udata->data_ref<vec3>();
  // zb::print(std::span<const float>((const float*)&v3, 3));
  vm.push_float(v3.y);
  //  vm.push(o._udata.data_ref<vec3>().x);
  return 1;
}

static inline zs::int_t function_vec3_set_x(zs::vm_ref vm) {
  zs::int_t nags = vm.stack_size();

  // zb::print("nags", nags);
  if (nags != 2) {
    zb::print("INVALID CALL");
    return -1;
  }

  zs::var& o = vm[0];

  if (!o.is_user_data()) {
    zb::print("Not a user data");
    return -1;
  }

  float x = o._udata->data_ref<vec3>().x;
  if (auto err = vm[1].get_value(x)) {
    zb::print("Invalid x value");
    return -1;
  }

  o._udata->data_ref<vec3>().x = x;
  vm.push(o);
  return 1;
}

// std::string operator ""_w(const char16_t*, size_t);

inline constexpr zs::var operator""_ss(const char* str, size_t) { return zs::_ss(str); }

static inline zs::int_t function_create_vec3(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();

  zs::int_t nargs = vm.stack_size();

  float x = 0;
  float y = 0;
  float z = 0;

  if (nargs == 1) {
  }
  else if (nargs == 2) {
    if (auto err = vm[1].get_value(x)) {
      zb::print("Invalid value type");
      return -1;
    }

    y = x;
    z = x;
  }
  else if (nargs == 3) {
    if (auto err = vm[1].get_value(x)) {
      zb::print("Invalid value type");
      return -1;
    }

    if (auto err = vm[2].get_value(y)) {
      zb::print("Invalid value type");
      return -1;
    }

    z = 0;
  }
  else if (nargs == 4) {
    if (auto err = vm[1].get_value(x)) {
      zb::print("Invalid value type");
      return -1;
    }

    if (auto err = vm[2].get_value(y)) {
      zb::print("Invalid value type");
      return -1;
    }

    if (auto err = vm[3].get_value(z)) {
      zb::print("Invalid value type");
      return -1;
    }
  }
  else {
    zb::print("WHDJAHDJK");
    return -1;
  }

  zs::var vec = zs::var::create_user_data<vec3>(eng, vec3{ x, y, z });

  zs::var delegate = zs::var::create_table(eng);
  zs::var get_x_fct = zs::var::create_native_closure(eng, function_vec3_get_x);
  zs::var get_y_fct = zs::var::create_native_closure(eng, function_vec3_get_y);
  zs::var set_x_fct = zs::var::create_native_closure(eng, function_vec3_set_x);
  REQUIRE(!vm->set(delegate, zs::_ss("get_x"), get_x_fct));
  REQUIRE(!vm->set(delegate, zs::_ss("get_y"), get_y_fct));
  REQUIRE(!vm->set(delegate, "set_x"_ss, set_x_fct));
  vec.set_delegate(delegate);

  vm.push(vec);
  return 1;
}

static inline void test_function(zs::virtual_machine& vm) {
  zs::engine* eng = vm.get_engine();
  //  zs::table_object* tbl = zs::object_proxy::as_table(t);

  zs::var create_vec3_closure = zs::var::create_native_closure(eng, function_create_vec3);

  zs::var fct = zs::var::create_native_closure(eng, [](zs::vm_ref vm) -> zs::int_t {
    zs::int_t nags = vm.stack_size();

    zb::print("nags", nags);
    if (nags != 2) {
      zb::print("INVALID CALL");
      return -1;
    }

    {
      zs::var& o = vm[0];
      zb::print("o.get_type()", o.get_type());
      zb::print("o.is_table()", o.is_table());
      zs::table_object* tbl = o._table;

      zb::print("tbl->contains(zs::_ss(fct))", tbl->contains(zs::_ss("fct")));
    }

    return 1;
  });

  // Create sub_table.
  zs::var sub_table = zs::var::create_table(vm.get_engine());

  // Add fct to sub_table.
  if (auto err = vm.set(sub_table, zs::_ss("fct"), fct)) {
    zb::print(err);
    return;
  }

  // Create main_table.
  zs::var main_table = zs::var::create_table(vm.get_engine());

  // Add sub_table to main_table.
  if (auto err = vm.set(main_table, zs::_ss("sub_table"), sub_table)) {
    zb::print(err);
    return;
  }

  // Add main_table to root.
  if (auto err = vm.set(vm.get_root(), zs::_ss("main_table"), main_table)) {
    zb::print(err);
    return;
  }

  if (auto err = vm.set(vm.get_root(), zs::_ss("vec3"), create_vec3_closure)) {
    zb::print(err);
    return;
  }
}

} // namespace function_test_02

// Bug a = v3.get_x() doesn't work.
ZS_CODE_TEST("function_test_02", R"""(
var v3 = vec3(27, 31, 4);
var a = 66;
a = v3.get_x();

var b = v3.get_x();
var c = v3.get_y();

v3.set_x(89);
var d = v3.get_x();
return [a, b, c, d];
)""",
    function_test_02::test_function) {
  //  zb::print(vm.top().convert_to_string());

  REQUIRE(vm.top().is_array());

  REQUIRE(vm.top()[0]);
  REQUIRE(vm.top()[1]);
  REQUIRE(vm.top()[2]);
  REQUIRE(vm.top()[3]);
  REQUIRE(*vm.top()[0] == 27);
  REQUIRE(*vm.top()[1] == 27);
  REQUIRE(*vm.top()[2] == 31);
  REQUIRE(*vm.top()[3] == 89);
}

ZS_CODE_TEST("function_test_03", R"""(
var a = 10;
var b = 80;
var c = 2;
var fct = function() {
  return (a * b) + c;
};

return fct();
)""") {
  REQUIRE(vm.top() == 802);
}

ZS_CODE_TEST("function_test_04", R"""(
var a = 10;
var b = 80;
var c = 2;
var fct = function() {
  var fct2 = function() {
    return (a * b) + c;
  };

  return fct2();
};

return fct();
)""") {
  REQUIRE(vm.top() == 802);
}

ZS_CODE_TEST("function_test_05", R"""(
var a = 10;
var b = 80;
var c = 2;
var fct = function() {
  var d = 100;
  var fct2 = function() {
    return d + (a * b) + c;
  };

  return fct2();
};

return fct();
)""") {
  REQUIRE(vm.top() == 902);
}

ZS_CODE_TEST("function_test_06", R"""(
var a = 10;
var b = 80;
var c = 2;
var gfunc = function() {
  return a;
};

var t = {
  fct = function() {
    return gfunc();
  }
};

return t.fct();
)""") {
  REQUIRE(vm.top() == 10);
}

ZS_CODE_TEST("function_test_07", R"""(
var a = 10;
var b = 80;
var c = 2;
var gfunc = function(z) {
  return a + z;
};

var t = {
  fct = function(k) {
    return gfunc(k);
  }
};

return t.fct(2);
)""") {
  REQUIRE(vm.top() == 12);
}

ZS_CODE_TEST("function_test_08", R"""(
var a = 10;

var t0 = {
  gfunc = function(z) {
    return a + z;
  }
};

var t = {
  fct = function(k) {
    return t0.gfunc(k);
  }
};

return t.fct(10);
)""") {
  REQUIRE(vm.top() == 20);
}

ZS_CODE_TEST("function_test_09", R"""(
var f = function() { return 10; };
var c = 100;

var f1 = function(x, y = f(), z = c) {
  return x + y + z;
};

var a = 200;

return f1(12, 10);
)""") {
  REQUIRE(vm.top() == 122);
}

ZS_CODE_TEST("function_test_10", R"""(
var f = function() { return 10; };
var c = 100;

var f1 = function(x, y = f(), z = c) {
  return x + y + z;
};

var a = 200;

return f1(12);
)""") {
  REQUIRE(vm.top() == 122);
}

// ZS_CODE_TEST("delegate_01", R"""(
//
// var banana_delegate = {
//   get_name = $() {
//     return this.name;
//   }
//
//   set_name = $(name) {
//     this.name = name;
//   }
// };
//
// function create_banana(name, obj = {}) {
//   obj.name = name;
//   set_delegate(obj, banana_delegate);
//   return obj;
// }
//
//
//
// var b1 = create_banana("A");
//
// var out = b1.get_name();
//
// b1.set_name("B");
//
// out += b1.get_name();
// var b2 = create_banana("C", {});
// out += b2.get_name();
//
// return out;
//)""") {
//   REQUIRE(vm.top() == "ABC");
// }

ZS_CODE_TEST("if_return", R"""(

var v1 = 23;
var v2 = 21;
var v3 = v2 - v1;

var a = true;
var zz = "john";
var arr = [1, 2, 3];
arr[1] = 10;

// zz[2] = 'P';

var k = function() {
  return [zz[2], arr[1], v3];
}

if(a) {
  return k();
}

return 33;
)""") {
  REQUIRE(vm.top().is_array());
  REQUIRE(vm.top().as_array()[0] == 'h');
  REQUIRE(vm.top().as_array()[1] == 10);
  REQUIRE(vm.top().as_array()[2] == -2);
}

struct my_object {

  static size_t destruction_count;

  my_object(zs::int_t e)
      : _value(e) {}

  ~my_object() { destruction_count++; }

  zs::int_t _value;
};

size_t my_object::destruction_count = 0;

static inline void if_test_01(zs::virtual_machine& vm) {

  my_object::destruction_count = 0;

  vm.get_root().as_table()["my_object"] = zs::_nc(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
    zs::object myobj = zs::_u<my_object>(vm.get_engine(), vm[1]._int);

    myobj.set_user_data_uid(zs::_ss("my_object"));
    vm.push(myobj);
    return 1;
  });
}

ZS_CODE_TEST("if_test_01", R"""(
var k;

if(true) {
  var a = my_object(33);
  var b = 32332;
  var d = a;
  k = d;
}

return k;
)""",
    if_test_01) {
  REQUIRE(vm.top().is_user_data());
  zs::var obj_uid;
  REQUIRE(!vm.top().get_user_data_uid(obj_uid));
  REQUIRE(obj_uid == "my_object");
  REQUIRE(vm.top().as_udata().data_ref<my_object>()._value == 33);
  REQUIRE(my_object::destruction_count == 0);
}

TEST_CASE("DSDKSKLDS") {
  zs::engine eng;
  {
    zs::vm vm(&eng);

    //  std::string filepath;
    std::string code = R"""(
      var k;
      var c1 = check_count();
      var c2;
      if(true) {
        var a = my_object(33);
        {
          var b = my_object(34);
        }
        c2 = check_count();
        var d = my_object(35);
        k = a;
      }
      var c3 = check_count();
      return [k, c1, c2, c3];
  )""";

    {
      my_object::destruction_count = 0;

      vm->get_root().as_table()["my_object"] = zs::_nc(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
        zs::object myobj = zs::_u<my_object>(vm.get_engine(), vm[1]._int);

        myobj.set_user_data_uid(zs::_ss("my_object"));
        vm.push(myobj);
        return 1;
      });

      vm->get_root().as_table()["check_count"] = zs::_nc(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
        vm.push(my_object::destruction_count);
        return 1;
      });
    }

    zs::object closure;
    if (auto err = vm->compile_buffer(code, "test", closure); err || !closure.is_closure()) {
      zb::print(err, vm.get_error());
      return;
    }

    zs::object arr;
    if (auto err = vm->call(closure, { vm->get_root() }, arr)) {
      zb::print(ZB_CURRENT_SOURCE_LOCATION(), err, vm.get_error());
      REQUIRE(false);
    }

    REQUIRE(arr.is_array());

    zs::object val = arr.as_array()[0];
    REQUIRE(val.is_user_data());

    REQUIRE(val.get_user_data_uid() == "my_object");
    REQUIRE(val.as_udata().data_ref<my_object>()._value == 33);
    REQUIRE(my_object::destruction_count == 2);

    REQUIRE(arr.as_array()[1] == 0);
    REQUIRE(arr.as_array()[2] == 1);
    REQUIRE(arr.as_array()[3] == 2);
  }

  REQUIRE(my_object::destruction_count == 3);
}
// static inline void for_test_01(zs::virtual_machine& vm) {
//
//   vm.get_root().as_table()["my_object"] = zs::_nc(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t {
//     vm.push(zs::_u<my_object>(vm.get_engine(), vm[1]._int));
//     return 1;
//   });
// }
//
// ZS_CODE_TEST("for_test_01", R"""(
// var k;
//
// for(var i = 0; i < 10; i++) {
// print(i);
// }
//
////print("JOHN");
// return k;
//)""",
//     for_test_01) {
//   zb::print(vm.top());
// }

// TEST_CASE("for_test_02") {
//
//   static constexpr std::string_view code = R"""(
//
// var i = 0;
// for(; i < 10; i++) {
////print(i);
// var k = 32;
// k = k * 32;
// }
// return 33;
//)""";
//
//   zs::vm vm;
//   zs::object_ptr closure;
//
//   if (auto err = vm->compile_buffer(code, "test", closure)) {
//     REQUIRE(false);
//     return;
//   }
//
//   REQUIRE(closure.is_closure());
//
//   zs::object_ptr value;
//   //
//   zs::int_t n_params = 1;
//   vm.push_root();
//
//   closure._closure->get_function_prototype()->_instructions.serialize();
//   closure._closure->get_function_prototype()->debug_print();
//   //
//   if (auto err = vm->call(closure, n_params, vm.stack_size() - n_params, value)) {
//     //
//     REQUIRE(false);
//     return;
//   }
//
//   zb::print("VALUE", value);
// }
