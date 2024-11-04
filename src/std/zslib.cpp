#include <zscript/zscript.h>
#include <zscript/std/zslib.h>
#include "zvirtual_machine.h"
#include "utility/zvm_module.h"
#include <zbase/strings/charconv.h>
#include <zbase/strings/unicode.h>
#include "utility/zparameter_stream.h"

namespace zs {

struct zslib_proxy_tag {};
template <>
struct internal::proxy<zslib_proxy_tag> {

  inline static void clear_errors(virtual_machine* vm) { vm->_errors.clear(); }
};

using zslib_proxy = internal::proxy<zslib_proxy_tag>;

namespace {
  template <class CharType>
  static inline std::basic_string_view<CharType> sv_from_char(const CharType& c) noexcept {
    return std::basic_string_view<CharType>(&c, 1);
  }
  static void* s_placeholder = nullptr;

  static std::string_view s_stable_metatable_name = "__stable_metatable__";
  static std::string_view s_print_to_string_name = "print_to_string";
  static std::string_view s_write_to_string_name = "write_to_string";
  static std::string_view s_is_number_or_bool_name = "is_number_or_bool";

  static std::string_view s_is_mutable_string_name = "is_mutable_string";

  static std::string_view s_is_null_or_none_name = "is_null_or_none";
  static std::string_view s_is_struct_instance_name = "is_struct_instance";

  int_t zslib_print_internal(
      zs::vm_ref vm, std::ostream& stream, std::string_view sep = "", std::string_view endl = "") {
    int_t nargs = vm.stack_size();

    if (nargs == 1) {
      if (!endl.empty()) {
        stream << endl;
      }
      return 0;
    }

    const object* objs = vm.stack_base_pointer() + 1;

    zs::string separator(sep, zs::string_allocator(vm.get_engine()));
    zs::string end_line(endl, zs::string_allocator(vm.get_engine()));

    if (objs->is_meta_argument()) {

      zs::array_object& arr = objs++->as_array();
      const int_t n_template = arr.size();

      if (n_template <= 0 or n_template > 2) {
        vm.set_error("Invalid meta arguments count in zs.print.\n");
        return -1;
      }

      const object& sep_obj = arr[0];

      if (sep_obj.is_string()) {
        separator = sep_obj.get_string_unchecked();
      }
      else if (sep_obj.is_integer()) {
        separator.clear();
        zb::unicode::append_to(sv_from_char<char32_t>((char32_t)sep_obj._int), separator);
      }
      else {
        vm.set_error("Invalid end-line meta argument in zs.print.\n");
        return -1;
      }

      if (n_template == 2) {

        const object& endl_obj = arr[1];

        if (endl_obj.is_string()) {
          end_line = endl_obj.get_string_unchecked();
        }
        else if (endl_obj.is_integer()) {
          end_line.clear();
          zb::unicode::append_to(sv_from_char<char32_t>((char32_t)endl_obj._int), end_line);
        }
        else {
          vm.set_error("Invalid end-line meta argument in zs.print.\n");
          return -1;
        }
      }

      nargs--;
    }

    for (int_t i = 1; i < nargs - 1; i++) {
      (objs++)->stream(stream);

      stream << separator;
    }
    (objs++)->stream(stream);

    if (!end_line.empty()) {
      stream << end_line;
    }
    return 0;
  }

  int_t zslib_print_impl(zs::vm_ref vm) {
    return zslib_print_internal(vm, vm.get_engine()->get_stream(), " ", "\n");
  }

  int_t zslib_write_impl(zs::vm_ref vm) { return zslib_print_internal(vm, vm.get_engine()->get_stream()); }

  int_t zslib_print_to_string_impl(zs::vm_ref vm) {
    zs::ostringstream stream(zs::create_string_stream(vm.get_engine()));

    if (zslib_print_internal(vm, stream, " ", "") == 0) {
      return vm.push_string(stream.str());
    }
    return -1;
  }

