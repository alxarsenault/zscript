
namespace zs {

ZS_DECL_RT_ACTION(new_closure, uint32_t fct_idx, uint8_t bounded_target, objref_t dest) {

  // Current function call info.
  const call_info& cinfo = _call_stack.back();

  // Get the current closure.
  const zs::object& current_closure = cinfo.closure;
  if (!current_closure.is_closure()) {
    return ZS_VM_ERROR(zs::errc::invalid_type, "Current closure is not a closure (",
        zs::get_object_type_name(current_closure.get_type()), ").");
  }

  // Get the current closure function prototype.
  const zs::object& current_closure_fct_proto = current_closure._closure->_function;
  if (!function_prototype_object::is_proto(current_closure_fct_proto)) {
    return ZS_VM_ERROR(zs::errc::invalid_type, "Could not retrieve function prototype from closure object.");
  }

  // We want to create a closure object with the function prototype at index
  // `inst.fct_idx` in the current closure function prototype.
  const zs::object& new_closure_fct_proto
      = function_prototype_object::as_proto(current_closure_fct_proto)._functions[fct_idx];

  // TODO: Use closure's root.
  // Create the new closure object.
  object new_closure_obj = zs::_c(_engine, new_closure_fct_proto, _global_table);
  zs::closure_object& new_closure = new_closure_obj.as_closure();

  if (bounded_target != k_invalid_target) {
    new_closure._this = _stack[bounded_target];
  }

  new_closure._module = current_closure.as_closure()._module;

  // In the new closure function prototype, we might have some captures to fetch.
  const zs::vector<captured_variable>& captures
      = function_prototype_object::as_proto(new_closure_fct_proto)._captures;
  if (const size_t capture_sz = captures.size()) {

    // Ref to the vector or capture values in the new closure object.
    // We will push all the required capture values in here.
    zs::vector<zs::object>& new_closure_captured_values = new_closure._captured_values;

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

        //        object obj = _stack[cap_idx];
        //        if (captured_var.is_weak) {
        //          obj = obj.get_weak_ref();
        //        }

        object* ptr = _stack.stack_base_pointer() + cap_idx;
        object* found_cap = nullptr;

        for (object& p : _open_captures) {
          if (capture::as_capture(p).get_value_ptr() == ptr) {
            found_cap = &p;
            break;
            //            zb::print("dsdsdsdsds");
          }
        }

        if (found_cap) {
          new_closure_captured_values.emplace_back(*found_cap);
        }
        else {
          new_closure_captured_values.push_back(zs::capture::create(_engine, ptr));
          //        new_closure_capture_values.push_back(std::move(obj));
          _open_captures.push_back(new_closure_captured_values.back());
        }
        //        if (captured_var.name == "__exports__") {
        //          new_closure_obj.as_closure()._module = _stack[cap_idx];
        //
        //          //          new_closure_obj.as_closure()._module = new_closure_capture_values.back();
        //        }
        break;
      }

      case captured_variable::outer: {
        // When the capture type is outer, the capture value is in the
        // `capture_values` vector of the current closure object.
        const zs::vector<zs::object>& current_closure_captured_values
            = current_closure._closure->_captured_values;
        //
        const int_t cap_idx = captured_var.src;
        //
        if (cap_idx >= (int_t)current_closure_captured_values.size()) {
          _error_message += zs::strprint(_engine, "op_new_closure could not find parent capture\n");
          return zs::error_code::out_of_bounds;
        }

        ZS_ASSERT(capture::is_capture(current_closure_captured_values[cap_idx]));
        new_closure_captured_values.push_back(current_closure_captured_values[cap_idx]);

        //_open_captures.push_back(new_closure_capture_values.back());
        //        //        new_closure_capture_values.push_back(current_closure_capture_values[cap_idx]);
        //        object obj = current_closure_capture_values[cap_idx];
        //        if (captured_var.is_weak) {
        //          obj = obj.get_weak_ref();
        //        }
        //
        //        new_closure_capture_values.push_back(std::move(obj));
        //
        //        if (captured_var.name == "__exports__") {
        //          new_closure._module = *new_closure_captured_values.back().as_capture().get_value_ptr();
        //        }
        //        _error_message += zs::strprint(_engine, "unimplemented", std::source_location::current());
        //        return zs::errc::unimplemented;
        break;
      }
      }
    }
  }

  // In the new closure function prototype, we might have some default
  // parameters to fetch.
  const zs::vector<zs::int_t>& default_params
      = function_prototype_object::as_proto(new_closure_fct_proto)._default_params;
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

  dest.get() = std::move(new_closure_obj);
  return zs::error_code::success;
}

} // namespace zs.
