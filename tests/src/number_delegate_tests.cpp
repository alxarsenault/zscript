#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("number", "return (32).to_string();") { REQUIRE(value == "32"); }

ZTEST_CASE("number", "return (32.23).to_string();") { REQUIRE(value == "32.23"); }

ZTEST_CASE("number", "return (0).to_string();") { REQUIRE(value == "0"); }

ZTEST_CASE("number.to_float", "return (0).to_float();") { REQUIRE(value == 0); }

ZTEST_CASE("number.to_float", "return (128).to_float();") { REQUIRE(value == 128.0); }

ZTEST_CASE("number.to_float", "return (12.12).to_float();") { REQUIRE(value == 12.12); }

ZTEST_CASE("number.to_int", "return (12.12).to_int();") { REQUIRE(value == 12); }

ZTEST_CASE("number.to_int", "return (12.99).to_int();") { REQUIRE(value == 12); }

ZTEST_CASE("number.to_int", "return (12.99999).to_int();") { REQUIRE(value == 12); }

ZTEST_CASE("number.to_int", "return (-12.12).to_int();") { REQUIRE(value == -12); }

ZTEST_CASE("number.to_int", "return (-12.99).to_int();") { REQUIRE(value == -12); }

ZTEST_CASE("number.to_int", "return (-128).to_int();") { REQUIRE(value == -128); }

ZTEST_CASE("number.to_char", "return ('A').to_char();") { REQUIRE(value == "A"); }

ZTEST_CASE("number.to_char", "return ('π').to_char();") { REQUIRE(value == "π"); }

ZTEST_CASE("number.to_char", "return (65).to_char();") { REQUIRE(value == "A"); }

ZTEST_CASE("number.is_neg", "return (0).is_neg();") { REQUIRE(value == false); }

ZTEST_CASE("number.is_neg", "return (1).is_neg();") { REQUIRE(value == false); }

ZTEST_CASE("number.is_neg", "return (-0.00001).is_neg();") { REQUIRE(value == true); }

ZTEST_CASE("ghjghgjhgj", R"""(
var delegate = {
  operator(>>=) = function(rhs, del) {
    this.a >>= rhs;
    return this;
  }
};

var t = {
  a = 56
};

zs::set_delegate(t, delegate);
t >>= 1;

return t.a;
)""") {
  //  REQUIRE(value == (56 >> 1));
}

ZTEST_CASE("tttt", R"""(
var t = zs::set_delegate({a = 56}, {
  operator(>>) = function(rhs, del) {
    return 78;
  }});

return t.a >> 1;
)""") {
  REQUIRE(value == 28);
}

ZTEST_CASE("function_delegate", R"""(
function A() {
  return 44;
}

return A.call( );
)""") {
  REQUIRE(value == 44);
}

ZTEST_CASE("function_delegate", R"""(

function A( ) {
  return john(bingo);
}
 

var kk = {};
var ddd = A.pcall(kk );
return ddd;
)""") {
  REQUIRE(value.is_null());
}

ZTEST_CASE("function_delegate", R"""(
 
function A() {
  return this.johnson;
}
var t  ={johnson = 89};
var B = A.with_binded_this(t);
return B();
)""") {
  REQUIRE(value == 89);
}

ZTEST_CASE("function_delegate", R"""(
 
function A() {
  return this.johnson;
}
var t  ={johnson = 89};
 A.bind_this(t);
return A();
)""") {
  REQUIRE(value == 89);
}

ZTEST_CASE("function_delegate", R"""(
function A() {}
var t = { johnson = 89 };
A.bind_this(t);
//return A.get_this() == t;
)""") {
  //  REQUIRE(value == true);
}

ZTEST_CASE("function_delegate", R"""(
var a = 0;
var b = a ? a : 55;
var c = a ?: 55;
return zs::all_equals(b, c, 55);
)""") {
  REQUIRE(value == true);
}

ZTEST_CASE("function_delegate", R"""(
return zs::in_range(9.9, 9.9, 90);
)""") {
  REQUIRE(value);
}

