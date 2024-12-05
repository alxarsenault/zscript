

namespace zs {
using enum opcode;

template <opcode Op>
zs::error_code virtual_machine::exec_op(zs::instruction_iterator& it, exec_op_data_t& op_data) {
  zb::print("unimplemented operation", it.get_opcode());
  return zs::error_code::unimplemented;
}

#define ZS_INST(name) \
  const zs::instruction_t<ZS_OPCODE_ENUM_VALUE(name)>& inst = it.get_ref<ZS_OPCODE_ENUM_VALUE(name)>()

#define ZS_DECL_EXEC_OP(name)                                          \
  template <>                                                          \
  zs::error_code virtual_machine::exec_op<ZS_OPCODE_ENUM_VALUE(name)>( \
      zs::instruction_iterator & it, exec_op_data_t & op_data)

// op_load_root.
ZS_DECL_EXEC_OP(load_root) {
  ZS_INST(load_root);
  _stack[inst.target_idx] = op_data.closure->_root;
  return zs::error_code::success;
}

// op_load_global.
ZS_DECL_EXEC_OP(load_global) {
  ZS_INST(load_global);
  _stack[inst.target_idx] = _root_table;
  return zs::error_code::success;
}

// op_load_int.
ZS_DECL_EXEC_OP(load_int) {
  ZS_INST(load_int);
  _stack[inst.target_idx] = inst.value;
  return zs::error_code::success;
}

// op_load_float.
ZS_DECL_EXEC_OP(load_float) {
  ZS_INST(load_float);
  _stack[inst.target_idx] = inst.value;
  return zs::error_code::success;
}

// op_load_bool.
ZS_DECL_EXEC_OP(load_bool) {
  ZS_INST(load_bool);
  _stack[inst.target_idx] = inst.value;
  return zs::error_code::success;
}

ZS_DECL_EXEC_OP(load_small_string) {
  ZS_INST(load_small_string);
  _stack[inst.target_idx] = inst.value.get_small_string();
  return zs::error_code::success;
}

// op_load_string.
ZS_DECL_EXEC_OP(load_string) {
  ZS_INST(load_string);
  _stack[inst.target_idx] = op_data.fct->_literals[inst.idx];
  return zs::error_code::success;
}

// op_load.
ZS_DECL_EXEC_OP(load) {
  ZS_INST(load);
  _stack[inst.target_idx] = op_data.fct->_literals[inst.idx];
  return zs::error_code::success;
}

// op_load_null.
ZS_DECL_EXEC_OP(load_null) {
  ZS_INST(load_null);
  _stack[inst.target_idx].reset();
  return zs::error_code::success;
}

// op_load_none.
ZS_DECL_EXEC_OP(load_none) {
  ZS_INST(load_none);
  _stack[inst.target_idx] = object::create_none();
  return zs::error_code::success;
}

// op_set_meta_argument.
ZS_DECL_EXEC_OP(set_meta_argument) {
  ZS_INST(set_meta_argument);
  _stack[inst.idx]._flags = object_flags_t::f_meta_argument;
  return zs::error_code::success;
}

// op_move.
ZS_DECL_EXEC_OP(move) {
  ZS_INST(move);

  //  zb::print("OP_MOVE=", _stack[inst.idx], "FROM ", (int)inst.idx, " TO ", (int)inst.target_idx, " STACK
  //  BASE ", _stack.get_stack_base());

  //  if(_stack[inst.idx] == 69) {
  //    zb::print("MOVE 609");
  //  }
  _stack[inst.target_idx] = _stack[inst.idx];

  return zs::error_code::success;
}
// op_assign.
ZS_DECL_EXEC_OP(assign) {
  ZS_INST(assign);

  //  zb::print("OP_MOVE=", _stack[inst.idx], "FROM ", (int)inst.idx, " TO ", (int)inst.target_idx, " STACK
  //  BASE ", _stack.get_stack_base());

  //  if(_stack[inst.idx] == 69) {
  //    zb::print("MOVE 609");
  //  }
  _stack[inst.target_idx] = _stack[inst.idx];

  return zs::errc::success;
}

// op_get.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_get>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_get>& inst = it.get_ref<op_get>();

  object dst;
  const object tbl = _stack[inst.table_idx];
  const object key = _stack[inst.key_idx];

