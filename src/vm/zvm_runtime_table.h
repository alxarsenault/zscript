
namespace zs {

ZS_DECL_RT_ACTION(table_contains, cobjref_t obj, cobjref_t key, objref_t dest) {
  using namespace constants;
  using enum object_type;
  using enum runtime_code;

  ZS_ASSERT(obj->is_table());
  table_object& tbl = obj->as_table();

  // Start by looking directly in the table.
  if ( tbl.contains(key )) {
    dest.get() =  true;
    return {};
  }

  object meta_get;

  // Look for the get operator directly in the table.
  if (!tbl.get(zs::_sv(k_mt_get_string), meta_get) and !meta_get.is_type(k_null, k_none)) {
    if (!meta_get.is_function()) {
      dest.get() =  false;
      return {};
    }

    // It is a function, let's call it.

    // Call the get operator.
    // If the method find the key, it will return success error_code and set the
    // dest value. If the method doesn't find the key and wants to keep looking,
    // it will return `none`.

    if (auto err = call(meta_get, { obj, key, obj }, dest)) {
      set_error("Error in __get meta method.\n");
      dest->reset();
      return err;
    }

    // If dest is none we need to keep looking.
    if (!dest->is_none()) {
      
      // A value was returned. All good.
      dest.get() =  true;
      return {};
    }
  }

  if (tbl.has_delegate()) {
    if (auto err = runtime_action<delegate_get>(obj, CREF(tbl.get_delegate()), key, dest)) {
      dest.get() =  false;
      return {};
    }

    // If dest is none we need to keep looking.
    if (!dest->is_none()) {
      // A value was returned. All good.
      dest.get() =  true;
      return {};
    }
  }

  if (auto err = runtime_action<delegate_get>(obj, CREF(_default_table_delegate), key, dest)) {
 
    dest.get() =  false;
    return {};
  }

  // If dest is none we need to keep looking.
  if (!dest->is_none()) {
    // A value was returned. All good.
    dest.get() =  true;
    return {};
  }

  dest.get() =  false;
  return {};
}

ZS_DECL_RT_ACTION(table_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  using namespace constants;
  using enum object_type;
  using enum runtime_code;

  ZS_ASSERT(obj->is_table());
  table_object& tbl = obj->as_table();

  // Start by looking directly in the table.
  if (!tbl.get(key, dest)) {
    return {};
  }

  object meta_get;

  // Look for the get operator directly in the table.
  if (!tbl.get(zs::_sv(k_mt_get_string), meta_get) and !meta_get.is_type(k_null, k_none)) {
    if (!meta_get.is_function()) {
      set_error("Invalid __get meta method type.\n");
      dest->reset();
      return zs::error_code::invalid_type;
    }

    // It is a function, let's call it.

    // Call the get operator.
    // If the method find the key, it will return success error_code and set the
    // dest value. If the method doesn't find the key and wants to keep looking,
    // it will return `none`.

    if (auto err = call(meta_get, { obj, key, obj }, dest)) {
      set_error("Error in __get meta method.\n");
      dest->reset();
      return err;
    }

    // If dest is none we need to keep looking.
    if (!dest->is_none()) {
      // A value was returned. All good.
      return {};
    }
  }

  if (tbl.has_delegate()) {
    if (auto err = runtime_action<delegate_get>(obj, CREF(tbl.get_delegate()), key, dest)) {
      dest->reset();
      return err;
    }

    // If dest is none we need to keep looking.
    if (!dest->is_none()) {
      // A value was returned. All good.
      return {};
    }
  }

  if (auto err = runtime_action<delegate_get>(obj, CREF(_default_table_delegate), key, dest)) {
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

ZS_DECL_RT_ACTION(table_set, objref_t obj, cobjref_t key, cobjref_t value) {
  using namespace constants;
  using enum object_type;
  using enum runtime_code;

  ZS_ASSERT(obj->is_table());
  table_object& tbl = obj->as_table();

  // Start by looking directly in the table.
  if (tbl.contains(key)) {
    return tbl.set(key, value);
  }

  object meta_set;

  // Look for the set operator directly in the table.
  if (!tbl.get(zs::_sv(k_mt_set_string), meta_set) and !meta_set.is_type(k_null, k_none)) {
    if (!meta_set.is_function()) {
      set_error("Invalid __set meta method type.\n");
      return zs::error_code::invalid_type;
    }

    // Call the set operator.
    object ret;
    auto err = call(meta_set, { obj, key, value, obj }, ret);
    if (!err) {
      return {};
    }

    if (err != zs::error_code::not_found) {
      return err;
    }
  }

  if (tbl.has_delegate()) {
    auto err = runtime_action<delegate_set>(obj, REF(tbl.get_delegate()), key, value);
    if (!err) {
      return {};
    }

    if (err != zs::error_code::not_found) {
      return err;
    }
  }

  return tbl.set(key, value);
}

ZS_DECL_RT_ACTION(table_set_if_exists, objref_t obj, cobjref_t key, cobjref_t value) {
  using namespace constants;
  using enum object_type;
  using enum runtime_code;

  ZS_ASSERT(obj->is_table());
  table_object& tbl = obj->as_table();

  // Start by looking directly in the table.
  if (tbl.contains(key)) {
    return tbl.set(key, value);
  }

  object meta_set;

  // Look for the set operator directly in the table.
  if (!tbl.get(zs::_sv(k_mt_set_string), meta_set) and !meta_set.is_type(k_null, k_none)) {
    if (!meta_set.is_function()) {
      set_error("Invalid __set meta method type.\n");
      return zs::error_code::invalid_type;
    }

    // Call the set operator.
    object ret;
    auto err = call(meta_set, { obj, key, value, obj }, ret);
    if (!err) {
      return {};
    }

    if (err != zs::error_code::not_found) {
      return err;
    }
  }

  if (tbl.has_delegate()) {
    auto err = runtime_action<delegate_set>(obj, REF(tbl.get_delegate()), key, value);
    if (!err) {
      return {};
    }

    if (err != zs::error_code::not_found) {
      return err;
    }
  }

  return zs::errc::not_found;
}
} // namespace zs.
