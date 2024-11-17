#include "unit_tests.h"

using namespace zs::literals;

#define ZS_CHECK_STRING()     \
  REQUIRE(value.is_string()); \
  std::string_view str = value.get_string_unchecked()

ZS_CODE_TEST("string_01", R"""(return "Alex";)""") {
  ZS_CHECK_STRING();
  REQUIRE(str == "Alex");
}

ZS_CODE_TEST("string_02", R"""(
var a = "Alex";
return a;
)""") {
  ZS_CHECK_STRING();
  REQUIRE(str == "Alex");
}

ZS_CODE_TEST("string_size_01", R"""(return "Alex".size();)""") { REQUIRE(value == 4); }
ZS_CODE_TEST("string_size_02", R"""(return "ⲗվ".size();)""") { REQUIRE(value == 2); }

ZS_CODE_TEST("string_ascii_size_01", R"""(return "Alex".ascii_size();)""") { REQUIRE(value == 4); }
ZS_CODE_TEST("string_ascii_size_02", R"""(return "ⲗվ".ascii_size();)""") { REQUIRE(value == 5); }

ZS_CODE_TEST("string_starts_with_01", R"""(return "Alex".starts_with("Al");)""") { REQUIRE(value == true); }
ZS_CODE_TEST("string_starts_with_02", R"""(return "Alex".starts_with("Al");)""") { REQUIRE(value == true); }
ZS_CODE_TEST("string_starts_with_03", R"""(return "Alex".starts_with("X");)""") { REQUIRE(value == false); }

ZS_CODE_TEST("string_ends_with_01", R"""(return "Alex".ends_with("ex");)""") { REQUIRE(value == true); }

ZS_CODE_TEST("string_contains_01", R"""(return "Alex".contains("ex");)""") { REQUIRE(value == true); }
ZS_CODE_TEST("string_contains_02", R"""(return "Alex".contains("John");)""") { REQUIRE(value == false); }

ZS_CODE_TEST("string_to_upper_01", R"""(return "Alex".to_upper();)""") { REQUIRE(value == "ALEX"); }
ZS_CODE_TEST("string_to_upper_02", R"""(return "ⲗվ".to_upper();)""") { REQUIRE(value == "ⲖՎ"); }

ZS_CODE_TEST("string_to_lower_01", R"""(return "Alex".to_lower();)""") { REQUIRE(value == "alex"); }
ZS_CODE_TEST("string_to_lower_02", R"""(return "ⲖՎ".to_lower();)""") { REQUIRE(value == "ⲗվ"); }

ZS_CODE_TEST("string_words_01", R"""(return "1 2 3 4 5".words();)""") {
  REQUIRE(value == zs::_a(eng, { "1"_ss, "2"_ss, "3"_ss, "4"_ss, "5"_ss }));
}

ZS_CODE_TEST("string_words_02", R"""(
var a = """1
2 3\t\n4
                        5""";
return a.words();
)""") {
  REQUIRE(value == zs::_a(eng, { "1"_ss, "2"_ss, "3"_ss, "4"_ss, "5"_ss }));
}

ZS_CODE_TEST("string_capitalize_01", R"""(return "alex".capitalize();)""") { REQUIRE(value == "Alex"); }
ZS_CODE_TEST("string_capitalize_02", R"""(return "aLEX".capitalize();)""") { REQUIRE(value == "ALEX"); }
ZS_CODE_TEST("string_capitalize_03", R"""(return "aLEX".capitalize(true);)""") { REQUIRE(value == "Alex"); }
ZS_CODE_TEST("string_capitalize_04", R"""(return "123".capitalize();)""") { REQUIRE(value == "123"); }
ZS_CODE_TEST("string_capitalize_05", R"""(return "ⲗվ".capitalize();)""") { REQUIRE(value == "Ⲗվ"); }
ZS_CODE_TEST("string_capitalize_06", R"""(return "ⲗՎ".capitalize(true);)""") { REQUIRE(value == "Ⲗվ"); }

ZS_CODE_TEST("string_decapitalize_01", R"""(return "Alex".decapitalize();)""") { REQUIRE(value == "alex"); }
ZS_CODE_TEST("string_decapitalize_02", R"""(return "ALEX".decapitalize();)""") { REQUIRE(value == "aLEX"); }
ZS_CODE_TEST("string_decapitalize_03", R"""(return "ⲖՎ".decapitalize();)""") { REQUIRE(value == "ⲗՎ"); }

ZS_CODE_TEST("string_to_camel_case_01", R"""(return "alex bingo".to_camel_case();)""") {
  REQUIRE(value == "alexBingo");
}

ZS_CODE_TEST("string_to_camel_case_02", R"""(return "Alex bingo".to_camel_case();)""") {
  REQUIRE(value == "alexBingo");
}

