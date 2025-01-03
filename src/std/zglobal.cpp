#include "std/zglobal.h"
#include "zvirtual_machine.h"
#include "utility/zparameter_stream.h"
#include "utility/zvm_module.h"
#include <zscript/std/zmutable_string.h>
#include <zscript/std/zfloat_array.h>

#include "object/delegate/znumber_delegate.h"
#include "object/delegate/zfunction_delegate.h"
#include "object/delegate/zarray_delegate.h"
#include "object/delegate/ztable_delegate.h"
#include "object/delegate/zstring_delegate.h"
#include "object/delegate/zstruct_delegate.h"

namespace zs {
using namespace zs::literals;

namespace {
  inline constexpr object k_global_table_is_struct_instance_name = zs::_sv("is_struct_instance");

  int_t global_table_delegate_size_impl(zs::vm_ref vm) noexcept {

    const int_t nargs = vm.stack_size();
    if (nargs != 1) {
      vm->handle_error(errc::invalid_parameter_count, { -1, -1 }, zb::source_location::current(),
          "Invalid parameter count (", nargs, ") in table.size(), expected 1.\n");
      return -1;
    }

    zs::parameter_stream ps(vm);

    const table_object* tbl_obj = nullptr;
    if (auto err = ps.optional<table_parameter>(tbl_obj)) {
      vm->handle_error(errc::not_a_table, { -1, -1 }, zb::source_location::current(),
          "Invalid table parameter in table.size().\n");
      return -1;
    }

    return vm.push(tbl_obj->size());
  }

  int_t global_table_delegate_is_empty_impl(zs::vm_ref vm) noexcept {
    const int_t count = vm.stack_size();
    if (count != 1) {
      zb::print("Error: math.zmath_random_normal (a, b)");
      return -1;
    }

    const object& obj = vm->top();
    table_object* tbl = obj._table;
    return vm.push(tbl->get_map().empty());
  }

  int_t global_table_delegate_contains_impl(zs::vm_ref vm) noexcept {
    const int_t nargs = vm.stack_size();
    if (nargs != 2) {
      zb::print("Error: table.table_contains_impl (a, b)");
      return -1;
    }

    const object& obj = vm[0];
    return vm.push(obj.as_table().contains(vm[1]));
  }

