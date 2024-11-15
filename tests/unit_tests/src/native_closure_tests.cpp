
#include "unit_tests.h"
#include <zscript.h>
#include <zbase/utility/print.h>
#include "lex/ztoken.h"
#include "lex/zlexer.h"

#include "lang/jit/zjit_compiler.h"

#include "objects/zfunction_prototype.h"
#include "zvirtual_machine.h"

struct native_closure_tests {};

template <>
struct zs::internal::test_helper<native_closure_tests> {
  using closure_type = zs::native_closure_object::closure_type;

  static inline closure_type get_closure_type(const zs::object& obj) { return obj._native_closure->_ctype; }

  static inline zs::vector<uint32_t> get_type_check(const zs::object& obj) {
    return obj._native_closure->_type_check;
  }
};

using native_closure_accessor = zs::internal::test_helper<native_closure_tests>;

TEST_CASE("zs::object_ptr::native_closure") {
  {

    zs::vm vm;

    bool did_call = false;
    zs::object obj = zs::object::create_native_closure(vm.get_engine(), [&](zs::vm_ref v) {
      REQUIRE(v.stack_size() == 0);
      did_call = true;
      return 3;
    });

    REQUIRE(obj.is_native_closure());
    REQUIRE(native_closure_accessor::get_closure_type(obj) == native_closure_accessor::closure_type::obj);

    REQUIRE(obj._native_closure->call(vm) == 3);
    REQUIRE(did_call);
  }

  {
    zs::engine eng;
    zs::object obj
        = zs::object::create_native_closure(&eng, [](zs::virtual_machine* vm) -> zs::int_t { return 0; });

    REQUIRE(obj.is_native_closure());
    REQUIRE(native_closure_accessor::get_closure_type(obj) == native_closure_accessor::closure_type::c_func);
  }

  {
    zs::engine eng;
    zs::object obj = zs::object::create_native_closure(&eng, [](zs::vm_ref vm) -> zs::int_t { return 0; });

    REQUIRE(obj.is_native_closure());
    REQUIRE(native_closure_accessor::get_closure_type(obj) == native_closure_accessor::closure_type::c_func);
  }

  {
    zs::vm vm;
    std::string str = "djhsajkdhsajkdhsajdhsajkhdjksahjdsajkd";
    zs::object obj
        = zs::object::create_native_closure(vm.get_engine(), [str](zs::vm_ref vm) -> zs::int_t { return 0; });

    REQUIRE(obj.is_native_closure());
    REQUIRE(native_closure_accessor::get_closure_type(obj) == native_closure_accessor::closure_type::obj);

    obj._native_closure->call(vm);
  }
}

TEST_CASE("zs::native_closure::release") {
  {

    zs::vm vm;

    zs::object obj
        = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t { return 0; });

    zs::object obj2(obj._native_closure, true);
    REQUIRE(obj2.is_native_closure());
  }
  {
    zs::vm vm;

    bool did_call_release = false;

    {

      zs::object obj
          = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t { return 0; });

      obj.set_closure_user_pointer(&did_call_release);
      obj.set_closure_release_hook([](zs::engine* eng, zs::raw_pointer_t uptr) { (*(bool*)uptr) = true; });

      REQUIRE(obj.is_native_closure());
      REQUIRE(
          native_closure_accessor::get_closure_type(obj) == native_closure_accessor::closure_type::c_func);

      obj._native_closure->call(vm);
    }

    REQUIRE(did_call_release);
  }
}

TEST_CASE("zs::native_closure::dsds") {
  {

    zs::vm vm;

    zs::object obj
        = zs::object::create_native_closure(vm.get_engine(), [](zs::vm_ref vm) -> zs::int_t { return 0; });

    REQUIRE(!obj._native_closure->parse_type_check(2, "ts|n"));

    zs::vector<uint32_t> type_check = native_closure_accessor::get_type_check(obj);

    REQUIRE(type_check[0] == ((uint32_t)zs::get_object_type_mask(zs::object_type::k_table)));

    REQUIRE(type_check[1]
        == ((uint32_t)zs::get_object_type_mask(zs::object_type::k_long_string)
            | (uint32_t)zs::get_object_type_mask(zs::object_type::k_small_string)
            | (uint32_t)zs::get_object_type_mask(zs::object_type::k_integer)
            | (uint32_t)zs::get_object_type_mask(zs::object_type::k_float)));
  }
}

//
//
///
//
//

