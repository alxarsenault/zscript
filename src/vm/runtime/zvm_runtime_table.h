#ifndef ZS_VIRTUAL_MACHINE_CPP
#error This file should only be included in virtual_machine.cpp
#endif // ZS_VIRTUAL_MACHINE_CPP.

namespace zs {

struct virtual_machine::runtime_table_proxy {

  static error_result find_and_call_meta_get_func(virtual_machine* vm, const object& obj, const object& key,
      const object& delegate, object& dest, bool& keep_looking) {

    object fct;

    if (auto err = vm->raw_get(delegate, zs::_sv(constants::k_mt_get_string), fct)) {
      keep_looking = true;
      return {};
    }

    if (fct.is_null()) {
      keep_looking = true;
      return {};
    }

    // ?????
    if (fct.is_none()) {
      keep_looking = true;
      return {};
    }

    if (!fct.is_function()) {
      keep_looking = false;
      return vm->ZS_VM_ERROR(errc::invalid_type, "Invalid __get function type.\n");
    }

    // It is a function, let's call it.

    // Call the get operator.
    // If the method find the key, it will return success error_code and set the
    // dest value. If the method doesn't find the key and wants to keep looking,
    // it will return `none`.

    if (auto err = vm->call(fct, { obj, key, delegate }, dest)) {
      if (err == errc::not_found) {
        keep_looking = true;
        return {};
      }

      keep_looking = false;
      return vm->ZS_VM_ERROR(err, "Error in __get function call with key: ", key, ".\n");
    }

    if (dest.is_none()) {
      keep_looking = true;
      return {};
    }

    keep_looking = false;
    return {};
  }
};

// zs::error_result virtual_machine::delegate_table_get(
//     const object& obj, const object& key, const object& delegate, object& dest, bool use_meta_get) {
//
//   if (delegate.is_null()) {
//     object def_del = get_default_delegate_for_type(obj.get_type());
//
//     if (def_del.is_null()) {
//       return errc::inaccessible;
//     }
//
//     return delegate_table_get(obj, key, def_del, dest);
//   }
//
//   // Start by looking directly in the object.
//   if (!raw_get(delegate, key, dest)) {
//     return {};
//   }
//
//   bool keep_looking = false;
//
//   if (use_meta_get) {
//     if (auto err
//         = runtime_table_proxy::find_and_call_meta_get_func(this, obj, key, delegate, dest, keep_looking)) {
//       return err;
//     }
//   }
//   else {
//     keep_looking = true;
//   }
//
//   if (!keep_looking) {
//     return {};
//   }
//
//   if (!delegate.is_delegable()) {
//     if (object def_del = get_default_delegate_for_type(obj.get_type()); !def_del.is_null()) {
//       return delegate_table_get(obj, key, def_del, dest, use_meta_get);
//     }
//     return errc::inaccessible;
//   }
//
//   ZS_ASSERT(delegate.is_delegable(), zs::get_object_type_name(delegate.get_type()));
//
//   zs::object& dobj = delegate.as_delegate().get_delegate();
//
//   if (dobj.is_type(object_type::k_table, object_type::k_array, object_type::k_user_data,
//   object_type::k_struct, object_type::k_struct_instance, object_type::k_mutable_string)) {
//     if (auto err = delegate_table_get(obj, key, dobj, dest, use_meta_get)) {
//       if (err != errc::not_found) {
//         return ZS_VM_ERROR(err, "Could not find key: ", key, " in object.\n");
//       }
//       return err;
//     }
//
//     // If dest is none we need to keep looking.
//     if (!dest.is_none()) {
//       // A value was returned. All good.
//       return {};
//     }
//
//     return errc::not_found;
//   }
//
//   if (dobj.is_null()) {
//     if (object def_del = get_default_delegate_for_type(obj.get_type()); !def_del.is_null()) {
//       return delegate_table_get(obj, key, def_del, dest, use_meta_get);
//     }
//   }
//
//   return errc::not_found;
// }

zs::error_result virtual_machine::delegate_table_contains(
    const object& obj, const object& key, const object& delegate, object& dest, bool use_meta_get) {

  if (delegate.is_null()) {
    if (object def_del = get_default_delegate_for_type(obj.get_type()); !def_del.is_null()) {
      return delegate_table_contains(obj, key, def_del, dest);
    }

    dest = false;
    return {};
  }

  // Start by looking directly in the object.
  if (!raw_get(delegate, key, dest)) {
    return {};
  }

  bool keep_looking = false;

  if (use_meta_get) {
    if (auto err
        = runtime_table_proxy::find_and_call_meta_get_func(this, obj, key, delegate, dest, keep_looking)) {
      return err;
    }
  }
  else {
    keep_looking = true;
  }

  if (!keep_looking) {
    return {};
  }

  if (!delegate.is_delegable()) {
    if (object def_del = get_default_delegate_for_type(obj.get_type()); !def_del.is_null()) {
      return proxy::get(this, obj, key, def_del, dest, use_meta_get);
    }
    return errc::inaccessible;
  }

  ZS_ASSERT(delegate.is_delegable(), zs::get_object_type_name(delegate.get_type()));

  zs::object& dobj = delegate.as_delegable().get_delegate();

  if (dobj.is_table()) {
    if (auto err = proxy::get(this, obj, key, dobj, dest, use_meta_get)) {
      if (err != errc::not_found) {
        return ZS_VM_ERROR(err, "Could not find key: ", key, " in object.\n");
      }
      return err;
    }

    // If dest is none we need to keep looking.
    if (!dest.is_none()) {
      // A value was returned. All good.
      return {};
    }

    return errc::not_found;
  }

  if (dobj.is_null()) {
    if (object def_del = get_default_delegate_for_type(obj.get_type()); !def_del.is_null()) {
      return proxy::get(this, obj, key, def_del, dest, use_meta_get);
    }
  }

  return errc::not_found;
}

// ZS_DECL_RT_ACTION(table_get, cobjref_t obj, cobjref_t key, objref_t dest) {
//   ZS_ASSERT(obj->is_table(), zs::get_object_type_name(obj->get_type()));
//   return delegate_table_get(obj.get(), key.get(), obj.get(), dest.get());
// }

ZS_DECL_RT_ACTION(table_set, objref_t obj, cobjref_t key, cobjref_t value) {
  using namespace constants;

  ZS_ASSERT(obj->is_table());
  table_object& tbl = obj->as_table();

  // Start by looking directly in the table.
  if (tbl.contains(key)) {
    return tbl.set(key, value);
  }

  object meta_set;

  // Look for the set operator directly in the table.
  if (!tbl.get(zs::_sv(k_mt_set_string), meta_set) and !meta_set.is_null_or_none()) {
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

  ZS_ASSERT(obj->is_table());
  table_object& tbl = obj->as_table();

  // Start by looking directly in the table.
  if (tbl.contains(key)) {
    return tbl.set(key, value);
  }

  object meta_set;

  // Look for the set operator directly in the table.
  if (!tbl.get(zs::_sv(k_mt_set_string), meta_set) and !meta_set.is_null_or_none()) {
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
