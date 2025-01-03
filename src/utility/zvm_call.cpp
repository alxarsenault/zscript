#include "utility/zvm_call.h"
#include "zvirtual_machine.h"

namespace zs {

zs::error_result call_from_top(zs::vm_ref vm, const object& closure, int_t n_params, object& ret_value) {
  return vm->call(closure, n_params, vm->stack_size() - n_params, ret_value);
}

zs::optional_result<zs::object> call_from_top_opt(zs::vm_ref vm, const object& closure, int_t n_params) {

  if (!n_params) {
    vm->push_global();
    n_params = 1;
  }

  object ret_value;
  if (auto err = call_from_top(vm, closure, n_params, ret_value)) {
    return err.code;
  }

  return ret_value;
}

zs::error_result call_from_top(zs::vm_ref vm, const object& closure, int_t n_params) {

  if (!n_params) {
    vm->push_global();
    n_params = 1;
  }

  object ret_value;
  if (auto err = call_from_top(vm, closure, n_params, ret_value)) {
    return err.code;
  }

  return {};
}
} // namespace zs.