ZS_CODE_TEST("string_to_camel_case_03", R"""(return "alex_bingo".to_camel_case();)""") {
  REQUIRE(value == "alexBingo");
}

ZS_CODE_TEST("string_to_camel_case_04", R"""(return "alex-bingo".to_camel_case();)""") {
  REQUIRE(value == "alexBingo");
}

ZS_CODE_TEST("string_to_snake_case_01", R"""(return "Alex Bingo".to_snake_case();)""") {
  REQUIRE(value == "alex_bingo");
}

ZS_CODE_TEST("string_to_snake_case_02", R"""(return "AlexBingo".to_snake_case();)""") {
  REQUIRE(value == "alexbingo");
}

ZS_CODE_TEST("string_to_title_case_01", R"""(return "alex bingo bongo".to_title_case();)""") {
  REQUIRE(value == "Alex Bingo Bongo");
}

ZS_CODE_TEST("string_to_title_case_02", R"""(return "alexbingo bongo".to_title_case();)""") {
  REQUIRE(value == "Alexbingo Bongo");
}

ZS_CODE_TEST("string_to_title_case_03", R"""(return "jean-luc is good-looking".to_title_case();)""") {
  REQUIRE(value == "Jean-luc Is Good-looking");
}

ZS_CODE_TEST("string_replace_01", R"""(return "Alexandre".replace("x", "Peter");)""") {
  REQUIRE(value == "AlePeterandre");
}

ZS_CODE_TEST("string_replace_02", R"""(return "Alexandre".replace('x', "Peter");)""") {
  REQUIRE(value == "AlePeterandre");
}

ZS_CODE_TEST("string_replace_03", R"""(return "alexandre".replace('a', "A");)""") {
  REQUIRE(value == "AlexAndre");
}

ZS_CODE_TEST("string_replace_04", R"""(return "Alexandre".replace('x', 'X');)""") {
  REQUIRE(value == "AleXandre");
}

ZS_CODE_TEST("string_replace_05", R"""(return "AleⲖandre".replace('Ⲗ', "XX");)""") {
  REQUIRE(value == "AleXXandre");
}

ZS_CODE_TEST("string_replace_first_01", R"""(
return "Alexandre".replace_first('A', 'X');
)""") {
  REQUIRE(value == "Xlexandre");
}

ZS_CODE_TEST("string_replace_first_02", R"""(
return "alexandre".replace_first('a', 'X');
)""") {
  REQUIRE(value == "Xlexandre");
}

ZS_CODE_TEST("string_replace_last_01", R"""(
return "alexandre".replace_last('a', 'X');
)""") {
  REQUIRE(value == "alexXndre");
}

ZS_CODE_TEST("string_replace_last_02", R"""(
return "alexalex".replace_last("alex", "andre");
)""") {
  REQUIRE(value == "alexandre");
}

ZS_CODE_TEST("string_replace_last_03", R"""(
return "alexalex".replace_last("alexbnmb", "andre");
)""") {
  REQUIRE(value == "alexalex");
}

ZS_CODE_TEST("string_lib_01", R"""(
return string.size("Bacon");
)""") {
  REQUIRE(value == 5);
}

ZS_CODE_TEST("string_lib_02", R"""(
return tostring(32);
)""") {
  REQUIRE(value == "32");
}

ZS_CODE_TEST("string_lib_03", R"""(
return string(32);
)""") {
  REQUIRE(value == "32");
}

ZS_CODE_TEST("string_lib_04", R"""(
return "A" + @(32);
)""") {
  REQUIRE(value == "A32");
}

ZS_CODE_TEST("string_lib_05", R"""(
return toint("32");
)""") {
  REQUIRE(value == 32);
}

ZS_CODE_TEST("string_lib_06", R"""(
return int("32");
)""") {
  REQUIRE(value == 32);
}

ZS_CODE_TEST("string_type.01", R"""(return "")""") {
  ZS_CHECK_STRING();
  REQUIRE(value.is_small_string());
  REQUIRE(str == "");
}

ZS_CODE_TEST("string_type.02", R"""(return "AlexandreAlexa")""") {
  ZS_CHECK_STRING();
  REQUIRE(value.is_small_string());
  REQUIRE(str == "AlexandreAlexa");
}

ZS_CODE_TEST("string_type.03", R"""(return "AlexandreAlexan")""") {
  ZS_CHECK_STRING();
  REQUIRE(value.is_long_string());
  REQUIRE(str == "AlexandreAlexan");
}

ZS_CODE_TEST("string_type.04", R"""(return "AlexandreAleπ")""") {
  ZS_CHECK_STRING();
  REQUIRE(value.is_small_string());
  REQUIRE(str == "AlexandreAleπ");
}

