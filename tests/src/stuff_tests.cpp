#include "unit_tests.h"
#include "string_template.h"
#include <zscript/utility/string_template.h>
#include <zscript/std/zmutable_string.h>

using namespace utest;

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

//
// MARK: Unary Arithmetic.
//

ZTEST_CASE("unary_arithmetic", R"""(
var t = { a = 0 };
var a0 = t.a++;
return [a0, t.a];
)""") {
  REQUIRE(value == zs::_a(vm, { 0, 1 }));
}

ZTEST_CASE("unary_arithmetic", R"""(
var t = { a = 0 };
var a0 = ++t.a;
return [a0, t.a];
)""") {
  REQUIRE(value == zs::_a(vm, { 1, 1 }));
}

//
// ZTEST_CASE("unary_arithmetic", R"""(
// var t = {
//  value = 10,
//
//  function __pre_incr() {
//    ++this.value;
//    return this;
//  }
//
//  function __incr() {
//    var v = this.value;
//    this.value = v + 1;
//    return v;
//  }
//};
//
// t++;
// return t;//[t++, t.value];
//)""") {
//  zs::print(value);
////  REQUIRE(value  == zs::_a(vm , {10, 11}));
//}

ZTEST_CASE("unary_arithmetic", R"""(
var t = {
  value = 10,

  function __pre_incr() {
    this.value = this.value + 1;
    return this.value;
  }
};

var k = ++t;
return [t, k];
)""") {

  //  zs::print( value);
  //  REQUIRE(value  == zs::_a(vm , {11, 11}));
}

ZTEST_CASE("fhjhkjh222", R"""(
var t = {
  value = 10,

  function __pre_incr() {
    ++this.value;
    return this;
  }

  function __incr() {
    return this.value++;
  }
};

var k = t++;
++t;
return [k, t.value];
)""") {
  REQUIRE(value == zs::_a(vm, { 10, 12 }));
}

ZTEST_CASE("unary_arithmetic", R"""(
var arr = [0, 1, 2, 3, 4, 5];

var it = arr.begin();
var i = it;
var itt = it++;
itt = it++;
return [i.get(), it.get(), itt.get()];
)""") {

  REQUIRE(value == zs::_a(vm, { 0, 2, 1 }));
}

ZTEST_CASE("unary_arithmetic", R"""(
var arr = [0, 1, 2, 3, 4, 5];
var it = arr.begin();

var i = it;
var itt = ++it;
itt = ++it;
return [i.get(), it.get(), itt.get()];
)""") {
  REQUIRE(value == zs::_a(vm, { 0, 2, 2 }));
}

ZTEST_CASE("copy", R"""(
var arr = [0, 1, 2, 3, 4, 5];
var t = {__copy = function(delegate) {
  return this;
}, a = 89};

var a = zs::copy(t);
a.a = 32;
return [t, a];
)""") {
//  zs::print(value);
  //  REQUIRE(value == zs::_a(vm, { 0, 2, 2 }));
}

ZTEST_CASE("copy", R"""(
var arr = [0, 1, 2, 3, 4, 5];
var t = { a = 89};

var a = zs::copy(t);
a.a = 32;
return [t, a];
)""") {
//  zs::print(value);
  //  REQUIRE(value == zs::_a(vm, { 0, 2, 2 }));
}

ZTEST_CASE("copy", R"""(
var arr = mutable_string("DSLKDJS");

var t = { a = 89};
var p  = fs::path("/Alex/sa");

var d = {__typeof = "DD"};
//zs::set_delegate(arr, d);
var a = zs::copy(arr);
//a.a = 32;
// zs::get_delegate(p),
// zs::get_delegate(p),
return [t, a == arr, p == "/Alex/sa",fs::is_path(p), fs::is_path("/Alex/sa")];
)""") {
  zs::print(value);
  //  REQUIRE(value == zs::_a(vm, { 0, 2, 2 }));
}
