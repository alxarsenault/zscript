
namespace zs {
ZS_DECL_RT_ACTION(extension_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  using enum object_type;
  using enum runtime_code;

  if (obj->is_color()) {
    return runtime_action<color_get>(obj, key, dest);
  }
  else if (obj->is_array_iterator()) {
    zs::object delegate_obj = _default_array_iterator_delegate;
    return runtime_action<runtime_code::table_get>(CREF(delegate_obj), key, dest);
  }

  else if (obj->is_table_iterator()) {
    zs::object delegate_obj = _default_table_iterator_delegate;
    return runtime_action<runtime_code::table_get>(CREF(delegate_obj), key, dest);
  }

  return zs::error_code::inaccessible;
}

ZS_DECL_RT_ACTION(extension_set, objref_t obj, cobjref_t key, cobjref_t value) {
  using enum runtime_code;

  if (obj->is_array_iterator()) {
    return zs::error_code::unimplemented;
  }
  else if (obj->is_table_iterator()) {
    return zs::error_code::unimplemented;
  }
  return zs::error_code::unimplemented;
}

} // namespace zs.