  int_t zslib_write_to_string_impl(zs::vm_ref vm) {
    zs::ostringstream stream(zs::create_string_stream(vm.get_engine()));

    if (zslib_print_internal(vm, stream) == 0) {
      return vm.push_string(stream.str());
    }
    return -1;
  }

  int_t zslib_is_empty_impl(zs::vm_ref vm) {
    using enum object_type;

    const object& obj = vm[1];
    switch (obj.get_type()) {
    case k_small_string:
      return vm.push(obj.get_small_string_unchecked().empty());
    case k_string_view:
      return vm.push(obj.get_string_view_unchecked().empty());
    case k_long_string:
      return vm.push(obj.get_long_string_unchecked().empty());

    case k_array:
      return vm.push(obj.as_array().empty());
    case k_table:
      return vm.push(obj.as_table().empty());
    default:
      return vm.push(true);
    }

    return vm.push(true);
  }

  int_t zslib_tostring_impl(zs::vm_ref vm) {
    const object& val = vm[1];
    if (val.is_string()) {
      return vm.push(val);
    }

    object res;
    if (auto err = vm->to_string(val, res)) {
      vm.set_error("Invalid string convertion");
      return -1;
    }
    return vm.push(res);
    //    zs::string s((zs::allocator<char>(vm.get_engine())));
    //
    //    if (auto err = val.convert_to_string(s)) {
    //      vm.set_error("Invalid string convertion");
    //      return -1;
    //    }

    //    return vm.push(zs::_s(vm.get_engine(), s));
  }

