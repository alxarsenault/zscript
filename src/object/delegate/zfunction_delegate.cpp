#include "zfunction_delegate.h"
#include "zvirtual_machine.h"
#include "object/zfunction_prototype.h"
#include <zscript/base/strings/unicode.h>
#include <zscript/base/utility/scoped.h>

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
    if (auto err = vm->call(fct, { vm->global() }, ret)) {
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
    //    vm->_error_message.clear();
    vm->_errors.clear();
  }

  static inline error_result call_closure(
      vm_ref vm, const object& cobj, int_t n_params, int_t stack_base, object& ret_value) {
    return vm->call_closure(cobj, vm->stack().get_relative_subspan(stack_base, n_params), ret_value, true);
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

static int_t function_delegate_get_stack_size_impl(zs::vm_ref vm) {
  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_get_stack_size_impl, only implemented for closure");
    return -1;
  }

  return vm.push(fct.as_closure().get_proto()._stack_size);
}

static int_t function_delegate_instructions_size_impl(zs::vm_ref vm) {
  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_instructions_size_impl, only implemented for closure");
    return -1;
  }

  return vm.push(fct.as_closure().get_proto()._instructions._data.size());
}
template <class T>
static object create_inst_object(zs::engine* eng, const T& t) {
  if constexpr (std::is_integral_v<T> or std::is_floating_point_v<T>) {
    return object(t);
  }
  else if constexpr (std::is_enum_v<T>) {
    return zs::_s(eng, zb::enum_name(t));
  }
  else if constexpr (std::is_same_v<std::remove_cvref_t<small_string_instruction_data>, T>) {
    return t.get_small_string();
  }
  else {
    return object();
  }
}
static int_t function_delegate_print_instructions_impl(zs::vm_ref vm) {
  const object& fct = vm[0];

  if (!fct.is_closure()) {
    vm->set_error("Error in function_delegate_print_instructions_impl, only implemented for closure");
    return -1;
  }

  //  fct.as_closure().get_proto().debug_print();

  zs::engine* eng = vm.get_engine();
  const auto& insts = fct.as_closure().get_proto()._instructions;
  //  size_t index = 0;

  object arr = _a(vm, 0);
  array_object& aobj = arr.as_array();
#define __dddZS_INSTRUCTION_PRINT_TYPE(type, Name) \
  { zs::_s(eng, ZBASE_STRINGIFY(Name)), create_inst_object(eng, it.get_ref<opname>().Name) },
  for (auto it = insts.begin(); it != insts.end(); ++it) {
    switch (it.get_opcode()) {
#define ZS_DECL_OPCODE(name, INST_TYPES)                                                               \
  case ZS_OPCODE_ENUM_VALUE(name): {                                                                   \
    const auto opname = ZS_OPCODE_ENUM_VALUE(name);                                                    \
    aobj.push_back(zs::_t(eng,                                                                         \
        { { _ss("name"), zs::_s(eng, ZBASE_STRINGIFY(name)) }, { _ss("offset"), it.get_index(insts) }, \
            { _ss("size"), zs::get_instruction_size<opname>() },                                       \
            INST_TYPES(__dddZS_INSTRUCTION_PRINT_TYPE) }));                                            \
    break;                                                                                             \
  }

#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE

    default:
      //      zb::stream_print<" ">(stream, zs::opcode_to_string(it.get_opcode()), "\n");
    }
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
  int_t npop = --nargs == 0 and ++nargs and vm.push_global() == 1;

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
  tbl.emplace(zs::_s(eng, "get_stack_size"), function_delegate_get_stack_size_impl);
  tbl.emplace(zs::_s(eng, "get_instructions_size"), function_delegate_instructions_size_impl);
  tbl.emplace(zs::_s(eng, "print_instructions"), function_delegate_print_instructions_impl);

  tbl.set_no_default_none();
  return obj;
}

} // namespace zs.
