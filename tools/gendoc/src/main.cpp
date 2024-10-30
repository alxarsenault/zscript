
#include <zscript/zscript.h>
#include "zvirtual_machine.h"
#include <zbase/path.h>
#include <fstream>

zs::error_result do_it(zs::vm_ref vm) {
  zs::table_object& root = vm->get_root().as_table();
  root["ZSCRIPT_DOC_DIRECTORY"] = zs::_s(vm, ZSCRIPT_DOC_DIRECTORY);

  root["stable"] = zs::_nf([](zs::vm_ref vm) {
    return vm.push(zs::object_ptr::create_table_with_delegate(vm.get_engine(),
        zs::_t(vm, { { zs::constants::get<zs::meta_method::mt_get>(), zs::_nf([](zs::vm_ref vm) {
                        return vm.push(vm[0].as_table().emplace(vm[1], zs::_t(vm)).first->second);
                      }) } })));
  });

  zs::object_ptr closure;

  ZS_RETURN_IF_ERROR(vm->compile_file(ZSCRIPT_DOC_DIRECTORY "/src/gendoc.zs", "gendoc.zs", closure));

  if (!closure.is_closure()) {
    return zs::error_code::invalid_type;
  }

  return vm->call_from_top(closure);
}

int main(int argc, char** argv) {
  zs::vm vm;
  if (auto err = do_it(vm)) {
    zb::print(err, vm.get_error());
    return -1;
  }

  return 0;
}