  if (auto err = this->get(tbl, key, dst)) {

    if (err == zs::error_code::not_found && (inst.flags & get_op_flags_t::gf_look_in_root) != 0) {

      // TODO: Use closure's root.
      if (auto err = this->get(_root_table, key, dst)) {
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

  _stack[inst.target_idx] = dst;
  return zs::error_code::success;
}

// op_load_lib_ss.
template <>
zs::error_code virtual_machine::exec_op<op_load_lib_ss>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_load_lib_ss>& inst = it.get_ref<op_load_lib_ss>();

  object& target = _stack[inst.target_idx];
  return this->get(_global_table, inst.key.get_small_string(), target);
}

// op_set.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_set>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_set>& inst = it.get_ref<op_set>();

  object& tbl = _stack[inst.table_idx];
  const object& key = _stack[inst.key_idx];
  const object& value = _stack[inst.value_idx];
  //  zb::print("DSKLDKSLKDLKLDS", key, inst.can_create);
  zs::error_code err = inst.can_create ? this->set(tbl, key, value) : this->set_if_exists(tbl, key, value);

  if (inst.target_idx != (uint8_t)-1) {
    _stack[inst.target_idx] = value;
  }

  return err;
}

// op_set_ss.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_set_ss>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_set_ss>& inst = it.get_ref<op_set_ss>();

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
//
//// op_rawset.
// template <>
// zs::error_code virtual_machine::exec_op<opcode::op_rawset>(
//     zs::instruction_iterator& it, exec_op_data_t& op_data) {
//   const zs::instruction_t<op_rawset>& inst = it.get_ref<op_rawset>();
//
//   object& tbl = _stack[inst.table_idx];
//
//   if (!tbl.is_table()) {
//     return zs::error_code::invalid_type;
//   }
//
//   const object& key = _stack[inst.key_idx];
//   const object value = _stack[inst.value_idx];
//   zb::print("DSKLDKSLKDLKLDS", key, inst.can_create);
//
//   if(inst.can_create) {
//
//   }
//   zs::error_code err = tbl.as_table().set(key, value);
//
//   if (inst.target_idx != (uint8_t)-1) {
//     _stack[inst.target_idx] = value;
//   }
//
//   return err;
// }

// op_rawset.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_rawset>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_rawset>& inst = it.get_ref<op_rawset>();

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
zs::error_code virtual_machine::exec_op<opcode::op_rawset_ss>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_rawset_ss>& inst = it.get_ref<op_rawset_ss>();

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

// op_object_add_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_object_add_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_object_add_eq>& inst = it.get_ref<op_object_add_eq>();

  object& obj = _stack[inst.obj_idx];
  const object& key = _stack[inst.key_idx];
  const object& value = _stack[inst.value_idx];
  object dest;
  //  zs::error_code err = this->set(obj, key, value);

  if (auto err = this->add_eq(obj, key, value, dest)) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }

  if (inst.target_idx != (uint8_t)-1) {
    _stack[inst.target_idx] = dest;
  }

  return errc::success;
}

// op_object_mul_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_object_mul_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_object_mul_eq>& inst = it.get_ref<op_object_mul_eq>();

  object& obj = _stack[inst.obj_idx];
  const object& key = _stack[inst.key_idx];
  const object& value = _stack[inst.value_idx];
  object dest;

  if (auto err = this->mul_eq(obj, key, value, dest)) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }

  if (inst.target_idx != (uint8_t)-1) {
    _stack[inst.target_idx] = dest;
  }

  return errc::success;
}

// op_close.
template <>
zs::error_code virtual_machine::exec_op<op_close>(zs::instruction_iterator& it, exec_op_data_t& op_data) {

  const zs::instruction_t<op_close>& inst = it.get_ref<op_close>();

  ZS_TRACE("VM - CLOSE_CAPTURE");
  //  zs::vector<zs::object>& closure_capture_values =
  //  _call_stack.back().closure.as_closure()._capture_values;// op_data.closure->_capture_values;
  const object* stack_ptr = _stack.stack_base_pointer() + inst.stack_size;
  if (auto err = runtime_action<rt_close_captures>(stack_ptr)) {
    return err;
  }

  return zs::errc::success;
}

// op_get_capture.
// ZS_DECL_EXEC_OP(get_capture)

template <>
zs::error_code virtual_machine::exec_op<op_get_capture>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_get_capture>& inst = it.get_ref<op_get_capture>();

  const zs::vector<zs::object>& closure_captured_values = op_data.closure->_captured_values;

  if (inst.idx >= closure_captured_values.size()) {
    return ZS_VM_ERROR(zs::errc::out_of_bounds, "Capture index out of bounds in op_get_capture.");
  }

  const object& value = closure_captured_values[inst.idx];
  if (value.is_capture()) {
    _stack[inst.target_idx] = value.as_capture().get_value();
  }
  else {
    _stack[inst.target_idx] = value;
  }

  return zs::errc::success;
}

// op_set_capture.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_set_capture>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_set_capture>& inst = it.get_ref<op_set_capture>();

  const object& value = _stack[inst.value_idx];

  const zs::vector<zs::object>& closure_captured_values = op_data.closure->_captured_values;

  if (inst.capture_idx >= closure_captured_values.size()) {
    return ZS_VM_ERROR(zs::errc::out_of_bounds, "Capture index out of bounds in op_set_capture.");
  }

  *(closure_captured_values[inst.capture_idx].as_capture().get_value_ptr()) = value;

  if (inst.target_idx != k_invalid_target) {

    //    zb::print("ASASASA", closure_captured_values[inst.capture_idx].as_capture().get_value_ptr(), &
    //    _stack[inst.target_idx]);

    _stack[inst.target_idx] = value;
  }

  return {};
}

