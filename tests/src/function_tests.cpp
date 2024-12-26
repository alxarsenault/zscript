#include "unit_tests.h"

using namespace utest;

ZTEST_CASE("function-table-call", R"""(
function A(var a) {
  return a.b;
}

return A({b = 32});
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("function-table-call", R"""(
function A(var a) {
  return a.b;
}

return A {b = 32};
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("function-table-call", R"""(
var t = {
  function A(var a) {
    return a.b * 2;
  }
};

return t.A { "b":32 };
)""") {
  REQUIRE(value == 64);
}

ZTEST_CASE("function-table-call", R"""(
return { A = $(var a) return a.b * 2; }.A { "b":32 };
)""") {
  REQUIRE(value == 64);
}

ZTEST_CASE("$function", R"""(
var fct = $(a) return a;
return fct(32);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("arrow-function", R"""(
var fct = (a) => { return a; }
return fct(32);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("arrow-function", R"""(
var fct = (a) => a;
return fct(32);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("arrow-function", R"""(

function Call(f, a) {
  return f(a);
}

var fct = (a) => a;
return Call(fct, 32);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("arrow-function", R"""(

function Call(f, a) {
  return f(a);
}
 
return Call((a) => a, 32);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("arropw", R"""(
var a = {
  value = 32,
  function A() {
    return (()=> this.value)();
  }
};

var f2 = $() return 32;
return f2();
return a.A();
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("raw_call", R"""(

var f2 = $(a) return a + 2;
var f3 = (a)=> a + 12
//zs::print(f3(12));
//zs::print(f2(32));

var g3 = (j)=> {
return j + 90;
}

var dd = (u)=> u + 8;
//zs::print(g3(10), dd(670), ((u)  => u + 8;)(89));

var t1 = {
  a = 1,
  __add = $(rhs) return this.a + rhs.a;
  //__add = (rhs) =>{return this.a + rhs.a;}
 jkjjk = (a)=> a + 12,
 b = 90
};

var t2 = { a = 2 };

//var delegate = {
//  __add = function(rhs) {
//    return this.a + rhs.a;
//  }
//};
//zs::set_delegate(t1, delegate);

return t1 + t2;

)""") {
  REQUIRE(value == 3);
}

ZTEST_CASE("raw_call", R"""(
function A(a) {
  return a;
}

return zs::call(A, __this__, 21);
)""") {
  REQUIRE(value == 21);
}

ZTEST_CASE("raw_call", R"""(
function A(a, b, c) {
  return a + b + c;
}

return zs::call(A, __this__, 1, 2, 3);
)""") {
  REQUIRE(value == 6);
}

ZTEST_CASE("raw_call", R"""(
return zs::call($(a, b, c) {
  return a + b + c;
}, __this__, 1, 2, 3);
)""") {
  REQUIRE(value == 6);
}
ZTEST_CASE("raw_call", R"""(

var t = { a = 32 };

function A[t](a, b, c) {
  return this.a;
}
  
return A(1, 2, 3);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("raw_call", R"""(
function A[{ peter = 32 }](a, b, c) {
  return this.peter;
}
  
return A(1, 2, 3);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("raw_call", R"""(
function A[{ peter = 32 }](a, b, c) {
  return this.peter;
}
  
return zs::call(A, __this__, 1, 2, 3);
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("raw_call", R"""(

var John = this.john;

var delegate = {
  __call = function(a) {
    John(a);
//    zs::print(a, "DFLSKDJSKJLDS", this);
  }
}
 
var tbl = zs::set_delegate({}, delegate);
 
tbl(1);
//
var f1 = zs::bind(tbl, tbl);
f1(2);
//
var f2 = zs::bind(tbl, zs::placeholder);
f2(tbl, 3);

var f3 = zs::bind(delegate, zs::placeholder, 4);
f3(tbl);

var f4 = zs::bind(delegate, tbl, zs::placeholder);
f4(5);

)""",
    [](zs::vm_ref vm) {
      static int counter = 0;
      vm->get_root()._table->emplace("john", [](zs::vm_ref vm) -> zs::int_t {
        REQUIRE(vm[1] == ++counter);
        return 0;
      });
    }) {}

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

ZTEST_CASE("zslib", R"""(
return ($() return 32)();
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("zslib", R"""(
return $() return 32;();
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("zslib", R"""(
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
