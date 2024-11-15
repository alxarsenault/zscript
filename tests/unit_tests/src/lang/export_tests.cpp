#include "ztests.h"
#include <tuple>
using enum zs::error_code;

// ZCODE_TEST("Module name (small)", R"""(
//@module A;
//)""") {
//   REQUIRE(closure.as_closure().get_proto()._module_name == "A");
//
//   vm->get_imported_module_cache();
// }

ZS_CODE_TEST("Module name (small)", R"""(
var john = import("john");
return john;
)""",
    [](zs::virtual_machine& vm) {
      zs::var tbl = zs::_t(vm.get_engine());
      tbl.as_table()["a"] = 55;
      vm.get_imported_module_cache().as_table()["john"] = tbl;
    }) {
  REQUIRE(value.is_table());
}

ZCODE_TEST("Module name (long)", R"""(
@module AshaSJHAgdhsaghgasdJHSKGADHJSAGDHJSAGDKJAGSJHDA;
)""") {
  REQUIRE(closure.as_closure().get_proto()._module_name == "AshaSJHAgdhsaghgasdJHSKGADHJSAGDHJSAGDKJAGSJHDA");
}

ZCODE_TEST("Module export var", R"""(
@module
export var a = 32;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 32);
}

ZCODE_TEST("Module export const", R"""(
@module
export const a = 32;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 32);
}

ZS_CODE_TEST("module.01", R"""(
@module
export var a = 32;
)""") {
  INFO("Module with var export");
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 32);
}

ZS_CODE_TEST("module.02", R"""(
@module
export const a = 32;
)""") {
  INFO("Module with const export");
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 32);
}

ZS_CODE_TEST("module.03", R"""(
@module
export var a = 32;
export var b = 33;
)""") {
  REQUIRE(value.is_table());
  REQUIRE(value.as_table()["a"] == 32);
  REQUIRE(value.as_table()["b"] == 33);
}

TEST_CASE("module.04") {
  static constexpr std::string_view code = R"""(
export var a = 32;
export var a = 33;
)""";
  zs::vm vm;
  zs::object closure;
  REQUIRE(vm->compile_buffer(code, ztest::s_current_test_name, closure) == already_exists);
}

TEST_CASE("module.05") {
  std::string_view code = R"""(
export var a = 32;
a = 123;
)""";
  zs::vm vm;
  zs::object closure;
  REQUIRE(vm->compile_buffer(code, ztest::s_current_test_name, closure) == cant_modify_export_table);
}

