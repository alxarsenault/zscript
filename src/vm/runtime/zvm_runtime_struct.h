
namespace zs {
namespace {
  struct virtual_machine_struct_proxy_tag {};
  struct virtual_machine_struct_instance_proxy_tag {};
} // namespace.

template <>
struct internal::proxy<virtual_machine_struct_proxy_tag> {
  inline static void set_initialized(struct_object* strct_obj, bool initialized) {
    strct_obj->_initialized = initialized;
  }

  //  inline static bool has_constructors(const struct_object& strct_obj) {
  //    return !strct_obj._constructors.is_null();
  //  }
  //
  //  inline static bool has_single_constructor(struct_object& strct_obj) {
  //    return strct_obj._constructors.is_function();
  //  }
  //
  //  inline static bool has_multi_constructors(struct_object& strct_obj) {
  //    return strct_obj._constructors.is_array();
  //  }

  inline static const object& get_constructors(struct_object& strct_obj) { return strct_obj._constructors; }

  inline static void set_default_constructor(struct_object& strct_obj) {
    strct_obj._has_default_constructor = true;
  }

  inline static void set_constructor(struct_object& strct_obj, const object& constructor) {
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
  }

  inline static bool is_closure_with_invalid_param_count(const object& obj, int_t n_params) {
    return obj.is_closure() and !obj.as_closure().is_possible_parameter_count(n_params);
  }

  struct constructor_info {
    const object* obj;
    int_t n_type_match;

    inline const zs::closure_object& get_closure() const noexcept { return obj->as_closure(); }

    inline const object& get_object() const noexcept { return *obj; }
  };

