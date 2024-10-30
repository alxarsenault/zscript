

namespace zs {
template <opcode Op>
zs::error_code virtual_machine::exec_op(zs::instruction_iterator& it, exec_op_data_t& op_data) {
  zb::print("unimplemented operation", it.get_opcode());
  return zs::error_code::unimplemented;
}

#define ZS_INST(name) const zs::instruction_t<opcode::op_##name>& inst = it.get_ref<opcode::op_##name>()

#define ZS_DECL_EXEC_OP(name)                                 \
  template <>                                                 \
  zs::error_code virtual_machine::exec_op<opcode::op_##name>( \
      zs::instruction_iterator & it, exec_op_data_t & op_data)

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

// op_load_small_string.
ZS_DECL_EXEC_OP(load_small_string) {
  ZS_INST(load_small_string);
  const char* sbuffer = (const char*)&inst.value_1;
  _stack[inst.target_idx] = zs::_ss(std::string_view(sbuffer, std::strlen(sbuffer)));
  return zs::error_code::success;
}

// op_load_string.
ZS_DECL_EXEC_OP(load_string) {
  ZS_INST(load_string);
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
  _stack[inst.idx]._flags = object_flags_t::meta_argument;
  return zs::error_code::success;
}

// op_load.
ZS_DECL_EXEC_OP(load) {
  ZS_INST(load);
  _stack[inst.target_idx] = op_data.fct->_literals[inst.idx];
  return zs::error_code::success;
}

// op_move.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_move>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_move>& inst = it.get_ref<op_move>();
  _stack[inst.target_idx] = _stack[inst.idx];
  return zs::error_code::success;
}

// op_get.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_get>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_get>& inst = it.get_ref<op_get>();

  object dst;
  const object& tbl = _stack[inst.table_idx];
  const object& key = _stack[inst.key_idx];
  //  zb::print(tbl, key);
  if (auto err = this->get(tbl, key, dst)) {

    if (err == zs::error_code::not_found && inst.look_in_root) {
      // TODO: Use closure's root.
      ZS_RETURN_IF_ERROR(this->get(_root_table, key, dst));
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

// op_set.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_set>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_set>& inst = it.get_ref<op_set>();

  object& tbl = _stack[inst.table_idx];
  const object& key = _stack[inst.key_idx];
  const object& value = _stack[inst.value_idx];
  return this->set(tbl, key, value);
}

// op_get_capture.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_get_capture>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_get_capture>& inst = it.get_ref<op_get_capture>();
  const zs::vector<zs::object>& closure_capture_values = op_data.closure->_capture_values;

  if (inst.idx >= closure_capture_values.size()) {
    _error_message += zs::strprint(_engine, "op_get_capture could not find capture\n");
    return zs::error_code::out_of_bounds;
  }

  _stack[inst.target_idx] = closure_capture_values[inst.idx];
  return zs::error_code::success;
}

// op_arith.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_arith>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_arith>& inst = it.get_ref<op_arith>();

  return runtime_arith_operation(
      inst.aop, _stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
}

// op_add.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_add>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_add>& inst = it.get_ref<op_add>();

  //  zb::print("op_add", _stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx],
  //  ZBASE_VNAME(inst.target_idx), ZBASE_VNAME(inst.lhs_idx),ZBASE_VNAME(inst.rhs_idx));

  if (auto err = add(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }

  //  zb::print("op_add_after", _stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx]);
  return zs::error_code::success;
}

// op_sub.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_sub>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_sub>& inst = it.get_ref<op_sub>();
  if (auto err = sub(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", __FILE__, __LINE__);
    return err;
  }
  return zs::error_code::success;
}

// op_mul.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_mul>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_mul>& inst = it.get_ref<op_mul>();
  if (auto err = mul(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", __FILE__, __LINE__);
    return err;
  }
  return zs::error_code::success;
}

