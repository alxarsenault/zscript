#include "unit_tests.h"

ZCODE_TEST("struct.01", R"""(
var a = struct{};
return a;
)""") {
  
}

ZCODE_TEST_FAIL_COMPILE("struct.02", R"""(
var a = struct Name{};
return a;
)""" ) {
}


ZCODE_TEST("struct.03", R"""(
struct Abc {
  k = 1;
  p = 9;
};

return Abc;
)""") {
  REQUIRE(value.is_struct());
}


ZCODE_TEST("struct.04", R"""(
var Abc = struct {
  k = 1;
  p = 9;
};

return Abc;
)""") {
  REQUIRE(value.is_struct());
}


ZCODE_TEST("struct.05", R"""(
struct Abc {
  constructor(var a = 1, var b = 2, var c = 3) {
    this.a = a;
    this.b = b;
    this.c = c;
  }

  var a = 1;
  var b = 2;
  var c = 3;
};

var a = Abc();
var b = Abc(10);
var c = Abc(10, 11);
var d = Abc(10, 11, 12);
return [Abc, a, b, c, d];
)""") {
  REQUIRE(value.is_array());
   
  auto test_struct = [](zs::object obj, std::initializer_list<const zs::object> vs) {
    const zs::struct_instance_object& sinst = obj.as_struct_instance();
    zb::span<const zs::object>  values = {vs};
    REQUIRE(sinst.size() == 3);
    REQUIRE(sinst.key(0) == "a");
    REQUIRE(sinst.key(1) == "b");
    REQUIRE(sinst.key(2) == "c");
    
    REQUIRE(sinst[0] == values[0]);
    REQUIRE(sinst[1] == values[1]);
    REQUIRE(sinst[2] == values[2]);

    REQUIRE(sinst.get_span() == values);
  };
  
  const zs::array_object& arr = value.as_array();
  
  test_struct(arr[1] , {1, 2, 3});
  test_struct(arr[2], {10, 2, 3});
  test_struct(arr[3], {10, 11, 3});
  test_struct(arr[4], {10, 11, 12});
}




ZCODE_TEST("struct.06", R"""(
struct Abc {
  constructor(var a, var b, var c) {
    this.a = a;
    this.b = b;
    this.c = c;
  }

  constructor(var a) {
    this.a = a;
    this.b = 2;
    this.c = 3;
  }

  constructor(var b) {
    this.a = 1;
    this.b = b;
    this.c = 3;
  }

  var a = 1;
  var b = 2;
  var c = 3;
};

var a = Abc(10, 11, 12);
var b = Abc(90);
return [Abc, a, b];
)""") {
  REQUIRE(value.is_array());
   
  auto test_struct = [](zs::object obj, std::initializer_list<const zs::object> vs) {
    const zs::struct_instance_object& sinst = obj.as_struct_instance();
    zb::span<const zs::object>  values = {vs};
    REQUIRE(sinst.size() == 3);
    REQUIRE(sinst.key(0) == "a");
    REQUIRE(sinst.key(1) == "b");
    REQUIRE(sinst.key(2) == "c");
    
    REQUIRE(sinst[0] == values[0]);
    REQUIRE(sinst[1] == values[1]);
    REQUIRE(sinst[2] == values[2]);

    REQUIRE(sinst.get_span() == values);
  };
  
  const zs::array_object& arr = value.as_array();
  
  test_struct(arr[1] , {10, 11, 12});
}


ZS_CODE_TEST("struct.23432", R"""(
struct John {
  static var S1 = 32;
  var k = 1;
  var p = 9;
};

John.S1 = 51;
var a = John();
return a;
)""") {
  REQUIRE(value.is_struct_instance());
}

ZS_CODE_TEST("struct.4343443", R"""(

struct John
{
  constructor(var k, var k2 = 32) {
    this.j1 = k;
    this.j3[0] = k2;
  }

  var j1 = 1;
  int j2 = 9;
  var j3 = [1, 2, 3];
};

var a = John("A", 66);
return a;
)""") {
  REQUIRE(value.is_struct_instance());
}

