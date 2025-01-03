

#include "bytecode/zopcode.h"

namespace zs {

using vm_t = virtual_machine;
using inst_it_t = instruction_iterator;

template <opcode Op>
using cinst_t = const instruction_t<Op>&;

template <opcode Op>
errc vm_t::exec_op(zs::inst_it_t& it, exec_op_data_t& op_data) {
  zb::print("unimplemented operation", it.get_opcode());
  return errc::unimplemented;
}

//
// MARK: Load.
//

// op_load_char.
template <>
errc vm_t::exec_op<op_load_char>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_char> inst = it;
  _stack[inst.target_idx] = object::create_char(inst.value);
  return errc::success;
}

// op_load_int.
template <>
errc vm_t::exec_op<op_load_int>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_int> inst = it;
  _stack[inst.target_idx] = inst.value;
  return errc::success;
}

// op_load_float.
template <>
errc vm_t::exec_op<op_load_float>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_float> inst = it;
  _stack[inst.target_idx] = inst.value;
  return errc::success;
}

// op_load_bool.
template <>
errc vm_t::exec_op<op_load_bool>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_bool> inst = it;
  _stack[inst.target_idx] = inst.value;
  return errc::success;
}

// op_load_small_string.
template <>
errc vm_t::exec_op<op_load_small_string>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_small_string> inst = it;
  _stack[inst.target_idx] = inst.value.get_small_string();
  return zs::error_code::success;
}

// op_load_string.
template <>
errc vm_t::exec_op<op_load_string>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_string> inst = it;
  _stack[inst.target_idx] = op_data.fct->_literals[inst.idx];
  return errc::success;
}

// op_load.
template <>
errc vm_t::exec_op<op_load>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load> inst = it;
  _stack[inst.target_idx] = op_data.fct->_literals[inst.idx];
  return errc::success;
}

// op_load_null.
template <>
errc vm_t::exec_op<op_load_null>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_null> inst = it;
  _stack[inst.target_idx].reset();
  return errc::success;
}

// op_load_none.
template <>
errc vm_t::exec_op<op_load_none>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_none> inst = it;
  _stack[inst.target_idx] = zs::none();
  return errc::success;
}

// op_load_root.
template <>
errc vm_t::exec_op<op_load_root>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_root> inst = it;
  _stack[inst.target_idx] = op_data.closure->_root;
  return errc::success;
}

// op_load_global.
template <>
errc vm_t::exec_op<op_load_global>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_global> inst = it;

  ZS_ASSERT(!_call_stack.empty());
  const object& closure = _call_stack.back().closure;

  ZS_ASSERT(closure.is_closure());

  _stack[inst.target_idx] = closure.as_closure()._root;
  //  _stack[inst.target_idx] = _global_table;
  return errc::success;
}

// op_load_lib_ss.
template <>
errc vm_t::exec_op<op_load_lib_ss>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_lib_ss> inst = it;
  object& target = _stack[inst.target_idx];
  return this->get(_global_table, inst.key.get_small_string(), target);
}

//
// MARK: Stack operations.
//

// op_move.
template <>
errc vm_t::exec_op<op_move>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_move> inst = it;
  _stack[inst.target_idx] = _stack[inst.value_idx];
  return errc::success;
}

// op_assign.
template <>
errc vm_t::exec_op<op_assign>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_assign> inst = it;
  _stack[inst.target_idx] = _stack[inst.value_idx];
  return errc::success;
}

//
// MARK: Conditional.
//

// op_jz.
template <>
errc vm_t::exec_op<op_jz>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_jz> inst = it;
  it.data_ptr_ref() += !_stack[inst.value_idx].is_if_true() ? inst.offset : zs::get_instruction_size<op_jz>();
  return errc::success;
}

// op_jmp.
template <>
errc vm_t::exec_op<op_jmp>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_jmp> inst = it;
  it.data_ptr_ref() += inst.offset;
  return errc::success;
}

