
#include "unit_tests.h"

TEST_CASE("zs::native_array_01") {
  zs::engine eng;

  zs::object obj = zs::object::create_native_array<float>(&eng, 2);
  REQUIRE(obj.is_native_array());
  REQUIRE(obj._na_type == zs::native_array_type::n_float);

  auto& arr = obj.as_native_array<float>();

  arr[0] = 2.3f;
  arr[1] = 2.4f;
  REQUIRE(arr[0] == 2.3f);
  REQUIRE(arr[1] == 2.4f);

  auto& arr2 = obj.as_native_array<zs::native_array_type::n_float>();
  REQUIRE(arr2[0] == 2.3f);
  REQUIRE(arr2[1] == 2.4f);
}