// op_div.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_div>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_div>& inst = it.get_ref<op_div>();
  if (auto err = div(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }
  return zs::error_code::success;
}

// op_exp.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_exp>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_exp>& inst = it.get_ref<op_exp>();
  if (auto err = exp(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }
  return zs::error_code::success;
}

// op_mod.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_mod>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_mod>& inst = it.get_ref<op_mod>();
  if (auto err = mod(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }
  return zs::error_code::success;
}

// op_bitwise_or.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_bitwise_or>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_bitwise_or>& inst = it.get_ref<op_bitwise_or>();
  if (auto err = bitwise_or(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }

  return zs::error_code::success;
}

// op_bitwise_and.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_bitwise_and>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_bitwise_and>& inst = it.get_ref<op_bitwise_and>();
  if (auto err = bitwise_and(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }

  return zs::error_code::success;
}

// op_bitwise_xor.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_bitwise_xor>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_bitwise_xor>& inst = it.get_ref<op_bitwise_xor>();
  if (auto err = bitwise_xor(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }

  return zs::error_code::success;
}

// op_lshift.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_lshift>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_lshift>& inst = it.get_ref<op_lshift>();
  if (auto err = lshift(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }
  return zs::error_code::success;
}

// op_rshift.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_rshift>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_rshift>& inst = it.get_ref<op_rshift>();
  if (auto err = rshift(_stack[inst.target_idx], _stack[inst.lhs_idx], _stack[inst.rhs_idx])) {
    zb::print("ERROR", err, __FILE__, __LINE__);
    return err;
  }
  return zs::error_code::success;
}

//
//
//

// op_add_eq.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_add_eq>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_add_eq>& inst = it.get_ref<op_add_eq>();
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

  if (inst.has_value) {
    this->push(_stack[inst.idx]);
    op_data.ret_value = _stack[inst.idx];
  }
  else {
    this->push_null();
    op_data.ret_value.reset();
  }

  //  if(_call_stack.size() == 2) {
  //    zb::print("DSLKDSKDJSKLJDKSLJDS");
  return zs::error_code::returned;
  //  }

  //  return zs::error_code::success;
}

// op_line.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_line>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  ZS_TODO("Implement");
  //  const zs::instruction_t<op_line>& inst = it.get_ref<op_line>();

  //  zb::print(inst.line, __FILE_NAME__, __LINE__);
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
        += zs::strprint(_engine, "wrong type", _stack[inst.idx].get_type(), "expected", inst.obj_type, "\n");
    //    helper::handle_error(this, op_data.fct, it,
    //    zs::error_code::invalid_value_type_assignment);
    return runtime_action<runtime_code::handle_error>(
        op_data.fct, it, zs::error_code::invalid_value_type_assignment);
    //    return zs::error_code::invalid_value_type_assignment;
  }

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
    _error_message += zs::strprint(_engine, "wrong type mask", _stack[inst.idx].get_type(), "expected",
        zs::object_type_mask_printer{ inst.mask }, "\n");

    return zs::error_code::invalid_value_type_assignment;
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

    return zs::error_code::invalid_value_type_assignment;
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
template <>
zs::error_code virtual_machine::exec_op<opcode::op_new_slot>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_new_slot>& inst = it.get_ref<op_new_slot>();

  object& table = _stack[inst.table_idx];
  object& key = _stack[inst.key_idx];
  object& value = _stack[inst.value_idx];

  if (auto err = runtime_action<runtime_code::table_set>(zb::wref(table), zb::wcref(key), zb::wcref(value))) {
    return err;
  }
  //  }

  //  if (auto err = helper::table_set(this, table, key, value)) {
  //    return err;
  //  }

  return zs::error_code::success;
}