// op_if_not.
template <>
errc vm_t::exec_op<op_if_not>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_if_not> inst = it;

  const object& obj = _stack[inst.value_idx];
  _stack[inst.target_idx] = obj;

  it.data_ptr_ref() += !((inst.null_only and obj.is_null_or_none())
                           or not(inst.null_only or obj.is_double_question_mark_true()))
      ? inst.offset
      : zs::get_instruction_size<op_if_not>();

  return errc::success;
}

// op_and.
template <>
errc vm_t::exec_op<op_and>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_and> inst = it;
  bool_t src = _stack[inst.src_idx].is_if_true();
  _stack[inst.target_idx] = src;
  it.data_ptr_ref() += !src ? inst.offset : zs::get_instruction_size<op_and>();
  return errc::success;
}

// op_to_bool.
template <>
errc vm_t::exec_op<op_to_bool>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_to_bool> inst = it;
  _stack[inst.target_idx] = _stack[inst.value_idx].is_if_true();
  return errc::success;
}

// op_or.
template <>
errc vm_t::exec_op<op_or>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_or> inst = it;
  bool_t src = _stack[inst.src_idx].is_if_true();
  _stack[inst.target_idx] = src;
  it.data_ptr_ref() += src ? inst.offset : zs::get_instruction_size<op_or>();
  return errc::success;
}

// op_triple_or.
template <>
errc vm_t::exec_op<op_triple_or>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_triple_or> inst = it;
  const object& src = _stack[inst.src_idx];
  _stack[inst.target_idx] = src;
  it.data_ptr_ref() += src ? inst.offset : zs::get_instruction_size<op_triple_or>();
  return errc::success;
}

//
//
//

// op_get.
template <>
errc vm_t::exec_op<op_get>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_get> inst = it;

  object dst;
  const object tbl = _stack[inst.table_idx];
  const object key = _stack[inst.key_idx];

  if (auto err = this->get(tbl, key, dst)) {

    if (err == zs::error_code::not_found) {
      if (zb::has_flag(inst.flags, get_op_flags_t::gf_look_in_root)) {
        // TODO: Use closure's root.
        if (auto err = this->get(_global_table, key, dst)) {
          zb::print("-------dsljkdjjksadl", key);
          return err;
        }

        _stack[inst.target_idx] = dst;
        return zs::error_code::success;
      }

      set_error("Get failed in type: '", tbl.get_type(), "' with key: ", key, ".\n");
      //      zb::print(stack_size(),inst.target_idx, key);
      _stack[inst.target_idx].reset();
      return errc::inaccessible;
    }

    return err;
  }
  //    if (err == zs::error_code::not_found && (inst.flags & get_op_flags_t::gf_look_in_root) != 0) {
  //
  //      // TODO: Use closure's root.
  //      if (auto err = this->get(_root_table, key, dst)) {
  //        zb::print("-------dsljkdjjksadl", key);
  //        return err;
  //      }
  //
  //      zb::print("FOUND IN ROOT");
  //    }
  //    else {
  //      set_error("Get failed in type: '", tbl.get_type(), "' with key: ", key, ".\n");
  //      zb::print(stack_size(),inst.target_idx, key);
  //      _stack[inst.target_idx].reset();
  //      return err;
  //    }
  //  }

  _stack[inst.target_idx] = dst;
  return zs::error_code::success;
}

// op_set.
template <>
errc vm_t::exec_op<op_set>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_set> inst = it;

  object& tbl = _stack[inst.table_idx];
  const object& key = _stack[inst.key_idx];
  const object& value = _stack[inst.value_idx];
  //  zb::print("DSKLDKSLKDLKLDS", key, inst.can_create);
  zs::error_code err = inst.can_create ? this->set(tbl, key, value) : this->set_if_exists(tbl, key, value);

  if (inst.target_idx != k_invalid_target) {
    _stack[inst.target_idx] = value;
  }

  return err;
}