  int_t zslib_toint_impl(zs::vm_ref vm) {
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

  int_t zslib_tofloat_impl(zs::vm_ref vm) {
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
  int_t zslib_copy_impl(zs::vm_ref vm) {
    const object& val = vm[1];

    object res;
    if (auto err = vm->copy(val, res)) {
      vm.set_error("Invalid copy");
      return -1;
    }
    return vm.push(res);
  }

  int_t zslib_optget_impl(zs::vm_ref vm) {

    zs::object ret;
    if (auto err = vm->get(vm[1], vm[2], ret)) {
      return vm.push_null();
    }

    return vm.push(ret);
  }

  int_t zslib_get_table_keys_impl(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();

    if (nargs != 2) {
      vm.set_error("Invalid number of parameters in zs::get_table_keys.\n");
      return -1;
    }

    const zs::object& obj = vm[1];

    if (!obj.is_table()) {
      vm->ZS_VM_ERROR(errc::not_a_table,
          "Invalid table parameter in zs::get_table_keys(this, table)." );
      return -1;
    }

    zs::table_object& tbl = obj.as_table();
    zs::object arr = _a(vm, tbl.size());
    zs::array_object& a = arr.as_array();

    object* arr_it = a.data();
    for (auto it : tbl) {
      *arr_it++ = it.first;
    }

    return vm.push(std::move(arr));
  }

  int_t zslib_raw_get_impl(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();

    if (nargs != 3) {
      vm.set_error("Invalid number of parameters in zs::raw_get.\n");
      return -1;
    }

    const zs::object& obj = vm[1];
    const zs::object& key = vm[2];

    zs::object ret;

    if (auto err = vm->raw_get(obj, key, ret)) {
      vm.set_error("Could not find value in zs::raw_get.\n");
      return -1;
    }

    return vm.push(ret);

    //    if (obj.is_table()) {
    //      if (auto err = obj.as_table().get(key, ret)) {
    //        vm.set_error("Could not find value in zs::raw_get.\n");
    //        return -1;
    //      }
    //
    //      return vm.push(ret);
    //    }
    //
    //    vm.set_error("Invalid object type in zs::raw_get.\n");
    //    return -1;
  }

  int_t zslib_apply_impl(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();

    if (nargs < 2) {
      vm.set_error("Invalid number of parameters in zs::apply.\n");
      return -1;
    }

    const object& fct = vm[1];
    if (!fct.is_function()) {
      vm.set_error("Invalid function parameter in zs::apply.\n");
      return -1;
    }

    if (nargs == 2) {
      zs::object ret_value;
      if (auto err = vm->call(fct, { vm[0] }, ret_value)) {
        return -1;
      }

      return vm.push(ret_value);
    }

    zs::small_vector<zs::object, 8> args((zs::allocator<zs::object>(vm.get_engine())));

    for (int_t i = 2; i < nargs; i++) {
      const object& p = vm[i];
      if (p.is_array()) {
        const array_object& parr = p.as_array();
        for (const object& obj : parr) {
          args.push_back(obj);
        }
      }
      else {
        args.push_back(p);
      }
    }

    zs::object ret_value;
    if (auto err = vm->call(fct, std::span<const object>(args), ret_value)) {
      return -1;
    }

    return vm.push(ret_value);
  }

  int_t zslib_strlen_impl(zs::vm_ref vm) {
    return vm.push(zb::unicode::length(vm[1].get_string_unchecked()));
  }

  int_t zslib_rawcall_impl(zs::vm_ref vm) {
    int_t nargs = vm.stack_size();

    if (nargs < 3) {
      vm.set_error("Invalid number of parameters in zs::rawcall.\n");
      return -1;
    }

    zs::object ret_value;
    //  zb::print("PPPPP", vm->stack().get_stack_view().subspan(2));
    if (auto err = vm->call(vm[1], vm->stack().get_stack_view().subspan(2), ret_value)) {

      return -1;
    }

    return vm.push(ret_value);
  }

  int_t zslib_bind_impl(zs::vm_ref vm) {
    int_t nargs = vm.stack_size();

    if (nargs < 3) {
      vm.set_error("Invalid number of parameters in zs::bind.\n");
      return -1;
    }

    auto arr = zs::_a(vm.get_engine(), vm->stack().get_stack_view().subspan(2));
    //  if(arr.as_array()[0] ==     zs::object  ((void*)&s_placeholder)) {
    //    arr.as_array()[0]= vm[0];
    //  }

    return vm.push(zs::_nc(vm.get_engine(), [fct = vm[1], arr = std::move(arr)](zs::vm_ref vm) -> int_t {
      zs::object placeholder((void*)&s_placeholder);

      zs::object ret_value;
      int_t nargs = vm.stack_size();
      int_t n_binded_args = 0;

      std::span<const object> sss = vm->stack().get_stack_view().subspan(1);
      auto it = sss.begin();
      //    if(arr.as_array()[0]==placeholder) {
      ////      it = sss.begin() - 1;
      ////      vm.push(vm[0]);
      ////      ++it;
      ////    sss = vm->stack().get_stack_view().subspan(0);
      //    }

      for (const auto& obj : arr.as_array()) {
        if (obj == placeholder) {
          vm.push(*it);
          ++it;
        }
        else {
          vm.push(obj);
        }
        n_binded_args++;
      }

      for (; it < sss.end(); ++it) {
        vm.push(*it);
        n_binded_args++;
      }

      if (auto err = vm->call(fct, vm->stack().get_stack_view().subspan(nargs), ret_value)) {
        vm->pop(n_binded_args);
        return -1;
      }

      vm->pop(n_binded_args);
      return vm.push(ret_value);
    }));
  }

  int_t zslib_contains_impl(zs::vm_ref vm) {

    const object& obj = vm[1];
    const object& key = vm[2];
    object dest;

    if (auto err = vm->contains(obj, key, dest)) {
      //      vm->set_error("Contains failed", err.message());

      zslib_proxy::clear_errors(vm.get_virtual_machine());
      //      vm->_errors.clear();
      return vm.push_bool(false);
    }

    return vm.push(!dest.is_null());
  }

  int_t zslib_set_delegate_impl(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();
    if (!zb::is_one_of(nargs, 3, 4)) {
      vm->ZS_VM_ERROR(errc::invalid_parameter_count,
          "Invalid parameter count (", nargs,
          ") in zs::set_delegate(this, table, delegate, var use_default_delegate = null), expected 3 or "
          "4.\n");

      return -1;
    }

    object& obj = vm[1];

    if (!obj.is_delegable()) {
      vm->ZS_VM_ERROR(errc::not_delegable,
          "Not a delegable type in zs::set_delegate(this, table, delegate, var use_default_delegate = null)." );
      return -1;
    }
    
    if(  obj.as_delegable().is_locked()) {
      vm->ZS_VM_ERROR(errc::not_delegable,
          "Delegate is locked in zs::set_delegate(this, table, delegate, var use_default_delegate = null)." );
      return -1;
    }

    const object& delegate = vm[2];

    if (auto err = obj.set_delegate(delegate)) {
      vm->ZS_VM_ERROR(err,
          "Could not set delegate in zs::set_delegate(this, table, delegate, var use_default_delegate = "
          "null).");
      return -1;
    }

    if (nargs == 4) {
      if (const object& use_default_delegate = vm[3]; use_default_delegate.is_bool()) {
        obj.as_delegable().set_use_default_delegate((bool)use_default_delegate._int);
      }
      else {
        vm->ZS_VM_ERROR(errc::not_a_bool,
            "Invalid bool parameter (use_default_delegate) in zs::set_delegate(this, table, delegate, var "
            "use_default_delegate = null)." );
        return -1;
      }
    }

    return vm.push(obj);
  }

  int_t zslib_get_delegate_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      return -1;
    }

    const object& obj = vm[1];
    if (!obj.is_delegable()) {
      return vm.push_null();
    }

    zs::delegable_object& del_obj = obj.as_delegable();

    if (!del_obj.has_delegate()) {
      return vm.push_null();
    }

    return vm.push(del_obj.get_delegate());
  }

