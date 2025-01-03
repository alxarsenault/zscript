#include "unit_tests.h"

#include <zscript/base/strings/parse_utils.h>

ZTEST_CASE("delegate", R"""(
var d = { a = 32 };
var t = zs::set_delegate({}, d);
return t.a;
)""") {
  REQUIRE(value == 32);
}

ZTEST_CASE("delegate", R"""(
var d = { a = 32 };
var t = zs::set_delegate({}, d);
 
d.a = 55;
return t.a;
)""") {
  REQUIRE(value == 55);
}

ZTEST_CASE("delegate", R"""(
var d = { a = 32 };
var t = zs::set_delegate({}, d);
 
t.a = 55;
return t.a;
)""") {
  REQUIRE(value == 55);
}

ZTEST_CASE("delegate", R"""(
var d = { a = 32 };
var t = zs::set_delegate({}, d);
 
t.a = 55;
return d.a;
)""") {
  REQUIRE(value == 55);
}

ZTEST_CASE("delegate", R"""(
var d = { a = 32 };
var t = zs::set_delegate({}, d);
 
t.a = 55;
var k1 = zs::contains(t, "a");
//io::print(k1);
// Remove the delegate.
//zs::set_delegate(t, null);

//bool k2 = zs::contains(t, "a");
//t.a = 89;

//return [k1, k2, t.a, d.a];
)""") {
  //  REQUIRE(value.as_array()[0] == true);
  //  REQUIRE(value.as_array()[1] == false);
  //  REQUIRE(value.as_array()[2] == 89);
  //  REQUIRE(value.as_array()[3] == 55);
}

ZTEST_CASE("delegate", R"""(
var d2 = { c = 125 };
var d1 = zs::set_delegate({ a = 32 }, d2);
var t = zs::set_delegate({}, d1);
return [d2.c, d1.c, t.c];
)""") {
  REQUIRE(value.as_array()[0] == 125);
  REQUIRE(value.as_array()[1] == 125);
  REQUIRE(value.as_array()[2] == 125);
}

ZTEST_CASE("delegate", R"""(
var d2 = { c = 125 };
var d1 = zs::set_delegate({ a = 32 }, d2);
var t = zs::set_delegate({}, d1);
t.c = 130;
return [d2.c, d1.c, t.c];
)""") {
  REQUIRE(value.as_array()[0] == 130);
  REQUIRE(value.as_array()[1] == 130);
  REQUIRE(value.as_array()[2] == 130);
}

ZTEST_CASE("delegate", R"""(
var t = {
  __get = function(key, delegate) {
    return "A";
  }
};

var k = t.john;

return k;
)""") {
  REQUIRE(value == "A");
}

ZTEST_CASE("delegate", R"""(
var t = {
  a = 55,
  __get = function(key, delegate) {
    return "A";
  }
};

var k = t.a;

return k;
)""") {
  REQUIRE(value == 55);
}

ZTEST_CASE("delegate", R"""(
var t = {
  _stuff = { Z = 89},
  __get = function(key, delegate) {
  
    if(key == "a") {
      return this._stuff.Z;
    }

    return "A";
  }
};
 
return [t.a, t.b];
)""") {
  REQUIRE(value.as_array()[0] == 89);
  REQUIRE(value.as_array()[1] == "A");
}

ZTEST_CASE("delegate", R"""(
var t = {
  _stuff = { a = 89},
  __get = function(key, delegate) {
    return this._stuff[key];
  }
};
 
return t.a;
)""") {
  REQUIRE(value == 89);
}

ZTEST_CASE("delegate", R"""(
var d = {
  __get = function(key, delegate) {
    if(key == "a") {
      return 52;
    }

    return 342;
  }
};

var t = zs::set_delegate({}, d);
 
return [t.a, t.b];
)""") {
  REQUIRE(value.as_array()[0] == 52);
  REQUIRE(value.as_array()[1] == 342);
}

ZTEST_CASE("delegate", R"""(
var d = {
  __get = function(key, delegate) {
    if(key == "a") {
      return 52;
    }

    return 342;
  }
};

var t = zs::set_delegate({}, d);
 

t.a = 89;
return t.a;
)""") {
  REQUIRE(value == 89);
  //  REQUIRE(value.as_array()[1] == 342);
}

ZTEST_CASE("delegate", R"""(
var d = {
  __get = function(key, delegate) {
    if(key == "b") {
      return 125;
    }

    return 342;
  }
};

var t = zs::set_delegate({
  __get = function(key, delegate) {
    if(key == "a") {
      return 52;
    }
 
    zs::error(zs::error_code.inaccessible);
  }
}, d);
  
return t.a;
)""") {
  REQUIRE(value == 52);
}

ZTEST_CASE("delegate", R"""(
var d = {
  __get = function(key, delegate) {
    if(key == "b") {
      return 125;
    }

    return 342;
  }
};

var t = zs::set_delegate({
  __get = function(key, delegate) {
    if(key == "a") {
      return 52;
    }
 
    zs::error(zs::error_code.inaccessible);
  }
}, d);
  
return t.b;
)""",
    utest::call_fail) {
  //  REQUIRE(value == 342);
}

ZTEST_CASE("delegate", R"""(
var d = {
  __get = function(key, delegate) {
    if(key == "b") {
      return 1234;
    }

    return 342;
  }
};

var t = zs::set_delegate({
  __get = function(key, delegate) {
    if(key == "a") {
      return 52;
    }

    return none;
  }
}, d);
 
return [t.a, t.b, t.c];
)""") {
  REQUIRE(value.as_array()[0] == 52);
  REQUIRE(value.as_array()[1] == 1234);
  REQUIRE(value.as_array()[2] == 342);
}

