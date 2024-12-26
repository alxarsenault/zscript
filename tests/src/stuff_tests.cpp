#include "unit_tests.h"
#include "string_template.h"
#include <zscript/utility/string_template.h>
#include <zscript/std/zmutable_string.h>

#include "jit/zclosure_compile_state.h"
using namespace utest;

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

ZTEST_CASE("dsdsds", R"""(
//zs::add_import_directory(ZSCRIPT_MODULES_DIRECTORY);
const argparser = import("argparser");


var p = argparser("John", "Description");

p.add_argument("file", "Filepath", false, 2);
p.add_argument("output", "output", false, 0);
p.add_option("include", ["-I", "--include"], "Include stuff", false, true);
p.add_flag("version", ["-v", "--version"], "Show version");

//p.print_help();

var result = p.parse(["John", "-I", "Steeve", "-I=Jason", "-v","Alexandre", "Alexandre", "kl"]);

//var result = p.parse(["John", "-I", "Steeve", "-I=Jason", "-v","Alexandre" , "Johnson"]);

//zs::print(result, result._error);
return 32;
)""") {

  //  zb::print("-----------------------",value.as_struct_instance().get_base().as_struct().get_doc());
}

TEST_CASE("render_template_string_html") {

  zs::vm vm;

  zs::object tbl = zs::_t(vm);

  tbl.as_table()["john"] = zs::_ss("Alex");

  zs::object files_obj = zs::_t(vm);
  zs::table_object& files = files_obj.as_table();

  files["header"] = zs::_s(vm, R""""(<head>
  <title>@<<title>>@</title>
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.5.0/font/bootstrap-icons.css">
  <link href="css/doc-style.css" rel="stylesheet">
</head>)"""");

  files["body"] = zs::_s(vm, R""""(<body>
var a = "@<<john>>@";
</body>)"""");

  tbl.as_table()["files"] = std::move(files_obj);

  tbl.as_table()["inplace"] = +[](zs::vm_ref vm) -> zs::int_t {
    zs::int_t nargs = vm.stack_size();

    if (!vm[1].is_string()) {
      return -1;
    }

    std::string_view name = vm[1].get_string_unchecked();

    zs::table_object& files = vm[0].as_table()["files"].as_table();
    if (auto it = files.find(name); it != files.end()) {
      return vm.push_string(zs::render_template_string(
          vm, nargs >= 3 ? vm[2] : vm[0], it->second.get_string_unchecked(), "@<<", ">>@"));
    }

    return -1;
  };

  std::string content = R""""(
@<<inplace("header", { title = "Alex" })>>@

@<<inplace("body")>>@
)"""";

  zs::string result = zs::render_template_string(vm, tbl, content, "@<<", ">>@");

  //  zb::print("--------------", result);
}

// TEST_CASE("render_template_string") {
//
//   zs::vm vm;
//
//   zs::object tbl = zs::_t(vm);
//   tbl.as_table()["john"] = zs::_s(vm, "Alexandre Arsenault");
//   tbl.as_table()["johnson"] = +[](zs::vm_ref vm) -> zs::int_t {
//     vm.push(vm[1]);
//     return 1;
//   };
//
//   tbl.as_table()["yes"] = +[](zs::vm_ref vm) -> zs::int_t {
//     vm.push(zs::object::create_concat_string(
//         vm.get_engine(), vm[1].get_string_unchecked(), vm[2].get_string_unchecked()));
//     return 1;
//   };
//
//   std::string content = R""""(@<>@
//   var peter = @<john>@ + "PPP";
//   )"""";
//
//   zs::string result = zs::render_template_string(vm, tbl, content, "@<", ">@");
//
//   zb::print("--------------", result);
//
//   zs::object closure;
//   if (auto err = vm->compile_buffer(result, "", closure)) {
//     zb::print(err, vm->get_error());
//   }
// }

// TEST_CASE("DSLKDJSKJDSL") {
//
//   zs::vm vm;
//
//   zs::object tbl = zs::_t(vm);
//   tbl.as_table()["john"] = zs::_s(vm, "Alexandre Arsenault");
//   tbl.as_table()["johnson"] = +[](zs::vm_ref vm) -> zs::int_t {
//     vm.push(vm[1]);
//     return 1;
//   };
//
//   tbl.as_table()["yes"] = +[](zs::vm_ref vm) -> zs::int_t {
//     vm.push(zs::object::create_concat_string(
//         vm.get_engine(), vm[1].get_string_unchecked(), vm[2].get_string_unchecked()));
//     return 1;
//   };
//
//   {
//     std::string content = R""""(
//
//   var peter = <%john%> + "PPP";
//   var alex = <%324%>;
//   var p = <%johnson(523)%>;
//   var p2 = <%yes("A", "B")%>;
//
//   var Q = {
//     A = <%yes("AAA", "CCC")%>,
//     B = 123,
//     D = <%150 + 50%>,
//     E = <%fs::value_file("/Users/alexarse/Develop/zscript/tests/unit_tests2/resources/value_01.zs")%>,
//     F = <%fs::string_file("/Users/alexarse/Develop/zscript/tests/unit_tests2/resources/string_01.zs")%>
//   };
//   )"""";
//
//     zs::string result = render_template_string(vm, tbl, content, "<%", "%>");
//
//     zb::print("--------------", result);
//
//     zs::object closure;
//     if (auto err = vm->compile_buffer(result, "", closure)) {
//       zb::print(err, vm->get_error());
//     }
//   }
// }

ZTEST_CASE("doc", ZSCRIPT_EXAMPLES_DIRECTORY "/doc.zs") {
  zs::object doc = value.as_struct_instance().get_base().as_struct().get_doc();
  //   zs::serialize_to_json(vm.get_engine(), std::cout, doc);
}

ZTEST_CASE("doc", R"""(

```
MyBongo Banasa
Alex. And bu
Andre
@detail NONO
BANANA.
```
struct MyBongo2 {

  ```
  Does a lot.
  A lot a lot.
  ```
  function DoIt() {
  }

  ``` Does a lot. ```
  function DoItAgain() {
  }
};
 

var b = MyBongo2();

return b;
)""") {

  //  zb::print("-----------------------",value.as_struct_instance().get_base().as_struct().get_doc());
}

// ZTEST_CASE("$expr", R"""(
// return fs::ls(ZSCRIPT_TESTS_RESOURCES_DIRECTORY, "zs");
//)""") {
//   zb::print(value);
// }
//
// ZTEST_CASE("$expr", R"""(
// return fs::ls(ZSCRIPT_TESTS_RESOURCES_DIRECTORY, [".sh", "sh"]);
//)""") {
//   zb::print(value);
// }

ZTEST_CASE("$expr", R"""(
return math::sin(0);
)""") {
  REQUIRE(value == 0);
}

ZTEST_CASE("$expr", R"""(
const math = import("math");
return math.sin(0);
)""") {
  REQUIRE(value == 0);
}

ZTEST_CASE("struct", R"""(
return struct{};
)""") {
  REQUIRE(value.is_struct());
}

ZTEST_CASE("Scope", R"""(
@module dev-uio
@author "Alexandre Arsenault"
@version 1.10.1
@date 2024-08-23
@copyright BananananP

const m1 = import("module_04");
//local var p1 = 23;
//
//
//var t = {
//  A = 32,
//  B = function() {
//
//  }
//};
//
//t.B();
//m1.a = 222;

function main(var args = []) {
//  print(args, p1);
  //p1 = 12;
  return 0;
}

)""") {
  //  zb::print(value);

  //  auto vv = value.as_table()["main"];
  //  zs::object ret;
  //  REQUIRE(!vm->call(vv, { value, zs::_a(vm.get_engine(), { zs::_ss("john"), 123, 12.12 }) }, ret));
  //  zb::print(ret);
  //
  //  REQUIRE(!vm->call(vv, { value, zs::_a(vm.get_engine(), { zs::_ss("john") }) }, ret));
  //  zb::print(ret);
}

ZTEST_CASE("ImportDev2", R"""(
@module dev-uio
@author "Alexandre Arsenault"
@author "Anghj" jk
@brief '''"BananananP"


jhjhjhjkh

'''
@version 1.10.1
@date 2024-08-23
@copyright BananananP

const m1 = import("module_04");

//local const john = 908;
//local var abcde = 12312312;
//__locals__.abcd = 123;

//export john;
//export.ABC = {};
var aa = 32;
//var aa = 333;

//export function BDSJHD() {
//  var abcdss = 232;
//  print("KKKK",  local.abcde, __locals__.abcde, local.abcd, __locals__.abcd);
//}

//print(abcd);

//BDSJHD();
//__exports__.ABC.g = 7787;
//export m1;
)""") {
  //  zb::print(value);

  //  auto vv = value.as_table()["BDSJHD"];
  //  zs::object ret;
  //  vm->call(vv, vm->get_root(), ret);
}

ZTEST_CASE("ImportDev", R"""(
@module dev
@author Alexandre Arsenault
@brief Banananan

const geo = import("geo");
const math = import("math");
const base64 = import("base64");
//const fs = import("fs");

var r1 = geo.Rect(10, 20, 30, 40);
 
var gino = base64.encode("ALexndre");
var gino2 = base64.decode(gino);

var s1 = r1.size();
var s2 = r1.size();

function make_it(a, b) {
  return geo.Point(a, b);
}

var m1 = make_it(1, 2);

function make_it_again(a, b) {
  var mi = make_it(a, b);
  mi.x = s1.width + s2.width;
  return mi;
}

var pts = [geo.Point(0, 0), geo.Point(0, 1), geo.Point(0, 2), geo.Point(1, 0)];


var m2 = make_it_again(100, 200);

var t = {
  function bingo(n) {
    return 8 * n + s1.width;
  }
};
 
s1.width = 1000;

const Alex = "Alex";

struct God {
  var a = 32;
};

var gggg = {
  bacon = "hotdog"
};

//var gggg = "BANANA";

var abc = 123;

return {"r1":r1, "s1":s1};
)""") {
  REQUIRE(value.is_table());
  // REQUIRE(value.as_table()["a"] == 678);
  //   zb::print("N", value);
  //   zb::print(vm->get_root());
}

ZTEST_CASE("JESUS", R"""(
const m1 = import("module_01.zs");
 
var f = 32;
f = 234;
m1.a = 92;
 

var K = 323;
K = 888;

var ttt = {
bingo = 234,

gg = function() {
  bingo = 32;
//  johnsonsh = 234;
}
};
function banana() {
  K = 234;
//  bagel = 32;
}

banana();

ttt.gg();

__this__.john = 32;

//john = 234;
 

//__exports__.somesome = "Some";
 
)""") {
  //  REQUIRE(value.is_table());
  // REQUIRE(value.as_table()["a"] == 678);
  //   zb::print("N", value);
  //   zb::print(vm->get_root());
}
// ZTEST_CASE("$expr", R"""(
// const var a = @expr {
// var fpath = fs::path("/Users/alexarse/Develop/zscript/samples/string_01.txt");
// var file_content = fpath.read_all();
// return file_content;
// };
////zs::print(a);
// return a;
//)""") {
//   //  REQUIRE(value == 121);
// }

// ZTEST_CASE("$expr", R"""(
// const var a = @expr { return fs::json_file("/Users/alexarse/Develop/zscript/samples/obj_01.json"); };
// return a;
//)""") {
//   REQUIRE(value.is_table());
//   REQUIRE(value.as_table()["a"] == 32);
//   REQUIRE(value.as_table()["b"] == "johnson");
// }

//
//
// ZTEST_CASE("graphics", R"""(
// const gfx = import("graphics");
// const math = import("math");
//
// var ctx = gfx.context(500, 500);
// ctx.set_fill_color(gfx.white);
// ctx.fill_rect(0, 0, 500, 500);
//
//
// ctx.rotate(0.2);
// ctx.set_fill_color(gfx.red);
// ctx.fill_rect(100, 100, 50, 50);
//
// var output_image_path = fs::join(ZSCRIPT_TESTS_OUTPUT_DIRECTORY, "test_img_04.png");
// if(var err = ctx.save_to_image(output_image_path)) {
//  zs::print("Save image failed", err);
//}
//)""") {}
//
// ZTEST_CASE("graphics", R"""(
// const gfx = import("graphics");
// const math = import("math");
//
// var ctx = gfx.context(500, 500);
// ctx.set_fill_color(gfx.white);
// ctx.fill_rect(0, 0, 500, 500);
//
// var output_image_path = fs::join(ZSCRIPT_TESTS_OUTPUT_DIRECTORY, "test_img_03.png");
// if(var err = ctx.save_to_image(output_image_path)) {
//  zs::print("Save image failed", err);
//}
//)""") {}
//
// ZTEST_CASE("$expr", R"""(
//
// struct George {
// static function john() {
////    zs::print("JOHN");
//  }
//
//  var Q = 32;
//  static var P = 21;
//};
//
// George.john();
////George.john = function () {
//// zs::print("JOHN");
////};
//
// George.P = 343;
//
// var g = George();
////g.john();
// g.Q = 123;
// return g;
//)""") {
//   //  zb::print("AAAAAAA",value);
// }
//
// ZTEST_CASE("graphics", R"""(
// const gfx = import("graphics");
// const math = import("math");
//
// var ctx = gfx.context(256, 256);
// ctx.set_fill_color(gfx.white);
// ctx.fill_rect(0, 0, 256, 256);
//
//
// var p0 = gfx.path();
// p0.add_rounded_rect(30, 30, 100, 100, 5.0, 5.0);
//
//
// ctx.set_fill_color(gfx.coral);
// ctx.fill_path(p0);
//
// ctx.set_stroke_width(2.0);
// ctx.set_stroke_color(gfx.black);
// ctx.stroke_path(p0);
//
//
// ctx.push();
// ctx.transform(gfx.transform.translation(100, 100));
// ctx.set_stroke_color(gfx.hot_pink);
// ctx.stroke_path(p0);
// ctx.pop();
//
// ctx.push();
// ctx.transform(gfx.transform.rotation(2.0 * math.pi) * gfx.transform.translation(115, 115));
////ctx.transform();
//
// ctx.set_stroke_color(gfx.green);
// ctx.stroke_path(p0);
//
// ctx.pop();
//
//
//{
//  var p1 = gfx.path();
//  p1.add_rounded_rect(gfx.transform.translation(-25, -25), 0, 0, 50, 50, 10.0, 10.0);
//  var p2 = gfx.path();
//  p2.add_path(gfx.transform.rotation(0.2), p1);
//  ctx.set_fill_color(gfx.blue);
//  ctx.translate(100, 100);
//  ctx.fill_path(p2);
//  ctx.translate(-100, -100);
//}
//
//{
//  ctx.push();
//  var p1 = gfx.path();
//  p1.add_rounded_rect(gfx.transform.translation(-25, -25), 0, 0, 50, 50, 10.0, 10.0);
//
//  ctx.transform(gfx.transform.translation(145, 350) * gfx.transform.rotation(0.25 * math.pi));
//
//  ctx.set_fill_color(gfx.orange);
//  ctx.fill_path(p1);
//  ctx.pop();
//}
//
// ctx.scoped_draw($() {
//  var p1 = gfx.path();
//  p1.add_rounded_rect(gfx.transform.translation(-25, -25), 0, 0, 50, 50, 10.0, 10.0);
//  p1.add_line(10, 10, 50, 50);
//  p1.line_to(gfx.point(100, 150));
//  p1.add_line(gfx.point(100, 150), gfx.point(100, 100));
//
//  transform(gfx.transform.translation(250, 250) * gfx.transform.rotation(0.25 * math.pi));
//  set_stroke_color(gfx.orange);
//  stroke_path(p1);
//});
//
////ctx.scoped_draw((gc) => {
////  var p1 = gfx.path();
////  p1.add_rounded_rect(gfx.transform.translation(-25, -25), 0, 0, 50, 50, 10.0, 10.0);
////  gc.transform(gfx.transform.translation(250, 250) * gfx.transform.rotation(0.25 * math.pi));
////  gc.set_fill_color(gfx.orange);
////  gc.fill_path(p1);
////});
//
// var output_image_path = fs::join(ZSCRIPT_TESTS_OUTPUT_DIRECTORY, "test_img_02.png");
// if(var err = ctx.save_to_image(output_image_path)) {
//  zs::print("Save image failed", err);
//}
//)""") {}
//
// ZTEST_CASE("$expr", R"""(
// const gfx = import("graphics");
//
// var ctx = gfx.context(550, 500);
////zs::print(ctx.is_bitmap_context(), ctx.get_size());
//
//// ctx.init(500, 500);
//
// var ts = gfx.transform();
// var ts2 = ts.translate(10, 10);
// var ts3 = gfx.transform.rotation(2.0);
////zs::print(ts, ts2, ts3);
//
// var ts4 = ts2.concat(ts3);
// var ts5 = ts2 * ts3;
////zs::print(ts4, ts5, typeof(ts5));
//
//
//
//
// var r0 = gfx.rect(10, 20, 30, 40);
////zs::print(r0);
// r0.x = 13;
////zs::print(r0, typeof(r0));
//
// struct John {
// var c = 9;
//};
//
////var j = John();
////zs::print(typeof(j));
//
// var pos = r0.position();
// var size = r0.size();
////zs::print(pos , size);
//
// ctx.set_fill_color(gfx.rgb(0xFF0000FF));
// ctx.fill_rect(0, 0, 500, 500);
//
// ctx.set_fill_color(gfx.rgb(0x00FF00FF));
// ctx.fill_rect(gfx.rect(10, 10, 480, 480));
//
// ctx.set_fill_color(gfx.rgb(0x0000FFFF));
// ctx.fill_rect(20, 20, 460, 460);
//
// ctx.set_fill_color(gfx.rgb(0xFFFFFFFF));
// ctx.fill_rect(40, 40, 420, 420);
//
// ctx.set_stroke_width(3);
// ctx.set_stroke_color( 0x000000FF);
// ctx.stroke_rect(40, 40, 420, 420);
//
// var ppp = gfx.path();
// ppp.move_to(50, 50);
// ppp.line_to(150, 150);
// ppp.add_line(200, 90, 200, 350);
//
// ctx.set_stroke_width(10);
// ctx.stroke_path(ppp);
//
// ctx.push();
// ctx.apply_transform(gfx.transform.translation(40, 0));
// ctx.stroke_path(ppp);
// ctx.pop();
//
// ctx.push();
// ctx.apply_transform(gfx.transform.translation(-40, 0));
// ctx.stroke_path(ppp);
//
// ctx.pop();
//
// var ts90 = gfx.transform(1, 0, 0, 20.2);
////ctx.apply_transform(ts90);
////ctx.apply_transform(gfx.transform());
////ctx.apply_transform(gfx.transform.translation(-500.0, 0));
//// var ts8 = ctx.get_transform();
////zs::print("ts90", ts90, typeof(ts90.a));//, "TS8", ts8);
//
//
////var img = gfx.image();
////img.load("/Users/alexarse/Develop/zscript/tests/unit_tests/resources/IMG_4921.png");
////zs::print(img, typeof(img), typeof(ctx), typeof(ppp));
//
////ctx.draw_image_resized(img, 100, 100, 40, 40);
////ctx.draw_image(img, gfx.point(0, 10));
////ctx.draw_image_part(img, gfx.point(40, 10), 17, 28, 52, 69);
//
////ctx.draw_image_part_resized(img, gfx.rect(10, 10, 200, 100), gfx.rect(17, 28, 52, 69));
//
////ctx.save_to_image("/Users/alexarse/Develop/zscript/tests/unit_tests2/resources/test_img.png");
//
// return ctx;
//)""") {
//  //  zb::print(value);
//}
//
// ZTEST_CASE("$expr", R"""(
// const graphics = import("graphics");
//
// var c = graphics.rgb(0xFF0000FF);
// return c;
//)""") {
//  //  zb::print(value);
//}
