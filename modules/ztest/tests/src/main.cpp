#define ZTESTS_MAIN
#include <catch2.h>

#include <ztest/ztest.h>
#include "zvirtual_machine.h"
#include "lang/preprocessor/zmacro_parser.h"
//


//TEST_CASE("test.05") {
//  static constexpr std::string_view code = R"""(
//
//@macro EXPECT(cond, msg = "") { ztest.check(cond, msg, __THIS_LINE__, __LINE__) }
//
//var a = 32;
//$EXPECT(a == 32);
//$EXPECT(a == 32, "Wrong");
//return a;
//)""";
//  
//  zs::vm vm;
//  
//  zs::macro_parser parser(vm.get_engine());
//  
//  zs::object obj;
//  if(auto err = parser.parse(code, "sd", obj)) {
//    zb::print(parser.get_error(), err);
//  }
//   
//}



//TEST_CASE("DSKDJJKSD") {
//  zs::vm vm;
//  zs::object ztest_lib = zs::create_ztest_lib(vm.get_virtual_machine());
//  REQUIRE(ztest_lib.is_table());
//}
//
//TEST_CASE("test.01") {
//  static constexpr std::string_view code = R"""(
//const ztest = import("ztest");
//ztest.check(false);
//return 32;
//)""";
//  
//  zs::vm vm;
//  REQUIRE(!zs::import_ztest_lib(vm));
//
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(code, "test.01", closure)) {
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
//  REQUIRE(value == 32);
////  REQUIRE(value.as_table()["johnson"] == "alexandre");
//  
//}
//
//
//TEST_CASE("test.02") {
//  static constexpr std::string_view code = R"""(
//  EXPECT(true);
//  return 32;
//)""";
//  
//  zs::vm vm;
//  REQUIRE(!zs::import_ztest_lib(vm));
//
//  zs::object closure;
//
//  if (auto err = vm->compile_buffer(code, "test.02", closure)) {
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
//  REQUIRE(value == 32);
////  REQUIRE(value.as_table()["johnson"] == "alexandre");
//  
//}
//
//
//TEST_CASE("test.03") {
//  static constexpr std::string_view code = R"""(
//#define k_my_value = 32;
//@macro john(a) { print($a) }
//
//// P
//@macro ppp(a, b) {
//  var gh = $a;
//  var k = $b;
//
//}
//var c = 223;
//
// 
//@macro ppp1() {
//  var gh = 90;
//}
//@john(c);
//
//var a = 78;
//@ppp(1, "SAJSAK");
//return a;
//)""";
//  
//  zs::vm vm;
//  
//  
//  zs::jit_macro_parser parser(vm.get_engine());
//  
//  zs::object obj;
//  parser.parse(code, "sd", obj);
//  
////  
////  REQUIRE(!zs::import_ztest_lib(vm));
////
////  zs::object closure;
////
////  if (auto err = vm->compile_buffer(code, "test.02", closure)) {
////    FAIL(vm.get_error());
////  }
////
////  REQUIRE(closure.is_closure());
////
////  zs::object value;
////  if (auto err = vm->call(closure, { vm->get_root() }, value)) {
////    FAIL(vm.get_error());
////  }
////
////  REQUIRE(value == 78);
//////  REQUIRE(value.as_table()["johnson"] == "alexandre");
//  
//}
//
//
//
//
//
//
//

//
//TEST_CASE("test.04") {
//  static constexpr std::string_view code = R"""(
//var kkk = 123;
//
//@macro indentity(a) { a }
//
//@macro john(a) {
//  print(a, @indentity(a))
//}
//
//// P
//var c = 223;
//
//@macro rep(a, b) {
//  a, b, @john(90)
//}
//
//@john(c);
//
//@john("PETER");
//
//var a = 32;
//var arr = [@rep(9, 8)];
//return a;
//)""";
//  
//  zs::vm vm;
//  
//  
//  zs::jit_macro_parser parser(vm.get_engine());
//  
//  zs::object obj;
//  if(auto err = parser.parse(code, "sd", obj)) {
//    zb::print(parser.get_error(), err);
//  }
//   
//}



//TEST_CASE("test.05") {
//  static constexpr std::string_view code = R"""(
//@macro banana(a) { a }
//@macro my_list(a, ji = 62, k = "JOHNSON") {
//  a, ji, $banana(k)
//}
//
//var a = [$my_list(123)];
//var b = [$my_list(a[0], "PETER")];
//var c = [$my_list($my_list(1))];
//return a;
//)""";
//  
//  zs::vm vm;
//  
//  zs::jit_macro_parser parser(vm.get_engine());
//  
//  zs::object obj;
//  if(auto err = parser.parse(code, "sd", obj)) {
//    zb::print(parser.get_error(), err);
//  }
//   
//}

