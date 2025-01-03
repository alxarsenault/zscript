#ifndef ZS_VIRTUAL_MACHINE_CPP
#error This file should only be included in virtual_machine.cpp
#endif // ZS_VIRTUAL_MACHINE_CPP.

namespace zs {
using enum opcode;
using enum arithmetic_op;

struct virtual_machine::proxy {

  inline static error_result string_raw_get_internal(std::string_view s, int_t u32_index, object& dest) {

    // Length of the mutable string in u32.
    const size_t u32_length = zb::unicode::length(s);

    if (u32_index < 0) {
      u32_index += u32_length;
    }

    if (u32_index < 0 or u32_index >= (int_t)u32_length) {
      return errc::out_of_bounds;
    }

    // Find the u8 index from the u32 input index.
    size_t u8_index = zb::unicode::u32_index_to_u8_index(s, u32_index, u32_length);
    if (u8_index == std::string_view::npos) {
      return errc::out_of_bounds;
    }

    uint32_t u32_char = zb::unicode::next_u8_to_u32_s(s.data() + u8_index);
    dest = object::create_char(u32_char);
    return {};
  }

  static error_result find_and_call_meta_get_func(virtual_machine* vm, const object& obj, const object& key,
      const object& delegate, object& dest, bool& keep_looking);

  static zs::error_result get(virtual_machine* vm, const object& obj, const object& key,
      const object& delegate, object& dest, bool use_meta_get = true);
};

} // namespace zs.

namespace zs {

zs::error_result virtual_machine::proxy::find_and_call_meta_get_func(virtual_machine* vm, const object& obj,
    const object& key, const object& delegate, object& dest, bool& keep_looking) {

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

  if (fct.is_table()) {

    if (auto err = proxy::get(vm, fct, key, fct, dest, true)) {
      if (err == errc::not_found) {
        keep_looking = true;
        return {};
      }

      //      if(err == errc::inaccessible) {
      //        keep_looking = true;
      //        return {};
      //      }

      keep_looking = false;
      return vm->ZS_VM_ERROR(err, "Invalid __get table type.\n");
    }

    keep_looking = false;
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

  if (dest.is_null()) {
    keep_looking = false;
    return vm->ZS_VM_ERROR(
        errc::inaccessible, "Error in __get function call with key: ", key, ", returned value is null.\n");
  }

  keep_looking = false;
  return {};
}

zs::error_result virtual_machine::proxy::get(virtual_machine* vm, const object& obj, const object& key,
    const object& delegate, object& dest, bool use_meta_get) {

  if (delegate.is_none()) {
    return errc::inaccessible;
  }

  if (delegate.is_null()) {
    if (obj.is_delegable() and !obj.as_delegable().is_use_default()) {
      return errc::inaccessible;
    }

    object def_del = vm->get_default_delegate_for_type(obj.get_type());

    if (def_del.is_null()) {
      return errc::inaccessible;
    }

    return proxy::get(vm, obj, key, def_del, dest);
  }

  //  ZS_ASSERT(delegate.is_table(), "Delgate should be a table.");

  // Start by looking directly in the object.
  if (!vm->raw_get(delegate, key, dest)) {
    return {};
  }

  bool keep_looking = false;

  if (use_meta_get) {
    if (auto err = proxy::find_and_call_meta_get_func(vm, obj, key, delegate, dest, keep_looking)) {
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

    if (obj.is_delegable() and !obj.as_delegable().is_use_default()) {
      return errc::inaccessible;
    }

    if (object def_del = vm->get_default_delegate_for_type(obj.get_type()); !def_del.is_null()) {
      return proxy::get(vm, obj, key, def_del, dest, use_meta_get);
    }
    return errc::inaccessible;
  }

  ZS_ASSERT(delegate.is_delegable(), zs::get_object_type_name(delegate.get_type()));

  zs::object dobj = delegate.as_delegable().get_delegate();

  if (dobj.is_none()) {
    return errc::inaccessible;
  }

  if (dobj.is_null()) {

    if (obj.is_delegable() and !obj.as_delegable().is_use_default()) {
      return errc::inaccessible;
    }

    if (object def_del = vm->get_default_delegate_for_type(obj.get_type()); !def_del.is_null()) {
      return proxy::get(vm, obj, key, def_del, dest, use_meta_get);
    }
  }

  if (dobj.is_table()) {

    if (auto err = proxy::get(vm, obj, key, dobj, dest, use_meta_get)) {
      if (err != errc::not_found) {
        return vm->ZS_VM_ERROR(err, "Could not find key: ", key, " in object.\n");
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

  return errc::not_found;
}

} // namespace zs.
