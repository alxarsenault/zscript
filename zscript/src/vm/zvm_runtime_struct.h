
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

  inline static bool has_constructor(struct_object& strct_obj) {
    return strct_obj._constructor.is_function();
  }

  inline static const object& get_constructor(struct_object& strct_obj) { return strct_obj._constructor; }

  inline static void set_constructor(struct_object& strct_obj, const object& constructor) {
    strct_obj._constructor = constructor;
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

  if (auto err = sobj.get(key, dest)) {
    set_error("Struct get. Field ", key, " doesn't exists.\n");
    return err;
  }

  return {};
}

ZS_DECL_RT_ACTION(struct_instance_set, objref_t obj, cobjref_t key, cobjref_t value) {
  zs::struct_instance_object& sobj = obj->as_struct_instance();

  if (auto err = sobj.set(key, value)) {
    switch (err) {
    case zs::error_code::cant_modify_const_member:
      set_error("\nCan't modify a struct const member ", key, ".\n");
      return err;
    case zs::error_code::cant_modify_static_const:
      set_error("\nCan't modify a struct static const member ", key, ".\n");
      return err;
    case zs::error_code::invalid_value_type_assignment:
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
    case zs::error_code::invalid_value_type_assignment:
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

  if (virtual_machine_struct_proxy::has_constructor(strct)) {
    zs::object tmp_ret;
    object sb = std::exchange(_stack.get_at(stack_base), new_obj);

    if (auto err = call(virtual_machine_struct_proxy::get_constructor(strct), n_params, stack_base,
            ret_value.get(), false)) {
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

ZS_DECL_RT_ACTION(struct_new_slot, objref_t obj, cobjref_t key, cobjref_t value, uint32_t mask,
    bool is_static, bool is_const) {
  ZS_ASSERT(obj->is_struct());
  return obj->as_struct().new_slot(key, value, mask, is_static, is_const);
}

ZS_DECL_RT_ACTION(
    struct_new_slot, objref_t obj, cobjref_t key, uint32_t mask, bool is_static, bool is_const) {
  ZS_ASSERT(obj->is_struct());
  return obj->as_struct().new_slot(key, mask, is_static, is_const);
}
} // namespace zs.