ZS_CODE_TEST("struct.07", R"""(

struct John
{
  static var Q = 66;

  constructor(var k, var k2 = 32) {
    this.j1 = k;
    this.j3[0] = k2;
  }

  var j1 = 1;
  int j2 = 9;
  var j3 = [1, 2, 3];
};

var a = John("A", 66);
var b = John("B");
 
return [a, b, a.Q];
)""") {
  //    zb::print(value);
}
//
ZS_CODE_TEST("struct.08", R"""(

struct John
{
  constructor(var k1, var k2) {
    this.j1 = k1;
    this.j2 = k2;
  }

  const var j1 = 1;
  var j2 = 2;
};

var a = John("A", "B");
var b = John("A", "B");

//a.j1 = 55;
a.j2 = 55;

return [a, b];
)""") {
  //  zb::print(value);
}

ZS_CODE_TEST("struct.09", R"""(

struct John
{
  static int Q1 = 155;
  static const var Q2 = 255;

  constructor(var k1, var k2) {
    this.j1 = k1;
    this.j2 = k2;
  }

  const var j1 = 1;
  int j2 = 2;
};

var a = John("A", 32);
a.Q1 = 123;

var b = John("A", 33);

return [a, b, a.Q1, b.Q1, a.Q2];
)""") {
  //  zb::print(value);
}

ZS_CODE_TEST("struct.10", R"""(

struct John
{
  static var bingo = 23;
  constructor(var k1, var k2) {
    this.j1 = k1;
    this.j2 = k2;
  }

  var banana = function(var k1, var k2) {
    this.j1 = k1;
    this.j2 = k2;
  };

  const var j1 = 1;
  string j2 = "bacon";
};

var a = John("A", "B");
 
return a;
)""") {
  //  zb::print(value);
}

ZS_CODE_TEST("struct.11", R"""(

struct John
{
  constructor(var val = null) {
    if(val != null) {
      this.value = val;
    }
  }

    int value = 0;
};

var a = John();
var v = a.value;
a.value = 32;
return [v, a.value];
)""") {
  REQUIRE(value.as_array()[0] == 0);
  REQUIRE(value.as_array()[1] == 32);
}

ZS_CODE_TEST("struct.12", R"""(
struct John {
  constructor(var val = 0) {
    this.value = val;
  }

  int value;
};

return John(322);
)""") {
  REQUIRE(value.is_struct_instance());
}

// ZS_CODE_TEST("struct.debug", R"""(
//
// struct John {
//   // Constructor.
//   constructor(var a){
//     this.value = a;
//   }
//   static var S2 = 200;
//   var m7 = [1, 2, 3];
//   var value;
// };
//
// var a = John(32);
//
// return a;
//)""") {
//
// }