// TEST_CASE("export.01") {
//   static constexpr std::string_view code_01 = R"""(
// export int k = 22;
// export var name = "Steeve";
//
// var a = 32;
//
// export function main() {
//   var kkk = k;
//   var aaa = a;
//   var nnn = name;
//   // print("Main", a, k, name);
// }
//
// export var tbl = {
//   fct = function() {
//     var kkk = k;
//     //print("BINGO", k);
//   }
//
//   banana = 55,
//
//   ohhnoo = main
// };
//
//
// var bing = function(var f) {
////  print("bingo");
//  f();
//};
//
// export var bingo = bing;
//
// function john() {
////  print("John", a, k);
//  main();
//  tbl.fct();
//  tbl.banana = 67;
//
//  bingo(main);
//}
//
// main();
// john();
//)""";
//
//  zs::vm vm;
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(code_01, "export.01", closure)) {
//    FAIL(vm.get_error());
//  }
//
//  REQUIRE(closure.is_closure());
//
//  zs::object value;
//  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//    FAIL(vm.get_error());
//  }
//
//  REQUIRE(value.is_table());
//
//  REQUIRE(value.as_table()["k"] == 22);
//  REQUIRE(value.as_table()["main"].is_closure());
//}
//
// TEST_CASE("export.02") {
//  static constexpr std::string_view code_01 = R"""(
// export var name = "Steeve";
//
// export function main() {
// var kkk = name;
////  print("banana", name);
//}
//)""";
//
//  zs::vm vm;
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(code_01, "export.02", closure)) {
//    FAIL(vm.get_error());
//  }
//
//  REQUIRE(closure.is_closure());
//
//  zs::object fct;
//  zs::object value;
//  {
//    //    zs::object value;
//    if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//      FAIL(vm.get_error());
//    }
//
//    REQUIRE(value.is_table());
//
//    REQUIRE(value.as_table()["main"].is_closure());
//
//    fct = value.as_table()["main"];
//  }
//
//  zs::object ret_value;
//  if (auto err = vm->call(fct, { vm->get_root() }, ret_value)) {
//    FAIL(vm.get_error());
//  }
//}

//
// TEST_CASE("export.03") {
//  static constexpr std::string_view code_01 = R"""(
// export var a = {};
//
// export var b = {a = a};
//
////a.b = b;
// var k = a;
// var k2 = b;
//  export function bingo() {
////  a.b.c = {};
// a.b = b;
//   b.a.c = k;
//   var kk = k2;
// }
//
//  export function bingo2() {
////  a.b.c = {};
//  a.b = b;
//  var kkkkk = bingo;
//  b.a.c = k;
//  var kk = k2;
//}
//
// bingo();
// bingo2();
// a.fff = bingo;
// a.ddd = bingo2;
//)""";
//
//  zs::engine eng;
//
//  zs::vm vm(&eng);
//
//  std::set<zs::reference_counted_object*> done_map = eng._gc_objs;
//
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(code_01, "export.02", closure)) {
//    FAIL(vm.get_error());
//  }
//
//  REQUIRE(closure.is_closure());
//  zs::object value;
//
//  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//    FAIL(vm.get_error());
//  }
//
//  {
//    zs::object a = zs::_t(&eng);
//    zs::object b = zs::_t(&eng);
//    zs::object c = zs::_a(&eng, 1);
//
//    a.as_table()["c"] = c;
//    a.as_table()["b"] = b;
//    b.as_table()["a"] = a;
//    b.as_table()["c"] = c;
//    b.as_table()["d"] = b;
//    c.as_array().push(a);
//    c.as_array().push(b);
//    c.as_array().push(c);
////    zb::print(eng._gc_objs.size());
//  }
//
//  zs::vector<zs::object> objs((zs::allocator<zs::object>(&eng)));
//
//  for(auto it : eng._gc_objs) {
//    zs::object_base ooo;
//    memset(&ooo, 0, sizeof(ooo));
//    ooo._type = it->_obj_type;
//    zb::print(ooo._type);
//    ooo._pointer = it;
//    objs.push_back(zs::object(ooo, true));
//  }
//
//  for(auto & obj : objs) {
//    if(obj.is_table()) {
//      obj.as_table().clear();
//    }
//    else if(obj.is_array()) {
//      obj.as_array().clear();
//    }
//    else if(obj.is_closure()) {
//      obj.as_closure().clear();
//    }
//  }
//
//}

// TEST_CASE("export.03") {
//   static constexpr std::string_view code_01 = R"""(
// export var a = {};
//
// export var b = {a = a};
//
////a.b = b;
// var k = a;
// var k2 = b;
//  export function bingo() {
////  a.b.c = {};
// a.b = b;
//   b.a.c = k;
//   var kk = k2;
// }
//
//  export function bingo2() {
////  a.b.c = {};
//  a.b = b;
//  var kkkkk = bingo;
//  b.a.c = k;
//  var kk = k2;
//}
//
// bingo();
// bingo2();
// a.fff = bingo;
// a.ddd = bingo2;
//)""";
//
//  zs::engine eng;
//
//  zs::vm vm(&eng);
//
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(code_01, "export.02", closure)) {
//    FAIL(vm.get_error());
//  }
//
//  REQUIRE(closure.is_closure());
//  zs::object value;
//
//  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//    FAIL(vm.get_error());
//  }
//
//  {
//    zs::object a = zs::_t(&eng);
//    zs::object b = zs::_t(&eng);
//    zs::object c = zs::_a(&eng, 1);
//
//    a.as_table()["c"] = c;
//    a.as_table()["b"] = b;
//    b.as_table()["a"] = a;
//    b.as_table()["c"] = c;
//    b.as_table()["d"] = b;
//    c.as_array().push(a);
//    c.as_array().push(b);
//    c.as_array().push(c);
//    //    zb::print(eng._gc_objs.size());
//  }
//
//  //  value.reset();
//  //  eng.destroy_all();
//}

// TEST_CASE("export.04") {
//   static constexpr std::string_view code_01 = R"""(
// return {};
//)""";
//
//   zs::engine eng;
//
//   zs::object vmobj = zs::object::create_vm(&eng);
//   zs::vm& vm = vmobj.as_udata().data_ref<zs::vm>();
//
//   zs::object closure;
//
//   if (auto err = vm->compile_buffer(code_01, "export.02", closure)) {
//     FAIL(vm.get_error());
//   }
//
//   REQUIRE(closure.is_closure());
//   zs::object value;
//
//   if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//     FAIL(vm.get_error());
//   }
// }
//
// TEST_CASE("export.05") {
//   static constexpr std::string_view code = R"""(
//
// var k = [1, 2, 3];
// var z = { abc = k };
//
// return {
//   a = 21,
//   b = 23,
//   k = k,
//   z = z
//   };
//)""";
//
//   zs::object vmobj = zs::object::create_vm();
//   {
//     zs::vm& vm = vmobj.as_udata().data_ref<zs::vm>();
//
//     zs::object closure;
//
//     if (auto err = vm->compile_buffer(code, "export.05", closure)) {
//       FAIL(vm.get_error());
//     }
//
//     REQUIRE(closure.is_closure());
//
//     zs::object value;
//     if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//       FAIL(vm.get_error());
//     }
//
//     REQUIRE(value.is_table());
//   }
// }

// TEST_CASE("module.01") {
//   static constexpr std::string_view code = R"""(
//@module;
//
// export struct abc {
//   constructor(a = 0, b = 0, c = 0) {
//     this.a = a;
//     this.b = b;
//     this.c = c;
//   }
//
//   var a;
//   var b;
//   var c;
// };
//
// var aaa = abc(1, 2, 3);
////print(aaa);
//
// var k = [1, 2, 3];
// var z = { abc = k };
//
// export var a = k;
//
// export {
//  k = k,
//  z = z
//}
//
// export {
//  john = "JOHN"
//};
//
// var bingo = "alex";
//
// var banana = { a = 55 };
//
// struct Peter {
//  constructor(a = 0, b = 0, c = 0) {
//    this.a = a;
//    this.b = b;
//    this.c = c;
//  }
//
//  var a;
//  var b;
//  var c;
//};
//
// export { bingo, banana, bacon = Peter };
//
//
// export {
//  function pouff() {
//
//  }
//
//  ghj = banana.a,
//
//  Peter
//}
//
//)""";
//
//  zs::object vmobj = zs::object::create_vm();
//  {
//    zs::vm& vm = vmobj.as_udata().data_ref<zs::vm>();
//
//    zs::object closure;
//
//    if (auto err = vm->compile_buffer(code, "module.01", closure)) {
//      FAIL(vm.get_error());
//    }
//
//    REQUIRE(closure.is_closure());
//
//    zs::object value;
//    if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//      FAIL(vm.get_error());
//    }
//
//    REQUIRE(value.is_table());
//    //        zb::print(value);
//  }
//}

// TEST_CASE("module.02") {
//   static constexpr std::string_view code = R"""(
//@module;
// var a = 1;
// var b = 1;
// var c = 1;
// var d = 1;
/////export c;
////export c;
// export var k = 32;
// export var kw = 32;
// export var kasdsadsa = "SAKSLAKSLKALS";
//
// export {
//   c = 323
// }
// ggg();
//
//)""";
//
//   zs::object vmobj = zs::object::create_vm();
//   {
//     zs::vm& vm = vmobj.as_udata().data_ref<zs::vm>();
//
//     //    zb::print("------------dsdsd--", vm->stack().get_absolute_top());
//
//     vm->get_root().as_table()["ggg"] = zs::object::create_native_function([](zs::vm_ref vm) -> zs::int_t {
//       //      zs::int_t ss = vm->stack().get_absolute_top();
//       //      zb::print("--------------", ss,vm.stack_size());
//       //      for(zs::int_t i = 0; i < ss; i++) {
//       //        zb::print(i, vm->stack().get_internal_vector()[i],
//       //        vm->stack().get_internal_vector()[i].get_type());
//       //      }
//       //      vm.push(128);
//       //      zb::print("DSLKDJSKLD", vm->stack_size(), vm->stack().get_absolute_top() , );
//       //      zb::print("DSLKDJSKLD", vm->stack_size(), vm->stack().get_absolute_top() ,
//       //      vm->stack().get_internal_vector()[2], vm->stack().get_internal_vector()[2].get_type());
//       //      zb::print("DSLKDJSKLD", vm->stack_size(), vm->stack().get_absolute_top() ,
//       //      vm->stack().get_internal_vector()[3], vm->stack().get_internal_vector()[3].get_type());
//       return 0;
//     });
//     //    zb::print("------------dsdsd--", vm->stack().get_absolute_top());
//     zs::object closure;
//
//     if (auto err = vm->compile_buffer(code, "module.02", closure)) {
//       FAIL(vm.get_error());
//     }
//
//     REQUIRE(closure.is_closure());
//     //    zb::print("--------dd----dsdsd--", vm->stack().get_absolute_top());
//     //    zb::print("----------------get_function_prototype------------------------",
//     //        closure.as_closure().get_function_prototype()->_stack_size);
//
//     zs::object value;
//     if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//       FAIL(vm.get_error());
//     }
//     //    zb::print("----asasasa--------dsdsd--", vm->stack().get_absolute_top());
//     //
//     //    REQUIRE(value.is_table());
//     //    zb::print(value);
//   }
// }

// TEST_CASE("module.04") {
//   static constexpr std::string_view code = R"""(
//@module;
//
// export {
//   a = {
//     b = 0
//   }
// }
//
// a.b = 90;
//)""";
//
//   zs::vm vm;
//   zs::object closure;
//
//   if (auto err = vm->compile_buffer(code, "module.04", closure)) {
//     FAIL(vm.get_error());
//   }
//
//   REQUIRE(closure.is_closure());
//
//   zs::object value;
//   if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//     FAIL(vm.get_error());
//   }
//
//   zb::print(value);
// }
//
//
// TEST_CASE("module.05") {
//   static constexpr std::string_view code = R"""(
//@module;
//
// export {
//   a = {
//     b = {
//       c = 89
//     }
//   }
// }
//
// a.b.c = 90;
//)""";
//
//   zs::vm vm;
//   zs::object closure;
//
//   if (auto err = vm->compile_buffer(code, "module.04", closure)) {
//     FAIL(vm.get_error());
//   }
//
//   REQUIRE(closure.is_closure());
//
//   zs::object value;
//   if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//     FAIL(vm.get_error());
//   }
//
//   zb::print(value);
// }

// TEST_CASE("module.06") {
//   static constexpr std::string_view code = R"""(
//@module;
//
// export {
//   a = 32,
//   b = {
//     bono = 55
//   }
// }
//
////a = 123;
// b.bono = 90;
//
// function f1() {
////  a = 90;
//}
//
// function f2() {
//  b.bono = 190;
//}
//
////f1();
////f2();
//)""";
//
//  zs::vm vm;
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(code, "module.02", closure)) {
//    FAIL(vm.get_error());
//  }
//
//  REQUIRE(closure.is_closure());
//
//  zs::object value;
//  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
//    FAIL(vm.get_error());
//  }
//
//  zb::print(value);
//}
//
template <class T>
using has_stream_getter_impl
    = decltype(T::stream_getter(std::declval<zs::engine*>(), std::declval<zs::raw_pointer_t>()));

template <class T>
using has_member_stream_getter_impl = decltype(std::declval<T&>().stream_getter(
    std::declval<zs::engine*>(), std::declval<zs::raw_pointer_t>()));

template <class T>
using has_get_stream_impl = decltype(std::declval<T&>().get_stream());

template <class T>
struct has_get_stream {
  static constexpr bool value = []() {
    if constexpr (zb::has_members<T, has_get_stream_impl>::value) {
      if constexpr (std::is_base_of_v<std::ostream, std::remove_cvref_t<has_get_stream_impl<T>>>) {
        return true;
      }
      else {
        return false;
      }
    }
    else {
      return false;
    }
  }();
};

template <class T, class... Args>
inline zs::config_t create_config(Args&&... args) {
  zs::config_t conf;

  conf.user_release = [](zs::allocate_t alloc_cb, zs::raw_pointer_t ptr) {
    zbase_assert(alloc_cb, "invalid alloc callback");
    zbase_assert(ptr, "invalid user pointer");

    T* obj = (T*)ptr;
    delete obj;
  };

  if constexpr (has_get_stream<T>::value) {
    conf.stream_getter = [](zs::engine* eng, zs::raw_pointer_t ptr) -> std::ostream& {
      zbase_assert(eng, "invalid engine");
      zbase_assert(ptr, "invalid user pointer");
      T* obj = (T*)ptr;
      return obj->get_stream();
    };
  }

  else if constexpr (zb::has_members<T, has_stream_getter_impl>::value
      or zb::has_members<T, has_member_stream_getter_impl>::value) {
    if constexpr (std::same_as<decltype(&T::stream_getter),
                      std::ostream& (*)(zs::engine*, zs::raw_pointer_t)>) {
      conf.stream_getter = &T::stream_getter;
    }
    else {
      conf.stream_getter = [](zs::engine* eng, zs::raw_pointer_t ptr) -> std::ostream& {
        zbase_assert(ptr, "invalid user pointer");
        T* obj = (T*)ptr;
        return obj->stream_getter(eng, ptr);
      };
    }
  }

  if constexpr (sizeof...(Args)) {
    conf.user_pointer = new std::tuple<Args&&...>(std::forward<Args>(args)...);
    ;
    conf.initializer = [](zs::engine* eng) -> zs::error_result {
      zbase_assert(eng, "invalid engine");
      zbase_assert(eng->get_user_pointer(), "invalid user pointer");
      std::tuple<Args&&...>* tup = (std::tuple<Args&&...>*)eng->get_user_pointer();
      T* obj = new T(std::make_from_tuple<T>(std::move(*tup)));
      eng->set_user_pointer(obj);
      delete tup;
      return {};
    };
  }
  else {
    conf.initializer = [](zs::engine* eng) -> zs::error_result {
      zbase_assert(eng, "invalid engine");
      T* obj = new T();
      eng->set_user_pointer(obj);
      return {};
    };
  }

  return conf;
}

struct user_data {

  user_data(const std::string& name)
      : _name(name) {}

  inline std::stringstream& get_stream() { return ss; }

  std::string _name;
  std::stringstream ss;
};

TEST_CASE("module.06") {
  static constexpr std::string_view mod1 = R"""(
@module mod1;
export var johnson = "alexandre";
export var steeve = {
  a = 1,
  b = 2
};

export function michel(var a) {
  steeve.a = 2;
  print(a, johnson);
}
)""";

  static constexpr std::string_view code = R"""(
var mod1 = import("mod1");

var king = function() {
  return "LOL";
}

function bingo() {
  function woop() {
    mod1.michel(king());
  }

  woop();
}

bingo();
return mod1;
)""";

  zs::vm vm(create_config<user_data>("john"));

  {
    zs::object closure;

    if (auto err = vm->compile_buffer(mod1, "mod1", closure)) {
      FAIL(vm.get_error());
    }

    REQUIRE(closure.is_closure());

    zs::object value;
    if (auto err = vm->call(closure, { vm->get_root() }, value)) {
      FAIL(vm.get_error());
    }

    REQUIRE(value.is_table());

    vm->get_imported_module_cache().as_table()["mod1"] = value;
  }

  {
    zs::object closure;

    if (auto err = vm->compile_buffer(code, "module.02", closure)) {
      FAIL(vm.get_error());
    }

    REQUIRE(closure.is_closure());

    zs::object value;
    if (auto err = vm->call(closure, { vm->get_root() }, value)) {
      FAIL(vm.get_error());
    }

    REQUIRE(value.is_table());
    REQUIRE(value.as_table()["johnson"] == "alexandre");
  }

  REQUIRE(((user_data*)vm.get_engine()->get_user_pointer())->ss.str() == "LOL alexandre\n");
}
