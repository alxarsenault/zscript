
namespace zs {

//ZS_DECL_RT_ACTION(delegate_get_type_of, cobjref_t obj, cobjref_t delegate_obj, objref_t dest) {
//  zs::table_object* delegate = delegate_obj->_table;
//
//  object meta_typeof;
//
//  // Look for typeof operator in the delegate table.
//  if (!delegate->get(constants::get<meta_method::mt_typeof>(), meta_typeof)) {
//    // The meta typeof was found in the delegate table.
//
//    // If it is a function, let's call it.
//    if (meta_typeof.is_function()) {
//      push(obj);
//      if (auto err = call(meta_typeof, 1, stack_size() - 1, dest)) {
//        return err;
//      }
//
//      if (!dest->is_string()) {
//        zb::print("Invalid typeof operator return type (should be a string)");
//        return zs::error_code::invalid_type;
//      }
//      return {};
//    }
//
//    // If it's a string, we return that string.
//    else if (meta_typeof.is_string()) {
//      dest.get() = meta_typeof;
//      return {};
//    }
//
//    else {
//      zb::print("Invalid typeof operator");
//      return zs::error_code::invalid_type;
//    }
//  }
//
//  // There was no operator typeof in this meta table.
//  // Let's look deeper into delegates.
//  if (delegate->has_delegate()) {
//    object& sub_delegate_obj = delegate->get_delegate();
//    if (!runtime_action<runtime_code::delegate_get_type_of>(obj, CREF(sub_delegate_obj), dest)) {
//      return {};
//    }
//    //     if (!helper::delegate_get_type_of(vm, obj, sub_delegate_obj, dest)) {
//    //       return {};
//    //     }
//  }
//
//  return zs::error_code::not_found;
//}

ZS_DECL_RT_ACTION(delegate_set, objref_t obj, objref_t delegate_obj, cobjref_t key, cobjref_t value) {
  zs::table_object* delegate = delegate_obj->_table;

  // Start by looking directly in the table.
  if (delegate->contains(key)) {
    return delegate->set(key, value);
  }

  object meta_set;

  // Look for the set operator in table.
  if (auto err = delegate->get(zs::constants::get<meta_method::mt_set>(), meta_set);
      !err and !meta_set.is_null()) {

    // The set operator was found in the table.

    if (!meta_set.is_function()) {
      zb::print("Invalid set operator");
      return zs::error_code::invalid_type;
    }
    //
    // It's a function, let's call it.
    //
    object ret;
    // TODO: Should be : obj, key, value, delegate.

    push(obj);
    push(key);
    push(value);
    push(delegate_obj);

    if (auto err = call(meta_set, 4, stack_size() - 4, ret)) {
      pop(4);
      return err;
    }

    pop(4);

    return {};
  }

  // There was no operator set in this meta table.
  // Let's look deeper into delegates.
  if (delegate->has_delegate()) {
    object& sub_delegate_obj = delegate->get_delegate();
    //    return helper::delegate_set(vm, obj, sub_delegate_obj, key, value);
    return runtime_action<runtime_code::delegate_set>(obj, REF(sub_delegate_obj), key, value);
  }

  //  return obj->_table->set(key, value);

  return zs::error_code::not_found;
}
} // namespace zs.