// op_set_ss.
template <>
errc vm_t::exec_op<op_set_ss>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_set_ss> inst = it;

  object& tbl = _stack[inst.table_idx];
  const object& value = _stack[inst.value_idx];

  zs::error_code err = inst.can_create ? this->set(tbl, inst.key.get_small_string(), value)
                                       : this->set_if_exists(tbl, inst.key.get_small_string(), value);

  //  zs::error_code err = this->set(tbl, inst.key.get_small_string(), value);

  if (inst.target_idx != (uint8_t)-1) {
    _stack[inst.target_idx] = value;
  }

  return err;
}

// op_rawset.
template <>
errc vm_t::exec_op<op_rawset>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_rawset> inst = it;

  object tbl = _stack[inst.table_idx];

  if (!tbl.is_table()) {
    return zs::error_code::invalid_type;
  }

  object key = _stack[inst.key_idx];
  object value = _stack[inst.value_idx];
  //  zb::print("DSKLDKSLKDLKLDS", key, inst.can_create);

  if (inst.target_idx != (uint8_t)-1) {
    _stack[inst.target_idx] = value;
  }

  if (inst.can_create) {
    return tbl.as_table().set(std::move(key), std::move(value));
  }

  //  return tbl.as_table().set_no_create(std::move(key), std::move(value));

  if (auto err = tbl.as_table().set_no_create(std::move(key), std::move(value))) {
    set_error("Can't create new element without 'this'.\n");
    return err;
  }

  return {};
  //  zs::error_code err = tbl.as_table().set(key, value);

  //  return err;
}

// op_rawset_ss.
template <>
errc vm_t::exec_op<op_rawset_ss>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_rawset_ss> inst = it;

  object& tbl = _stack[inst.table_idx];

  if (!tbl.is_table()) {
    return zs::error_code::invalid_type;
  }

  const object& value = _stack[inst.value_idx];

  zs::error_code err = tbl.as_table().set(inst.key.get_small_string(), value);

  if (inst.target_idx != (uint8_t)-1) {
    _stack[inst.target_idx] = value;
  }

  return err;
}

//
// MARK: Arithmetic.
//

// op_arith.
template <>
errc vm_t::exec_op<op_arith>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_arith> inst = it;
  return arithmetic_operation(inst.aop, _stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_arith_eq.
template <>
errc vm_t::exec_op<op_arith_eq>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_arith_eq> inst = it;
  object& target = _stack[inst.target_idx];
  return arithmetic_operation(inst.aop, target, target, _stack[inst.rhs_idx]);
}

// op_uarith.
template <>
errc vm_t::exec_op<op_uarith>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_uarith> inst = it;
  return unary_arithmetic_operation(inst.uop, _stack[inst.target_idx], _stack[inst.value_idx]);
}

// op_obj_arith_eq.
template <>
errc vm_t::exec_op<op_obj_arith_eq>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_obj_arith_eq> inst = it;
  object& obj = _stack[inst.obj_idx];
  const object& key = _stack[inst.key_idx];
  const object& rhs = _stack[inst.rhs_idx];

  object lhs;
  if (auto err = this->get(obj, key, lhs)) {

    if (err == errc::not_found) {
      // TODO: Use closure's root.
      object root_table = _global_table;
      ZS_RETURN_IF_ERROR(this->get(root_table, key, lhs));
    }
    else {
      return err;
    }
  }

  object target;
  if (auto err = arithmetic_operation(inst.aop, target, lhs, rhs)) {
    return err;
  }

  if (auto err = set(obj, key, target)) {
    return err;
  }

  if (inst.target_idx != k_invalid_target) {
    _stack[inst.target_idx] = target;
  }

  return errc::success;
}