  int_t zslib_has_delegate_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      return -1;
    }

    const object& obj = vm[1];
    return vm.push_bool(obj.is_delegable() and obj.as_delegable().has_delegate());
  }

  int_t zslib_set_use_default_delegate_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 3) {
      return -1;
    }

    const object& obj = vm[1];

    if (!obj.is_delegable()) {
      vm->handle_error(errc::not_delegable, { -1, -1 },
          "Not a delegable type in zs::set_use_default_delegate().", ZS_DEVELOPER_SOURCE_LOCATION());
      return -1;
    }

    const object& use_delegate = vm[2];
    if (!use_delegate.is_bool()) {
      vm->handle_error(errc::not_delegable, { -1, -1 },
          "Invalid bool parameter in zs::set_use_default_delegate().", ZS_DEVELOPER_SOURCE_LOCATION());
      return -1;
    }

    obj.as_delegable().set_use_default_delegate((bool)use_delegate._int);
    return vm.push(obj);
  }

  int_t zslib_get_use_default_delegate_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      return -1;
    }

    const object& obj = vm[1];
    return vm.push_bool(obj.is_delegable() and obj.as_delegable().get_use_default_delegate());
  }

  int_t zslib_get_addr_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      return -1;
    }

    const object& obj = vm[1];
    return vm.push((int_t)obj._pointer);
  }

  int_t zslib_get_table_default_delegate_impl(zs::vm_ref vm) {

    return vm.push(vm->get_default_table_delegate());
  }
 
  int_t zslib_create_object_impl(zs::vm_ref vm) {

    int_t nargs = vm.stack_size();

    object obj = zs::_t(vm);

    if (nargs >= 2) {
      object& delegate = vm[1];

      if (nargs >= 3) {
        const object& should_append = vm[2];
        if (!should_append.is_bool()) {
          vm->set_error("Invalid parameter in zs::create_object()");
          return -1;
        }

        if ((bool)should_append._int) {

          if (delegate.is_table()) {
            delegate.as_table().set_delegate(vm->get_default_table_delegate());
          }
          else {
            zb::print("Not a table delegate.");
          }
        }
      }

      obj.as_table().set_delegate(delegate);
    }

    return vm.push(std::move(obj));
  }

  int_t zslib_stable_delegate_get_impl(zs::vm_ref vm);

  zs::object zslib_stable_get_delegate_table(zs::vm_ref vm) {
    zs::table_object& reg_table = vm.get_engine()->get_registry_table().as_table();

    zs::object& stable_metatable = reg_table[zs::_sv(s_stable_metatable_name)];

    if (stable_metatable.is_table()) {
      return stable_metatable;
    }

    zs::table_object& tbl = (stable_metatable = zs::_t(vm.get_engine())).as_table();
    tbl.emplace(zs::_sv(zs::constants::k_mt_get_string), zslib_stable_delegate_get_impl);
    return stable_metatable;
  }

  int_t zslib_stable_delegate_get_impl(zs::vm_ref vm) {
    object& stable = vm[0];
    const object& key = vm[1];

    object child_stable
        = zs::object::create_table_with_delegate(vm.get_engine(), zslib_stable_get_delegate_table(vm));

    std::pair<zs::table_object::iterator, bool> res
        = stable.as_table().insert_or_assign(key, std::move(child_stable));
    return vm.push(res.first->second);
  }

  int_t zslib_create_stable_impl(zs::vm_ref vm) {
    const int_t nargs = vm.stack_size();

    if (nargs == 1) {
      zs::object tbl
          = zs::object::create_table_with_delegate(vm.get_engine(), zslib_stable_get_delegate_table(vm));
      return vm.push(std::move(tbl));
    }

    else if (nargs == 2) {
      object tbl = vm[1];

      if (!tbl.is_table()) {
        vm.set_error("Invalid table parameter in stable().\n");
        return -1;
      }

      tbl.as_table().set_delegate(zslib_stable_get_delegate_table(vm));
      return vm.push(std::move(tbl));
    }

    else {
      vm.set_error("To many parameters in stable().\n");
      return -1;
    }
  }

  int_t zslib_to_weak_impl(zs::vm_ref vm) { return vm.push(vm[1].get_weak_ref()); }

  int_t zslib_add_import_directory_impl(zs::vm_ref vm) {

    int_t nargs = vm.stack_size();
    if (nargs <= 1) {
      vm->handle_error(zs::errc::invalid_parameter_count, { -1, -1 },
          "Missing directory parameter in sys::add_import_directory().", ZS_DEVELOPER_SOURCE_LOCATION());
      return -1;
    }

    if (auto err = vm->get_engine()->add_import_directory(vm[1].get_string_unchecked())) {
      vm->handle_error(
          err, { -1, -1 }, "Invalid sys::add_import_directory().", ZS_DEVELOPER_SOURCE_LOCATION());
      return -1;
    }

    return 0;
  }

  int_t zslib_is_one_of_impl(zs::vm_ref vm) {
    int_t nargs = vm.stack_size();

    if (nargs < 3) {
      vm->set_error("Invalid number of parameters in zs::is_one_of(...)");
      return -1;
    }

    const object& obj = vm[1];

    for (int_t i = 2; i < nargs; i++) {
      if (obj == vm[i]) {
        return vm.push(true);
      }
    }

    return vm.push(false);
  }

  int_t zslib_in_range_impl(zs::vm_ref vm) {
    int_t nargs = vm.stack_size();

    if (nargs < 4) {
      vm->set_error("Invalid number of parameters in zs::in_range(v, l, r)");
      return -1;
    }

    float_t val, l, r;

    if (auto err = vm[1].convert_to_float(val)) {
      vm->set_error("Invalid number  parameter in zs::in_range(value, left, right)");
      return -1;
    }

    if (auto err = vm[2].convert_to_float(l)) {
      vm->set_error("Invalid number  parameter in zs::in_range(value, left, right)");
      return -1;
    }

    if (auto err = vm[3].convert_to_float(r)) {
      vm->set_error("Invalid number  parameter in zs::in_range(value, left, right)");
      return -1;
    }

    bool with_side = true;
    if (nargs == 5) {

      if (!vm[4].is_bool()) {
        vm->set_error("Invalid number  parameter in zs::in_range(value, left, right)");
        return -1;
      }

      with_side = (bool)vm[4]._int;
    }

    if (with_side) {
      return vm.push(val >= l and val <= r);
    }

    return vm.push(val > l and val < r);
  }

  int_t zslib_all_equals_impl(zs::vm_ref vm) {
    int_t nargs = vm.stack_size();

    if (nargs < 3) {
      vm->set_error("Invalid number of parameters in zs::all_equals(...)");
      return -1;
    }

    const object& obj = vm[1];

    for (int_t i = 2; i < nargs; i++) {
      if (obj != vm[i]) {
        return vm.push(false);
      }
    }

    return vm.push(true);
  }

  int_t zslib_all_true_impl(zs::vm_ref vm) {
    int_t nargs = vm.stack_size();

    if (nargs < 2) {
      vm->set_error("Invalid number of parameters in zs::all_true(...)");
      return -1;
    }

    for (int_t i = 1; i < nargs; i++) {
      if (!vm[i]) {
        return vm.push(false);
      }
    }

    return vm.push(true);
  }

  int_t zslib_error_impl(zs::vm_ref vm) {

    int_t nargs = vm.stack_size();
    if (nargs == 1) {
      //      zb::print("ERROR no message");
      vm->set_error("ERROR no message");
      return -1;
    }

    zs::parameter_stream ps(vm, true);

    //    bool has_ec_code = false;
    bool has_msg = false;

    zs::error_code ec_code = zs::errc::success;
    std::string_view message;

    //    if (auto err = ps.check<error_code_parameter>(nargs == 3, ec_code)) {
    //      if (nargs == 3) {
    //        vm->set_error("Invalid error_code parameter in zs::error()");
    //        return -1;
    //      }
    //    }
    //    else {
    //      has_ec_code = true;
    //    }

    if (ps) {
      if (auto err = ps.require<string_parameter>(message)) {
        vm->set_error("Invalid parameter in zs::error()");
        return -1;
      }

      has_msg = true;
    }
    //    else if (!has_ec_code) {
    //      vm->set_error("Invalid parameter in zs::error()");
    //      return -1;
    //    }
    //
    //    if (has_ec_code and has_msg) {
    //      //      zb::print("User Error: code:", ec_code, "message:", message);
    //      vm->set_error("User Error: code: ", zs::error_result(ec_code).message(), " message: ", message);
    //      return -1;
    //    }
    //
    //    if (has_ec_code) {
    //      //      zb::print("User Error: code:", ec_code);
    //      vm->set_error("User Error: code: ", zs::error_result(ec_code).message());
    //      return -1;
    //    }

    if (has_msg) {
      //      zb::print("User Error: message:", message);
      vm->set_error("User Error: message: ", message);
      return -1;
    }

    //    zb::print("ERROR no message");
    vm->set_error("ERROR no message");
    return -1;
  }

} // namespace.