ZS_CODE_TEST("string_type.05", R"""(return "AlexandreAlexπ")""") {
  ZS_CHECK_STRING();
  REQUIRE(value.is_long_string());
  REQUIRE(str == "AlexandreAlexπ");
}

ZS_CODE_TEST("mutable_string.01", R"""(
return mutable_string("John");
)""") {
  ZS_CHECK_STRING();
  REQUIRE(value.is_mutable_string());
  REQUIRE(str == "John");
}

ZS_CODE_TEST("mutable_string.02", R"""(
var a = mutable_string("πohn");
a[1] = 'P';
return a;
)""") {
  ZS_CHECK_STRING();
  REQUIRE(value.is_mutable_string());
  REQUIRE(str == "πPhn");
}

ZS_CODE_TEST("mutable_string.03", R"""(
var a = mutable_string("Jπhn");
a[1] = 'o';
return a;
)""") {
  ZS_CHECK_STRING();
  REQUIRE(value.is_mutable_string());
  REQUIRE(str == "John");
}

ZS_CODE_TEST("mutable_string.04", R"""(
var a = mutable_string("Jπhn");
a[1] = "Bingo";
return a;
)""") {
  ZS_CHECK_STRING();
  REQUIRE(value.is_mutable_string());
  REQUIRE(str == "JBingohn");
}

ZS_CODE_TEST("mutable_string.05", R"""(
var a = mutable_string("John");
return a[1];
)""") {
  //  ZS_CHECK_STRING();
  //  REQUIRE(value.is_integer());
  //  REQUIRE(str == 'o');
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// DEBUG HELPER.
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// static inline void print_stack(zs::vm_ref vm) {
//
//   for (zs::int_t i = 0; i < vm->stack_size(); i++) {
//     zb::print("value", i, vm->stack_get(i).to_debug_string());
//   }
// }

// TEST_CASE("for_test_02") {
//
//   static constexpr std::string_view code = R"""(
//
//  var i = 0;
//  for(; i < 10; i++) {
//      print("I", i);
//  var k = "john";
//  k = k + "johnny";
//
////  i = i + 1;
// }
// return i;
//)""";
//
//  zs::vm vm;
//  zs::object_ptr closure;
//
//  if (auto err = vm->compile_buffer(code, "test", closure)) {
//    REQUIRE(false);
//    return;
//  }
//
//  REQUIRE(closure.is_closure());
//
//  zs::object_ptr value;
//  //
//  zs::int_t n_params = 1;
//  vm.push_root();
//
//  //   closure._closure->get_function_prototype()->_instructions.serialize();
//  closure._closure->get_function_prototype()->debug_print();
//  //
//  if (auto err = vm->call(closure, n_params, vm.stack_size() - n_params, value)) {
//    //
//    REQUIRE(false);
//    return;
//  }
//
//  zb::print("VALUE", value, vm->stack().get_stack_base());
//
//  print_stack(vm);
//}

// TEST_CASE("dsadsadsadsad") {
//
//   static constexpr std::string_view code = R"""(
// var a = {
//   f1 = function(v) {
//     this.vf1 = v;
//   }
// };
//
// var hj = "KK";
// var v0 = "capture v0";
// a.v0 = "member v0";
//
// var f1 = function(v) {
//   //print(this, v);
// }
//
// global.ui = "UI"
//
// a.f2 = function(v) {
//   var hj = 3;
//   this.hj = 32;
//   this.v1 = ::hj;
//   this.v2 = hj;
//   this.v3 = global.hj;
//   global.hj = 88987897;
//   this.v4 = ::hj;
//   this.v5 = v0;
//   this.v6 = this.v0;
//
//   {
//     var v0 = "local v0";
//     this.v7 = v0;
//   }
//
//   this.v8 = v0;
//
//    this.v9 = optget(global, "ui");
//    this.v11 = optget(global, "dsd");
//
//   this.f1(89);
//   f1("777777767676768767687");
// }
//
////a.peter(333);
// a.f2(3333);
//  //print(hj);
//  return a;
//)""";
//
//   zs::vm vm;
//   zs::object_ptr closure;
//
//   vm->get_root().as_table()["hj"] = 332;
//
//   if (auto err = vm->compile_buffer(code, "test", closure)) {
//     zb::print(vm.get_error());
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
//   //   closure._closure->get_function_prototype()->_instructions.serialize();
//   //   closure._closure->get_function_prototype()->debug_print();
//   //
//   if (auto err = vm->call(closure, n_params, vm.stack_size() - n_params, value)) {
//     zb::print(vm.get_error());
//     REQUIRE(false);
//     return;
//   }
//
//   //  zb::print("VALUE", value, vm->stack().get_stack_base());
//
//   //   print_stack(vm);
// }