// op_obj_uarith.
template <>
errc vm_t::exec_op<op_obj_uarith>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_obj_uarith> inst = it;

  object tbl = _stack[inst.table_ix];
  object key = _stack[inst.key_ix];
  object& target = _stack[inst.target_idx];

  object obj;

  if (auto err = this->get(tbl, key, obj)) {
    if (err == errc::not_found) {
      // TODO: Use closure's root.
      object root_table = _global_table;
      ZS_RETURN_IF_ERROR(this->get(root_table, key, obj));
    }
    else {
      return err;
    }
  }

  ZS_RETURN_IF_ERROR(unary_arithmetic_operation(inst.uop, target, obj));

  ZS_RETURN_IF_ERROR(this->set(tbl, key, obj));

  return errc::success;
}

// eq, // ==
// ne, // !=
// lt, // <
// le, // <=
// gt, // >
// ge, // >=
// compare, // <=>
inline constexpr zb::enum_array<object (*)(const object&), compare_op> ds
    = { [](const object& obj) -> object { return obj._int == 0; },
        [](const object& obj) -> object { return obj._int != 0; },
        [](const object& obj) -> object { return obj._int == -1; },
        [](const object& obj) -> object { return obj._int <= 0; },
        [](const object& obj) -> object { return obj._int == 1; },
        [](const object& obj) -> object { return obj._int >= 0; }, //
        [](const object& obj) { return obj; } };

// op_cmp.
template <>
errc vm_t::exec_op<op_cmp>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_cmp> inst = it;

  object result;
  if (auto err = this->compare(result, _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    return err;
  }

  ZS_ASSERT(result.is_integer());
  _stack[inst.target_idx] = ds[inst.cmp_op](result);
  return errc::success;
}

// op_strict_eq
template <>
errc vm_t::exec_op<op_strict_eq>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_strict_eq> inst = it;
  _stack[inst.target_idx] = _stack[inst.lhs_idx].strict_equal(_stack[inst.rhs_idx]);
  return {};
}

// op_close.
template <>
errc vm_t::exec_op<op_close>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_close> inst = it;
  ZS_TRACE("VM - CLOSE_CAPTURE");

  const object* stack_ptr = _stack.stack_base_pointer() + inst.stack_size;
  if (auto err = close_captures(stack_ptr)) {
    return err;
  }

  return zs::errc::success;
}

// op_get_capture.
template <>
errc vm_t::exec_op<op_get_capture>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_get_capture> inst = it;

  const zs::vector<zs::object>& closure_captured_values = op_data.closure->_captured_values;

  if (inst.idx >= closure_captured_values.size()) {
    return ZS_VM_ERROR(zs::errc::out_of_bounds, "Capture index out of bounds in op_get_capture.");
  }

  const object& value = closure_captured_values[inst.idx];
  if (capture::is_capture(value)) {
    _stack[inst.target_idx] = capture::as_capture(value).get_value();
  }
  else {
    _stack[inst.target_idx] = value;
  }

  return zs::errc::success;
}

// op_set_capture.
template <>
errc vm_t::exec_op<op_set_capture>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_set_capture> inst = it;

  const object& value = _stack[inst.value_idx];

  const zs::vector<zs::object>& closure_captured_values = op_data.closure->_captured_values;

  if (inst.capture_idx >= closure_captured_values.size()) {
    return ZS_VM_ERROR(zs::errc::out_of_bounds, "Capture index out of bounds in op_set_capture.");
  }

  *capture::as_capture(closure_captured_values[inst.capture_idx]).get_value_ptr() = value;

  if (inst.target_idx != k_invalid_target) {

    //    zb::print("ASASASA", closure_captured_values[inst.capture_idx].as_capture().get_value_ptr(), &
    //    _stack[inst.target_idx]);

    _stack[inst.target_idx] = value;
  }

  return {};
}

