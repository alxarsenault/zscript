#include "unit_tests.h"

using namespace utest;

UTEST_CASE("native_pfunction") {
  zs::vm vm;

  zs::var obj([](zs::vm_ref vm, zs::parameter_list params) { return zs::_ss("John"); });
  REQUIRE(obj.is_function());
  REQUIRE(obj.is_native_pfunction());

  REQUIRE(obj.as_native_pfunction()(vm, {}) == "John");
  REQUIRE((obj._npfct)(vm, {}) == "John");
}

UTEST_CASE("native_pfunction") {
  zs::vm vm;
  zs::var obj = +[](zs::vm_ref vm, zs::parameter_list params) -> zs::var {
    return params.get_normal_arguments().size();
  };

  REQUIRE(obj.get_type() == zs::var_type::k_native_pfunction);
  REQUIRE((*obj._npfct)(vm, { vm->get_root(), 1, 2, 3 }) == 3);
  REQUIRE((*obj._npfct)(vm, { vm->get_root() }) == 0);
}
