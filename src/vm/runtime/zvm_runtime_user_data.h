
namespace zs {

// ZS_DECL_RT_ACTION(user_data_get, cobjref_t obj, cobjref_t key, objref_t dest) {
//   zbase_assert(obj->is_user_data(), "should be a user data");
//
//
//   zs::user_data_object& uobj = obj->as_udata();
//
//
//   return  this->delegate_table_get(obj.get(), key.get(), uobj.get_delegate(), dest.get());
//
//
////  // Look in the delegate.
////  {
////    zs::user_data_object* uobj = obj->_udata;
////
////    // If the user_data has no delegate, the item is not found.
////    if (!uobj->has_delegate()) {
////      return zs::error_code::not_found;
////    }
////
////    zs::object& delegate_obj = uobj->get_delegate();
////    zs::table_object* delegate = delegate_obj._table;
////
////    // Look directly in the delegate table.
////    // If the `delegate->get` returns without errors, the item was found without
////    // meta method.
////    if (!delegate->get(key, dest)) {
////      return {};
////    }
////
////    // Let's try to get the `__operator_get` meta method from delegate.
////    if (object meta_get; !delegate->get(zs::constants::get<meta_method::mt_get>(), meta_get)) {
////
////      if (!meta_get.is_function()) {
////        _error_message += zs::strprint(_engine, "The user data __operator_get is not callable");
////        return zs::error_code::invalid_type;
////      }
////
////      ZS_RETURN_IF_ERROR(call(meta_get, { obj, key }, dest); err and !dest->is_none());
////
////      if (!dest->is_none()) {
////        return {};
////      }
////    }
////
////    if (zs::status_result status = runtime_action<runtime_code::table_get>(CREF(delegate_obj), key, dest))
///{ /      return status; /    } /  }
////
////  return zs::error_code::inaccessible;
//}

ZS_DECL_RT_ACTION(user_data_set, objref_t user_data_obj, cobjref_t key, cobjref_t value) {
  zs::user_data_object* uobj = user_data_obj->_udata;

  // If the user_data has no delegate, the item is not found.
  if (!uobj->has_delegate()) {
    return zs::error_code::inaccessible;
  }

  // Look in delegate.
  zs::object& delegate_obj = uobj->get_delegate();

  // Let's try to get the `__operator_set` meta method from delegate.
  if (object meta_set; !delegate_obj._table->get(constants::get<meta_method::mt_set>(), meta_set)) {

    if (!meta_set.is_function()) {
      _error_message += zs::strprint(_engine, "The user data __operator_set is not callable");
      return zs::error_code::invalid_type;
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