// op_arith.
// template <>
// zs::error_code virtual_machine::exec_op<opcode::op_arith>(
//    zs::instruction_iterator& it, exec_op_data_t& op_data) {
//  const zs::instruction_t<op_arith>& inst = it.get_ref<op_arith>();
//
//  return runtime_arith_operation(
//      inst.aop, _stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
//}

// op_add.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_add>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_add>();
  return add(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_sub.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_sub>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_sub>();
  return sub(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_mul.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_mul>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_mul>();

  return mul(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_div.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_div>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_div>();
  return div(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_mod.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_mod>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_mod>();
  return mod(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_exp.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_exp>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_exp>();
  return exp(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_bitwise_or.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_bitwise_or>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_bitwise_or>();
  return bitwise_or(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_bitwise_and.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_bitwise_and>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_bitwise_and>();
  return bitwise_and(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_bitwise_xor.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_bitwise_xor>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_bitwise_xor>();
  return bitwise_xor(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_lshift.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_lshift>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_lshift>();
  return lshift(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_rshift.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_rshift>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_rshift>();
  return rshift(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

//
//
//

// op_add_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_add_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_add_eq>();
  if (auto err = add_eq(_stack[inst.target_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err.code;
  }
  return zs::error_code::success;
}

// op_mul_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_mul_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_mul_eq>& inst = it.get_ref<op_mul_eq>();
  //  zb::print("OPMULEQ", inst);
  if (auto err = mul_eq(_stack[inst.target_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err.code;
  }
  return zs::error_code::success;
}

// op_sub_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_sub_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_sub_eq>& inst = it.get_ref<op_sub_eq>();
  if (auto err = sub_eq(_stack[inst.target_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err.code;
  }
  return zs::error_code::success;
}

// op_div_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_div_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_div_eq>& inst = it.get_ref<op_div_eq>();
  if (auto err = div_eq(_stack[inst.target_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err.code;
  }
  return zs::error_code::success;
}

// op_mod_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_mod_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_mod_eq>& inst = it.get_ref<op_mod_eq>();
  if (auto err = mod_eq(_stack[inst.target_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err.code;
  }
  return zs::error_code::success;
}

// op_exp_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_exp_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_exp_eq>& inst = it.get_ref<op_exp_eq>();
  if (auto err = exp_eq(_stack[inst.target_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err.code;
  }
  return zs::error_code::success;
}

// op_return.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_return>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_return>& inst = it.get_ref<op_return>();

  ZS_TRACE("VM - op_return");

  if (inst.has_value) {
    this->push(_stack[inst.idx]);
    op_data.ret_value = _stack[inst.idx];
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

ZS_DECL_EXEC_OP(return_export) {
  ZS_INST(return_export);

  object export_table = _stack[inst.idx];

  //  zbase_assert(export_table.is_table(), "returned export table is not a table");

  // Look for cycling references.
  //  table_object& tbl = export_table.as_table();

  //  for(auto it : tbl) {
  //    zb::print(it);
  //
  //    if(it.second.is_closure()) {
  //      zb::print("DSLKDAAAAA");
  //
  //      closure_object& closure_obj = it.second.as_closure();
  //      zs::vector<zs::object>& captures = closure_obj._capture_values;
  //
  //      for(object& captured_obj : captures) {
  //        if(captured_obj == export_table) {
  //          zb::print("SAMASASMAMSMAS");
  //          captured_obj = export_table.get_weak_ref();
  //        }
  //      }
  //    }
  //  }

  this->push(export_table);
  op_data.ret_value = export_table;

  return zs::error_code::returned;
}

// op_line.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_line>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  ZS_TODO("Implement");
  const zs::instruction_t<op_line>& inst = it.get_ref<op_line>();

  ZS_TRACE("op_line", inst.line);
  return zs::error_code::success;
}

// op_check_type.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_check_type>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_check_type>& inst = it.get_ref<op_check_type>();

  // Check if object at 'idx' is type same as 'inst.obj_type'.
  if (!_stack[inst.idx].is_type(inst.obj_type)) {
    //    zb::print("ERROR wrong type", _stack[inst.idx].get_type(), "expected",
    //    inst.obj_type);
    _error_message
        += zs::sstrprint(_engine, "wrong type", _stack[inst.idx].get_type(), "expected", inst.obj_type, "\n");
    //    helper::handle_error(this, op_data.fct, it,
    //    zs::error_code::invalid_value_type_assignment);
    return runtime_action<runtime_code::handle_error>(
        op_data.fct, it, zs::error_code::invalid_type_assignment);
    //    return zs::error_code::invalid_value_type_assignment;
  }

  return zs::error_code::success;
}

// op_assign_w_mask.
ZS_DECL_EXEC_OP(assign_w_mask) {
  ZS_INST(assign_w_mask);
  // Check if object at 'idx' has type mask.
  if (!_stack[inst.idx].has_type_mask(inst.mask)) {
    //    zb::print("ERROR wrong type", _stack[inst.idx].get_type(), "expected
    //    mask", inst.mask);
    _error_message += zs::sstrprint(_engine, "wrong type mask", _stack[inst.idx].get_type(), "expected",
        zs::object_type_mask_printer{ inst.mask }, "\n");

    return zs::error_code::invalid_type_assignment;
  }
  _stack[inst.target_idx] = _stack[inst.idx];

  return zs::error_code::success;
}
// op_assign_custom.
ZS_DECL_EXEC_OP(assign_custom) {
  ZS_INST(assign_custom);
  // Check if object at 'idx' has type mask.
  if (!_stack[inst.idx].has_type_mask(inst.mask)) {
    //    zb::print("ERROR wrong type", _stack[inst.idx].get_type(), "expected
    //    mask", inst.mask);
    _error_message += zs::sstrprint(_engine, "wrong type mask", _stack[inst.idx].get_type(), "expected",
        zs::object_type_mask_printer{ inst.mask }, "\n");

    return zs::error_code::invalid_type_assignment;
  }
  _stack[inst.target_idx] = _stack[inst.idx];

  return zs::error_code::success;
}
// op_check_type_mask.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_check_type_mask>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_check_type_mask>& inst = it.get_ref<op_check_type_mask>();

  // Check if object at 'idx' has type mask.
  if (!_stack[inst.idx].has_type_mask(inst.mask)) {
    //    zb::print("ERROR wrong type", _stack[inst.idx].get_type(), "expected
    //    mask", inst.mask);
    _error_message += zs::sstrprint(_engine, "wrong type mask", _stack[inst.idx].get_type(), "expected",
        zs::object_type_mask_printer{ inst.mask }, "\n");

    return zs::error_code::invalid_type_assignment;
  }

  return zs::error_code::success;
}

// op_check_custom_type_mask.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_check_custom_type_mask>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_check_custom_type_mask>& inst = it.get_ref<op_check_custom_type_mask>();
  // Check if object at 'idx' has type mask.
  if (!_stack[inst.idx].has_type_mask(inst.mask)) {
    //    zb::print("ERROR wrong type", _stack[inst.idx].get_type(), "expected
    //    mask", inst.mask);
    _error_message += zs::strprint(
        _engine, "wrong type mask", _stack[inst.idx].get_type(), "expected mask", inst.mask, "\n");

    return zs::error_code::invalid_type_assignment;
  }

  // Check if object at 'idx' has type custom_mask.
  ZS_TODO("Implement custom type mask");

  return zs::error_code::success;
}

// op_new_obj.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_new_obj>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_new_obj>& inst = it.get_ref<op_new_obj>();
  switch (inst.type) {
  case object_type::k_array:
    _stack[inst.target_idx] = object::create_array(_engine, 0);
    break;

  case object_type::k_table:
    _stack[inst.target_idx] = object::create_table(_engine);
    break;

  case object_type::k_class:
    _stack[inst.target_idx] = object::create_class(_engine);
    break;
  case object_type::k_struct:
    _stack[inst.target_idx] = object::create_struct(_engine);
    break;

  default:
    return zs::error_code::invalid_type;
  }

  return zs::error_code::success;
}

// op_array_append.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_array_append>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_array_append>& inst = it.get_ref<op_array_append>();
  array_object* arr = _stack[inst.array_idx]._array;
  arr->push(_stack[inst.idx]);
  return zs::error_code::success;
}

// op_get_base.
ZS_DECL_EXEC_OP(get_base) {
  ZS_INST(get_base);

  if (zs::object base = op_data.closure->_base; !base.is_null()) {
    _stack[inst.target_idx] = base;
    return zs::error_code::success;
  }

  //  SQClosure* clo = _closure(ci->_closure);
  //         if (clo->_base) {
  //           TARGET = clo->_base;
  //         }
  return zs::error_code::inaccessible;
}

// op_new_slot.
ZS_DECL_EXEC_OP(new_slot) {
  ZS_INST(new_slot);
  object& table = _stack[inst.table_idx];
  const object& key = _stack[inst.key_idx];
  const object& value = _stack[inst.value_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(key, value);
}

// op_new_slot_ss.
ZS_DECL_EXEC_OP(new_slot_ss) {
  ZS_INST(new_slot_ss);
  const object& table = _stack[inst.table_idx];
  const object& value = _stack[inst.value_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), value);
}

// op_new_slot_ss_integer.
ZS_DECL_EXEC_OP(new_slot_ss_integer) {
  ZS_INST(new_slot_ss_integer);
  const object& table = _stack[inst.table_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), inst.value);
}

// op_new_slot_ss_float.
ZS_DECL_EXEC_OP(new_slot_ss_float) {
  ZS_INST(new_slot_ss_float);
  const object& table = _stack[inst.table_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), inst.value);
}

// op_new_slot_ss_bool.
ZS_DECL_EXEC_OP(new_slot_ss_bool) {
  ZS_INST(new_slot_ss_bool);
  const object& table = _stack[inst.table_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), inst.value);
}

// op_new_slot_ss_small_string.
ZS_DECL_EXEC_OP(new_slot_ss_small_string) {
  ZS_INST(new_slot_ss_small_string);
  const object& table = _stack[inst.table_idx];
  ZS_ASSERT(table.is_table());
  return table.as_table().set(inst.key.get_small_string(), inst.value.get_small_string());
}

//// op_new_struct_slot.
// ZS_DECL_EXEC_OP(new_struct_slot) {
//   ZS_INST(new_struct_slot);
//   object& strct = _stack[inst.struct_idx];
//   const object& key = _stack[inst.key_idx];
//
//   //  object value = inst.has_value ?
//   if (inst.value_idx != k_invalid_target) {
//     return strct.as_struct().new_slot(
//         key, _stack[inst.value_idx], inst.mask, inst.is_static, inst.is_private, inst.is_const);
//   }
//
//   return strct.as_struct().new_slot(key, inst.mask, inst.is_static, inst.is_private, inst.is_const);
// }

//// op_new_struct_slot.
// ZS_DECL_EXEC_OP(new_struct_slot) {
//   ZS_INST(new_struct_slot);
//   object& strct = _stack[inst.struct_idx];
//   const object& key = _stack[inst.key_idx];
//
//   //  object value = inst.has_value ?
//   if (inst.has_value) {
//     return strct.as_struct().new_slot(
//         key, _stack[inst.value_idx], inst.mask, inst.is_static, inst.is_private, inst.is_const);
//   }
//
//   return strct.as_struct().new_slot(key, inst.mask, inst.is_static, inst.is_private, inst.is_const);
// }

// op_new_struct_slot.
ZS_DECL_EXEC_OP(new_struct_slot) {
  ZS_INST(new_struct_slot);
  object& strct = _stack[inst.struct_idx];
  const object& key = _stack[inst.key_idx];

  //  object value = inst.has_value ?
  if (inst.value_idx != k_invalid_target) {
    return strct.as_struct().new_slot(key, _stack[inst.value_idx], inst.mask, inst.vdecl_flags);
  }

  return strct.as_struct().new_slot(key, inst.mask, inst.vdecl_flags);
}

// op_new_struct_slot_ss.
ZS_DECL_EXEC_OP(new_struct_slot_ss) {
  ZS_INST(new_struct_slot_ss);
  object& strct = _stack[inst.struct_idx];
  ZS_ASSERT(strct.is_struct());

  object key = inst.key.get_small_string();

  if (inst.value_idx != k_invalid_target) {
    return strct.as_struct().new_slot(key, _stack[inst.value_idx], inst.mask, inst.vdecl_flags);
  }

  return strct.as_struct().new_slot(key, inst.mask, inst.vdecl_flags);
}

// op_new_struct_default_constructor.
ZS_DECL_EXEC_OP(new_struct_default_constructor) {
  ZS_INST(new_struct_default_constructor);
  object& strct = _stack[inst.struct_idx];
  return runtime_action<runtime_code::struct_new_default_constructor>(zb::wref(strct));
}

// op_new_struct_constructor.
ZS_DECL_EXEC_OP(new_struct_constructor) {
  ZS_INST(new_struct_constructor);

  object closure;
  uint8_t bounded_target = k_invalid_target;
  if (auto err = runtime_action<runtime_code::new_closure>(inst.fct_idx, bounded_target, zb::wref(closure))) {
    return err;
  }

  object& strct = _stack[inst.struct_idx];
  return runtime_action<runtime_code::struct_new_constructor>(zb::wref(strct), zb::wcref(closure));
}

// op_new_struct_method.
ZS_DECL_EXEC_OP(new_struct_method) {
  ZS_INST(new_struct_method);

  object closure;
  if (auto err = runtime_action<runtime_code::new_closure>(inst.fct_idx, (uint8_t)-1, zb::wref(closure))) {
    return err;
  }

  object& strct = _stack[inst.struct_idx];
  //  const object& name = _stack[inst.key_idx];
  return runtime_action<runtime_code::struct_new_method>(zb::wref(strct), zb::wcref(closure), inst.decl_flag);
}

// op_set_struct_name.
ZS_DECL_EXEC_OP(set_struct_name) {
  ZS_INST(set_struct_name);

  object& strct = _stack[inst.struct_idx];
  strct.as_struct().set_name(_stack[inst.name_idx]);
  return zs::errc::success;
}

// op_set_struct_doc.
ZS_DECL_EXEC_OP(set_struct_doc) {
  ZS_INST(set_struct_doc);

  object& strct = _stack[inst.struct_idx];
  //  zb::print("STRUCT DOC", _stack[inst.doc_idx]);
  strct.as_struct().set_doc(_stack[inst.doc_idx]);
  return zs::errc::success;
}

// op_set_struct_member_doc.
ZS_DECL_EXEC_OP(set_struct_member_doc) {
  ZS_INST(set_struct_member_doc);

  object& strct = _stack[inst.struct_idx];

  strct.as_struct().set_member_doc(_stack[inst.name_idx], _stack[inst.doc_idx]);

  //  zb::print("STRUCT MEMBER DOC", _stack[inst.name_idx], _stack[inst.doc_idx]);
  //  strct.as_struct().set_doc(_stack[inst.doc_idx]);
  return zs::errc::success;
}

//// op_new_struct_method_ss.
// ZS_DECL_EXEC_OP(new_struct_method_ss) {
//   ZS_INST(new_struct_method_ss);
//
//   object closure;
//   if (auto err = runtime_action<runtime_code::new_closure>(inst.fct_idx, (uint8_t)-1, zb::wref(closure))) {
//     return err;
//   }
//
//   object& strct = _stack[inst.struct_idx];
//   object name = inst.key.get_small_string();
//   return runtime_action<runtime_code::struct_new_method>(zb::wref(strct), zb::wcref(name),
//   zb::wcref(closure));
// }

///// .
// #d efine ZS_INSTRUCTION_NEW_STRUCT_METHOD(X) \
//  X(u8, struct_idx)                       \
//  X(u8, key_idx)                          \
//  X(u32, fct_idx)
// ZS_DECL_OPCODE(new_struct_method, ZS_INSTRUCTION_NEW_STRUCT_METHOD)
//
///// op_new_struct_method_ss
// #d efine ZS_INSTRUCTION_NEW_STRUCT_METHOD_SS(X) \
//  X(u8, struct_idx)                          \
//  X(ss_inst_data, key)                       \
//  X(u32, fct_idx)
// ZS_DECL_OPCODE(new_struct_method_ss, ZS_INSTRUCTION_NEW_STRUCT_METHOD_SS)
//

//// op_new_closure.
// ZS_DECL_EXEC_OP(new_closure) {
//   ZS_INST(new_closure);
//   return
// }
//  op_new_class_slot.
ZS_DECL_EXEC_OP(new_class_slot) {
  ZS_INST(new_class_slot);

  object& cls = _stack[inst.table_idx];
  object& key = _stack[inst.key_idx];
  object& value = _stack[inst.value_idx];
  //          table_object* tbl = object_proxy::as_table(s[inst.table_idx]);

  //  zb::print("DSDJADHJSHDHHSAKJHDHHsssssss", table.get_type(), value.get_type());

  if (value.is_closure()) {
    value.as_closure()._base = cls.get_weak_ref();

    //    value.as_closure().
  }

  if (auto err = runtime_action<runtime_code::class_set>(zb::wref(cls), zb::wcref(key), zb::wcref(value))) {
    return err;
  }
  //  if (auto err = helper::table_set(this, table, key, value)) {
  //    return err;
  //  }

  return zs::error_code::success;
}

// op_new_closure.
ZS_DECL_EXEC_OP(new_closure) {
  ZS_INST(new_closure);
  return runtime_action<runtime_code::new_closure>(
      inst.fct_idx, inst.bounded_target, zb::wref(_stack[inst.target_idx]));
}

// op_typeid.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_typeid>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_typeid>& inst = it.get_ref<op_typeid>();
  _stack[inst.target_idx]
      = object::create_small_string(zs::get_exposed_object_type_name(_stack[inst.idx].get_type()));
  return zs::error_code::success;
}

// op_typeof.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_typeof>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_typeof>& inst = it.get_ref<op_typeof>();
  if (auto err = this->type_of(_stack[inst.idx], _stack[inst.target_idx])) {
    return err;
  }

  return zs::error_code::success;
}

// op_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_eq>& inst = it.get_ref<op_eq>();

  ZS_TODO("Implement a proper compare function in virtual_machine")
  _stack[inst.target_idx] = _stack[inst.lhs_idx] == _stack[inst.rhs_idx];

  return zs::error_code::success;
}

// op_ne.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_ne>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_ne>& inst = it.get_ref<op_ne>();

  _stack[inst.target_idx] = _stack[inst.lhs_idx] != _stack[inst.rhs_idx];

  return zs::error_code::success;
}

// op_call.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_call>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const auto& inst = it.get_ref<op_call>();

  object closure_obj = _stack[inst.closure_idx];
  object ret_value;

  ZS_RETURN_IF_ERROR(this->call(closure_obj, inst.n_params, inst.stack_base, ret_value, true));
  _stack[inst.target_idx] = ret_value;

  return zs::error_code::success;
}

// op_incr.
ZS_DECL_EXEC_OP(incr) {
  ZS_INST(incr);
  return runtime_arith_operation(
      inst.is_incr ? arithmetic_uop::incr : arithmetic_uop::decr, _stack[inst.target_idx], _stack[inst.idx]);
}

// op_pincr.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_pincr>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_pincr>& inst = it.get_ref<op_pincr>();
  return runtime_arith_operation(inst.is_incr ? arithmetic_uop::pincr : arithmetic_uop::pdecr,
      _stack[inst.target_idx], _stack[inst.idx]);
}

// op_pobjincr.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_pobjincr>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_pobjincr>& inst = it.get_ref<op_pobjincr>();

  object tbl = _stack[inst.table_ix];
  object key = _stack[inst.key_ix];

  object val;
  if (auto err = this->get(tbl, key, val)) {

    if (err == zs::error_code::not_found) {
      // TODO: Use closure's root.
      object root_table = _root_table;
      ZS_RETURN_IF_ERROR(this->get(root_table, key, val));
    }
    else {
      return err;
    }
  }

  ZS_RETURN_IF_ERROR(runtime_arith_operation(
      inst.is_incr ? arithmetic_uop::pincr : arithmetic_uop::pdecr, _stack[inst.target_idx], val));
  ZS_RETURN_IF_ERROR(this->set(tbl, key, val));

  return zs::error_code::success;
}

// op_use.
ZS_DECL_EXEC_OP(use) {
  ZS_INST(use);

  const object& tbl = _stack[inst.target_idx];
  const object& src = _stack[inst.src_idx];

  if (tbl.is_table() and src.is_table()) {
    for (auto t : src.as_table()) {
      tbl.as_table().insert(t);
    }
  }

  return {};
}

// op_if_null.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_if_null>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_if_null>& inst = it.get_ref<op_if_null>();

  const object& obj = _stack[inst.value_idx];

  if ((inst.null_only and obj.is_null_or_none())
      or not(inst.null_only or obj.is_double_question_mark_true())) {
    ++it;
  }
  else {
    it = op_data.get_instruction(op_data.get_iterator_index(it) + inst.offset);
  }

  _stack[inst.target_idx] = obj;
  return zs::error_code::success;
}

// op_jz.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_jz>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_jz>& inst = it.get_ref<op_jz>();

  //  bool_t value = false;

  const object& obj = _stack[inst.idx];

  //  zb::print("op_jz", inst.idx, obj, inst.offset);

  bool_t value = obj.is_if_true();

  if (value) {
    ++it;
  }
  else {
    zs::instruction_vector& inst_vec = op_data.instructions();
    size_t current_inst_index = inst_vec.get_iterator_index(it);
    int_t dest_inst_index = int_t(current_inst_index) + inst.offset;

    //    zb::print("op_jz", inst.idx, obj, inst.offset, ZBASE_VNAME(current_inst_index),
    //    ZBASE_VNAME(dest_inst_index));
    it = inst_vec[dest_inst_index];
  }

  return zs::error_code::success;

  //  switch (obj.get_type()) {
  //  case object_type::k_null:
  //  case object_type::k_none:
  //    value = false;
  //    break;
  //
  //  case object_type::k_bool:
  //    value = obj_bool;
  //    break;
  //
  //  case object_type::k_integer:
  //    value = (bool)obj._int;
  //    break;
  //
  //  case object_type::k_float:
  //    value = (bool)obj._float;
  //    break;
  //
  //  case object_type::k_small_string:
  //    value = !obj.get_small_string_unchecked().empty();
  //    break;
  //
  //  case object_type::k_string_view:
  //    value = !obj.get_string_view_unchecked().empty();
  //    break;
  //
  //  case object_type::k_long_string:
  //    value = !obj.get_long_string_unchecked().empty();
  //    break;
  //
  //  default:
  //    // TODO: Compare other types.
  //    if (auto err = _stack[inst.idx].convert_to_bool(value)) {
  //      _error_message += zs::strprint(_engine, "cannot convert to bool",
  //      _stack[inst.idx].get_type(),
  //      "\n"); return err;
  //    }
  //  }
}