ZTEST_CASE("function_delegate", R"""(
return zs::in_range(9.9, 9.9, 90, false);
)""") {
  REQUIRE(!value);
}

ZTEST_CASE("all_true", R"""(
return zs::all_true(1, 3);
)""") {
  REQUIRE(value);
}

ZTEST_CASE("all_true", R"""(
return zs::all_true(1, 3, false);
)""") {
  REQUIRE(!value);
}

ZTEST_CASE("all_truekk", R"""(
return ($() return 32)();
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("all_truekk", R"""(
return $() return 32;();
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("all_truekk", R"""(
function A(var k = ...) {
  return 32;
}

return A();
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("all_truekk", R"""(
function A(... = 32 ) {
//  zs::print(vargv);
  return 32;
}

return A(1, 2, 3, {});
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("CASE1", R"""(
function A(... = 32 ) {
//  zs::print(vargv);
  return 32;
}
return A(1, 2, 3, 4);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("CASE2", R"""(
function A(k ... = 32 ) {
//  zs::print(k);
  return 32;
}
return A(1, 2, 3, 4);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("CASE3", R"""(
function A(var k ... = 32 ) {
//  zs::print(k);
  return 32;
}
return A(1, 2, 3, 4);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("CASE3", R"""(
function A(k ... = [8, 9] ) {
//  zs::print(k);
  return 32;
}
return A(1, 2, 3, 4);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("CASE3", R"""(
function A(k = ...) {
//  zs::print(k);
  return 32;
}
return A(1, 2, 3, 4);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("CASE4", R"""(
function A(var b ... = []) {
//  zs::print(b);
  return 32;
}

return zs::apply(A, this, [1, 2, [3, 3, 4], 4], 13, [10, 20, 30], [[1, 2]]);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("get_parameter_names", R"""(

function A(var a, var b, var c) {
  return a;
}

return A.get_parameter_names();
)""") {
  //  zs::print(value);
  //  REQUIRE(value == 4);
}

ZTEST_CASE("get_default_params", R"""(

function A(var a, var b, var c = 32, d = ...) {
  return a;
}

return A.get_default_params();
)""") {
  //  zs::print(value);
  //  REQUIRE(value == 4);
}

ZTEST_CASE("get_parameter_count", R"""(

function A(var a, var b, var c) {
  return a;
}

return A.get_parameter_count();
)""") {
  REQUIRE(value == 4);
}

ZTEST_CASE("variadic", R"""(

function Add(values = ...) {
  var sum = 0;
  for(int i = 0; i < values.size(); i++) {
    sum += values[i];
  }

  return sum;
}

var RAdd;

RAdd = function(a, b, values = ...) {
  return values ? zs::apply(RAdd, this, a + b, values) : a + b;
}

return RAdd(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
)""") {
  REQUIRE(value == 120);
}

ZTEST_CASE("function_abc", R"""(

function A() {
  var a = 332;
  var b = 445;
  return b;
}

return A();
)""") {
  REQUIRE(value == 445);
}
ZTEST_CASE("gkloo", R"""(

global.set("k", 90);
global.k = 89;
//zs::print(global);
return 445;
)""") {
  REQUIRE(value == 445);
}

ZTEST_CASE("float_array", R"""(
var arr = np.array([1, 2, 3, -4, 5]);
var s  = np.ramp(0, 5);

arr[2] = 12.12;
arr[2] = 123.12;

function john() {
}

john();

//var a = arr;
//a[2] = 89.12;

//a.push(1232.2);

//zs::print(89, arr.size(), 78, arr[2], 99, arr, 777, typeof(arr), 88, arr.min(),   789);

//for(int i = 0; i < s.size(); i++) {
//    zs::print(s[i]);
//}

return arr;
)""") {

  zs::object f = vm->get_global().as_table().get_delegate().as_table()["np"].as_table()["ramp"];
  //  zs::print(f);

  REQUIRE(!vm->call(f, { vm->get_global(), zs::object(0), 10, 10 }, value));
}
