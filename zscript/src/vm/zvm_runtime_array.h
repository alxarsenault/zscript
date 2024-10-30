
namespace zs {

ZS_DECL_RT_ACTION(array_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  using enum runtime_code;

  ZS_ASSERT(obj->is_array());

  zs::array_object& arr = obj->as_array();

  // If the key is a number, we access the element directly.
  if (key->is_number()) {
    const int_t index = key->convert_to_integer_unchecked();
    return arr.get(index, dest);
  }

  // Array only looks in delegate for non-number keys.

  // If the array has a valid delegate, we'll go look in there first.

  if (arr.has_delegate()) {
    if (auto err = runtime_action<delegate_get>(obj, CREF(arr.get_delegate()), key, dest)) {
      dest->reset();
      return err;
    }

    // If dest is none we need to keep looking.
    if (!dest->is_none()) {
      // A value was returned. All good.
      return {};
    }
  }

  if (auto err = runtime_action<delegate_get>(obj, CREF(_default_array_delegate), key, dest)) {
    dest->reset();
    return err;
  }

  // If dest is none we need to keep looking.
  if (!dest->is_none()) {
    // A value was returned. All good.
    return {};
  }

  dest->reset();
  return zs::error_code::not_found;
}

ZS_DECL_RT_ACTION(array_set, objref_t obj, cobjref_t key, cobjref_t value) {
  array_object& arr = obj->as_array();

  if (!key->is_number()) {
    return zs::error_code::unimplemented;
  }

  const int_t sz = arr.size();

  int_t index = key->convert_to_integer_unchecked();

  if (index < 0) {
    index += sz;
  }

  return arr.set(index, value);
}
} // namespace zs.