  inline static zs::error_result resolve_constructor(zs::virtual_machine* vm, struct_object& strct_obj,
      int_t n_params, int_t stack_base, zs::object& constructor) {

    // Call default constructor.
    if (n_params == 1 and strct_obj.has_default_constructor()) {
      return {};
    }

    // If the struct has no constructor, we could still call the default constructor,
    // only if there is no parameters other than 'this'.
    if (!strct_obj.has_constructors()) {
      return n_params == 1 ? errc::success : errc::invalid_parameter_count;
    }

    // The struct has only one constructor, strct_obj._constructors is a function.
    if (strct_obj.has_single_constructor()) {
      const zs::object& constructor_obj = strct_obj._constructors;

      if (constructor_obj.is_closure()) {

        zb::span<const object> params = vm->_stack.get_absolute_subspan(stack_base, n_params);

        int_t n_type_match = -1;
        if (!constructor_obj.as_closure().is_valid_parameters(vm_ref(vm), params, n_type_match)) {
          return zs::errc::invalid_parameters;
        }
      }

      constructor = constructor_obj;
      return {};
    }

    // If 'has_multi_constructors' returns false, there's no constructor.
    ZS_ASSERT(strct_obj.has_multi_constructors());

    // An array of constructors.
    const zs::array_object& strc_ctors = strct_obj._constructors.as_array();

    zs::small_vector<constructor_info, 8> potential_constructors(
        (zs::allocator<constructor_info>(vm->get_engine())));

    for (const object& obj : strc_ctors) {
      if (!is_closure_with_invalid_param_count(obj, n_params) and obj.is_function()) {
        potential_constructors.push_back({ &obj, -1 });
      }
    }

    zb::span<const object> params = vm->_stack.get_absolute_subspan(stack_base, n_params);
    for (auto it = potential_constructors.begin(); it != potential_constructors.end();) {
      if (!it->obj->is_closure()) {
        ++it;
        continue;
      }

      int_t n_type_match = -1;
      if (!it->get_closure().is_valid_parameters(vm_ref(vm), params, n_type_match)) {
        it = potential_constructors.erase(it);
        continue;
      }

      it->n_type_match = n_type_match;
      ++it;
    }

    if (potential_constructors.size() == 1) {
      constructor = potential_constructors.back().get_object();
      return {};
    }

    if (potential_constructors.empty()) {
      return zs::errc::invalid_operation;
    }

    return zs::errc::invalid;
  }
};

template <>
struct internal::proxy<virtual_machine_struct_instance_proxy_tag> {
  inline static void set_initialized(struct_instance_object* strct_obj, bool initialized) {
    strct_obj->_initialized = initialized;
  }
};

using virtual_machine_struct_proxy = internal::proxy<virtual_machine_struct_proxy_tag>;
using virtual_machine_struct_instance_proxy = internal::proxy<virtual_machine_struct_instance_proxy_tag>;

ZS_DECL_RT_ACTION(struct_instance_set, objref_t obj, cobjref_t key, cobjref_t value) {
  zs::struct_instance_object& sobj = obj->as_struct_instance();

  if (auto err = sobj.set(key, value, _stack[0]._struct_instance == obj->_struct_instance)) {
    switch (err) {
    case zs::error_code::inaccessible_private:
      set_error("Can't access or modify private struct member ", key, ".\n");
      return err;
    case zs::error_code::cant_modify_const_member:
      set_error("\nCan't modify a struct const member ", key, ".\n");
      return err;
    case zs::error_code::cant_modify_static_const:
      set_error("\nCan't modify a struct static const member ", key, ".\n");
      return err;
    case zs::error_code::invalid_type_assignment:
      set_error("\nInvalid value type assignment for struct field ", key, ".\n");
      return err;
    default:
      set_error("\nCan't add new value to a struct. Field ", key, " doesn't exists.\n");
      return err;
    }
  }

  return {};
}

ZS_DECL_RT_ACTION(struct_set, objref_t obj, cobjref_t key, cobjref_t value) {
  struct_object& sobj = obj->as_struct();

  if (auto err = sobj.set_static(key, value)) {
    switch (err) {
    case zs::error_code::cant_modify_const_member:
      set_error("\nCan't modify a struct const member ", key, ".\n");
      return err;
    case zs::error_code::cant_modify_static_const:
      set_error("\nCan't modify a struct static const member ", key, ".\n");
      return err;
    case zs::error_code::invalid_type_assignment:
      set_error("\nInvalid value type assignment for struct field ", key, ".\n");
      return err;
    default:
      set_error("\nCan't add new value to a struct. Field ", key, " doesn't exists.\n");
      return err;
    }
  }

  return {};
}

ZS_DECL_RT_ACTION(struct_call_create, cobjref_t obj, int_t n_params, int_t stack_base, objref_t ret_value) {

  zs::struct_object& strct = obj->as_struct();

  struct_instance_object* new_strct_obj = strct.create_instance();
  virtual_machine_struct_instance_proxy::set_initialized(new_strct_obj, false);

  object new_obj;
  new_obj._type = k_struct_instance;
  new_obj._struct_instance = new_strct_obj;
  ret_value.get() = new_obj;

  zs::object constructor;
  if (auto err
      = virtual_machine_struct_proxy::resolve_constructor(this, strct, n_params, stack_base, constructor)) {
    return ZS_VM_ERROR(err, "Could not resolve constructor for struct.");
  }

  if (constructor.is_function()) {
    zs::object tmp_ret;
    object sb = std::exchange(_stack.get_at(stack_base), new_obj);

    if (auto err = call(constructor, n_params, stack_base, ret_value.get(), false)) {
      virtual_machine_struct_instance_proxy::set_initialized(new_strct_obj, true);
      _stack.get_at(stack_base) = sb;
      return err;
    }
    _stack.get_at(stack_base) = sb;
  }

  virtual_machine_struct_instance_proxy::set_initialized(new_strct_obj, true);
  return {};
}

ZS_DECL_RT_ACTION(struct_new_constructor, objref_t obj, cobjref_t value) {
  struct_object& sobj = obj->as_struct();
  virtual_machine_struct_proxy::set_constructor(sobj, value);
  return {};
}

ZS_DECL_RT_ACTION(struct_new_default_constructor, objref_t obj) {
  struct_object& sobj = obj->as_struct();
  virtual_machine_struct_proxy::set_default_constructor(sobj);
  return {};
}

ZS_DECL_RT_ACTION(struct_new_slot, objref_t obj, cobjref_t key, cobjref_t value, uint32_t mask,
    bool is_static, bool is_private, bool is_const) {
  ZS_ASSERT(obj->is_struct());
  return obj->as_struct().new_slot(key, value, mask, is_static, is_private, is_const);
}

ZS_DECL_RT_ACTION(struct_new_slot, objref_t obj, cobjref_t key, uint32_t mask, bool is_static,
    bool is_private, bool is_const) {
  ZS_ASSERT(obj->is_struct());
  return obj->as_struct().new_slot(key, mask, is_static, is_private, is_const);
}

ZS_DECL_RT_ACTION(struct_new_method, objref_t obj, cobjref_t closure, variable_attribute_t decl_flags) {
  ZS_ASSERT(obj->is_struct());
  bool is_private = zb::has_flag<variable_attribute_t::va_private>(decl_flags);

  if (zb::has_flag<variable_attribute_t::va_static>(decl_flags)) {
    return obj->as_struct().new_static_method(
        closure->as_closure().get_proto()._name, closure, is_private, false);
  }
  else {
    return obj->as_struct().new_method(closure->as_closure().get_proto()._name, closure, is_private, false);
  }
}

} // namespace zs.
