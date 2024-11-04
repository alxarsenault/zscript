#include "zfunction_delegate.h"
#include "zvirtual_machine.h"
#include "object/zfunction_prototype.h"
#include <zbase/strings/unicode.h>
#include <zbase/utility/scoped.h>

namespace zs {

static int_t function_delegate_call_impl(zs::vm_ref vm) {

  int_t nargs = vm.stack_size();

  const object& fct = vm[0];

  if (!fct.is_function()) {
    vm->set_error("Not a function in call");
    return -1;
  }

  object ret;
  if (nargs == 1) {
    if (auto err = vm->call(fct, { vm->get_root() }, ret)) {
      vm->set_error("Error in function.call() : ", err.message());
      return -1;
    }
  }
  else {
    if (auto err = vm->call(fct, nargs - 1, 1, ret)) {
      vm->set_error("Error in function.call() : ", err.message());
      return -1;
    }
  }

  return vm.push(ret);
}

struct tahjdfks {};

template <>
struct internal::proxy<tahjdfks> {
  static inline void clear_errors(vm_ref vm) {
    vm->_error_message.clear();
    vm->_errors.clear();
  }

  static inline error_result call_closure(
      vm_ref vm, const object& cobj, int_t n_params, int_t stack_base, object& ret_value) {
    return vm->call_closure(cobj, n_params, stack_base, ret_value, true);
  }
};

static int_t function_delegate_this_impl(zs::vm_ref vm) {
  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_this_impl, only implemented for closure");
    return -1;
  }

  return vm.push(fct.as_closure()._this);
}

static int_t function_delegate_get_parameter_count_impl(zs::vm_ref vm) {
  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_get_parameter_count_impl, only implemented for closure");
    return -1;
  }

  return vm.push((int_t)fct.as_closure().get_proto()._parameter_names.size());
}

static int_t function_delegate_get_parameter_names_impl(zs::vm_ref vm) {
  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_get_parameter_names_impl, only implemented for closure");
    return -1;
  }

  const auto& param_names = fct.as_closure().get_proto()._parameter_names;
  size_t sz = param_names.size();
  auto arr = zs::_a(vm.get_engine(), sz);
  for (size_t i = 0; i < sz; i++) {
    arr.as_array()[i] = param_names[i];
  }

  return vm.push(arr);
}

static int_t function_delegate_get_default_params_impl(zs::vm_ref vm) {
  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_get_default_params_impl, only implemented for closure");
    return -1;
  }

  const auto& default_params = fct.as_closure()._default_params;
  size_t sz = default_params.size();
  auto arr = zs::_a(vm.get_engine(), sz);
  for (size_t i = 0; i < sz; i++) {
    arr.as_array()[i] = default_params[i];
  }

  return vm.push(arr);
}

static int_t function_delegate_bind_env_impl(zs::vm_ref vm) {
  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_bind_env_impl, only implemented for closure");
    return -1;
  }

  fct.as_closure()._this = vm[1];
  return 0;
}

static int_t function_delegate_with_binded_this_impl(zs::vm_ref vm) {
  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_bind_env_impl, only implemented for closure");
    return -1;
  }
  return vm.push(fct.as_closure().copy_with_binded_this(vm[1]));
}

static int_t function_delegate_pcall_impl(zs::vm_ref vm) {

  int_t nargs = vm.stack_size();

  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_pcall_impl, only implemented for closure");
    return -1;
  }

  if (!fct.is_function()) {
    return vm.push_null();
  }

  const int_t stack_base = vm->stack().get_stack_base() + 1;

  // If nargs == 1, we need to push the root table, nargs wil be 1 after this.
  // If nargs > 1, nargs will be nargs - 1 after this.
  int_t npop = --nargs == 0 and ++nargs and vm.push_root() == 1;

  object ret;
  if (auto err = internal::proxy<tahjdfks>::call_closure(vm, fct, nargs, stack_base, ret)) {
    internal::proxy<tahjdfks>::clear_errors(vm);
    ret = nullptr;
  }

  vm->pop(npop);
  return vm.push(ret);
}

zs::object create_function_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  table_object& tbl = obj.as_table();

  tbl.reserve(6);
  tbl.emplace(zs::_ss("call"), function_delegate_call_impl);
  tbl.emplace(zs::_ss("pcall"), function_delegate_pcall_impl);
  tbl.emplace(zs::_s(eng, "with_binded_this"), function_delegate_with_binded_this_impl);
  tbl.emplace(zs::_ss("bind_this"), function_delegate_bind_env_impl);
  tbl.emplace(zs::_ss("get_this"), function_delegate_this_impl);

  tbl.emplace(zs::_s(eng, "get_parameter_count"), function_delegate_get_parameter_count_impl);
  tbl.emplace(zs::_s(eng, "get_parameter_names"), function_delegate_get_parameter_names_impl);
  tbl.emplace(zs::_s(eng, "get_default_params"), function_delegate_get_default_params_impl);

  tbl.set_delegate(object::create_none());
  tbl.set_use_default_delegate(false);
  return obj;
}

} // namespace zs.