ZS_CODE_TEST("struct.debug", R"""(

struct John {
  // Constructor.
  constructor( var a) {
  }

  // Static member (no type).
  static S1 = 100;

  // Static member (var type).
  static var S2 = 200;

  // Static member (int type).
  static int S3 = 300;

  // Const static member (no type).
  static const S4 = 400;

  // Const static member (var type).
  static const var S5 = 500;
  
  // Const static member (int type).
  static const int S6 = 600;

  // Member(no type).
  m1 = 1;

  // Member(var type).
  var m2 = 2;

  // Member(int type).
  int m3 = 3;

  // Const member (no type).
  const m4 = 4;

  // Const member (var type).
  const var m5 = 5;

  // Const member (int type).
  const int m6 = 6;

  var m7 = [1, 2, 3];
};
 

var a = John(32);
 

return [a];
)""") {
  //  REQUIRE(value.as_array()[0].is_struct_instance());
  //  REQUIRE(value.as_array()[1].is_struct_instance());
  //  REQUIRE(value.as_array()[2].is_struct());
  //
  //  //    zb::print(value.as_array()[0].to_json());
  //  //    zb::print(value.as_array()[0].convert_to_string());
  //  // A.
  //  {
  //    const zs::struct_instance_object& a = value.as_array()[0].as_struct_instance();
  //
  //    zs::object m1;
  //    zs::object m2;
  //    zs::object m3;
  //    zs::object m4;
  //    zs::object m5;
  //    zs::object m6;
  //    zs::object m7;
  //
  //    zs::object S1;
  //    zs::object S2;
  //    zs::object S3;
  //    zs::object S4;
  //    zs::object S5;
  //    zs::object S6;
  //
  //    REQUIRE(!a.get(zs::_ss("m1"), m1));
  //    REQUIRE(!a.get(zs::_ss("m2"), m2));
  //    REQUIRE(!a.get(zs::_ss("m3"), m3));
  //    REQUIRE(!a.get(zs::_ss("m4"), m4));
  //    REQUIRE(!a.get(zs::_ss("m5"), m5));
  //    REQUIRE(!a.get(zs::_ss("m6"), m6));
  //    REQUIRE(!a.get(zs::_ss("m7"), m7));
  //
  //    REQUIRE(!a.get(zs::_ss("S1"), S1));
  //    REQUIRE(!a.get(zs::_ss("S2"), S2));
  //    REQUIRE(!a.get(zs::_ss("S3"), S3));
  //    REQUIRE(!a.get(zs::_ss("S4"), S4));
  //    REQUIRE(!a.get(zs::_ss("S5"), S5));
  //    REQUIRE(!a.get(zs::_ss("S6"), S6));
  //
  //    REQUIRE(m1 == 1);
  //    REQUIRE(m2 == 2);
  //    REQUIRE(m3 == 3);
  //    REQUIRE(m4 == 4);
  //    REQUIRE(m5 == 5);
  //    REQUIRE(m6 == 6);
  //    REQUIRE(m7.is_array());
  //    REQUIRE(m7.as_array()[0] == -1);
  //
  //    REQUIRE(S1 == 51);
  //    REQUIRE(S2 == 52);
  //    REQUIRE(S3 == 300);
  //    REQUIRE(S4 == 400);
  //    REQUIRE(S5 == 500);
  //    REQUIRE(S6 == 600);
  //  }
  //
  //  // B.
  //  {
  //    const zs::struct_instance_object& b = value.as_array()[1].as_struct_instance();
  //
  //    zs::object m1;
  //    zs::object m2;
  //    zs::object m3;
  //    zs::object m4;
  //    zs::object m5;
  //    zs::object m6;
  //    zs::object m7;
  //
  //    zs::object S1;
  //    zs::object S2;
  //    zs::object S3;
  //    zs::object S4;
  //    zs::object S5;
  //    zs::object S6;
  //
  //    REQUIRE(!b.get(zs::_ss("m1"), m1));
  //    REQUIRE(!b.get(zs::_ss("m2"), m2));
  //    REQUIRE(!b.get(zs::_ss("m3"), m3));
  //    REQUIRE(!b.get(zs::_ss("m4"), m4));
  //    REQUIRE(!b.get(zs::_ss("m5"), m5));
  //    REQUIRE(!b.get(zs::_ss("m6"), m6));
  //    REQUIRE(!b.get(zs::_ss("m7"), m7));
  //
  //    REQUIRE(!b.get(zs::_ss("S1"), S1));
  //    REQUIRE(!b.get(zs::_ss("S2"), S2));
  //    REQUIRE(!b.get(zs::_ss("S3"), S3));
  //    REQUIRE(!b.get(zs::_ss("S4"), S4));
  //    REQUIRE(!b.get(zs::_ss("S5"), S5));
  //    REQUIRE(!b.get(zs::_ss("S6"), S6));
  //
  //    REQUIRE(m1 == 1000);
  //    REQUIRE(m2 == 1001);
  //    REQUIRE(m3 == 1002);
  //    REQUIRE(m4 == 1003);
  //    REQUIRE(m5 == 5);
  //    REQUIRE(m6 == 6);
  //    REQUIRE(m7.is_array());
  //    REQUIRE(m7.as_array()[0] == -2);
  //
  //    REQUIRE(S1 == 51);
  //    REQUIRE(S2 == 52);
  //    REQUIRE(S3 == 300);
  //    REQUIRE(S4 == 400);
  //    REQUIRE(S5 == 500);
  //    REQUIRE(S6 == 600);
  //  }
}