//
// ZTEST_CASE("delegate", R"""(
// var t1 = { a = 32};
//
// var d1 = { b = 51};
//
// zs::set_delegate(t1, d1);
//
// var dd1 = zs::get_delegate(t1);
// io::print(dd1);
//
// io::print(zs::get_addr(d1), zs::get_addr(dd1));
// return 32;
//)""") {
//  //  zb::print(get_test_name(), "MULEQ", value);
//  //  zs::float_t norm = std::sqrt(1.0 + 2.0 * 2.0 + 3.0 * 3.0);
//}

ZTEST_CASE("delegate", R"""(
var t1 = { a = 32};

var d1 = { b = 51};

zs::set_delegate(t1, d1);

var dd1 = zs::get_delegate(t1);
//io::print(dd1);
 


var dd2 = zs::set_delegate({}, dd1);
//io::print(dd2);


//io::print(zs::get_addr(d1), zs::get_addr(dd1), zs::get_addr(dd2));

return 32;
)""") {
  //  zb::print(get_test_name(), "MULEQ", value);
  //  zs::float_t norm = std::sqrt(1.0 + 2.0 * 2.0 + 3.0 * 3.0);
}

#define META_GET_TEST_CASE_CODE \
  R"""(


var k = {z = 123};

var d = {
 
__get = function(key, delegate) {
  if(key == "b") {
    return 125;
  }

  return k[key];
// zs::error(zs::error_code.inaccessible);
}
};

var t = zs::set_delegate({bnm=890, __get = function(key, delegate) { return key == "a" ? 52 : none; }}, d);
)"""

ZTEST_CASE("delegate-meta-get", META_GET_TEST_CASE_CODE "return t.a;") { REQUIRE(value == 52); }
ZTEST_CASE("delegate-meta-get", META_GET_TEST_CASE_CODE "return t.b;") { REQUIRE(value == 125); }
ZTEST_CASE("delegate-meta-get", META_GET_TEST_CASE_CODE "return t.z;") { REQUIRE(value == 123); }

ZTEST_CASE("delegate-meta-get", META_GET_TEST_CASE_CODE "return t.c;", utest::call_fail) {}

ZTEST_CASE("delegate_get", R"""(
var ccc = mutable_string("ABCD");
var b = ccc[2];
return b;
)""") {
  //  zb::print((char)(zs::int_t)value._int);
  REQUIRE(value._int == 'C');
}

ZTEST_CASE("Alexandre", R"""(
var a = [1, 2, 3];
var t = {[1] = 123, size = function() {return 32;}};
zs::set_delegate(a, t);
return zs::call(zs::raw_get(t, "size"), t);
)""") {
  // zb::print(value);//, );
  REQUIRE(value == 32);
}

ZTEST_CASE("Alexandre2232", R"""(
var a =  mutable_string("Alex");

var delegate = {
  __get = function(key, d) {
    if(typeof(key) == "integer") {
      return zs::raw_get(this, key);
    }

    return none;
  }
};

zs::set_delegate(a, delegate);

//io::print(a[0], 'A');
//io::print(a.size());


return a;
)""") {

  //  REQUIRE(value =="Alex");
}

ZTEST_CASE("Alexandre2", R"""(
var a =  mutable_string("Alex");

var t = zs::set_delegate({}, none);
//io::print(t.size());

var delegate = {
  __get = t
};

zs::set_delegate(a, delegate);

//io::print(a[0], 'A');
//io::print(a.size());


return a;
)""",
    [](zs::vm_ref vm) {
      //  zb::print()
    }) {

  //  REQUIRE(value =="Alex");
}
//-- create private index
//    local index = {}
//
//    -- create metatable
//    local mt = {
//      __index = function (t,k)
//        print("*access to element " .. tostring(k))
//        return t[index][k]   -- access the original table
//      end,
//
//      __newindex = function (t,k,v)
//        print("*update of element " .. tostring(k) ..
//                             " to " .. tostring(v))
//        t[index][k] = v   -- update original table
//      end
//    }
//
//    function track (t)
//      local proxy = {}
//      proxy[index] = t
//      setmetatable(proxy, mt)
//      return proxy
//    end

ZTEST_CASE("dsadsadsa", R"""(


const property_getter_key = {}

const tbl_default = zs::get_table_default_delegate();

var proxy_metatable = zs::set_delegate({
  __get = function(key, d) {
//    io::print("*access to element", key);

    
    if(zs::contains(tbl_default, key)) {
        return (...) => {
          const tbl = this[property_getter_key];
          const fct = tbl_default[key];
          if(vargv) {
            return zs::apply(fct, tbl, vargv);
          }
  
          return zs::apply(fct, tbl);
        };
    }
    // Access the original table.
    return this[property_getter_key][key];
  }
}, none);

function track(t) {
  zs::set_delegate(t, none, false);

  return zs::set_delegate({
    [property_getter_key] = t
  }, proxy_metatable);
}


var gdel = {
  __get = function(key, d) {
    if(zs::is_one_of(key, "a", "b", "c")) {
      return zs::raw_get(this, key);
    }

    return null;
  }
};

var t = zs::set_delegate({
  a = 32,
  b = 55,
  c = 12,
  d = 123
}, gdel);

var gg = track(t);

var k = gg.size();
return k;

//var gg = track({
//  a = 32,
//  b = 55,
//  c = 12
//});

//io::print(zs::contains(zs::get_table_default_delegate(), "emplace"));
//var kk = gg.size();
//
//function abc(a, ...) {
//  io::print(a,vargv);
//}
//
//zs::apply(abc, this, [1, 2, 3, 4]);
//abc(1, 2, 3, 4);
//return 32;
)""",
    [](zs::vm_ref vm) {

    }) {
  //  zb::print(value);
  //  REQUIRE(value ==55);
}