// op_new_struct_slot.
ZS_DECL_EXEC_OP(new_struct_slot) {
  ZS_INST(new_struct_slot);
  object& table = _stack[inst.table_idx];
  object& key = _stack[inst.key_idx];

  if (inst.has_value) {
    object value = _stack[inst.value_idx];
    return runtime_action<runtime_code::struct_new_slot>(
        zb::wref(table), zb::wcref(key), zb::wcref(value), inst.mask, inst.is_static, inst.is_const);
  }

  return runtime_action<runtime_code::struct_new_slot>(
      zb::wref(table), zb::wcref(key), inst.mask, inst.is_static, inst.is_const);
}

// op_new_struct_constructor.
ZS_DECL_EXEC_OP(new_struct_constructor) {
  ZS_INST(new_struct_constructor);
  object& table = _stack[inst.table_idx];
  object value = _stack[inst.value_idx];
  return runtime_action<runtime_code::struct_new_constructor>(zb::wref(table), zb::wcref(value));
}

// op_new_class_slot.
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
template <>
zs::error_code virtual_machine::exec_op<opcode::op_new_closure>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  const zs::instruction_t<op_new_closure>& inst = it.get_ref<op_new_closure>();

  // Current function call info.
  const call_info& cinfo = _call_stack.back();

  // Get the current closure.
  const zs::object& current_closure = cinfo.closure;
  if (!current_closure.is_closure()) {
    return zs::error_code::invalid_type;
  }

  // Get the current closure function prototype.
  const zs::object& current_closure_fct_proto = current_closure._closure->_function;
  if (!current_closure_fct_proto.is_function_prototype()) {
    return zs::error_code::invalid_type;
  }

  // We want to create a closure object with the function prototype at index
  // `inst.fct_idx` in the current closure function prototype.
  const zs::object& new_closure_fct_proto = current_closure_fct_proto._fproto->_functions[inst.fct_idx];

  // TODO: Use closure's root.
  // Create the new closure object.
  object new_closure_obj = zs::object::create_closure(this->get_engine(), new_closure_fct_proto, _root_table);

  // In the new closure function prototype, we might have some captures to
  // fetch.
  const zs::vector<captured_variable>& captures = new_closure_fct_proto._fproto->_captures;
  if (const size_t capture_sz = captures.size()) {

    // Ref to the vector or capture values in the new closure object.
    // We will push all the required capture values in here.
    zs::vector<zs::object>& new_closure_capture_values = new_closure_obj._closure->_capture_values;

    for (size_t i = 0; i < capture_sz; i++) {
      const captured_variable& captured_var = captures[i];

      switch (captured_var.type) {
      case captured_variable::local: {
        // The capture value is on the stack. `captured_var.src` is the index of
        // the value on the stack. We push that value in the new closure capture
        // values vector.
        const int_t cap_idx = captured_var.src;

        if (cap_idx >= (int_t)_stack.stack_size()) {
          _error_message += zs::strprint(_engine, "op_new_closure could not find local capture\n");
          return zs::error_code::out_of_bounds;
        }

        new_closure_capture_values.push_back(_stack[cap_idx]);
        break;
      }

      case captured_variable::outer: {
        // When the capture type is outer, the capture value is in the
        // `capture_values` vector of the current closure object.
        const zs::vector<zs::object>& current_closure_capture_values
            = current_closure._closure->_capture_values;

        const int_t cap_idx = captured_var.src;

        if (cap_idx >= (int_t)current_closure_capture_values.size()) {
          _error_message += zs::strprint(_engine, "op_new_closure could not find parent capture\n");
          return zs::error_code::out_of_bounds;
        }

        new_closure_capture_values.push_back(current_closure_capture_values[cap_idx]);
        break;
      }
      }
    }
  }

  // In the new closure function prototype, we might have some default
  // parameters to fetch.
  const zs::vector<zs::int_t>& default_params = new_closure_fct_proto._fproto->_default_params;
  if (const size_t default_params_sz = default_params.size()) {
    zs::vector<zs::object>& new_closure_default_param_values = new_closure_obj._closure->_default_params;

    const int_t stack_sz = _stack.stack_size();

    for (size_t i = 0; i < default_params_sz; i++) {
      int_t default_param_idx = default_params[i];

      if (default_param_idx >= stack_sz) {
        _error_message += zs::strprint(_engine, "op_new_closure could not find default param\n");
        return zs::error_code::out_of_bounds;
      }

      new_closure_default_param_values.push_back(_stack[default_param_idx]);
    }
  }

  _stack[inst.target_idx] = std::move(new_closure_obj);
  return zs::error_code::success;
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
// template <>
// zs::error_code virtual_machine::exec_op<opcode::op_incr>(
//     zs::instruction_iterator& it, exec_op_data_t& op_data) {
//   const zs::instruction_t<op_incr>& inst = it.get_ref<op_incr>();
//   return runtime_arith_operation(
//       inst.is_incr ? arithmetic_uop::incr : arithmetic_uop::decr, _stack[inst.target_idx],
//       _stack[inst.idx]);
// }

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