TEST_CASE("function_prototype.serialize") {
  constexpr std::string_view code = R"""(
var a = 32;

var b = function() {
  return 2;
}

return a;
)""";

  zs::vm vm;
  zs::engine* eng = vm.get_engine();

  zs::object closure;
  REQUIRE(!vm->compile_buffer(code, "test", closure));
  REQUIRE(closure.is_closure());

  zs::var ret_value;
  vm.push_root();
  auto err = vm->call_from_top(closure, 1, ret_value);
  REQUIRE(!err);
  REQUIRE(ret_value == 32);

  REQUIRE(closure.is_closure());

  zs::function_prototype_object* fct = closure._closure->get_function_prototype();

  zs::vector<uint8_t> buffer((zs::allocator<uint8_t>(eng)));

  err = fct->serialize(buffer);
  REQUIRE(!err);

  size_t offset = 0;
  zs::object new_data;
  err = zs::object::from_binary(eng, buffer, new_data, offset);
  REQUIRE(!err);

  //  zb::print("function_prototype.serialize");
  //  zb::print(new_data);
  REQUIRE(new_data.is_table());
  REQUIRE(new_data._table->contains(zs::_ss("source_name")));
  REQUIRE(new_data._table->get_map()[zs::_ss("source_name")] == "test");

  REQUIRE(new_data._table->contains(zs::_ss("name")));
  REQUIRE(new_data._table->get_map()[zs::_ss("name")] == "main");

  REQUIRE(new_data._table->contains(zs::_ss("stack_size")));
  REQUIRE(new_data._table->get_map()[zs::_ss("stack_size")] == fct->_stack_size);

  REQUIRE(new_data._table->contains(zs::_ss("literals")));
  //  zb::print("literals", new_data._table->get_map()[zs::_ss("literals")]);

  REQUIRE(new_data._table->contains(zs::_s(eng, "default_params")));
  //  zb::print("default_params", new_data._table->get_map()[zs::_s(eng,
  //  "default_params")]);

  REQUIRE(new_data._table->contains(zs::_s(eng, "parameter_names")));
  //  zb::print("parameter_names", new_data._table->get_map()[zs::_s(eng,
  //  "parameter_names")]);

  REQUIRE(new_data._table->contains(zs::_s(eng, "restricted_types")));
  //  zb::print("restricted_types", new_data._table->get_map()[zs::_s(eng,
  //  "restricted_types")]);

  //  REQUIRE(new_data._table->contains(zs::_s(eng, "functions")));
  //  zb::print("functions",new_data._table->get_map()[zs::_s(eng, "functions")]
  //  );
}

TEST_CASE("zs::object_ptr::native_function2.01") {
  //  constexpr std::string_view code = R"""(
  // class a {
  // john = function() {
  //  }
  //
  //  function peter() {
  //  print("JOHNSON");
  //  }
  //
  //  constructor() {
  //  }
  //};
  //
  // return a;
  //)""";

  constexpr std::string_view code = R"""(
var a = {function peter() {
}
};
  a.peter = function() {
  
  }

class c {
  function peter() {
  }
};

return 32;
)""";
  //  zs::object t1;
  zs::vm vm;
  //    zs::object t2=zs::_t(vm);
  //  t1 = zs::_t(vm);

  zs::object closure;
  REQUIRE(!vm->compile_buffer(code, "test", closure));
  REQUIRE(closure.is_closure());

  zs::var ret_value;
  vm.push_root();
  auto err = vm->call_from_top(closure, 1, ret_value);
  REQUIRE(!err);
  REQUIRE(ret_value == 32);

  vm->pop();
  //  zb::print(vm.stack_size());
  //  zs::var inst = value.as_class().create_instance();
  //  REQUIRE(inst.is_instance());
}

// TEST_CASE("zs::object_ptr::native_function2.01") {
//   zs::object tt;
//   zs::vm vm;
//   zs::object obj = [](zs::vm_ref vm, zs::parameter_list params) -> zs::object { return params.size(); };
//
//
//   tt = zs::_t(vm);
//
//   REQUIRE(obj._type == zs::object_type::k_native_function2);
//   zs::object ret = (*(obj._fct))(vm, {});
//   REQUIRE(ret == 0);
//
//   ret = (*(obj._cfct.fct))(vm, {});
//   REQUIRE(ret == 0);
//
//   ret = obj._cfct.call(vm, { 1, 2, 3 });
//   REQUIRE(ret == 3);
//
//   ret = obj._cfct.call(vm, { vm->get_root(), 1, 2, 3 });
//   REQUIRE(ret == 4);
// }

TEST_CASE("zs::object_ptr::native_function2.02") {
  zs::vm vm;
  zs::var obj = [](zs::vm_ref vm, zs::parameter_list params) { return zs::_ss("John"); };

  REQUIRE(obj._cfct.call(vm, {}) == "John");
}

TEST_CASE("zs::object::native_function2::get_normal_arguments.02") {
  zs::vm vm;
  zs::var obj = [](zs::vm_ref vm, zs::parameter_list params) -> zs::var {
    return params.get_normal_arguments().size();
  };

  REQUIRE(obj._type == zs::var_type::k_native_function2);

  REQUIRE(obj._cfct.call(vm, { vm->get_root(), 1, 2, 3 }) == 3);
  REQUIRE(obj._cfct.call(vm, { vm->get_root() }) == 0);
}
