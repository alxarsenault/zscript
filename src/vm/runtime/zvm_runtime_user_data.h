
namespace zs {

ZS_DECL_RT_ACTION(user_data_set, objref_t user_data_obj, cobjref_t key, cobjref_t value) {
  zs::user_data_object* uobj = user_data_obj->_udata;

  // If the user_data has no delegate, the item is not found.
  if (!uobj->has_delegate()) {
    return zs::error_code::inaccessible;
  }

  // Look in delegate.
  zs::object delegate_obj = uobj->get_delegate();

  // Let's try to get the `__operator_set` meta method from delegate.
  if (object meta_set; !delegate_obj._table->get(constants::get<meta_method::mt_set>(), meta_set)) {

    if (!meta_set.is_function()) {
      return ZS_VM_ERROR(errc::invalid_type, "The user data __operator_set is not callable");
    }

    // When a `__operator_set` meta method returns `none`, we can keep looking
    // deeper.
    object ret;
    if (auto err = call(meta_set, { user_data_obj, key, value, delegate_obj }, ret)) {
      if (!ret.is_none()) {
        return err;
      }
    }

    // If the returned object is not `none`, the `set` worked.
    if (!ret.is_none()) {
      return {};
    }
  }

  // If there was no error in the `table_set`, the `set` worked.
  if (!runtime_action<runtime_code::table_set>(REF(delegate_obj), key, value)) {
    return {};
  }

  object kobj;

  object tkey = constants::get<meta_method::mt_set>();
  ZS_RETURN_IF_ERROR(proxy::get(this, user_data_obj, tkey, delegate_obj, kobj, false));

  //  ZS_RETURN_IF_ERROR(runtime_action<runtime_code::table_get>(CREF(delegate_obj), CREF(tkey), REF(kobj)));

  //    ZS_RETURN_IF_ERROR(table_get(vm, delegate_obj,
  //    constants::get<meta_method::mt_set>(), kobj));

  if (kobj.is_function()) {
    object ret;
    return call(kobj, { user_data_obj, key, value, delegate_obj }, ret);
  }

  return zs::error_code::inaccessible;
}
} // namespace zs.
