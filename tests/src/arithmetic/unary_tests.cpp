#include "unit_tests.h"

using namespace utest;

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
  REQUIRE(value == zs::_a(vm, { 11, 11 }));
}

ZTEST_CASE("unary_arithmetic", R"""(
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

ZTEST_CASE("unary_arithmetic", R"""(
var a = 2;
return ~a;
)""") {
  REQUIRE(value == ~2);
}

ZTEST_CASE("unary_arithmetic", R"""(
var a = -2;
return ~a;
)""") {
  REQUIRE(value == ~(-2));
}

ZTEST_CASE("unary_arithmetic", R"""(
var t = {
  __bw_not = function(delegate) {
    return 55;
  }
};

return ~t;
)""") {
  REQUIRE(value == 55);
}

ZTEST_CASE("unary_arithmetic", R"""(
var t = {
  __unm = function(delegate) {
    return 100;
  }
};

return -t;
)""") {
  REQUIRE(value == 100);
}

ZTEST_CASE("unary_arithmetic", R"""(
//var my_type;

var my_type = {
  function new(v) {
    return zs::set_delegate({value = v}, my_type);
  }

  operator typeof = "john"

  function operator+(rhs, d) {
    if(typeof(rhs) == "john") {
      return new(this.value + rhs.value);
    }
    else if(zs::is_number(rhs)) {
      return new(this.value + rhs);
    }
    else {
      return zs::error("Invalid operation");
    }
  }

  function operator+=(rhs, d) {
    if(typeof(rhs) == "john") {
      this.value += rhs.value;
    }
    else if(zs::is_number(rhs)) {
      this.value += rhs;
    }
    else {
      return zs::error("Invalid operation");
    }

    return this;
  }

  function operator<<(rhs, d) {
    if(typeof(rhs) == "john") {
      return new(this.value << rhs.value);
    }
    else if(zs::is_number(rhs)) {
      return new(this.value << rhs);
    }

    return zs::error("Invalid operation");
  }

  function operator<<=(rhs, d) {
    if(typeof(rhs) == "john") {
      this.value <<= rhs.value;
    }
    else if(zs::is_number(rhs)) {
      this.value <<= rhs;
    }
    else {
      return zs::error("Invalid operation");
    }

    return this;
  }

  function operator|(rhs, d) {
    if(typeof(rhs) == "john") {
      return new(this.value | rhs.value);
    }
    else if(zs::is_number(rhs)) {
      return new(this.value | rhs);
    }

    return zs::error("Invalid operation");
  }
};

var t1 = my_type.new(55);
var t2 = my_type.new(56);

var k1 = t1 + 32;
var k2 = t1 + t2;

var t3 = my_type.new(57);
t3 += 3;

var t4 = my_type.new(1);
t4 <<= 2;

var t5 = t4 << 2;

return [k1.value, k2.value, t3.value, t4.value, t5.value, (t5 | 32).value];
)""") {
  zs::int_t t4 = 1 << 2;
  zs::int_t t5 = t4 << 2;
  REQUIRE(value == zs::_a(vm, { 32 + 55, 55 + 56, 57 + 3, t4, t5, t5 | 32 }));
}