// op_return.
template <>
errc vm_t::exec_op<op_return>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_return> inst = it;

  ZS_TRACE("VM - op_return");

  if (inst.has_value) {
    this->push(_stack[inst.value_idx]);
    op_data.ret_value = _stack[inst.value_idx];
  }
  else {
    this->push_null();
    op_data.ret_value.reset();
  }

  //  const object* stack_ptr = _stack.get_internal_vector().data()+ _call_stack.back().previous_stack_base;
  //  if(auto err = runtime_action<rt_close_captures>(stack_ptr)) {
  //    return err;
  //  }

  //  call_info cinfo = _call_stack.get_pop_back();
  //  _stack.set_stack_base(cinfo.previous_stack_base);
  //
  //  if(cinfo.closure.is_closure()) {
  //    zs::closure_object& cobj = cinfo.closure.as_closure();
  //        zs::vector<zs::object>& closure_capture_values = cobj._capture_values;
  //    const object* stack_ptr = _stack.stack_base_pointer() ;//+cinfo.previous_top_index;
  //     for(object& cap : closure_capture_values) {
  //       if(!cap.as_capture().is_baked() and cap.as_capture().get_value_ptr() >= stack_ptr) {
  //         cap.as_capture().bake();
  //         zb::print(cinfo.closure.as_closure().get_proto()._name);
  //       }
  //     }
  //  }

  //  if(_call_stack.size() == 2) {
  //    zb::print("DSLKDSKDJSKLJDKSLJDS");
  return zs::error_code::returned;
  //  }

  //  return zs::error_code::success;
}

// ZS_DECL_EXEC_OP(return_export) {
//   ZS_INST(return_export);
//
//   object export_table = _stack[inst.idx];
//
//   //  zbase_assert(export_table.is_table(), "returned export table is not a table");
//
//   // Look for cycling references.
//   //  table_object& tbl = export_table.as_table();
//
//   //  for(auto it : tbl) {
//   //    zb::print(it);
//   //
//   //    if(it.second.is_closure()) {
//   //      zb::print("DSLKDAAAAA");
//   //
//   //      closure_object& closure_obj = it.second.as_closure();
//   //      zs::vector<zs::object>& captures = closure_obj._capture_values;
//   //
//   //      for(object& captured_obj : captures) {
//   //        if(captured_obj == export_table) {
//   //          zb::print("SAMASASMAMSMAS");
//   //          captured_obj = export_table.get_weak_ref();
//   //        }
//   //      }
//   //    }
//   //  }
//
//   this->push(export_table);
//   op_data.ret_value = export_table;
//
//   return zs::error_code::returned;
// }

// op_line.
template <>
errc vm_t::exec_op<op_line>(inst_it_t& it, exec_op_data_t& op_data) {
  [[maybe_unused]] cinst_t<op_line> inst = it;
  ZS_TODO("Implement");

  ZS_TRACE("op_line", inst.line);
  return zs::error_code::success;
}

// op_check_type.
template <>
errc vm_t::exec_op<op_check_type>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_check_type> inst = it;

  // Check if object at 'idx' is type same as 'inst.obj_type'.
  if (!_stack[inst.value_idx].is_type(inst.obj_type)) {

    ZS_VM_ERROR(errc::invalid_type_assignment, " wrong type ", _stack[inst.value_idx].get_type(),
        " expected ", inst.obj_type, "\n");

    return runtime_action<runtime_code::handle_error>(
        op_data.fct, it, zs::error_code::invalid_type_assignment);
  }

  return zs::error_code::success;
}

// op_assign_w_mask.
template <>
errc vm_t::exec_op<op_assign_w_mask>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_assign_w_mask> inst = it;

  // Check if object at 'idx' has type mask.
  if (!_stack[inst.value_idx].has_type_mask(inst.mask)) {

    return ZS_VM_ERROR(errc::invalid_type_assignment, "wrong type mask", _stack[inst.value_idx].get_type(),
        "expected mask", zs::object_type_mask_printer{ inst.mask }, "\n");
  }
  _stack[inst.target_idx] = _stack[inst.value_idx];

  return zs::error_code::success;
}

