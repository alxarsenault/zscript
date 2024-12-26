
namespace zs {

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
