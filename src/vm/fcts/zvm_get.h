#ifndef ZS_VIRTUAL_MACHINE_CPP
#error This file should only be included in virtual_machine.cpp
#endif // ZS_VIRTUAL_MACHINE_CPP.

namespace zs {

template <>
inline error_result virtual_machine::proxy::get<object_type::k_integer>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return proxy::get(vm, obj, key, vm->get_default_number_delegate(), dest, true);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_float>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return proxy::get(vm, obj, key, vm->get_default_number_delegate(), dest, true);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_closure>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return proxy::get(vm, obj, key, vm->get_default_function_delegate(), dest, true);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_native_closure>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return proxy::get(vm, obj, key, vm->get_default_function_delegate(), dest, true);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_native_function>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return proxy::get(vm, obj, key, vm->get_default_function_delegate(), dest, true);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_weak_ref>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return vm->get(obj.get_weak_ref_value(), key, dest);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_table>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return proxy::get(vm, obj, key, obj, dest);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_atom>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {

  if (obj.is_atom(atom_type::atom_custom)) {
    zs::object delegate_obj;
    if (auto err = vm->get_delegated_atom_delegates_table().as_table().get(
            (int_t)obj._ex2_delegated_atom_delegate_id, delegate_obj)) {
      return err;
    }
    return proxy::get(vm, obj, key, delegate_obj, dest);
  }

  return errc::inaccessible;
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_array>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  array_object& arr = obj.as_array();

  if (!arr.has_delegate()) {
    if (key.is_integer()) {
      return arr.get(key._int, dest);
    }

    if (arr.get_use_default_delegate()) {
      return proxy::get(vm, obj, key, vm->get_default_array_delegate(), dest, true);
    }

    return errc::not_found;
  }

  if (auto err = proxy::get(vm, obj, key, arr.get_delegate(), dest, true); err and err != errc::not_found) {
    return err;
  }
  else if (err == errc::not_found) {
    if (key.is_integer()) {
      return arr.get(key._int, dest);
    }

    return errc::inaccessible;
  }

  return {};
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_user_data>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  user_data_object& udata = obj.as_udata();

  if (!udata.has_delegate()) {
    return errc::inaccessible;
  }

  return proxy::get(vm, obj, key, udata.get_delegate(), dest, true);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_struct_instance>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  struct_instance_object& sobj = obj.as_struct_instance();

  if (auto err = sobj.get(key, dest, vm->_stack[0]._struct_instance == &sobj)) {
    if (err == zs::errc::inaccessible_private) {
      return vm->ZS_VM_ERROR(err, "Could not access private struct member ", key, ".\n");
    }

    return vm->ZS_VM_ERROR(err, "Struct get. Field ", key, " doesn't exists.\n", err.message());
  }

  return {};
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_small_string>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {

  if (key.is_integer()) {
    return string_raw_get_internal(vm, obj.get_small_string_unchecked(), key._int, dest);
  }

  return proxy::get(vm, obj, key, vm->get_default_string_delegate(), dest, true);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_long_string>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {

  if (key.is_integer()) {
    return string_raw_get_internal(vm, obj.get_long_string_unchecked(), key._int, dest);
  }

  return proxy::get(vm, obj, key, vm->get_default_string_delegate(), dest, true);
}

template <>
inline error_result virtual_machine::proxy::get<object_type::k_string_view>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {

  if (key.is_integer()) {
    return string_raw_get_internal(vm, obj.get_string_view_unchecked(), key._int, dest);
  }

  return proxy::get(vm, obj, key, vm->get_default_string_delegate(), dest, true);
}

} // namespace zs.
