
namespace zs {

// TODO: Should we check if `stack_base` + `n_params` is already on top of the stack?
ZS_DECL_RT_ACTION(enter_function_call, cobjref_t closure_obj, int_t n_params, int_t stack_base) {
  zb::execution_stack_state stack_state = _stack.get_state();
  _call_stack.emplace_back(closure_obj, stack_state);

  // The current top of the stack will be the next stack base.
  const size_t next_base = stack().get_absolute_top();

  // Push `n_params` elements starting at `stack_base` on top of the stack.
  stack().absolute_repush_n(stack_base, n_params);

  // Set the new stack base.
  _stack.set_stack_base(next_base);

  zbase_assert(next_base == (stack().get_absolute_top() - n_params), "invalid stack parameters");
  zbase_assert(stack_size() == n_params, "invalid stack parameters");
  return {};
}

ZS_DECL_RT_ACTION(enter_function_call, cobjref_t closure_obj, zs::parameter_list params) {
  const int_t n_params = params.size();

  zb::execution_stack_state stack_state = _stack.get_state();
  _call_stack.emplace_back(closure_obj, stack_state);

  // The current top of the stack will be the next stack base.
  const size_t next_base = stack().get_absolute_top();

  // Push `n_params` elements starting at `stack_base` on top of the stack.
  for (const auto& p : params) {
    push(p);
  }

  // Set the new stack base.
  _stack.set_stack_base(next_base);

  zbase_assert(next_base == (stack().get_absolute_top() - n_params), "invalid stack parameters");
  zbase_assert(stack_size() == n_params, "invalid stack parameters");
  return {};
}

ZS_DECL_RT_ACTION(leave_function_call) {
  zbase_assert(!_call_stack.empty());
  ZS_TRACE("VM - leave_function_call");
  call_info cinfo = _call_stack.get_pop_back();
  _stack.set_stack_base(cinfo.previous_stack_base);

  if (cinfo.closure.is_closure()) {
    zs::closure_object& cobj = cinfo.closure.as_closure();
    zs::vector<zs::object>& closure_captured_values = cobj._captured_values;
    const object* stack_ptr = _stack.get_internal_vector().data()
        + cinfo.previous_top_index; //_stack.stack_base_pointer() ;//+cinfo.previous_top_index;

    if (auto err = runtime_action<rt_close_captures>(stack_ptr)) {
      return err;
    }
    //    for(object& cap : closure_capture_values) {
    //       if(!cap.as_capture().is_baked() and cap.as_capture().get_value_ptr() >= stack_ptr) {
    //         cap.as_capture().bake();
    //         zb::print(std::source_location::current(),cinfo.closure.as_closure().get_proto()._name);
    //       }
    //     }
  }

  _stack.pop_to(cinfo.previous_top_index);

  // Just in case we somehow went below. Is this possible?
  if (stack().get_absolute_top() < (size_t)cinfo.previous_top_index) {
    size_t diff = cinfo.previous_top_index - stack().get_absolute_top();
    stack().push_n(diff);
  }

  zbase_assert(cinfo.previous_top_index == (int_t)stack().get_absolute_top());

  return {};
}

//
ZS_DECL_RT_ACTION(
    call_native_closure, cobjref_t closure_obj, int_t n_params, int_t stack_base, objref_t ret_value) {
  ZS_RETURN_IF_ERROR(runtime_action<runtime_code::enter_function_call>(closure_obj, n_params, stack_base));

  zs::native_closure_object* nclosure = closure_obj->_native_closure;
  const int_t native_call_result = nclosure->call(zs::vm_ref(this));

  if (native_call_result < 0) {
    (void)runtime_action<runtime_code::leave_function_call>();
    return zs::error_code::invalid_native_function_call;
  }

  ret_value.get() = native_call_result ? _stack.top() : nullptr;

  return runtime_action<runtime_code::leave_function_call>();
}

ZS_DECL_RT_ACTION(call_native_closure, cobjref_t closure_obj, zs::parameter_list params, objref_t ret_value) {
  ZS_RETURN_IF_ERROR(runtime_action<runtime_code::enter_function_call>(closure_obj, params));

  zs::native_closure_object* nclosure = closure_obj->_native_closure;
  const int_t native_call_result = nclosure->call(zs::vm_ref(this));

  if (native_call_result < 0) {
    ret_value.get() = nullptr;
    (void)runtime_action<runtime_code::leave_function_call>();
    return zs::error_code::invalid_native_function_call;
  }

  ret_value.get() = native_call_result ? _stack.top() : nullptr;

  return runtime_action<runtime_code::leave_function_call>();
}

//

ZS_DECL_RT_ACTION(call_native_function, //
    cobjref_t closure_obj, //
    int_t n_params, //
    int_t stack_base, // Absolute stack index.
    objref_t ret_value) {

  ZS_RETURN_IF_ERROR(runtime_action<runtime_code::enter_function_call>(closure_obj, n_params, stack_base));

  zs::function_t fct = closure_obj->_nfct;
  const int_t native_call_result = (*fct)(zs::vm_ref(this));

  if (native_call_result < 0) {
    ret_value.get() = nullptr;
    (void)runtime_action<runtime_code::leave_function_call>();
    return zs::error_code::invalid_native_function_call;
  }

  ret_value.get() = native_call_result ? _stack.top() : nullptr;

  return runtime_action<runtime_code::leave_function_call>();
}

ZS_DECL_RT_ACTION(
    call_native_function, cobjref_t closure_obj, zs::parameter_list params, objref_t ret_value) {

  ZS_RETURN_IF_ERROR(runtime_action<runtime_code::enter_function_call>(closure_obj, params));

  zs::function_t fct = closure_obj->_nfct;
  const int_t native_call_result = (*fct)(zs::vm_ref(this));

  if (native_call_result < 0) {
    ret_value.get() = nullptr;
    (void)runtime_action<runtime_code::leave_function_call>();
    return zs::error_code::invalid_native_function_call;
  }

  ret_value.get() = native_call_result ? _stack.top() : nullptr;

  return runtime_action<runtime_code::leave_function_call>();
}

ZS_DECL_RT_ACTION(call_native_pfunction, //
    cobjref_t closure_obj, //
    int_t n_params, //
    int_t stack_base, // Absolute stack index.
    objref_t ret_value) {

  ZS_RETURN_IF_ERROR(runtime_action<runtime_code::enter_function_call>(closure_obj, n_params, stack_base));

  zs::parameter_list_function_t fct = closure_obj->_npfct;
  ret_value.get() = (*fct)(zs::vm_ref(this), _stack.get_stack_view());

  if (ret_value->is_error()) {
    (void)runtime_action<runtime_code::leave_function_call>();
    return zs::error_code::invalid_native_function_call;
  }

  return runtime_action<runtime_code::leave_function_call>();
}

ZS_DECL_RT_ACTION(
    call_native_pfunction, cobjref_t closure_obj, zs::parameter_list params, objref_t ret_value) {

  ZS_RETURN_IF_ERROR(runtime_action<runtime_code::enter_function_call>(closure_obj, params));

  zs::parameter_list_function_t fct = closure_obj->_npfct;
  ret_value.get() = (*fct)(zs::vm_ref(this), params);

  if (ret_value->is_error()) {
    (void)runtime_action<runtime_code::leave_function_call>();
    return zs::error_code::invalid_native_function_call;
  }

  return runtime_action<runtime_code::leave_function_call>();
}

ZS_DECL_RT_ACTION(call_closure, cobjref_t closure_obj, int_t n_params, int_t stack_base, objref_t ret_value) {
  zs::closure_object* closure = closure_obj->_closure;
  zs::function_prototype_object* fpo = closure->get_function_prototype();

  // The call was made with `n_params` parameters.
  // The function expects `n_expected_params` parameters.
  const int_t n_expected_params = fpo->_parameter_names.size();

  // The closure has `n_default_params` default parameters values.
  const int_t n_default_params = closure->_default_params.size();

  // A span of parameters that will be pushed after the `n_params` given params
  // if needed.
  std::span<object> default_params;

  // Some parameters are missing?
  if (n_default_params && n_params < n_expected_params) {

    // We check if some of those have a default parameter value.
    const int_t diff = n_expected_params - n_params;

    // If `diff` is bigger than `n_default_params`, we're still gonna be short
    // on parameters. Let's leave this condition like this for now, we'll check
    // it again below. Maybe we'll add some other kind of default parameters
    // (e.g. named parameters) in the future.
    if (diff <= n_default_params) {
      const int_t start_index = n_default_params - diff;
      default_params = std::span<object>(&closure->_default_params[start_index], diff);
    }
  }

  // We have `n_extra_default` that we can add to `n_params`.
  const int_t n_extra_default = default_params.size();
  if (n_expected_params != n_params + n_extra_default) {
    zb::print("WRONG NUMBER OF PARAMETERS");
    return zs::error_code::invalid_parameter_count;
  }

  zs::object_stack& stack = _stack;

  // This is a modified version of `enter_function_call`.
  {
    zb::execution_stack_state stack_state = stack.get_state();
    _call_stack.emplace_back(closure_obj, stack_state);

    // The current top of the stack will be the next stack base.
    const size_t next_base = stack.get_absolute_top();

    // Push `n_params` elements starting at `stack_base` on top of the stack.
    stack.absolute_repush_n(stack_base, n_params);

    // This section differs from `enter_function_call`.
    {
      // We push the extra default params.
      for (const auto& param : default_params) {
        stack.push(param);
      }

      n_params += n_extra_default;
    }

    // Set the new stack base.
    stack.set_stack_base(next_base);

    zbase_assert(next_base == (stack.get_absolute_top() - n_params), "invalid stack parameters");
    zbase_assert((int_t)stack.stack_size() == n_params, "invalid stack parameters");
  }

  // We make room for the function stack by pushing some empty objects.
  stack.push_n(fpo->_stack_size - n_params);

  if (!closure->_this.is_null()) {
    stack[0] = closure->_this;
  }

  if (auto err = runtime_action<runtime_code::execute>(closure, zb::wref(ret_value))) {
    return err;
  }

  return runtime_action<runtime_code::leave_function_call>();
}

ZS_DECL_RT_ACTION(call_closure, cobjref_t closure_obj, zs::parameter_list params, objref_t ret_value) {
  zs::closure_object* closure = closure_obj->_closure;
  zs::function_prototype_object* fpo = closure->get_function_prototype();

  int_t n_params = params.size();

  // The call was made with `n_params` parameters.
  // The function expects `n_expected_params` parameters.
  const int_t n_expected_params = fpo->_parameter_names.size();

  // The closure has `n_default_params` default parameters values.
  const int_t n_default_params = closure->_default_params.size();

  // A span of parameters that will be pushed after the `n_params` given params
  // if needed.
  std::span<object> default_params;

  // Some parameters are missing?
  if (n_default_params && n_params < n_expected_params) {

    // We check if some of those have a default parameter value.
    const int_t diff = n_expected_params - n_params;

    // If `diff` is bigger than `n_default_params`, we're still gonna be short
    // on parameters. Let's leave this condition like this for now, we'll check
    // it again below. Maybe we'll add some other kind of default parameters
    // (e.g. named parameters) in the future.
    if (diff <= n_default_params) {
      const int_t start_index = n_default_params - diff;
      default_params = std::span<object>(&closure->_default_params[start_index], diff);
    }
  }

  // We have `n_extra_default` that we can add to `n_params`.
  const int_t n_extra_default = default_params.size();
  if (n_expected_params != n_params + n_extra_default) {
    zb::print("WRONG NUMBER OF PARAMETERS", n_expected_params, n_params + n_extra_default);
    return zs::error_code::invalid_parameter_count;
  }

  zs::object_stack& stack = _stack;

  // This is a modified version of `enter_function_call`.
  {
    zb::execution_stack_state stack_state = stack.get_state();
    _call_stack.emplace_back(closure_obj, stack_state);

    // The current top of the stack will be the next stack base.
    const size_t next_base = stack.get_absolute_top();

    // Push `n_params` elements starting at `stack_base` on top of the stack.
    //      stack.absolute_repush_n(stack_base, n_params);
    for (const auto& p : params) {
      stack.push(p);
    }

    // This section differs from `enter_function_call`.
    {
      // We push the extra default params.
      for (const auto& param : default_params) {
        stack.push(param);
      }

      n_params += n_extra_default;
    }

    // Set the new stack base.
    stack.set_stack_base(next_base);

    zbase_assert(next_base == (stack.get_absolute_top() - n_params), "invalid stack parameters");
    zbase_assert((int_t)stack.stack_size() == n_params, "invalid stack parameters");
  }

  // We make room for the function stack by pushing some empty objects.
  stack.push_n(fpo->_stack_size - n_params);

  if (!closure->_this.is_null()) {
    stack[0] = closure->_this;
  }

  if (auto err = runtime_action<runtime_code::execute>(closure, REF(ret_value))) {
    return err;
  }

  return runtime_action<runtime_code::leave_function_call>();
}
} // namespace zs.
