#include "unit_tests.h"

#include "jit/zclosure_compile_state.h"

// ZTEST_CASE("Capture", R"""(
// var f2;
//
//{
//   var a = 32;
//
//   f2 = function(tag) {
//     print(tag, a);
//     a = 9;
//   }
//
//   a = 69;
//   f2("A(69)");
// }
//)""") {
//   REQUIRE(vm->_open_captures.empty());
// }

// ZTEST_CASE("Capture", R"""(
//
// var f2;
// var f3;
// var arr = [];
//
//{
//   var a = 32;
//
//   f2 = function(tag) {
//     arr.push(a);
//     print(tag, a);
//     a = 55;
//     return;
//   }
//
//   f3 = function() {
//     a = 21;
//   }
//
//   f2("A(32)");
//   f2("A(55)");
//   a = 69;
//   f2("A(69)");
//   f2("A(55)");
//   print("A(55)", a);
//
//   f3();
//   print("A(21)", a);
//   f2("A(21)");
//   print(arr);
// }
//)""") {
//   REQUIRE(vm->_open_captures.empty());
// }

// ZTEST_CASE("Capture", R"""(
//
//@macro CHECK(e) { __zcheck(e, {cond = @str(e), file = __FILE__, line = __LINE__, code = __THIS_LINE__}) }
//
// var f2;
// var f3;
//
//{
//   var a = 32;
//
//
//   $CHECK(a == 32);
//
//   __zcheck(a == 32, __FILE__, __LINE__, __THIS_LINE__);
//
//   f2 = function(tag) {
//     print(tag, a);
//     a = 55;
//     return;
//   }
//
//   f3 = function() {
//     a = 21;
//   }
//
//   f2("A(32)");
//   f2("A(55)");
//   a = 69;
//   f2("A(69)");
//   f2("A(55)");
//   print("A(55)", a);
//
//   f3();
//   print("A(21)", a);
//   f2("A(21)");
//
// }
//)""") {
//   REQUIRE(vm->_open_captures.empty());
// }

ZTEST_CASE("Capture", R"""(
var f2;

var arr = [];

function add_to_output(var id, var val) {
  arr.push({name = id, value = val });
}

{
  var a = 32;

  f2 = function(tag) {
    add_to_output(tag, a);
    a = 55;
    return;
  }

  add_to_output("a0", a);
  f2("A(32)");
  f2("A(55)");
  a = 69;
  f2("A(69)");
  f2("A(55)");
//  print("A(55)", a);
}

return arr;
)""") {
  REQUIRE(vm->_open_captures.empty());
  //  zb::print(value);
}

// ZTEST_CASE("Capture", R"""(
// var f2;
//
//{
//   var a = 32;
//
//   f2 = function(tag) {
//     print(tag, a);
//     return;
//   }
//
//   f2("A(32)");
//   a = 69;
//   f2("A(69)");
// }
//)""") {
//   REQUIRE(vm->_open_captures.empty());
// }
//
// ZTEST_CASE("Capture", R"""(
// var f2;
//
//{
//   var a = 32;
//
//   f2 = function(tag) {
//     print(tag, a);
//   }
//
//   f2("A(32)");
//   a = 69;
//   f2("A(69)");
// }
//)""") {
//   REQUIRE(vm->_open_captures.empty());
// }

// ZTEST_CASE("Capture", R"""(
// var f2;
//
//{
//   var a = 32;
//
//   f2 = function(tag) {
//     print(tag, a);
//     a = 9;
//   }
//
//   f2("A(32)");
//   print("A(9)", a);
//   a = 69;
//   f2("A(69)");
// }
//)""") {
//   REQUIRE(vm->_open_captures.empty());
// }

// ZTEST_CASE("Capture", R"""(
// var f2;
// var f3;
//
//{
//   var a = 32;
//
//   f2 = function(tag) {
//     print(tag, a);
//     a = 9;
//   }
//
////  f3 = function() {
////   a = 21;
////  }
////
////  f2("A(32)");
//
//  a = 69;
//  f2("A(69)");
//}
//
////f2("A(9)");
////f3();
////f2("A(21)");
////print(__locals__);
//)""") {
//  //
//  //  zb::print(vm->_open_captures);
////  REQUIRE(vm->_open_captures.empty());
//  //
//  //  if(a == 32) {
//  //    int a = 33;
//  //
//  //    zb::print("A", a);
//  //  }
//}

//
// ZTEST_CASE("Capture", R"""(
// var f2;
//{
//  var a = 32;
//
//  function A() {
//    a = 25;
//  }
//
//  print("A25", a);
//
//    f2 = function () {
//      print("A4", a);
//       //a = 123321;
//    }
////  {
////    function f5() {
////      print("A4", a);
////      a = 123321;
////    }
////
////    f2 = f5;
////  }
//
//  f2();
//  print("A1", a);
//
//  a  = 879;
//  print("A2", a);
// A();
//  f2();
//  print("A3", a);
//
//  a = 33333;
//  print("A3", a);
//}
//
//
// f2( );
//
//)""") {
////
//
////
////  if(a == 32) {
////    int a = 33;
////
////    zb::print("A", a);
////  }
//
//}

UTEST_CASE("dddddjhkljljkjlk") {

  zs::vm vm;
  zs::engine* eng = vm.get_engine();

  zs::jit::shared_state_data sdata(eng);
  zs::closure_compile_state c_compile_state(eng, sdata);
  c_compile_state.name = zs::_ss("main");
  c_compile_state._sdata._source_name = zs::_ss("test");

  REQUIRE(!c_compile_state.add_parameter(zs::_ss("__this__")));
  REQUIRE(!c_compile_state.add_parameter(zs::_ss("banana")));

  c_compile_state.add_instruction<zs::opcode::op_line>(1);

  c_compile_state.add_instruction<zs::opcode::op_load_small_string>(0, zs::ss_inst_data::create("bingo"));

  zs::int_t previous_n_capture = c_compile_state.get_current_capture_count();
  //  zb::print("_target_stack.size()", c_compile_state._target_stack.size());
  (void)c_compile_state.new_target();
  (void)c_compile_state.pop_target();
  REQUIRE(!c_compile_state.push_local_variable(zs::_ss("john"), 0));

  (void)c_compile_state.new_target();
  (void)c_compile_state.pop_target();
  REQUIRE(!c_compile_state.push_local_variable(zs::_ss("john"), 1));

  c_compile_state.set_stack_size(0);

  zs::object fpo = c_compile_state.build_function_prototype();

  //     zb::print(fpo->_parameter_names);
  //  fpo->debug_print();
}

// ZTEST_CASE("dddddd", R"""(
// var a = 32;
//
// function F() {
//   a = 34;
// }
// F();
// print(a);
//
// local var g = "George";
//
// export var JohnDow = 12;
//
//
//__exports__.peterson = 123;
//
////__exports__.peterson = 1237;
//
//
////k = 90;
//
// var kkk = function(k) { return $(v) { return v; }(k); }($(v) { return v; }(90)) + function(z) { return z +
// 1; }(32);
//
// var zp = function(f = $(){ return 32; }) {
//  return f();
//};
//
// if(a == 32) {
//  var a = 33;
////  print("AAAA", a, __locals__);
// var __locals__ = 1232;
////  print("AAAA", a, __locals__);
//  {
//    var a = 3232;
//
////  print("AAAA", a, __locals__);
//  }
//
//}
//  for(var a = 0; a < 12; a++) {
//    var a = 12;
//  }
//
////var a = 12332;
//)""") {
////
//
////
////  if(a == 32) {
////    int a = 33;
////
////    zb::print("A", a);
////  }
//
//}