ZS_CODE_TEST("streamer.01", R"""(
var lllll = struct();
struct John {
  static var S1 = 32;
  var k = 1;
  var p = 9;
};

 return {
  jjjjjj = John(),
  a = lllll,
  kkds = 21,
  b = "Skjsakjlsa",
  c = [1, 2, 3, 4],
  d = {
    k = [12, 3],
    nd = {}
  }
 };

)""") {

  //  zb::print(value.stream_streamer<zs::object::serializer_type::plain>());
  //  zb::print(value.stream_streamer<zs::object::serializer_type::plain_compact>());
  //  zb::print(value.stream_streamer<zs::object::serializer_type::quoted>());

  auto aa = zs::_ss("alex");
  auto bb = zs::_t(eng, { { zs::_ss("a"), 21 }, { zs::_ss("b"), 22 } });
  //  zb::print(aa.stream_streamer<zs::object::serializer_type::plain>());
  //  zb::print(aa.stream_streamer<zs::object::serializer_type::quoted>());
  //  zb::print(aa.stream_streamer<zs::object::serializer_type::json>());
  //  zb::print(aa.stream_streamer<zs::object::serializer_type::json_compact>());
  //  zb::print(aa.stream_streamer<zs::object::serializer_type::plain_compact>());
  //  zb::print("JKJ");

  //  zb::print(bb.stream_streamer<zs::object::serializer_type::plain>());
  //  zb::print(bb.stream_streamer<zs::object::serializer_type::quoted>());
  //  zb::print(bb.stream_streamer<zs::object::serializer_type::json>());
  //  zb::print(bb.stream_streamer<zs::object::serializer_type::json_compact>());
  //  zb::print(bb.stream_streamer<zs::object::serializer_type::plain_compact>());

  //  auto table = zs::_t(eng);
  //  auto& tbl = table.as_table();
  //  tbl["a"] = 32;
  //  tbl["b"] = 33;
}


