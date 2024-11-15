namespace zs {

enum class runtime_code {
  invalid_get,
  invalid_set,
  delegate_get,
  delegate_set,

  weak_get,
  weak_set,

  table_get,
  table_set,
  array_get,
  array_set,

  // Struct.
  struct_get,
  struct_set,
  struct_new_slot,
  struct_new_constructor,
  struct_call_create,

  // Struct instance.
  struct_instance_get,
  struct_instance_set,

  user_data_get,
  user_data_set,

  native_array_get,
  native_array_set,

  string_get,
  string_set,
  extension_get,
  extension_set,

  class_get,
  class_set,
  instance_get,

  color_get,
  //  array_iterator_get,
  //  array_iterator_set,

  // vm/zvm_runtime_call.h
  enter_function_call,
  leave_function_call,
  call_native_closure,
  call_native_function,
  call_native_function2,
  call_closure,

  handle_error,
  execute,
  delegate_get_type_of,
  call_arith_op,
  meta_arith,
};

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
ZS_DECL_RT_ACTION(enter_function_call, cobjref_t closure_obj, std::span<const object> params);
ZS_DECL_RT_ACTION(leave_function_call);
ZS_DECL_RT_ACTION(call_closure, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value);
ZS_DECL_RT_ACTION(call_closure, cobjref_t obj, std::span<const object> params, objref_t ret_value);
ZS_DECL_RT_ACTION(call_native_closure, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value);
ZS_DECL_RT_ACTION(call_native_closure, cobjref_t obj, std::span<const object> params, objref_t ret_value);
ZS_DECL_RT_ACTION(call_native_function, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value);
ZS_DECL_RT_ACTION(call_native_function, cobjref_t obj, std::span<const object> params, objref_t ret_value);

ZS_DECL_RT_ACTION(call_native_function2, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value);
ZS_DECL_RT_ACTION(call_native_function2, cobjref_t obj, std::span<const object> params, objref_t ret_value);
// clang-format on

ZS_DECL_RT_ACTION(invalid_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(invalid_set, objref_t obj, cobjref_t key, cobjref_t value);

// String.
ZS_DECL_RT_ACTION(string_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(string_set, objref_t obj, cobjref_t key, cobjref_t value);

ZS_DECL_RT_ACTION(delegate_get_type_of, cobjref_t obj, cobjref_t delegate_obj, objref_t dest);
ZS_DECL_RT_ACTION(delegate_get, cobjref_t obj, cobjref_t delegate_obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(class_get, cobjref_t obj, cobjref_t class_obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(table_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(array_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(array_set, objref_t obj, cobjref_t key, cobjref_t value);

ZS_DECL_RT_ACTION(weak_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(weak_set, objref_t obj, cobjref_t key, cobjref_t value);

// Struct.
ZS_DECL_RT_ACTION(struct_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(struct_set, objref_t obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(struct_new_slot, objref_t obj, cobjref_t key, cobjref_t value, uint32_t mask,
    bool is_static, bool is_const);
ZS_DECL_RT_ACTION(struct_new_slot, objref_t obj, cobjref_t key, uint32_t mask, bool is_static, bool is_const);
ZS_DECL_RT_ACTION(struct_new_constructor, objref_t obj, cobjref_t value);
ZS_DECL_RT_ACTION(struct_call_create, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value);

// Struct instance.
ZS_DECL_RT_ACTION(struct_instance_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(struct_instance_set, objref_t obj, cobjref_t key, cobjref_t value);

ZS_DECL_RT_ACTION(instance_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(delegate_set, objref_t obj, objref_t delegate_obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(table_set, objref_t obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(class_set, objref_t obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(user_data_set, objref_t user_data_obj, cobjref_t key, cobjref_t value);
ZS_DECL_RT_ACTION(meta_arith, meta_method mt, cobjref_t lhs, cobjref_t rhs, objref_t dest, objref_t del);
ZS_DECL_RT_ACTION(user_data_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(native_array_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(native_array_set, objref_t obj, cobjref_t key, cobjref_t value);

ZS_DECL_RT_ACTION(color_get, cobjref_t obj, cobjref_t key, objref_t dest);
// ZS_DECL_RT_ACTION(array_iterator_get, cobjref_t obj, cobjref_t key, objref_t dest);
// ZS_DECL_RT_ACTION(array_iterator_set, objref_t obj, cobjref_t key, cobjref_t value);

ZS_DECL_RT_ACTION(extension_get, cobjref_t obj, cobjref_t key, objref_t dest);
ZS_DECL_RT_ACTION(extension_set, objref_t obj, cobjref_t key, cobjref_t value);

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
  _error_message += zs::strprint(_engine, "error: opcode:", it.get_opcode(), //
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
    _error_message += zs::strprint<"">(_engine, //
        pre, " line: [ ", last_linfo->line, " : ", last_linfo->column, " ]");
  }

  if (linfo) {
    _error_message += zs::strprint<"">(_engine, //
        pre, " line end: [ ", linfo->line, " : ", linfo->column, " ]");
  }

  _error_message += "\n";

  return ec;
}

ZS_DECL_RT_ACTION(invalid_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  set_error("Can't get a value from '", zs::get_object_type_name(obj->get_type()), "'.\n");
  dest->reset();
  return zs::error_code::inaccessible;
}

ZS_DECL_RT_ACTION(invalid_set, objref_t obj, cobjref_t key, cobjref_t value) {
  set_error("Can't assign a value to '", zs::get_object_type_name(obj->get_type()), "'.\n");
  return zs::error_code::inaccessible;
}

ZS_DECL_RT_ACTION(color_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  zs::object delegate_obj = _default_color_delegate;
  return runtime_action<runtime_code::table_get>(CREF(delegate_obj), key, dest);
}

ZS_DECL_RT_ACTION(meta_arith, meta_method mt, cobjref_t lhs, cobjref_t rhs, objref_t dest, objref_t del) {
  //  zs::table_object* del_table = lhs->get_delegate();
  //
  zs::table_object& del_table = del->as_table();

  object meta_oper;
  //
  // Look for typeof operator in the delegate table.
  if (!del_table.get(zs::_s(_engine, meta_method_name(mt)), meta_oper)) {
    // The meta typeof was found in the delegate table.

    // If it is a function, let's call it.
    if (meta_oper.is_function()) {
      push(lhs);
      push(rhs);
      push(del);
      if (auto err = call(meta_oper, 3, stack_size() - 3, dest)) {
        pop(3);
        return err;
      }

      pop(3);
      //      if (!dest->is_string()) {
      //        zb::print("Invalid typeof operator return type (should be a
      //        string)"); return zs::error_code::invalid_type;
      //      }
      return {};
    }

    // If it's a string, we return that string.
    //    else if (meta_oper.is_string()) {
    //      dest.get() = meta_oper;
    //      return {};
    //    }

    else {
      zb::print("Invalid typeof operator");
      return zs::error_code::invalid_type;
    }
  }

  // There was no operator typeof in this meta table.
  // Let's look deeper into delegates.
  if (del_table.has_delegate()) {
    object& sub_delegate_obj = del_table.get_delegate();
    if (!runtime_action<runtime_code::meta_arith>(mt, lhs, rhs, dest, REF(sub_delegate_obj))) {
      return {};
    }
  }

  return zs::error_code::not_found;
}

//
// MARK: - Native array.
//

ZS_DECL_RT_ACTION(native_array_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  ZS_ASSERT(obj->is_native_array());

  // If the key is a number, we access the element directly.
  if (key->is_number()) {
    const int_t index = key->convert_to_integer_unchecked();
    return obj->as_native_array_interface().get(index, dest);
  }

  // If the native array has a valid delegate, we'll go look in there first.
  if (zs::table_object* delegate = obj->get_delegate()) {
    return zs::error_code::unimplemented;
  }

  // The key wasn't found in the delegate, let's look in the default delegate.
  zs::object delegate_obj = _default_native_array_delegate;
  return runtime_action<runtime_code::delegate_get>(obj, CREF(delegate_obj), key, dest);

  //  return runtime_action<runtime_code::table_get>(CREF(delegate_obj), key, dest);
}

ZS_DECL_RT_ACTION(native_array_set, objref_t obj, cobjref_t key, cobjref_t value) {
  ZS_ASSERT(obj->is_native_array());

  if (!key->is_number()) {
    return zs::error_code::unimplemented;
  }

  const int_t index = key->convert_to_integer_unchecked();
  return obj->as_native_array_interface().set(index, value);
}
} // namespace zs.

#include "vm/zvm_runtime_string.h"
#include "vm/zvm_runtime_call.h"
#include "vm/zvm_runtime_table.h"
#include "vm/zvm_runtime_class.h"
#include "vm/zvm_runtime_user_data.h"
#include "vm/zvm_runtime_weak.h"
#include "vm/zvm_runtime_array.h"
#include "vm/zvm_runtime_delegate.h"
#include "vm/zvm_runtime_instance.h"
#include "vm/zvm_runtime_extension.h"
#include "vm/zvm_runtime_struct.h"