  int_t global_table_delegate_optset_impl(zs::vm_ref vm) noexcept {
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

  int_t global_table_delegate_create_array_impl(zs::vm_ref vm) {
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

  //  int_t global_table_delegate_create_struct_impl(zs::vm_ref vm) {
  //    return vm.push(zs::object::create_struct(vm.get_engine()));
  //  }

  int_t global_table_delegate_set_impl(zs::vm_ref vm) {
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
  }

  int_t global_table_delegate_meta_set_impl(zs::vm_ref vm) {
    // vm[0] should be the global table.
    // vm[1] should be the key.
    // vm[2] should be the value.
    // vm[3] should be the delegate.

    if (vm.stack_size() != 4) {
      zs::print("FSLKFJSKFJKLJFSKSF");
      return -1;
    }

    const zs::object& table = vm[0];
    const zs::object& key = vm[1];
    const object& value = vm[2];

    if (!table.is_table()) {
      return vm.set_error("Invalid table");
    }

    if (auto err = table.as_table().set(key, value)) {
      return vm.set_error("Invalid table");
    }

    return 0;
  }

  zs::object create_global_table_delegate(zs::engine* eng) {
    using namespace zs::literals;

    object obj = object::create_table(eng);
    table_object& tbl = obj.as_table();
    tbl.reserve(20);

    tbl.emplace("size"_ss, global_table_delegate_size_impl);
    tbl.emplace("is_empty"_ss, global_table_delegate_is_empty_impl);
    tbl.emplace("contains"_ss, global_table_delegate_contains_impl);
    tbl.emplace("emplace"_ss, global_table_delegate_optset_impl);
    tbl.emplace("set"_ss, global_table_delegate_set_impl);

    tbl.emplace("__create_array"_ss, global_table_delegate_create_array_impl);

    tbl.emplace(constants::get<meta_method::mt_set>(), global_table_delegate_meta_set_impl);

    tbl.set_no_default_none();
    return obj;
  }

  int_t global_table_import_impl(zs::vm_ref vm) {
    zs::parameter_stream ps(vm, true);

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

    object res;
    if (auto err = vm->to_string(val, res)) {
      return vm.set_error("Invalid string convertion");
    }

    return vm.push(res);
  }

  int_t global_table_to_json_impl(zs::vm_ref vm) {
    const object& val = vm[1];
    zs::string s(vm.get_engine());

    if (auto err = val.to_json(s)) {
      return vm.set_error("Could not convert to json.");
    }

    return vm.push_string(s);
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

  int_t global_table_is_empty_impl(zs::vm_ref vm) {
    using enum object_type;

    const object& obj = vm[1];
    switch (obj.get_type()) {
    case k_small_string:
      return vm.push_bool(obj.get_small_string_unchecked().empty());
    case k_string_view:
      return vm.push_bool(obj.get_string_view_unchecked().empty());
    case k_long_string:
      return vm.push_bool(obj.get_long_string_unchecked().empty());

    case k_array: {
      array_object& arr = obj.as_array();

      if (!arr.has_delegate()) {
        return vm.push_bool(arr.empty());
      }

      object get_is_empty_result;
      if (auto err = vm->get(obj, zs::_ss("is_empty"), get_is_empty_result)) {
        return vm.push_bool(arr.empty());
      }

      if (get_is_empty_result.is_bool()) {
        return vm.push(get_is_empty_result);
      }

      if (!get_is_empty_result.is_function()) {
        return vm.push_bool(arr.empty());
      }

      object res_value;
      if (auto err = vm->call(get_is_empty_result, obj, res_value)) {
        return vm.push_bool(arr.empty());
      }

      if (res_value.is_bool()) {
        return vm.push(res_value);
      }
      else {
        zs::print("DLSKJDSLKDHSGHSJGDJSHDKJS");
      }
      return vm.push_bool(arr.empty());
    }

    case k_table: {
      table_object& tbl = obj.as_table();

      object get_is_empty_result;
      if (auto err = vm->get(obj, zs::_ss("is_empty"), get_is_empty_result)) {
        return vm.push_bool(tbl.empty());
      }

      if (get_is_empty_result.is_bool()) {
        return vm.push(get_is_empty_result);
      }

      if (!get_is_empty_result.is_function()) {
        return vm.push_bool(tbl.empty());
      }

      object res_value;
      if (auto err = vm->call(get_is_empty_result, obj, res_value)) {
        return vm.push_bool(tbl.empty());
      }

      if (res_value.is_bool()) {
        return vm.push(res_value);
      }
      else {
        zs::print("DLSKJDSLKDHSGHSJGDJSHDKJS");
      }
      return vm.push_bool(tbl.empty());
    }
    case k_user_data: {
      object get_is_empty_result;
      if (auto err = vm->get(obj, zs::_ss("is_empty"), get_is_empty_result)) {
        return vm.push_false();
      }

      if (get_is_empty_result.is_bool()) {
        return vm.push(get_is_empty_result);
      }

      if (!get_is_empty_result.is_function()) {
        return vm.push_false();
      }

      object res_value;
      if (auto err = vm->call(get_is_empty_result, obj, res_value)) {
        return vm.push_false();
      }

      if (res_value.is_bool()) {
        return vm.push(res_value);
      }
      else {
        zs::print("DLSKJDSLKDHSGHSJGDJSHDKJS");
      }

      return vm.push_false();
    }

    default:
      return vm.push_true();
    }
    return vm.push_true();
  }

  template <auto... Masks>
  int_t global_table_is_type_mask_helper(zs::vm_ref vm) noexcept {
    static constexpr uint32_t mask = zs::create_type_mask(Masks...);
    return vm.push_bool(vm[1].has_type_mask(mask));
  }

} // namespace.

zs::object create_global_table(zs::engine* eng) {
  using enum object_type;

  object global_table = object::create_table_with_delegate(
      eng, zs::create_global_table_delegate(eng), delegate_flags_t::df_none);
  table_object& g = global_table.as_table();

  g.emplace(k_imported_modules_name, zs::_t(eng));
  g.emplace(k_module_loaders_name, zs::_t(eng));
  g.emplace(k_number_delegate_name, zs::create_number_default_delegate(eng));
  g.emplace(k_function_delegate_name, zs::create_function_default_delegate(eng));
  g.emplace(k_array_delegate_name, zs::create_array_delegate(eng));
  g.emplace(k_table_delegate_name, zs::create_table_default_delegate(eng));
  g.emplace(k_string_delegate_name, zs::create_string_default_delegate(eng));
  g.emplace(k_struct_delegate_name, zs::create_struct_default_delegate(eng));
  g.emplace(k_delegated_atom_delegates_table_name, zs::_t(eng));

  g.emplace(_ss("import"), global_table_import_impl);

  g.emplace(_ss("mutable_string"), zs::vm_create_mutable_string);
  g.emplace(_ss("np"), zs::create_float_array_lib(eng));
  g.emplace(_ss("float_array"), zs::vm_create_float_array);

  // Not '__tostring'.
  g.emplace(_ss("__to_string"), global_table_to_string_impl);
  g.emplace(_ss("to_string"), global_table_to_string_impl);

  g.emplace(_ss("__to_int"), global_table_to_int_impl);
  g.emplace(_ss("to_int"), global_table_to_int_impl);

  g.emplace(_ss("__to_float"), global_table_to_float_impl);
  g.emplace(_ss("to_float"), global_table_to_float_impl);

  g.emplace(_ss("to_json"), global_table_to_json_impl);

  g.emplace(_ss("is_empty"), global_table_is_empty_impl);

  g.emplace(_ss("is_float"), global_table_is_type_mask_helper<k_float>);
  g.emplace(_ss("is_int"), global_table_is_type_mask_helper<k_integer>);
  g.emplace(_ss("is_bool"), global_table_is_type_mask_helper<k_bool>);
  g.emplace(_ss("is_array"), global_table_is_type_mask_helper<k_array>);
  g.emplace(_ss("is_table"), global_table_is_type_mask_helper<k_table>);
  g.emplace(_ss("is_null"), global_table_is_type_mask_helper<k_null>);
  g.emplace(_ss("is_none"), global_table_is_type_mask_helper<k_none>);
  g.emplace(_ss("is_number"), global_table_is_type_mask_helper<zs::constants::k_number_mask>);
  g.emplace(_ss("is_string"), global_table_is_type_mask_helper<zs::constants::k_string_mask>);
  g.emplace(_ss("is_struct"), global_table_is_type_mask_helper<k_struct>);
  g.emplace(_ss("is_weak"), global_table_is_type_mask_helper<k_weak_ref>);
  g.emplace(_ss("is_atom"), global_table_is_type_mask_helper<k_atom>);
  g.emplace(_ss("is_closure"), global_table_is_type_mask_helper<zs::constants::k_function_mask>);
  g.emplace(k_global_table_is_struct_instance_name, global_table_is_type_mask_helper<k_struct_instance>);

  return global_table;
}
} // namespace zs.
