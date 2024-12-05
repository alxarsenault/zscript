#include "unit_tests.h"

ZS_CODE_TEST("array_01", R"""(
var a = [];
return a;
)""") {
  REQUIRE(value.is_array());
  REQUIRE(value._array->size() == 0);
}

ZS_CODE_TEST("array_02", R"""(
var a = [1, 2, 3, "A"];
return a;
)""") {
  REQUIRE(vm.top().is_array());
  REQUIRE(vm.top()._array->size() == 4);
  REQUIRE(vm.top().as_array()[0] == 1);
  REQUIRE(vm.top().as_array()[1] == 2);
  REQUIRE(vm.top().as_array()[2] == 3);
  REQUIRE(vm.top().as_array()[3] == "A");
}

ZS_CODE_TEST("array_03", R"""(
var a = [1, 2, 3, "A"];
return a.size();
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 4);
}

ZS_CODE_TEST("array_04", R"""(
var a = [1, 2, 3, "A"];
a.resize(5);
return a.size();
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 5);
}

ZS_CODE_TEST("array_05", R"""(
var a = [1, 2, 3, "A"];
return a.get(2);
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 3);
}

ZS_CODE_TEST("array_06", R"""(
var a = [1, 2, 3];
return a.is_number_array();
)""") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == true);
}

ZS_CODE_TEST("array_07", R"""(
   var a = [1, 2, 3];
   return a.is_number_array();
)""") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == true);
}

ZS_CODE_TEST("array_08", R"""(
   var a = ["a", "b", "c"];
   return a.is_string_array();
)""") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == true);
}

ZS_CODE_TEST("array_09", R"""(
   var a = ["a", "b", "c", 22];
   return a.is_string_array();
)""") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == false);
}

ZS_CODE_TEST("array_10", R"""(
   var a = ["a", "b", "c"];
   a.push("d");
   return a.is_string_array();
)""") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == true);
}

ZS_CODE_TEST("array_11", R"""(
   var a = ["a", "b", "c"];
   a.push(3);
   return a.is_string_array();
)""") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == false);
}

ZS_CODE_TEST("array_12", R"""(
   var a = [1.0, 2.0, 3.0];
   return a.is_float_array();
)""") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == true);
}

ZS_CODE_TEST("array_13", R"""(
   var a = [1, 2, 3];
   return a.is_integer_array();
)""") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == true);
}

ZS_CODE_TEST("array_14", R"""(
   var a = [1, 2, 3];
   a.clear();
   return a.size();
)""") {
  REQUIRE(vm.top().is_integer());
  REQUIRE(vm.top() == 0);
}

ZS_CODE_TEST("array_15", R"""(
   var a = [1, 2, 3];
   a.clear();
   return a.is_empty();
)""") {
  REQUIRE(vm.top().is_bool());
  REQUIRE(vm.top() == true);
}

ZS_CODE_TEST("array_sort_01", R"""(
var arr = [5, 4, 3, 2, 1, 0];
arr.sort();
return arr;
)""") {

  REQUIRE(vm.top() == zs::_a(vm.get_engine(), { 0, 1, 2, 3, 4, 5 }));
}

ZS_CODE_TEST("array_sort_02", R"""(
var utest = import("utest");
utest.info("Array");

var arr = [0, 1, 2, 3, 4, 5];
arr.sort($(a, b) { return a > b; });
return arr;
)""") {

  REQUIRE(value == zs::_a(vm.get_engine(), { 5, 4, 3, 2, 1, 0 }));
}

ZS_CODE_TEST("array_sort_03", R"""(
return [5, 4, 3, 2, 1, 0].sort();
)""") {
  REQUIRE(value == zs::_a(eng, { 0, 1, 2, 3, 4, 5 }));
}

ZS_CODE_TEST("array_sort_04", R"""(
return [5, 4, 3, 2, 1, 0].sort();
)""") {
  REQUIRE(value == zs::_a(eng, { 0, 1, 2, 3, 4, 5 }));
}

ZS_CODE_TEST("for_thjhjkhj", R"""(
var arr = [];
arr.push(89);
arr.push(90);

var v1 = arr[0];
var v2 = arr.get(1);

arr.insert(0, 123);
arr.insert(2, 3, "2");

arr.erase(1);
arr.push(900);
arr.push(901);
arr.pop();
arr.append([5, 6, 7]);
return [arr, v1, v2, arr.size(), arr[-2]];
)""") {
  //  zb::print(vm.top());
}

ZS_CODE_TEST("for_thjhjkhjjkjkjk", R"""(
var utest = import("utest");
utest.info("Array");

var arr = [];
utest.check(arr.is_empty(), __THIS_LINE__);

arr = [1,2,3,4];
utest.check(arr.size() == 4, __THIS_LINE__);
utest.check_eq(arr.length(), 4, __THIS_LINE__);
arr.push(5, 6, 3);

utest.vals.a0 = arr[0];

utest.check_eq(arr[0], 1, __THIS_LINE__);
utest.check_eq(arr[1], 2, __THIS_LINE__);
utest.check_eq(arr[2], 3, __THIS_LINE__);

var a = arr.copy();
var n = arr.erase_if($(e) { return e == 3; });

// Two should have been deleted.
utest.check_eq(n, 2, __THIS_LINE__);

// Size is now 5.
utest.check_eq(arr.size(), 5, __THIS_LINE__);

var aa = [];
aa.append(arr);

arr.pop(2);

a.erase_if(2);

aa.append([9, 10], [11, 21], 88);
aa.sort();
return [arr, n, aa, a];
)""") {
  //  zb::print(vm.top(), vals());
}

ZS_CODE_TEST("assdasdas", R"""(
var a = ["A", "B", "C"];

var concat = "";
for(var it = a.begin(); it != a.end();) {
  concat += (it++).get();
}

var it = a.begin();
var end = a.end();
var v0 = it == end;

var v1 = it.get();
it = it.next();
var v2 = it.get();
++it;
var v3 = it++.get();
var v4 = it == end;

it = a.begin();
var v5 = (it++).get();
var v6 = it.get();


it = a.begin();
var v7 = (++it).get();

it = a.end();
--it;

var v8 = it.get();
var v9 = a.end() - 1;
 return [v0, v1, v2, v3, v4, v5, v6, v7, v8, concat];
)""") {
  REQUIRE(value
      == zs::_a(eng,
          { false, zs::_ss("A"), zs::_ss("B"), zs::_ss("C"), true, zs::_ss("A"), zs::_ss("B"), zs::_ss("B"),
              zs::_ss("C"), zs::_ss("ABC") }));
}

ZS_CODE_TEST("assdjkkjkasnjnkkdas", R"""(
var a = ["A", "B"];
var concat = "";
var concat2 = "";
for(var v = a[0], it = a.begin(); it != a.end(); ++it, v = it.get_if_not(a.end())) {
  concat += v;
  concat2 += it.get();
}

return concat + concat2;
)""") {
  REQUIRE(value == "ABAB");
}

//
ZS_CODE_TEST("for_loop_auto.01", R"""(
var banana = [1, 2, 3, 4, 5, 6];
var sum = 0;
var sum2 = 0;

for(var it = banana.begin(), value = 0; value = it.safe_get(banana); ++it) {
  sum += value;
  sum2 += it.get();
}

return [sum ,sum2];
)""") {
  REQUIRE(value == zs::_a(eng, { 21, 21 }));
}

ZS_CODE_TEST("for_loop_auto.02", R"""(
var banana = [1, 2, 3, 4, 5, 6];
var it = banana.begin();
var value = 0;
var sum = 0;
var sum2 = 0;
for(; value = it.safe_get(banana); ++it) {
  sum += value;
  sum2 += it.get();
}
return [sum ,sum2];
)""") {
  REQUIRE(value == zs::_a(eng, { 21, 21 }));
}

ZS_CODE_TEST("for_loop_auto.03", R"""(
var t = {
  a = ["A", "B"]
};

var concat = "";
for(auto i : t.a) {
  concat += i;
}

return concat;
)""") {
  REQUIRE(value == "AB");
}

ZS_CODE_TEST("for_loop_auto.04", R"""(
var a = [1, 3, "67"];
var concat = "";

for(var<int, string, john> i : a) {
  concat += zs::to_string(i);
}

return concat;
)""") {
  REQUIRE(value == "1367");
}

ZS_CODE_TEST("for_loop_auto.05", R"""(
var a = [1, 3, "67"];
var concat = "";

for(var i : a) {
  concat += zs::to_string(i);
}

return concat;
)""") {
  REQUIRE(value == "1367");
}

ZS_CODE_TEST("for_loop_auto.06", R"""(
var b = ["1", "3", "67"];
var concat = "";

for(string s : b) {
  concat += s;
}
return concat;
)""") {
  REQUIRE(value == "1367");
}

ZS_CODE_TEST("for_loop_auto.07", R"""(
var sum = 0;
for(var i : [1, 2, 3]) {
  sum += i;
}

return sum;
)""") {
  REQUIRE(value == 6);
}

ZS_CODE_TEST("for_loop_auto.08", R"""(
var a = [1, 2, 3, 4, 5];

var output = array(5);
var i = 0;

for(var it = a.begin(); it != a.end(); ++it) {
  output[i++] = it.get_key();
}
return output;
)""") {
  REQUIRE(value == zs::_a(eng, { 0, 1, 2, 3, 4 }));
}

ZS_CODE_TEST("for_loop_auto.09", R"""(
var a = [1, 2, 3, 4, 5];

var output = array(5);
//var i = 0;
for(var it = a.begin(), i = 0; it != a.end(); ++it, i++) {
  output[i] = [it.get_key(), it.get()];
}
return output;
)""") {

  zs::object arr = zs::_a(eng,
      {
          zs::_a(eng, { 0, 1 }), //
          zs::_a(eng, { 1, 2 }), //
          zs::_a(eng, { 2, 3 }), //
          zs::_a(eng, { 3, 4 }), //
          zs::_a(eng, { 4, 5 }) //
      });
  REQUIRE(value == arr);
}

ZS_CODE_TEST("table_iterator.01", R"""(
var a = { a = 2, b = 3};
var arr = [];
for(var k, i : a) {
  arr.push([k, i]);
}

return arr.sort($(a, b) { return a[0] < b[0]; });
)""") {

  REQUIRE(value
      == zs::_a(eng,
          {
              zs::_a(eng, { zs::_ss("a"), 2 }), //
              zs::_a(eng, { zs::_ss("b"), 3 }) //
          }));
}

ZS_CODE_TEST("table_iterator.02", R"""(
var a = { a = 2, b = 3};
var sum = 0;
for(var it : a) {
 sum += it;
}
return sum;
)""") {
  REQUIRE(value == 5);
}

ZS_CODE_TEST("table_iterator.03", R"""(
var a = { a = 2, b = 3};
var arr = [];
for(var it = a.begin() ; it != a.end(); ++it ) {
  arr.push([ it.get(), it.get_key()]);
}

return arr.sort($(a, b) { return a[0] < b[0]; });
)""") {

  REQUIRE(value
      == zs::_a(eng,
          {
              zs::_a(eng, { 2, zs::_ss("a") }), //
              zs::_a(eng, { 3, zs::_ss("b") }) //
          }));
}

ZS_CODE_TEST("table_iterator.04", R"""(
var a = { a = 2, b = 3};
var sum = 0;
for(var k, it in a) {
 sum += it;
}
return sum;
)""") {
  REQUIRE(value == 5);
}

ZS_CODE_TEST("table_iterator.05", R"""(
return { a = 2, b = 3}.to_array().sort($(a, b) { return a[0] < b[0]; });;
)""") {
  REQUIRE(value
      == zs::_a(eng,
          {
              zs::_a(eng, { zs::_ss("a"), 2 }), //
              zs::_a(eng, { zs::_ss("b"), 3 }) //
          }));
}

ZS_CODE_TEST("table_iterator.06", R"""(
return { a = 2, b = 3}.to_sorted_array();
)""") {

  REQUIRE(value
      == zs::_a(eng,
          {
              zs::_a(eng, { zs::_ss("a"), 2 }), //
              zs::_a(eng, { zs::_ss("b"), 3 }) //
          }));
}

ZS_CODE_TEST("table_iterator.07", R"""(
return { a = 2, b = 3}.to_sorted_array($(a, b) { return a[0] < b[0]; });
)""") {

  REQUIRE(value
      == zs::_a(eng,
          {
              zs::_a(eng, { zs::_ss("a"), 2 }), //
              zs::_a(eng, { zs::_ss("b"), 3 }) //
          }));
}

ZS_CODE_TEST("table_iterator.08", R"""(
return { a = 2, b = 3}.to_pairs_array().sort($(a, b) { return a.key < b.key; });;
)""") {
  REQUIRE(value
      == zs::_a(eng,
          {
              zs::_t(eng, { { zs::_ss("value"), 2 }, { zs::_ss("key"), zs::_ss("a") } }), //
              zs::_t(eng, { { zs::_ss("value"), 3 }, { zs::_ss("key"), zs::_ss("b") } }), //
          }));
}

ZS_CODE_TEST("table_iterator.09", R"""(
return { a = 2, b = 3}.to_sorted_pairs_array();
)""") {
  REQUIRE(value
      == zs::_a(eng,
          {
              zs::_t(eng, { { zs::_ss("value"), 2 }, { zs::_ss("key"), zs::_ss("a") } }), //
              zs::_t(eng, { { zs::_ss("value"), 3 }, { zs::_ss("key"), zs::_ss("b") } }), //
          }));
}

ZS_CODE_TEST("table_iterator.10", R"""(
return { a = 2, b = 3 }.to_sorted_pairs_array($(a, b) { return a.key >= b.key; });
)""") {
  REQUIRE(value
      == zs::_a(eng,
          {
              zs::_t(eng, { { zs::_ss("value"), 3 }, { zs::_ss("key"), zs::_ss("b") } }), //
              zs::_t(eng, { { zs::_ss("value"), 2 }, { zs::_ss("key"), zs::_ss("a") } }), //
          }));
}
