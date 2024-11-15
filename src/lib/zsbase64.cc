namespace zs {

namespace {

  int_t zbase64_encode_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      vm.set_error("Invalid number of arguments in base64.encode.\n");
      return -1;
    }

    const object& sobj = vm[1];
    if (!sobj.is_string()) {
      vm.set_error("Invalid string argument in base64.encode.\n");
      return -1;
    }

    return vm.push_string(
        zb::base64::encode<zs::string>(zs::string_allocator(vm.get_engine()), sobj.get_string_unchecked()));
  }

  int_t zbase64_decode_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      vm.set_error("Invalid number of arguments in base64.decode.\n");
      return -1;
    }

    const object& sobj = vm[1];
    if (!sobj.is_string()) {
      vm.set_error("Invalid string argument in base64.decode.\n");
      return -1;
    }

    return vm.push_string(
        zb::base64::decode<zs::string>(zs::string_allocator(vm.get_engine()), sobj.get_string_unchecked()));
  }

} // namespace.

zs::object create_base64_lib(zs::virtual_machine* vm) {
  using namespace zs::literals;

  zs::engine* eng = vm->get_engine();

  zs::object base64_module = zs::_t(eng);
  zs::table_object& base64_tbl = base64_module.as_table();

  base64_tbl["encode"] = zbase64_encode_impl;
  base64_tbl["decode"] = zbase64_decode_impl;

  return base64_module;
}
} // namespace zs.