// op_jmp.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_jmp>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_jmp>& inst = it.get_ref<op_jmp>();

  zs::instruction_vector& inst_vec = op_data.instructions();
  size_t current_inst_index = inst_vec.get_iterator_index(it);
  int_t jmp_to_inst_index = int_t(current_inst_index) + inst.offset;

  // TODO: Jump to the one before? Since it will be incremented?

  //  zb::print("op_jmp", ZBASE_VNAME(current_inst_index), ZBASE_VNAME(jmp_to_inst_index),
  //  ZBASE_VNAME(inst.offset));
  it = inst_vec[jmp_to_inst_index];

  return zs::error_code::success;
}

// op_cmp.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_cmp>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_cmp>& inst = it.get_ref<op_cmp>();

  //  zb::print("op_cmp", _stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx],
  //  ZBASE_VNAME(inst.target_idx), ZBASE_VNAME(inst.lhs_idx),ZBASE_VNAME(inst.rhs_idx));

  //  zb::print("op_cmp", inst.target_idx, inst.lhs_idx, inst.rhs_idx, _stack[inst.lhs_idx] ,
  //  _stack[inst.rhs_idx]);
  switch (inst.cmp_op) {
  case compare_op::lt:
    _stack[inst.target_idx] = _stack[inst.lhs_idx] < _stack[inst.rhs_idx];
    break;

  case compare_op::gt:
    _stack[inst.target_idx] = _stack[inst.lhs_idx] > _stack[inst.rhs_idx];
    break;

  case compare_op::le:
    _stack[inst.target_idx] = _stack[inst.lhs_idx] <= _stack[inst.rhs_idx];
    break;

  case compare_op::ge:
    _stack[inst.target_idx] = _stack[inst.lhs_idx] >= _stack[inst.rhs_idx];
    break;

  case compare_op::tw:
    _stack[inst.target_idx] = _stack[inst.lhs_idx].strict_equal(_stack[inst.rhs_idx]);
    break;

  case compare_op::double_arrow:
    _stack[inst.target_idx] = _stack[inst.lhs_idx].strict_equal(_stack[inst.rhs_idx]);
    break;

  case compare_op::double_arrow_eq:
    _stack[inst.target_idx] = _stack[inst.lhs_idx].strict_equal(_stack[inst.rhs_idx]);
    break;
  }

  //  zb::print("op_cmp_after", _stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx],
  //  ZBASE_VNAME(inst.target_idx), ZBASE_VNAME(inst.lhs_idx),ZBASE_VNAME(inst.rhs_idx));

  return zs::error_code::success;
}

