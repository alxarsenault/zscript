#include "zglobal_table_delegate.h"
#include "zvirtual_machine.h"
#include "utility/zparameter_stream.h"
#include "utility/zvm_module.h"
#include <zscript/std/zmutable_string.h>
#include <zscript/std/zfloat_array.h>

namespace zs {
namespace {
  int_t global_table_size_impl(zs::vm_ref vm) noexcept {

    const int_t nargs = vm.stack_size();
    if (nargs != 1) {
      vm->handle_error(errc::invalid_parameter_count, { -1, -1 }, ZS_DEVELOPER_SOURCE_LOCATION(),
          "Invalid parameter count (", nargs, ") in table.size(), expected 1.\n");
      return -1;
    }

    zs::parameter_stream ps(vm);

    const table_object* tbl_obj = nullptr;
    if (auto err = ps.optional<table_parameter>(tbl_obj)) {
      vm->handle_error(errc::not_a_table, { -1, -1 }, ZS_DEVELOPER_SOURCE_LOCATION(),
          "Invalid table parameter in table.size().\n");
      return -1;
    }

    return vm.push(tbl_obj->size());
  }

  int_t global_table_is_empty_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    const object& obj = vm->top();
    table_object* tbl = obj._table;
    return vm.push(tbl->get_map().empty());
  }

  int_t global_table_contains_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (nargs != 2) {
      zb::print("Error: table.table_contains_impl (a, b)");
      return -1;
    }

    const object& obj = vm[0];
    return vm.push(obj.as_table().contains(vm[1]));
  }

  int_t global_table_optset_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 3) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    object& obj = vm[0];
    const object& key = vm[1];
    const object& value = vm[2];

    if (auto err = obj.as_table().set_no_replace(key, value); err and err != error_code::already_exists) {
      return -1;
    }

    return vm.push(obj.as_table()[key]);
  }

  int_t global_table_import_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm);
    const int_t nargs = vm.stack_size();

    ++ps;

    std::string_view name;
    ZS_RETURN_IF_ERROR(ps.require<string_parameter>(name), -1);

    object output_module;
    if (auto err = zs::import_module(vm, zs::_s(vm, name), output_module)) {
      vm->set_error("Can't import module ", name, ".\n");
      return -1;
    }

    return vm.push(output_module);
  }

  int_t global_table_to_string_impl(zs::vm_ref vm) {
    const object& val = vm[1];
    if (val.is_string()) {
      return vm.push(val);
    }

    zs::string s((zs::allocator<char>(vm.get_engine())));

    if (auto err = val.convert_to_string(s)) {
      vm.set_error("Invalid string convertion");
      return -1;
    }

    return vm.push(zs::_s(vm.get_engine(), s));
  }

  int_t global_table_to_int_impl(zs::vm_ref vm) {
    const object& val = vm[1];
    if (val.is_integer()) {
      return vm.push(val);
    }

    int_t v = 0;
    if (auto err = val.convert_to_integer(v)) {
      vm.set_error("Invalid integer convertion");
      return -1;
    }

    return vm.push(v);
  }

  int_t global_table_to_float_impl(zs::vm_ref vm) {
    const object& val = vm[1];
    if (val.is_float()) {
      return vm.push(val);
    }

    float_t v = 0;
    if (auto err = val.convert_to_float(v)) {
      vm.set_error("Invalid float convertion");
      return -1;
    }

    return vm.push(v);
  }

  int_t global_table_create_array_impl(zs::vm_ref vm) {
    int_t nargs = vm.stack_size();

    if (nargs == 1) {
      return vm.push(zs::_a(vm, 0));
    }

    const object& arg1 = vm[1];

    if (!arg1.is_integer()) {
      return -1;
    }

    return vm.push(zs::_a(vm, arg1._int));
  }

  int_t global_table_create_struct_impl(zs::vm_ref vm) {
    return vm.push(zs::object::create_struct(vm.get_engine()));
  }
} // namespace

zs::object create_global_table_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  table_object& tbl = obj.as_table();
  tbl.reserve(20);

  tbl.emplace(_ss("import"), global_table_import_impl);
  tbl.emplace(_ss("__tostring"), global_table_to_string_impl);
  tbl.emplace(_ss("__toint"), global_table_to_int_impl);
  tbl.emplace(_ss("__tofloat"), global_table_to_float_impl);
  tbl.emplace(_ss("__create_array"), global_table_create_array_impl);

  tbl.emplace(_ss("mutable_string"), zs::vm_create_mutable_string);
  tbl.emplace(_ss("np"), zs::create_float_array_lib(eng));
  tbl.emplace(_ss("float_array"), zs::vm_create_float_array);

  tbl.emplace(_ss("size"), global_table_size_impl);
  tbl.emplace(_ss("is_empty"), global_table_is_empty_impl);
  tbl.emplace(_ss("contains"), global_table_contains_impl);
  tbl.emplace(_ss("emplace"), global_table_optset_impl);

  tbl.emplace(zs::_ss("set"), [](zs::vm_ref vm) -> int_t {
    const int_t nargs = vm.stack_size();
    if (nargs != 3) {
      zb::print("Error: table_set_impl");
      return -1;
    }

    object& obj = vm[0];
    const object& key = vm[1];
    const object& value = vm[2];

    if (auto err = obj.as_table().set(key, value)) {
      return -1;
    }

    return vm.push(obj.as_table()[key]);
  });

  tbl.emplace(zs::constants::get<meta_method::mt_set>(), [](zs::vm_ref vm) -> int_t {
    // vm[0] should be the global table.
    // vm[1] should be the key.
    // vm[2] should be the value.
    // vm[3] should be the delegate.

    if (vm.stack_size() != 4) {
      return -1;
    }

    const zs::object& table = vm[0];
    const zs::object& key = vm[1];
    const object& value = vm[2];

    return -1;
  });

  tbl.set_delegate(zs::object::create_none());
  tbl.set_use_default_delegate(false);

  return obj;
}
} // namespace zs.