// op_assign_custom.
template <>
errc vm_t::exec_op<op_assign_custom>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_assign_custom> inst = it;

  // Check if object at 'idx' has type mask.
  if (!_stack[inst.value_idx].has_type_mask(inst.mask)) {

    return ZS_VM_ERROR(errc::invalid_type_assignment, "wrong type mask", _stack[inst.value_idx].get_type(),
        "expected mask", zs::object_type_mask_printer{ inst.mask }, "\n");
  }
  _stack[inst.target_idx] = _stack[inst.value_idx];

  return zs::error_code::success;
}

// op_check_type_mask.
template <>
errc vm_t::exec_op<op_check_type_mask>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_check_type_mask> inst = it;

  // Check if object at 'idx' has type mask.
  if (!_stack[inst.value_idx].has_type_mask(inst.mask)) {
    return ZS_VM_ERROR(errc::invalid_type_assignment, "wrong type mask", _stack[inst.value_idx].get_type(),
        "expected mask", zs::object_type_mask_printer{ inst.mask }, "\n");
  }

  return zs::error_code::success;
}

// op_check_custom_type_mask.
template <>
errc vm_t::exec_op<op_check_custom_type_mask>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_check_custom_type_mask> inst = it;

  // Check if object at 'idx' has type mask.
  if (!_stack[inst.value_idx].has_type_mask(inst.mask)) {
    return ZS_VM_ERROR(errc::invalid_type_assignment, "wrong type mask", _stack[inst.value_idx].get_type(),
        "expected mask", inst.mask, "\n");
  }

  // Check if object at 'idx' has type custom_mask.
  ZS_TODO("Implement custom type mask");

  return zs::error_code::success;
}

// op_new_obj.
template <>
errc vm_t::exec_op<op_new_obj>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_obj> inst = it;

  switch (inst.type) {
  case k_array:
    _stack[inst.target_idx] = object::create_array(_engine, 0);
    break;

  case k_table:
    _stack[inst.target_idx] = object::create_table(_engine);
    break;

  case k_struct:
    _stack[inst.target_idx] = object::create_struct(_engine);
    break;

  case k_atom: {
    object& target = _stack[inst.target_idx];
    target = nullptr;
    target._type = k_atom;
    target._atom_type = atom_type::atom_user;
    break;
  }
  default:
    return errc::invalid_type;
  }

  return errc::success;
}

// op_new_array.
template <>
errc vm_t::exec_op<op_new_array>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_array> inst = it;
  object arr = object::create_array(_engine, inst.sz);
  _stack[inst.target_idx] = std::move(arr);
  return errc::success;
}

// op_array_append.
template <>
errc vm_t::exec_op<op_array_append>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_array_append> inst = it;
  array_object* arr = _stack[inst.array_idx]._array;
  arr->push(_stack[inst.value_idx]);
  return zs::error_code::success;
}

// op_array_set.
template <>
errc vm_t::exec_op<op_array_set>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_array_set> inst = it;
  _stack[inst.array_idx].as_array()[inst.index] = _stack[inst.value_idx];
  return zs::error_code::success;
}

// op_new_slot.
template <>
errc vm_t::exec_op<op_new_slot>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_slot> inst = it;
  object& table = _stack[inst.table_idx];
  const object& key = _stack[inst.key_idx];
  const object& value = _stack[inst.value_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(key, value);
}

// op_new_slot_ss.
template <>
errc vm_t::exec_op<op_new_slot_ss>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_slot_ss> inst = it;
  const object& table = _stack[inst.table_idx];
  const object& value = _stack[inst.value_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), value);
}

// op_new_slot_ss_integer.
template <>
errc vm_t::exec_op<op_new_slot_ss_integer>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_slot_ss_integer> inst = it;
  const object& table = _stack[inst.table_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), inst.value);
}

// op_new_slot_ss_float.
template <>
errc vm_t::exec_op<op_new_slot_ss_float>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_slot_ss_float> inst = it;
  const object& table = _stack[inst.table_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), inst.value);
}

