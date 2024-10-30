namespace zs {

namespace {
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
    zs::ostringstream stream(std::ios_base::out, zs::string_allocator(vm.get_engine()));

    if (zslib_print_internal(vm, stream, " ", "") == 0) {
      return vm.push_string(stream.str());
    }
    return -1;
  }

  int_t zslib_write_to_string_impl(zs::vm_ref vm) {
    zs::ostringstream stream(std::ios_base::out, zs::string_allocator(vm.get_engine()));

    if (zslib_print_internal(vm, stream) == 0) {
      return vm.push_string(stream.str());
    }
    return -1;
  }
} // namespace.

zs::object create_zs_lib(zs::virtual_machine* vm) {
  using namespace zs::literals;

  zs::engine* eng = vm->get_engine();

  zs::object zs_module = zs::_t(eng);
  zs::table_object& zs_tbl = zs_module.as_table();
  zs_tbl.reserve(8);

  zs_tbl["print"] = zslib_print_impl;
  zs_tbl["write"] = zslib_write_impl;

  zs_tbl["print_to_string"] = zslib_print_to_string_impl;
  zs_tbl["write_to_string"] = zslib_write_to_string_impl;

  return zs_module;
}
} // namespace zs.
