#include <zscript/std/zbase64.h>
#include "zvirtual_machine.h"
#include <zbase/crypto/base64.h>

namespace zs {
using namespace zs::literals;

namespace {
  int_t zbase64_encode_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      return vm.set_error("Invalid number of arguments in base64.encode.\n");
    }

    const object& sobj = vm[1];
    if (!sobj.is_string()) {
      return vm.set_error("Invalid string argument in base64.encode.\n");
    }

    return vm.push_string(
        zb::base64::encode<zs::string>(zs::string_allocator(vm.get_engine()), sobj.get_string_unchecked()));
  }

  int_t zbase64_decode_impl(zs::vm_ref vm) {
    if (vm.stack_size() != 2) {
      return vm.set_error("Invalid number of arguments in base64.decode.\n");
    }

    const object& sobj = vm[1];
    if (!sobj.is_string()) {
      return vm.set_error("Invalid string argument in base64.decode.\n");
    }

    return vm.push_string(
        zb::base64::decode<zs::string>(zs::string_allocator(vm.get_engine()), sobj.get_string_unchecked()));
  }
} // namespace.

zs::object create_base64_lib(zs::vm_ref vm) {
  zs::engine* eng = vm->get_engine();

  zs::object base64_module = zs::_t(eng);
  zs::table_object& base64_tbl = base64_module.as_table();

  base64_tbl.emplace("encode"_ss, zbase64_encode_impl);
  base64_tbl.emplace("decode"_ss, zbase64_decode_impl);

  return base64_module;
}
} // namespace zs.
