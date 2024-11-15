
namespace zs {

ZS_DECL_RT_ACTION(class_get, cobjref_t obj, cobjref_t class_obj, cobjref_t key, objref_t dest) {
  zs::class_object* delegate = class_obj->_class;

  // Start by looking directly in the table.
  if (!delegate->get(key, dest)) {
    return {};
  }

  object meta_get;
  //    zb::print("DSLKDJKDLS");
  // Look for the get operator in table.
  if (auto err = delegate->get(zs::constants::get<meta_method::mt_get>(), meta_get);
      !err and !meta_get.is_null()) {

    // The get operator was found in the table.

    if (!meta_get.is_function()) {
      zb::print("Invalid typeof operator");
      return zs::error_code::invalid_type;
    }

    // It's a function, let's call it.

    // Call the get operator.
    // If the method find the key, it will return success error_code and set the
    // dest value. If the method doesn't find the key and wants to keep looking,
    // it will return `none`.
    push(obj);
    push(key);
    push(class_obj);

    //      zb::print(ZBASE_VNAME(obj));
    if (auto err = call(meta_get, 3, stack_size() - 3, dest)) {
      pop(3);
      return err;
    }

    pop(3);

    // A value was returned. All good.
    if (!dest->is_none()) {
      return {};
    }

    // If dest is none we need to keep looking.
  }

  // There was no operator get in this meta table.
  // Let's look deeper into delegates.
  if (delegate->has_delegate()) {
    object& sub_delegate_obj = delegate->get_delegate();
    if (!runtime_action<runtime_code::delegate_get>(obj, CREF(sub_delegate_obj), key, dest)) {
      return {};
    }
  }

  return zs::error_code::not_found;
}

ZS_DECL_RT_ACTION(class_set, objref_t obj, cobjref_t key, cobjref_t value) {
  using namespace constants;
  using enum object_type;
  using enum runtime_code;

  ZS_ASSERT(obj->is_class());
  class_object& cls = obj->as_class();

  // Start by looking directly in the table.
  if (cls.contains(key)) {
    return cls.set(key, value);
  }

  object meta_set;

  // Look for the set operator directly in the table.
  if (!cls.get(zs::_sv(k_mt_set_string), meta_set) and !meta_set.is_type(k_null, k_none)) {
    if (!meta_set.is_function()) {
      set_error("Invalid __set meta method type.\n");
      return zs::error_code::invalid_type;
    }

    // Call the set operator.
    object ret;
    return call(meta_set, { obj, key, value, obj }, ret);
  }

  if (cls.has_delegate()) {
    return runtime_action<delegate_set>(obj, REF(cls.get_delegate()), key, value);
  }

  return cls.set(key, value);
}
} // namespace zs.
