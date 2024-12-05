#include <ztest/ztest.h>
#include "zvirtual_machine.h"

namespace zs {

std::string rewrite_code(std::string_view code) {
  std::string s(code);

  return "";
}

int_t check_impl(zs::vm_ref vm) {
  int_t nargs = vm.stack_size();

  for (int_t i = 1; i < nargs; i++) {
    if (!vm[i].is_if_true()) {
      return vm.push_bool(false);
    }
  }

  return vm.push_bool(true);
}

int_t expect_impl(zs::vm_ref vm) {
  int_t nargs = vm.stack_size();

  for (int_t i = 1; i < nargs; i++) {
    if (!vm[i].is_if_true()) {
      vm->set_error("EXPECT failed: ", vm[i]);
      return -1;
    }
  }

  return vm.push_bool(true);
}

zs::object create_ztest_lib(zs::virtual_machine* vm) {
  zs::engine* eng = vm->get_engine();

  zs::object test_module = zs::_t(eng);
  zs::table_object& tbl = test_module.as_table();

  tbl["check"] = check_impl;
  tbl["EXPECT"] = expect_impl;
  return test_module;
}

zs::error_result import_ztest_lib(zs::vm_ref vm) {
  zs::object& module_cache = vm->get_imported_modules();
  module_cache.as_table()["ztest"] = create_ztest_lib(vm.get_virtual_machine());

  zs::table_object& root = vm->get_root().as_table();

  for (auto it : module_cache.as_table()["ztest"].as_table()) {
    root[it.first] = it.second;
  }

  return {};
}
} // namespace zs.
