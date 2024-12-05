#include <zscript.h>
#include "zslib.h"
#include "zvirtual_machine.h"
#include "vm_utils/zvm_module.h"
#include <zbase/strings/charconv.h>
#include <zbase/strings/unicode.h>

namespace zs {

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
      objs++->stream(stream);

      stream << separator;
    }
    objs++->stream(stream);

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
    case k_mutable_string:
      return vm.push(obj.get_mutable_string_unchecked().empty());
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

    zs::string s((zs::allocator<char>(vm.get_engine())));

    if (auto err = val.convert_to_string(s)) {
      vm.set_error("Invalid string convertion");
      return -1;
    }

    return vm.push(zs::_s(vm.get_engine(), s));
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

  int_t zslib_optget_impl(zs::vm_ref vm) {

    zs::object ret;
    if (auto err = vm->get(vm[1], vm[2], ret)) {
      return vm.push_null();
    }

    return vm.push(ret);
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

    zs::vector<zs::object> args((zs::allocator<zs::object>(vm.get_engine())));

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

  int_t zslib_set_metadata_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 3) {
      return -1;
    }

    object& dst = vm[1];
    const object& delegate = vm[2];

    if (auto err = dst.set_delegate(delegate)) {
      return -1;
    }

    return vm.push(dst);
  }

  int_t zslib_get_metadata_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      return -1;
    }

    const object& obj = vm[1];
    if (!obj.is_delegable()) {
      return vm.push_null();
    }

    zs::delegate_object& del_obj = obj.as_delegate();

    if (!del_obj.has_delegate()) {
      return vm.push_null();
    }

    return vm.push(del_obj.get_delegate());
  }

  int_t zslib_has_metadata_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      return -1;
    }

    const object& obj = vm[1];
    return vm.push_bool(obj.is_delegable() and obj.as_delegate().has_delegate());
  }

int_t zslib_contains_impl(zs::vm_ref vm) {
 
  const object& obj = vm[1];
  const object& key = vm[2];
  object dest;
  if(auto err = vm->contains(obj, key, dest)) {
    vm->set_error("Contains failed", err.message());
    return -1;
  }
  
  return vm.push(dest);
}

int_t zslib_get_default_table_metadata_impl(zs::vm_ref vm) {
  
  return vm.push(vm->get_default_table_delegat());
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
      vm->handle_error(zs::errc::invalid_type, { -1, -1 }, "Invalid sys::add_import_directory().",
          ZS_DEVELOPER_SOURCE_LOCATION());
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
} // namespace.

zs::object create_zs_lib(zs::virtual_machine* vm) {
  using namespace zs::literals;

  zs::engine* eng = vm->get_engine();

  zs::object zs_module = zs::_t(eng);
  zs::table_object& zs_tbl = zs_module.as_table();
  zs_tbl.reserve(16);

  zs_tbl.emplace("to_string"_ss, zslib_tostring_impl);
  zs_tbl.emplace("to_int"_ss, zslib_toint_impl);
  zs_tbl.emplace("to_float"_ss, zslib_tofloat_impl);
  zs_tbl.emplace("to_weak"_ss, zslib_to_weak_impl);

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
  zs_tbl.emplace("is_class"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_class()); });
  zs_tbl.emplace("is_table"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_table()); });
  zs_tbl.emplace("is_weak"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_weak_ref()); });
  zs_tbl.emplace("is_node"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_node()); });
  zs_tbl.emplace(
      "is_delegable"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_delegable()); });
  zs_tbl.emplace("is_enum"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_enum()); });
  zs_tbl.emplace(
      "is_user_data"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_user_data()); });
  zs_tbl.emplace(
      "is_raw_pointer"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_raw_pointer()); });
  zs_tbl.emplace("is_none"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_none()); });
  zs_tbl.emplace("is_error"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_error()); });
  zs_tbl.emplace(
      "is_ref_counted"_ss, +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_ref_counted()); });

  zs_tbl.emplace(
      zs::_sv(s_is_null_or_none_name),
      +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_null_or_none()); });

  zs_tbl.emplace(
      zs::_sv(s_is_mutable_string_name),
      +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_mutable_string()); });

  zs_tbl.emplace(
      zs::_sv(s_is_number_or_bool_name),
      +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_number_or_bool()); });

  zs_tbl.emplace(
      zs::_sv(s_is_struct_instance_name),
      +[](zs::vm_ref vm) -> int_t { return vm.push_bool(vm[1].is_struct_instance()); });

  zs_tbl.emplace("set_metadata"_ss, zslib_set_metadata_impl);
  zs_tbl.emplace("get_metadata"_ss, zslib_get_metadata_impl);
  zs_tbl.emplace("has_metadata"_ss, zslib_has_metadata_impl);

  zs_tbl.emplace("optget"_ss, zslib_optget_impl);
  zs_tbl.emplace("call"_ss, zslib_rawcall_impl);
  zs_tbl.emplace("bind"_ss, zslib_bind_impl);
  zs_tbl.emplace("apply"_ss, zslib_apply_impl);
  zs_tbl.emplace("strlen"_ss, zslib_strlen_impl);
  zs_tbl.emplace("placeholder"_ss, zs::object((void*)&s_placeholder));
  
  zs_tbl.emplace("contains"_ss, zslib_contains_impl);
  zs_tbl.emplace("get_default_table_metadata", zslib_get_default_table_metadata_impl);

  
  zs_tbl.emplace("stable"_ss, zslib_create_stable_impl);

  zs_tbl.emplace("print"_ss, zslib_print_impl);
  zs_tbl.emplace("write"_ss, zslib_write_impl);
  zs_tbl.emplace(zs::_sv(s_print_to_string_name), zslib_print_to_string_impl);
  zs_tbl.emplace(zs::_sv(s_write_to_string_name), zslib_write_to_string_impl);

  zs_tbl.emplace(zs::_s(eng, "add_import_directory"), zslib_add_import_directory_impl);

  zs_tbl.emplace("is_one_of"_ss, zslib_is_one_of_impl);

  return zs_module;
}
} // namespace zs.