// op_new_slot_ss_bool.
template <>
errc vm_t::exec_op<op_new_slot_ss_bool>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_slot_ss_bool> inst = it;
  const object& table = _stack[inst.table_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), inst.value);
}

// op_new_slot_ss_small_string.
template <>
errc vm_t::exec_op<op_new_slot_ss_small_string>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_slot_ss_small_string> inst = it;
  const object& table = _stack[inst.table_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), inst.value.get_small_string());
}

// op_new_struct_slot.
template <>
errc vm_t::exec_op<op_new_struct_slot>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_struct_slot> inst = it;
  object& strct = _stack[inst.struct_idx];
  const object& key = _stack[inst.key_idx];

  //  object value = inst.has_value ?
  if (inst.value_idx != k_invalid_target) {
    return strct.as_struct().new_slot(key, _stack[inst.value_idx], inst.mask, inst.vdecl_flags);
  }

  return strct.as_struct().new_slot(key, inst.mask, inst.vdecl_flags);
}

// op_new_struct_slot_ss.
template <>
errc vm_t::exec_op<op_new_struct_slot_ss>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_struct_slot_ss> inst = it;
  object& strct = _stack[inst.struct_idx];
  ZS_ASSERT(strct.is_struct());

  object key = inst.key.get_small_string();

  if (inst.value_idx != k_invalid_target) {
    return strct.as_struct().new_slot(key, _stack[inst.value_idx], inst.mask, inst.vdecl_flags);
  }

  return strct.as_struct().new_slot(key, inst.mask, inst.vdecl_flags);
}

// op_new_struct_default_constructor.
template <>
errc vm_t::exec_op<op_new_struct_default_constructor>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_struct_default_constructor> inst = it;
  object& strct = _stack[inst.struct_idx];

  struct_object& strct_obj = strct.as_struct();
  strct_obj._has_default_constructor = true;

  return {};
}

// op_new_struct_constructor.
template <>
errc vm_t::exec_op<op_new_struct_constructor>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_struct_constructor> inst = it;

  object constructor;
  uint8_t bounded_target = k_invalid_target;
  if (auto err = this->new_closure(inst.fct_idx, bounded_target, constructor)) {
    return err;
  }

  object& strct = _stack[inst.struct_idx];
  struct_object& strct_obj = strct.as_struct();

  if (strct_obj._constructors.is_array()) {
    strct_obj._constructors.as_array().push(constructor);
  }
  else if (strct_obj._constructors.is_function()) {
    zs::object last_constructor = strct_obj._constructors;
    strct_obj._constructors = zs::_a(strct_obj.get_engine(), 2);
    strct_obj._constructors.as_array()[0] = std::move(last_constructor);
    strct_obj._constructors.as_array()[1] = constructor;
  }
  else {
    strct_obj._constructors = constructor;
  }

  return {};
}

// op_new_struct_method.
template <>
errc vm_t::exec_op<op_new_struct_method>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_struct_method> inst = it;

  object closure;
  if (auto err = this->new_closure(inst.fct_idx, k_invalid_target, closure)) {
    return err;
  }

  object& strct = _stack[inst.struct_idx];
  struct_object& strct_obj = strct.as_struct();

  bool is_private = zb::has_flag<variable_attribute_t::va_private>(inst.decl_flag);

  if (zb::has_flag<variable_attribute_t::va_static>(inst.decl_flag)) {
    return strct_obj.new_static_method(closure.as_closure().get_proto()._name, closure, is_private, false);
  }
  else {
    return strct_obj.new_method(closure.as_closure().get_proto()._name, closure, is_private, false);
  }
}

// op_set_struct_name.
template <>
errc vm_t::exec_op<op_set_struct_name>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_set_struct_name> inst = it;

  object& strct = _stack[inst.struct_idx];
  strct.as_struct().set_name(_stack[inst.name_idx]);
  return zs::errc::success;
}

