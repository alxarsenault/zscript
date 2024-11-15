
#include "ztests.h"

ZS_CODE_TEST("for_01", R"""(
var i = 0;
for(; i < 10; i++) {
  //   print("I", i);
  var k = "john";
  k = k + "johnny";
}
return i;
)""") {
  REQUIRE(value == 10);
}

ZS_CODE_TEST("for_02", R"""(
var k = 0;
for(var i = 0; i < 10; i++) {
  k = i;
}

return k;
)""") {
  REQUIRE(value == 9);
}

ZS_CODE_TEST("for_03", R"""(
var k = 0;
for(var i = 0; i < 10; i++) {
  k++;
}

return k;
)""") {
  REQUIRE(value == 10);
}

ZS_CODE_TEST("for_04", R"""(
var arr = [];
for(var i = 0; i < 10; i++) {
  arr.push(i);
}

return arr;
)""") {
  REQUIRE(value.is_array());

  zs::array_object& arr = value.as_array();

  for (size_t i = 0; i < 10; i++) {
    REQUIRE(arr[i] == i);
  }
}

ZS_CODE_TEST("for_05", R"""(
var arr = [];
arr.resize(10);
for(var i = 0; i < arr.size(); i++) {
  arr[i] = 10 + i;
}

return arr;
)""") {
  REQUIRE(value.is_array());

  zs::array_object& arr = value.as_array();

  for (size_t i = 0; i < 10; i++) {
    REQUIRE(arr[i] == 10 + i);
  }
}

ZS_CODE_TEST("for_06", R"""(
var arr = array(10);
for(var i = 0; i < arr.size(); i++) {
  arr[i] = 10 + i;
}

return arr;
)""") {
  REQUIRE(value.is_array());

  zs::array_object& arr = value.as_array();

  for (size_t i = 0; i < 10; i++) {
    REQUIRE(arr[i] == 10 + i);
  }
}

ZS_CODE_TEST("for.07", R"""(
var arr = array(4);
for (var i = 0, s = "john"; i < s.size(); ++i) {
  arr[i] = s.replace(s[i], '#');
}

return arr;
)""") {
  REQUIRE(value.is_array());

  zs::array_object& arr = value.as_array();

  for (size_t i = 0; i < 4; i++) {
    REQUIRE(arr[i].get_string_unchecked()[i] == '#');
  }
}

//
// TEST_CASE("dsadsadsadsad") {
//
//  static constexpr std::string_view code_01 = R"""(
// var arr = array(10);
// for(var i = 0; i < arr.size(); i++) {
//
//}
//
// return arr;
//
//)""";
//
//
//  zs::vm vm;
//  zs::object_ptr closure_01;
//
//  if (auto err = vm->compile_buffer(code_01, "test", closure_01)) {
//    REQUIRE(false);
//    return;
//  }
//
//
//  REQUIRE(closure_01.is_closure());
//    zs::object_ptr value;
//    zs::int_t n_params = 1;
//    vm.push_root();
//
//
// closure_01.as_closure().get_function_prototype()->debug_print();
//    if (auto err = vm->call(closure_01, n_params, vm.stack_size() - n_params, value)) {
//      //
//      REQUIRE(false);
//      return;
//    }
//
//}
//
////
////TEST_CASE("dsadsadsadsad2") {
////
////  static constexpr std::string_view code_01 = R"""(
////var arr = array(10);
////for(var i = 0; i < arr.size(); i++) {
////  arr[i] = i;
////}
////
////foreach(var a : arr) {
////}
////
////return arr;
////
////)""";
////
////
////  zs::vm vm;
////  zs::object_ptr closure_01;
////
////  if (auto err = vm->compile_buffer(code_01, "test", closure_01)) {
////    REQUIRE(false);
////    return;
////  }
////
////
////  REQUIRE(closure_01.is_closure());
////    zs::object_ptr value;
////    zs::int_t n_params = 1;
////    vm.push_root();
////
////
//// closure_01.as_closure().get_function_prototype()->debug_print();
////    if (auto err = vm->call(closure_01, n_params, vm.stack_size() - n_params, value)) {
////      //
////      REQUIRE(false);
////      return;
////    }
////
////}