//
ZS_CODE_TEST("struct.78", R"""(

struct John {
  // Constructor.
  constructor(var a = null, var b = null, var c = null, var d = null,
    var e = null, var f = null, var g = null) {
    if(a != null) this.m1 = a;
    if(b != null) this.m2 = b;
    if(c != null) this.m3 = c;
    if(d != null) this.m4 = d;
    if(e != null) this.m5 = e;
    if(f != null) this.m6 = f;
    if(g != null) this.m7 = g;
  }

  // Static member (no type).
  static S1 = 100;

  // Static member (var type).
  static var S2 = 200;

  // Static member (int type).
  static int S3 = 300;

  // Const static member (no type).
  static const S4 = 400;

  // Const static member (var type).
  static const var S5 = 500;
  
  // Const static member (int type).
  static const int S6 = 600;

  // Member(no type).
  m1 = 1;

  // Member(var type).
  var m2 = 2;

  // Member(int type).
  int m3 = 3;

  // Const member (no type).
  const m4 = 4;

  // Const member (var type).
  const var m5 = 5;

  // Const member (int type).
  const int m6 = 6;

  var m7 = [1, 2, 3];
};

// It's possible to set a static variable from a stuct.
John.S1 = 51;

var a = John();
a.m7[0] = -1;

var b = John(1000, 1001, 1002, 1003);
b.m7[0] = -2;

// It's possible to set a static variable from a stuct.
b.S2 = 52;

return [a, b, John];
)""") {
  REQUIRE(value.as_array()[0].is_struct_instance());
  REQUIRE(value.as_array()[1].is_struct_instance());
  REQUIRE(value.as_array()[2].is_struct());

  //    zb::print(value.as_array()[0].to_json());
  //    zb::print(value.as_array()[0].convert_to_string());
  // A.
  {
    const zs::struct_instance_object& a = value.as_array()[0].as_struct_instance();

    zs::object m1;
    zs::object m2;
    zs::object m3;
    zs::object m4;
    zs::object m5;
    zs::object m6;
    zs::object m7;

    zs::object S1;
    zs::object S2;
    zs::object S3;
    zs::object S4;
    zs::object S5;
    zs::object S6;

    REQUIRE(!a.get(zs::_ss("m1"), m1));
    REQUIRE(!a.get(zs::_ss("m2"), m2));
    REQUIRE(!a.get(zs::_ss("m3"), m3));
    REQUIRE(!a.get(zs::_ss("m4"), m4));
    REQUIRE(!a.get(zs::_ss("m5"), m5));
    REQUIRE(!a.get(zs::_ss("m6"), m6));
    REQUIRE(!a.get(zs::_ss("m7"), m7));

    REQUIRE(!a.get(zs::_ss("S1"), S1));
    REQUIRE(!a.get(zs::_ss("S2"), S2));
    REQUIRE(!a.get(zs::_ss("S3"), S3));
    REQUIRE(!a.get(zs::_ss("S4"), S4));
    REQUIRE(!a.get(zs::_ss("S5"), S5));
    REQUIRE(!a.get(zs::_ss("S6"), S6));

    REQUIRE(m1 == 1);
    REQUIRE(m2 == 2);
    REQUIRE(m3 == 3);
    REQUIRE(m4 == 4);
    REQUIRE(m5 == 5);
    REQUIRE(m6 == 6);
    REQUIRE(m7.is_array());
    REQUIRE(m7.as_array()[0] == -1);

    REQUIRE(S1 == 51);
    REQUIRE(S2 == 52);
    REQUIRE(S3 == 300);
    REQUIRE(S4 == 400);
    REQUIRE(S5 == 500);
    REQUIRE(S6 == 600);
  }

  // B.
  {
    const zs::struct_instance_object& b = value.as_array()[1].as_struct_instance();

    zs::object m1;
    zs::object m2;
    zs::object m3;
    zs::object m4;
    zs::object m5;
    zs::object m6;
    zs::object m7;

    zs::object S1;
    zs::object S2;
    zs::object S3;
    zs::object S4;
    zs::object S5;
    zs::object S6;

    REQUIRE(!b.get(zs::_ss("m1"), m1));
    REQUIRE(!b.get(zs::_ss("m2"), m2));
    REQUIRE(!b.get(zs::_ss("m3"), m3));
    REQUIRE(!b.get(zs::_ss("m4"), m4));
    REQUIRE(!b.get(zs::_ss("m5"), m5));
    REQUIRE(!b.get(zs::_ss("m6"), m6));
    REQUIRE(!b.get(zs::_ss("m7"), m7));

    REQUIRE(!b.get(zs::_ss("S1"), S1));
    REQUIRE(!b.get(zs::_ss("S2"), S2));
    REQUIRE(!b.get(zs::_ss("S3"), S3));
    REQUIRE(!b.get(zs::_ss("S4"), S4));
    REQUIRE(!b.get(zs::_ss("S5"), S5));
    REQUIRE(!b.get(zs::_ss("S6"), S6));

    REQUIRE(m1 == 1000);
    REQUIRE(m2 == 1001);
    REQUIRE(m3 == 1002);
    REQUIRE(m4 == 1003);
    REQUIRE(m5 == 5);
    REQUIRE(m6 == 6);
    REQUIRE(m7.is_array());
    REQUIRE(m7.as_array()[0] == -2);

    REQUIRE(S1 == 51);
    REQUIRE(S2 == 52);
    REQUIRE(S3 == 300);
    REQUIRE(S4 == 400);
    REQUIRE(S5 == 500);
    REQUIRE(S6 == 600);
  }
}