// op_close.
template <>
zs::error_code virtual_machine::exec_op<opcode::op_close>(
    zs::instruction_iterator& it, exec_op_data_t& op_data) {
  ZS_TODO("Close captures");
  //  const zs::instruction_t<op_close>& inst = it.get_ref<op_close>();
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

// bool SQVM::StartCall(
//     SQClosure* closure, SQInteger target, SQInteger args, SQInteger
//     stackbase, bool tailcall) {
//   SQFunctionProto* func = closure->_function;
//
//   SQInteger paramssize = func->_nparameters;
//   const SQInteger newtop = stackbase + func->_stacksize;
//   SQInteger nargs = args;
//   if (func->_varparams) {
//     paramssize--;
//     if (nargs < paramssize) {
//       Raise_Error(
//           _SC("wrong number of parameters (%d passed, at least %d
//           required)"), (int)nargs, (int)paramssize);
//       return false;
//     }
//
//     // dumpstack(stackbase);
//     SQInteger nvargs = nargs - paramssize;
//     SQArray* arr = SQArray::Create(_ss(this), nvargs);
//     SQInteger pbase = stackbase + paramssize;
//     for (SQInteger n = 0; n < nvargs; n++) {
//       arr->_values[n] = _stack._vals[pbase];
//       _stack._vals[pbase].Null();
//       pbase++;
//     }
//     _stack._vals[stackbase + paramssize] = arr;
//     // dumpstack(stackbase);
//   }
//   else if (paramssize != nargs) {
//     SQInteger ndef = func->_ndefaultparams;
//     SQInteger diff;
//     if (ndef && nargs < paramssize && (diff = paramssize - nargs) <= ndef) {
//       for (SQInteger n = ndef - diff; n < ndef; n++) {
//         _stack._vals[stackbase + (nargs++)] = closure->_defaultparams[n];
//       }
//     }
//     else {
//       Raise_Error(_SC("wrong number of parameters (%d passed, %d required)"),
//       (int)nargs, (int)paramssize); return false;
//     }
//   }
//
//   if (closure->_env) {
//     _stack._vals[stackbase] = closure->_env->_obj;
//   }
//
//   if (!EnterFrame(stackbase, newtop, tailcall))
//     return false;
//
//   ci->_closure = closure;
//   ci->_literals = func->_literals;
//   ci->_ip = func->_instructions;
//   ci->_target = (SQInt32)target;
//
//   if (_debughook) {
//     CallDebugHook(_SC('c'));
//   }
//
//   if (closure->_function->_bgenerator) {
//     SQFunctionProto* f = closure->_function;
//     SQGenerator* gen = SQGenerator::Create(_ss(this), closure);
//     if (!gen->Yield(this, f->_stacksize))
//       return false;
//     SQObjectPtr temp;
//     Return(1, target, temp);
//     STK(target) = gen;
//   }
//
//   return true;
// }