zs::object create_zs_lib(zs::vm_ref vm) {
  using namespace zs::literals;

  zs::engine* eng = vm->get_engine();

  zs::object zs_module = zs::_t(eng);
  zs::table_object& zs_tbl = zs_module.as_table();
  zs_tbl.reserve(16);

  zs_tbl.emplace("to_string"_ss, zslib_tostring_impl);
  zs_tbl.emplace("to_int"_ss, zslib_toint_impl);
  zs_tbl.emplace("to_float"_ss, zslib_tofloat_impl);
  zs_tbl.emplace("to_weak"_ss, zslib_to_weak_impl);
  zs_tbl.emplace("copy"_ss, zslib_copy_impl);
  //  zs_tbl.emplace("create_mutable_string"_ss, zslib_tostring_impl);

  zs_tbl.emplace("is_empty"_ss, zslib_is_empty_impl);
   zs_tbl.emplace("is_array"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_array()); });
  zs_tbl.emplace("is_true"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_if_true()); });
  zs_tbl.emplace("is_null"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_null()); });
  zs_tbl.emplace("is_bool"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_bool()); });
  zs_tbl.emplace("is_int"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_integer()); });
  zs_tbl.emplace("is_float"_ss, +[](zs::vm_ref vm) -> int_t { return vm->push(vm[1].is_float()), 1; });
  zs_tbl.emplace("is_number"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_number()); });
  zs_tbl.emplace("is_string"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_string()); });
  zs_tbl.emplace("is_function"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_function()); });
  zs_tbl.emplace("is_struct"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_struct()); });
  zs_tbl.emplace("is_table"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_table()); });
  zs_tbl.emplace("is_weak"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_weak_ref()); });
  zs_tbl.emplace(
      "is_delegable"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_delegable()); });
  zs_tbl.emplace("is_enum"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_enum()); });
  zs_tbl.emplace(
      "is_user_data"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_user_data()); });

  zs_tbl.emplace(
      "is_pointer"_ss,
      +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_atom(atom_type::atom_pointer)); });

  zs_tbl.emplace("is_none"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_none()); });

  zs_tbl.emplace(
      "is_ref_counted"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_ref_counted()); });

  zs_tbl.emplace(
      zs::_sv(s_is_null_or_none_name),
      +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_null_or_none()); });

  zs_tbl.emplace(
      zs::_sv(s_is_number_or_bool_name),
      +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_number_or_bool()); });

  zs_tbl.emplace(
      zs::_sv(s_is_struct_instance_name),
      +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_struct_instance()); });

  zs_tbl.emplace("set_delegate"_ss, zslib_set_delegate_impl);
  zs_tbl.emplace("get_delegate"_ss, zslib_get_delegate_impl);
  zs_tbl.emplace("has_delegate"_ss, zslib_has_delegate_impl);

  zs_tbl.emplace("set_use_default_delegate", zslib_set_use_default_delegate_impl);
  zs_tbl.emplace("get_use_default_delegate", zslib_get_use_default_delegate_impl);

  zs_tbl.emplace("get_table_default_delegate", zslib_get_table_default_delegate_impl);

  zs_tbl.emplace("get_addr"_ss, zslib_get_addr_impl);

  zs_tbl.emplace("raw_get"_ss, zslib_raw_get_impl);
  zs_tbl.emplace("get_table_keys"_ss, zslib_get_table_keys_impl);

  zs_tbl.emplace("optget"_ss, zslib_optget_impl);
  zs_tbl.emplace("call"_ss, zslib_rawcall_impl);
  zs_tbl.emplace("bind"_ss, zslib_bind_impl);
  zs_tbl.emplace("apply"_ss, zslib_apply_impl);
  zs_tbl.emplace("strlen"_ss, zslib_strlen_impl);
  zs_tbl.emplace("placeholder"_ss, zs::object((void*)&s_placeholder));

  zs_tbl.emplace("contains"_ss, zslib_contains_impl);
   zs_tbl.emplace("create_object", zslib_create_object_impl);

  zs_tbl.emplace("stable"_ss, zslib_create_stable_impl);

  zs_tbl.emplace("print"_ss, zslib_print_impl);
  zs_tbl.emplace("write"_ss, zslib_write_impl);
  zs_tbl.emplace(zs::_sv(s_print_to_string_name), zslib_print_to_string_impl);
  zs_tbl.emplace(zs::_sv(s_write_to_string_name), zslib_write_to_string_impl);

  zs_tbl.emplace(zs::_s(eng, "add_import_directory"), zslib_add_import_directory_impl);

  zs_tbl.emplace("is_one_of"_ss, zslib_is_one_of_impl);
  zs_tbl.emplace("all_equals"_ss, zslib_all_equals_impl);
  zs_tbl.emplace("all_true"_ss, zslib_all_true_impl);
  zs_tbl.emplace("in_range"_ss, zslib_in_range_impl);

  zs_tbl.emplace("error"_ss, zslib_error_impl);

  //  {
  //    zs::object error_code_table = zs::_t(eng);
  //    zs::table_object& error_code_table_obj = error_code_table.as_table();
  //    error_code_table_obj.reserve(40);
  //
  // #define ZS_DECL_ERROR_CODE(name, str) error_code_table_obj.emplace(str, errc::name);
  // #include <zscript/error_codes_def.h>
  // #undef ZS_DECL_ERROR_CODE
  //
  //    zs_tbl.emplace("error_code"_ss, std::move(error_code_table));
  //  }
  return zs_module;
}
} // namespace zs.