// op_not.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_not>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_not>& inst = it.get_ref<op_not>();

  bool_t value = _stack[inst.idx].is_if_true();

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
zs::error_code virtual_machine::exec_op<opcode::op_obj_not>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_obj_not>& inst = it.get_ref<op_obj_not>();
  object dst;

  const object tbl = _stack[inst.table_idx];
  const object key = _stack[inst.key_idx];

  if (auto err = this->get(tbl, key, dst)) {

    if (err == zs::error_code::not_found) {

      // TODO: Use closure's root.
      if (auto err = this->get(_root_table, key, dst)) {
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

// op_and.
template <>
zs::error_code virtual_machine::exec_op<op_and>(zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_and>& inst = it.get_ref<op_and>();

  bool_t src = _stack[inst.src_idx].is_if_true();

  if (!src) {
    zs::instruction_vector& inst_vec = op_data.instructions();
    size_t current_inst_index = inst_vec.get_iterator_index(it);
    int_t jmp_to_inst_index = int_t(current_inst_index) + inst.offset;

    it = inst_vec[jmp_to_inst_index];
  }
  else {
    ++it;
  }

  _stack[inst.target_idx] = src;
  return zs::errc::success;
}

// op_or.
template <>
zs::error_code virtual_machine::exec_op<op_or>(zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_or>& inst = it.get_ref<op_or>();

  bool_t src = _stack[inst.src_idx].is_if_true();

  if (src) {
    zs::instruction_vector& inst_vec = op_data.instructions();
    size_t current_inst_index = inst_vec.get_iterator_index(it);
    int_t jmp_to_inst_index = int_t(current_inst_index) + inst.offset;

    it = inst_vec[jmp_to_inst_index];
  }
  else {
    ++it;
  }

  _stack[inst.target_idx] = src;
  return zs::errc::success;
}

// op_close_enum.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_close_enum>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_close_enum>& inst = it.get_ref<op_close_enum>();
  return make_enum_table(_stack[inst.idx]);
}

// op_new_enum_slot.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_new_enum_slot>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_new_enum_slot>& inst = it.get_ref<op_new_enum_slot>();

  object& table = _stack[inst.table_idx];
  const object& key = _stack[inst.key_idx];
  object& value = _stack[inst.value_idx];

  if (!key.is_string()) {
    set_error(
        "Enum keys can only be strings, key : '", key, "' is a '", key.get_type(), "' which is not valid.\n");
    return zs::error_code::invalid_type;
  }

  object& counter = table.as_table()[k_enum_counter_name];
  if (!counter.is_integer()) {
    counter = 0;
  }

  switch (value.get_type()) {
  case object_type::k_none:
    value = counter;
    counter = counter._int + 1;
    break;

  case object_type::k_integer:
    counter = value._int + 1;
    break;

  case object_type::k_string_view:
  case object_type::k_long_string:
  case object_type::k_small_string:
  case object_type::k_mutable_string:
  case object_type::k_float:
  case object_type::k_bool:
    break;

  default:
    set_error("Enum can only contain integers, floats, bools and strings, field : ", key, " is a '",
        value.get_type(), "' which is not valid.\n");
    return zs::error_code::invalid_type;
  }

  if (!table.as_table().emplace(key, value).second) {
    set_error("Duplicated enum key : ", key, ".\n");
    return zs::error_code::invalid_type;
  }

  object& enum_array = table.as_table()[k_enum_array_name];
  if (!enum_array.is_array()) {
    enum_array = zs::object::create_array(_engine, 0);
  }
  enum_array.as_array().push_back(key);

  return zs::error_code::success;
}

} // namespace zs.
