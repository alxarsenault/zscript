
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

  inline static zs::error_result resolve_constructor(zs::virtual_machine* vm, struct_object& strct_obj,
      int_t n_params, int_t stack_base, zs::object& constructor) {

    if (n_params == 1 and strct_obj.has_default_constructor()) {
      return {};
    }

    if (!strct_obj.has_constructors()) {
      if (n_params != 1) {
        return zs::error_code::invalid_parameter_count;
      }

      return {};
    }

    const zs::object& strc_ctors_obj = strct_obj._constructors;

    if (strct_obj.has_single_constructor()) {
      if (strc_ctors_obj.is_closure()
          and !strc_ctors_obj.as_closure().is_possible_parameter_count(n_params)) {
        return zs::error_code::invalid_parameter_count;
      }

      constructor = strc_ctors_obj;
      return {};
    }

    // If 'has_multi_constructors' returns false, there's no constructor.
    if (!strct_obj.has_multi_constructors()) {
      zb::print("SDLSJHDSJKSJDKLSJJSHDJSHDJKS", n_params);
      return {};
    }

    const zs::array_object& strc_ctors_arr = strc_ctors_obj.as_array();

    zs::engine* eng = vm->get_engine();
    zs::small_vector<size_t, 8> constructor_indexes((zs::allocator<size_t>(eng)));

    for (size_t i = 0; i < strc_ctors_arr.size(); i++) {
      if (strc_ctors_arr[i].is_closure()
          and !strc_ctors_arr[i].as_closure().is_possible_parameter_count(n_params)) {
        continue;
      }

      constructor_indexes.push_back(i);
    }

    if (constructor_indexes.empty()) {
      //          set_error("\nCould not find constructor for struct.\n");
      return zs::error_code::invalid_operation;
    }

    if (constructor_indexes.size() == 1) {
      constructor = strc_ctors_arr[constructor_indexes[0]];

      if (constructor.is_closure() and !constructor.as_closure().is_possible_parameter_count(n_params)) {
        return zs::error_code::invalid_parameter_count;
      }

      return {};
    }

    zs::small_vector<size_t, 8> second_pass_constructor_indexes((zs::allocator<size_t>(eng)));

    auto check_parameters = [&](const zs::function_prototype_object& fpo) {
      bool keep = false;

      for (size_t k = 0; k < n_params; k++) {
        const zs::local_var_info_t* vinfo = fpo.find_local(fpo._parameter_names[k]);

        if (!vinfo) {
          continue;
        }

        if (vinfo->mask and vinfo->custom_mask) {
          if (vm->_stack.get_at(stack_base + k).has_type_mask(vinfo->mask)) {
            keep = true;
          }
          else {
            return -1;
          }
        }
        else if (vinfo->mask) {
          if (vm->_stack.get_at(stack_base + k).has_type_mask(vinfo->mask)) {
            keep = true;
          }
          else {
            return -1;
          }
        }
      }

      return keep ? 1 : 0;
    };

    for (size_t i = 0; i < constructor_indexes.size();) {
      const zs::function_prototype_object& fpo
          = strc_ctors_arr[constructor_indexes[i]].as_closure().get_proto();

      int ret = check_parameters(fpo);
      if (ret == -1) {
        constructor_indexes.erase_at(i);
        continue;
      }

      if (ret == 1) {
        second_pass_constructor_indexes.push_back(i);
      }

      ++i;
    }

    if (constructor_indexes.empty()) {
      //          set_error("\nCould not find constructor for struct.\n");
      return zs::error_code::invalid_operation;
    }

    if (constructor_indexes.size() == 1) {
      constructor = strc_ctors_arr[constructor_indexes.back()];
      return {};
    }

    constructor_indexes = second_pass_constructor_indexes;

    if (constructor_indexes.empty()) {
      //          set_error("\nCould not find constructor for struct.\n");
      return zs::error_code::invalid_operation;
    }

    if (constructor_indexes.size() == 1) {
      constructor = strc_ctors_arr[constructor_indexes.back()];
      return {};
    }

    zb::print("DSLKDJSKJDKSKL");

    return zs::error_code::invalid;
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

ZS_DECL_RT_ACTION(struct_instance_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  zs::struct_instance_object& sobj = obj->as_struct_instance();

  if (auto err = sobj.get(key, dest, _stack[0]._struct_instance == obj->_struct_instance)) {

    if (err == zs::errc::inaccessible_private) {
      set_error("Could not access private struct member ", key, ".\n");
    }
    else {
      set_error("Struct get. Field ", key, " doesn't exists.\n", err.message());
    }

    return err;
  }

  return {};
}

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

ZS_DECL_RT_ACTION(struct_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  zs::struct_object& sobj = obj->as_struct();

  if (auto err = sobj.get(key, dest)) {
    set_error("Struct get. Field ", key, " doesn't exists.\n");
    return err;
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
  using enum object_type;

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
    set_error("\nCould not find constructor for struct.\n");
    return err;
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

ZS_DECL_RT_ACTION(struct_new_method, objref_t obj, cobjref_t closure, var_decl_flags_t decl_flags) {
  ZS_ASSERT(obj->is_struct());
  bool is_private = (decl_flags & var_decl_flags_t::vdf_private) != 0;

  if ((decl_flags & var_decl_flags_t::vdf_static) != 0) {
    return obj->as_struct().new_static_method(
        closure->as_closure().get_proto()._name, closure, is_private, false);
  }
  else {
    return obj->as_struct().new_method(closure->as_closure().get_proto()._name, closure, is_private, false);
  }
}

} // namespace zs.
