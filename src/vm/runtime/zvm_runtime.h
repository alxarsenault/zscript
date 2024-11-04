namespace zs {

using objref_t = zb::ref_wrapper<object>;
using cobjref_t = zb::ref_wrapper<const object>;

#define ZS_DECL_RT_ACTION(name, ...) \
  template <>                        \
  zs::error_result virtual_machine::runtime_action<runtime_code::name>(__VA_ARGS__)

////////////////////////////////////////////////////////
ZS_DECL_RT_ACTION(execute, closure_object* closure, zb::ref_wrapper<object> ret_value);

ZS_DECL_RT_ACTION(
    handle_error, zs::function_prototype_object* fct, zs::instruction_iterator it, zs::error_code ec);

// clang-format off
ZS_DECL_RT_ACTION(enter_function_call, cobjref_t closure_obj, int_t n_params, int_t stack_base);
ZS_DECL_RT_ACTION(enter_function_call, cobjref_t closure_obj, zs::parameter_list params);
ZS_DECL_RT_ACTION(leave_function_call);
ZS_DECL_RT_ACTION(call_closure, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value);
ZS_DECL_RT_ACTION(call_closure, cobjref_t obj, zs::parameter_list params, objref_t ret_value);
ZS_DECL_RT_ACTION(call_native_closure, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value);
ZS_DECL_RT_ACTION(call_native_closure, cobjref_t obj, zs::parameter_list params, objref_t ret_value);
ZS_DECL_RT_ACTION(call_native_function, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value);
ZS_DECL_RT_ACTION(call_native_function, cobjref_t obj, zs::parameter_list params, objref_t ret_value);

// clang-format on

// ZS_DECL_RT_ACTION(invalid_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(invalid_set, objref_t obj, cobjref_t key, cobjref_t value);

// String.
//ZS_DECL_RT_ACTION(delegate_get_type_of, cobjref_t obj, cobjref_t delegate_obj, objref_t dest);
// ZS_DECL_RT_ACTION(delegate_get, cobjref_t obj, cobjref_t delegate_obj, cobjref_t key, objref_t dest);
//  ZS_DECL_RT_ACTION(table_get, cobjref_t obj, cobjref_t key, objref_t dest);
// ZS_DECL_RT_ACTION(table_contains, cobjref_t obj, cobjref_t key, objref_t dest);
//  ZS_DECL_RT_ACTION(array_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(array_set, objref_t obj, cobjref_t key, cobjref_t value);

// ZS_DECL_RT_ACTION(weak_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(weak_set, objref_t obj, cobjref_t key, cobjref_t value);

// Struct.
// ZS_DECL_RT_ACTION(struct_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(struct_set, objref_t obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(struct_new_slot, objref_t obj, cobjref_t key, cobjref_t value, uint32_t mask,
    bool is_static, bool is_private, bool is_const);
ZS_DECL_RT_ACTION(struct_new_slot, objref_t obj, cobjref_t key, uint32_t mask, bool is_static,
    bool is_private, bool is_const);
ZS_DECL_RT_ACTION(struct_new_constructor, objref_t obj, cobjref_t value);
ZS_DECL_RT_ACTION(struct_new_default_constructor, objref_t obj);
ZS_DECL_RT_ACTION(struct_new_method, objref_t obj, cobjref_t closure, variable_attribute_t decl_flags);
ZS_DECL_RT_ACTION(struct_call_create, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value);

// Struct instance.
// ZS_DECL_RT_ACTION(struct_instance_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(struct_instance_set, objref_t obj, cobjref_t key, cobjref_t value);

ZS_DECL_RT_ACTION(delegate_set, objref_t obj, objref_t delegate_obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(table_set, objref_t obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(table_set_if_exists, objref_t obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(user_data_set, objref_t user_data_obj, cobjref_t key, cobjref_t value);

ZS_DECL_RT_ACTION(atom_set, objref_t obj, cobjref_t key, cobjref_t value);

ZS_DECL_RT_ACTION(new_closure, uint32_t fct_idx, uint8_t bounded_target, objref_t dest);
ZS_DECL_RT_ACTION(rt_close_captures, const object* stack_ptr);

//
//
//

ZS_DECL_RT_ACTION(
    handle_error, zs::function_prototype_object* fct, zs::instruction_iterator it, zs::error_code ec) {

  size_t inst_byte_index = it.get_index(fct->_instructions);

  const line_info_op_t* last_linfo = nullptr;
  const line_info_op_t* linfo = nullptr;

  for (const line_info_op_t& line : fct->_line_info) {
    if (line.op_index <= (int_t)inst_byte_index) {
      last_linfo = linfo;
      linfo = &line;

      if (line.op_index == (int_t)inst_byte_index) {
        break;
      }
    }
    else {
      break;
    }
  }

  constexpr std::string_view pre = "\n      ";
  _error_message += zs::sstrprint(_engine, "error: opcode:", it.get_opcode(), //
      pre, "error_code:", zs::error_code_to_string(ec), //
      pre, "closure name:", fct->_name, //
      pre, "closure source name:", fct->_source_name, //
      pre, "instruction index:", it.get_instruction_index(fct->_instructions), //
      pre, "instruction byte index:", inst_byte_index, //
      pre, "stack base index:", _stack.get_stack_base(), //
      pre, "stack size:", _stack.stack_size(), //
      pre, "call stack index:", _call_stack.size(), //
      pre, "call stack previous stack base:", _call_stack.back().previous_stack_base, //
      pre, "call stack previous top index:", _call_stack.back().previous_top_index);

  if (last_linfo) {
    _error_message += zs::strprint(_engine, //
        pre, " line: [ ", last_linfo->line, " : ", last_linfo->column, " ]");
  }

  if (linfo) {
    _error_message += zs::strprint(_engine, //
        pre, " line end: [ ", linfo->line, " : ", linfo->column, " ]");
  }

  _error_message += "\n";

  return ec;
}

ZS_DECL_RT_ACTION(invalid_set, objref_t obj, cobjref_t key, cobjref_t value) {
  set_error("Can't assign a value to '", zs::get_object_type_name(obj->get_type()), "'.\n");
  return zs::error_code::inaccessible;
}

ZS_DECL_RT_ACTION(rt_close_captures, const object* stack_ptr) {
  ZS_TRACE("VM - rt_close_captures - CLOSE_CAPTURE", stack_ptr - _stack.get_internal_vector().data());

  for (auto it = _open_captures.begin(); it != _open_captures.end();) {
    if (capture::as_capture(*it).is_baked()) {
      it = _open_captures.erase(it);
      continue;
    }
    if (capture::as_capture(*it).get_value_ptr() >= stack_ptr) {
      capture::as_capture(*it).bake();
      it = _open_captures.erase(it);
    }
    else {
      ++it;
    }
  }
  return {};
}
} // namespace zs.

#include "vm/runtime/zvm_runtime_call.h"
#include "vm/runtime/zvm_runtime_table.h"
#include "vm/runtime/zvm_runtime_user_data.h"
#include "vm/runtime/zvm_runtime_weak.h"
#include "vm/runtime/zvm_runtime_array.h"
#include "vm/runtime/zvm_runtime_delegate.h"
#include "vm/runtime/zvm_runtime_atom.h"
#include "vm/runtime/zvm_runtime_struct.h"
#include "vm/runtime/zvm_runtime_closure.h"