// op_new_closure.
template <>
errc vm_t::exec_op<op_new_closure>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_new_closure> inst = it;
  return this->new_closure(inst.fct_idx, inst.bounded_target, _stack[inst.target_idx]);
}

// op_typeid.
template <>
errc vm_t::exec_op<op_typeid>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_typeid> inst = it;
  _stack[inst.target_idx] = zs::_ss(zs::get_exposed_object_type_name(_stack[inst.value_idx].get_type()));
  return zs::error_code::success;
}

// op_typeof.
template <>
errc vm_t::exec_op<op_typeof>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_typeof> inst = it;
  if (auto err = this->type_of(_stack[inst.value_idx], _stack[inst.target_idx])) {
    return err;
  }

  return errc::success;
}

// op_set_register.
template <>
errc vm_t::exec_op<op_set_register>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_set_register> inst = it;
  _registers[inst.reg_idx] = _stack[inst.value_idx];
  return zs::error_code::success;
}

// op_load_register.
template <>
errc vm_t::exec_op<op_load_register>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_load_register> inst = it;
  _stack[inst.target_idx] = _registers[inst.reg_idx];
  _registers[inst.reg_idx] = nullptr;
  return zs::error_code::success;
}

// op_call.
template <>
errc vm_t::exec_op<op_call>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_call> inst = it;

  object closure_obj = _stack[inst.closure_idx];
  object ret_value;

  ZS_RETURN_IF_ERROR(this->call(closure_obj, inst.n_params, inst.stack_base, ret_value, true));
  _stack[inst.target_idx] = ret_value;

  return zs::error_code::success;
}

// op_tail_call.
template <>
errc vm_t::exec_op<op_tail_call>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_tail_call> inst = it;

  object closure = _stack[inst.closure_idx];
  object ret_value;

  const object_type otype = closure.get_type();

  zbase_assert(zb::is_one_of(otype, k_closure), get_object_type_name(otype));

  ZS_RETURN_IF_ERROR(
      tail_call_closure(closure, _stack.get_relative_subspan(inst.stack_base, inst.n_params), ret_value));
  _stack[inst.target_idx] = ret_value;

  return zs::error_code::success;
}

// op_not.
template <>
errc vm_t::exec_op<op_not>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_not> inst = it;

  bool_t value = _stack[inst.value_idx].is_if_true();

  //  bool_t value = false;
  //
  //  if (auto err = _stack[inst.idx].convert_to_bool(value)) {
  //    _error_message += zs::strprint(_engine, "cannot convert to bool",
  //    _stack[inst.idx].get_type(), "\n"); return err;
  //  }

  _stack[inst.target_idx] = !value;
  return zs::error_code::success;
}

// op_obj_not.
template <>
errc vm_t::exec_op<op_obj_not>(inst_it_t& it, exec_op_data_t& op_data) {
  cinst_t<op_obj_not> inst = it;
  object dst;

  const object tbl = _stack[inst.table_idx];
  const object key = _stack[inst.key_idx];

  if (auto err = this->get(tbl, key, dst)) {

    if (err == zs::error_code::not_found) {

      // TODO: Use closure's root.
      if (auto err = this->get(_global_table, key, dst)) {
        zb::print("-------dsljkdjjksadl", key);
        return err;
      }
    }
    else {
      set_error("Get failed in type: '", tbl.get_type(), "' with key: ", key, ".\n");
      _stack[inst.target_idx].reset();
      return err;
    }
  }

  bool_t value = dst.is_if_true();

  //  bool_t value = false;
  //
  //  if (auto err = _stack[inst.idx].convert_to_bool(value)) {
  //    _error_message += zs::strprint(_engine, "cannot convert to bool",
  //    _stack[inst.idx].get_type(), "\n"); return err;
  //  }

  _stack[inst.target_idx] = !value;
  return zs::error_code::success;
}

} // namespace zs.
