#include "ztests.h"

ZS_CODE_TEST("native_array_01", R"""(
var na = import("na");
return na.type;
)""") {
  REQUIRE(value.is_enum());
}

ZS_CODE_TEST("native_array_02", R"""(
var na = import("na");
return [na.type.int32, na.type.float32];
)""") {
  //  zb::print(value);
  //  REQUIRE(value.is_color());
}

ZS_CODE_TEST("native_array_03", R"""(
var na = import("na");
var a = na.array(32);
return a;
)""") {
  REQUIRE(value.is_native_array());
}

ZS_CODE_TEST("native_array_04", R"""(
var na = import("na");
var a = na.array(8, na.type.float32);
return a;
)""") {
  REQUIRE(value.is_native_array());

  REQUIRE(value._na_type == zs::native_array_type::n_float);
  auto& arr = value.as_native_array<float>();
  REQUIRE(arr.size() == 8);

  //  arr[0] = 2.3f;
  //  arr[1] = 2.4f;
  //  REQUIRE(arr[0] == 2.3f);
  //  REQUIRE(arr[1] == 2.4f);
  //
  //  auto& arr2 = obj.as_native_array<zs::native_array_type::n_float>();
  //  REQUIRE(arr2[0] == 2.3f);
  //  REQUIRE(arr2[1] == 2.4f);
}

ZS_CODE_TEST("native_array_05", R"""(
var na = import("na");
var a = na.array(8, na.type.int32);
return a;
)""") {
  REQUIRE(value.is_native_array());

  REQUIRE(value._na_type == zs::native_array_type::n_int32);
  //
}

ZS_CODE_TEST("native_array_06", R"""(
var na = import("na");
var a = na.array(8, na.type.float32);
a[0] = 32.2;
return [a.size(), a[0], a[1]];
)""") {
  //  zb::print(value);
  //  REQUIRE(value.is_native_array());
  //  REQUIRE(value._na_type == zs::native_array_type::n_float);
}
//
// ZS_CODE_TEST("fdsdsadas", R"""(
// var a = #as_table("/Users/alexarse/Develop/wp/projects/zscript/doc/content.zs");
// return a;
//)""") {
//  zb::print(value.to_json());
//
//  std::ofstream file;
//  file.open("/Users/alexarse/Develop/wp/projects/zscript/docs/data/sections.js");
//  REQUIRE(file.is_open());
//
//  file << "const documentation_content =";
//  file << value.to_json();
//  file.close();
//}

// TEST_CASE("dsadsadsadsad") {
//
//   static constexpr std::string_view code_01 = R"""(
// var na = import("na");
//
// function create_matrix(n) {
//  var arr = array(n);
//  for(var i = 0; i < n; i++) {
//    arr[i] = na.array(n, na.type.f32);
//  }
//
//  return arr;
//}
//
// function matgen(n, seed) {
//  var y = create_matrix(n);
//  var tmp = seed / n / n;
//  for (var i = 0; i < n; i++) {
//    for (var j = 0; j < n; j++) {
//      y[i][j] = tmp * (i - j) * (i + j);
//    }
//  }
//
//  return y;
//}
//
// function matrix_transpose(a) {
//  var n = a.size();
//  var y = create_matrix(n);
//  for (var i = 0; i < n; i++) {
//    for (var j = 0; j < n; j++) {
//      y[i][j] = a[j][i];
//    }
//  }
//  return y;
//}
//
// function matrix_mul(a, b) {
//  var n = a.size();
//  var y = create_matrix(n);
//  var c = matrix_transpose(b);
//  for (var i = 0; i < n; i++) {
//    for (var j = 0; j < n; j++) {
//      var sum = 0;
//      for (var k = 0; k < n; k++) {
//        sum = sum + a[i][k] * c[j][k];
//      }
//      y[i][j] = sum;
//    }
//  }
//  return y;
//}
//
// var a = create_matrix(2);
// a[0][0] = 1;
// a[0][1] = 2;
// a[1][0] = 3;
// a[1][1] = 4;
//
// var b = create_matrix(2);
// b[0][0] = 5;
// b[0][1] = 6;
// b[1][0] = 7;
// b[1][1] = 8;
//
// return matrix_mul(a, b);
//)""";
//
//
//   static constexpr std::string_view code_02 = R"""(
// function create_matrix(n) {
//  var arr = array(n);
//  for(var i = 0; i < n; i++) {
//    arr[i] = array(n);
//  }
//
//  return arr;
//}
//
// function matgen(n, seed) {
//  var y = create_matrix(n);
//  var tmp = seed / n / n;
//  for (var i = 0; i < n; i++) {
//    for (var j = 0; j < n; j++) {
//      y[i][j] = tmp * (i - j) * (i + j);
//    }
//  }
//
//  return y;
//}
//
// function matrix_transpose(a) {
//  var n = a.size();
//  var y = create_matrix(n);
//  for (var i = 0; i < n; i++) {
//    for (var j = 0; j < n; j++) {
//      y[i][j] = a[j][i];
//    }
//  }
//
//  return y;
//}
//
// function matrix_mul(a, b) {
//  var n = a.size();
//  var y = create_matrix(n);
//  var c = matrix_transpose(b);
//  var sum = 0;
//
//  for (var i = 0; i < n; i++) {
//    for (var j = 0; j < n; j++) {
//      sum = 0;
//      for (var k = 0; k < n; k++) {
//        sum += a[i][k] * c[j][k];
//      }
//      y[i][j] = sum;
//    }
//  }
//  return y;
//}
//
// var a = create_matrix(2);
// a[0][0] = 1;
// a[0][1] = 2;
// a[1][0] = 3;
// a[1][1] = 4;
//
// var b = create_matrix(2);
// b[0][0] = 5;
// b[0][1] = 6;
// b[1][0] = 7;
// b[1][1] = 8;
//
// return matrix_mul(a, b);
//)""";
//
//   zs::vm vm;
//   zs::object_ptr closure_01;
//   zs::object_ptr closure_02;
//
//   if (auto err = vm->compile_buffer(code_01, "test", closure_01)) {
//     REQUIRE(false);
//     return;
//   }
//
//   if (auto err = vm->compile_buffer(code_02, "test", closure_02)) {
//     REQUIRE(false);
//     return;
//   }
//
//   REQUIRE(closure_01.is_closure());
//   REQUIRE(closure_02.is_closure());
//
//
//   BENCHMARK("code_01") {
//     zs::object_ptr value;
//     zs::int_t n_params = 1;
//     vm.push_root();
//
//     if (auto err = vm->call(closure_01, n_params, vm.stack_size() - n_params, value)) {
//       //
//       REQUIRE(false);
//       return;
//     }
//
////     zb::print("VALUE", value );
//   };
//
//   BENCHMARK("code_02") {
//     zs::object_ptr value;
//     zs::int_t n_params = 1;
//     vm.push_root();
//
//     if (auto err = vm->call(closure_02, n_params, vm.stack_size() - n_params, value)) {
//       REQUIRE(false);
//       return;
//     }
//
////     zb::print("VALUE", value );
//   };
// }
