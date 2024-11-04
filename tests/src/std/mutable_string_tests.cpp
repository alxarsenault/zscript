#include "unit_tests.h"
#include <zscript/std/zmutable_string.h>

using namespace utest;

// Create an empty mutable string.
ZTEST_CASE("mutable_string", R"""(
return mutable_string();
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(zs::mutable_string::as_mutable_string(value) == "");
  REQUIRE(value.get_string_unchecked() == "");
}

// Create an empty mutable string.
ZTEST_CASE("mutable_string", R"""(
return mutable_string("");
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked() == "");
}

// Create an empty mutable string.
ZTEST_CASE("mutable_string", R"""(
return mutable_string(0);
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked() == "");
}

// Create a mutable string of size N.
ZTEST_CASE("mutable_string", R"""(
return mutable_string(32);
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked().size() == 32);
}

// Create a mutable string from an array should fail.
ZTEST_CASE("mutable_string", compile_good | call_fail, R"""(
return mutable_string([]);
)""") {
  REQUIRE(value.is_null());
}

// Create a mutable string from a string.
ZTEST_CASE("mutable_string", R"""(
return mutable_string("ZScript");
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked() == "ZScript");
}

// Create a mutable string from a mutable string.
ZTEST_CASE("mutable_string", R"""(
var a = mutable_string("ZScript");
return mutable_string(a);
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked() == "ZScript");
}

// Create a mutable string from adding two strings.
ZTEST_CASE("mutable_string", R"""(
return mutable_string("Z" + "Script");
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked() == "ZScript");
}

// Adding a string with a mutable string should return a string.
ZTEST_CASE("mutable_string", R"""(
return "Z" + mutable_string("Script");
)""") {
  REQUIRE(value.is_string());
  REQUIRE(!zs::mutable_string::is_mutable_string(value));
  REQUIRE(value == "ZScript");
}

// Adding a mutable string with a string should return a mutable string.
ZTEST_CASE("mutable_string", R"""(
return mutable_string("Z") + "Script";
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked() == "ZScript");
}

// Adding a mutable string with a mutable string should return a mutable string.
ZTEST_CASE("mutable_string", R"""(
return mutable_string("Z") + mutable_string("Script");
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked() == "ZScript");
}

// Append a string to a mutable string.
ZTEST_CASE("mutable_string", R"""(
var a = mutable_string("Z");
a.append("Script");
return a;
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked() == "ZScript");
}

// Append a mutable string to a mutable string.
ZTEST_CASE("mutable_string", R"""(
var a = mutable_string("Z");
a.append(mutable_string("Script"));
return a;
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(value.get_string_unchecked() == "ZScript");
}

ZTEST_CASE("mutable_string", R"""(
var s = mutable_string("ZScript");
var sc = mutable_string("Sc");
return [
  /* 0 */ s.to_string(),
  /* 1 */ s.size(),
  /* 2 */ s.starts_with(sc),
  /* 3 */ s.starts_with("ZS"),
  /* 4 */ s.ends_with("pt"),
  /* 5 */ s.ends_with("A"),
  /* 6 */ s.contains(sc),
  /* 7 */ s.contains("ip"),
  /* 8 */ s.contains("ipp"),
  /* 9 */ s.is_ascii()
];
)""") {
  REQUIRE(value.is_array());
  zs::array_object& arr = value.as_array();
  REQUIRE(arr.size() == 10);
  REQUIRE(arr[0] == "ZScript");
  REQUIRE(arr[1] == 7);
  REQUIRE(arr[2] == false);
  REQUIRE(arr[3] == true);
  REQUIRE(arr[4] == true);
  REQUIRE(arr[5] == false);
  REQUIRE(arr[6] == true);
  REQUIRE(arr[7] == true);
  REQUIRE(arr[8] == false);
  REQUIRE(arr[9] == true);
}

ZTEST_CASE("mutable_string", R"""(
var a = mutable_string("A");
a.append("B");
a.append('π');
a.append("D", 'E', "FG", '\U0001f34c');
return a;
)""") {
  REQUIRE(zs::mutable_string::is_mutable_string(value));
  REQUIRE(zs::mutable_string::as_mutable_string(value) == "ABπDEFG\U0001f34c");
}

// Iterator tests.
ZTEST_CASE("mutable_string", R"""(
var a = mutable_string("ABπDEF");
var arr = [];
for(auto v : a) {
  arr.push(v);
}

return arr;
)""") {
  REQUIRE(value.is_array());
  zs::array_object& arr = value.as_array();
  REQUIRE(arr.size() == 6);
  REQUIRE(arr[0] == 'A');
  REQUIRE(arr[1] == 'B');
  REQUIRE(arr[2] == u'π');
  REQUIRE(arr[3] == 'D');
  REQUIRE(arr[4] == 'E');
  REQUIRE(arr[5] == 'F');
}

// Iterator tests.
ZTEST_CASE("mutable_string", R"""(
var a = mutable_string("ABπDEF");
var arr = [];
for(auto k, v : a) {
  arr.push(v);
}

return arr;
)""") {
  REQUIRE(value.is_array());
  zs::array_object& arr = value.as_array();
  REQUIRE(arr.size() == 6);
  REQUIRE(arr[0] == 'A');
  REQUIRE(arr[1] == 'B');
  REQUIRE(arr[2] == u'π');
  REQUIRE(arr[3] == 'D');
  REQUIRE(arr[4] == 'E');
  REQUIRE(arr[5] == 'F');
}

// Iterator tests.
ZTEST_CASE("mutable_string", R"""(
var a = mutable_string("ABπDEF");
var arr = [];
for(auto k, v : a) {
  arr.push(k);
}

return arr;
)""") {
  REQUIRE(value.is_array());
  zs::array_object& arr = value.as_array();
  REQUIRE(arr.size() == 6);
  REQUIRE(arr[0] == 0);
  REQUIRE(arr[1] == 1);
  REQUIRE(arr[2] == 2);
  REQUIRE(arr[3] == 3);
  REQUIRE(arr[4] == 4);
  REQUIRE(arr[5] == 5);
}
