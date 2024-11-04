#ifndef ZS_VIRTUAL_MACHINE_CPP
#error This file should only be included in virtual_machine.cpp
#endif // ZS_VIRTUAL_MACHINE_CPP.

namespace zs {

//
// proxy::raw_get.
//

template <>
inline error_result virtual_machine::proxy::raw_get<object_type::k_array>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return obj.as_array().get(key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_get<object_type::k_table>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return obj.as_table().get(key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_get<object_type::k_struct>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return obj.as_struct().get(key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_get<object_type::k_user_data>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  vm->set_error("Can't get a value from '", zs::get_object_type_name(obj.get_type()), "'.\n");
  dest.reset();
  return zs::error_code::inaccessible;
}

template <>
inline error_result virtual_machine::proxy::raw_get<object_type::k_weak_ref>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return vm->raw_get(obj.get_weak_ref_value(), key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_get<object_type::k_long_string>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return string_raw_get<object_type::k_long_string>(vm, obj, key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_get<object_type::k_small_string>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return string_raw_get<object_type::k_small_string>(vm, obj, key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_get<object_type::k_string_view>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return string_raw_get<object_type::k_string_view>(vm, obj, key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_get<object_type::k_struct_instance>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {

  zs::struct_instance_object& sobj = obj.as_struct_instance();

  if (auto err = sobj.get(key, dest, vm->_stack[0]._struct_instance == &sobj)) {

    if (err == zs::errc::inaccessible_private) {
      vm->set_error("Could not access private struct member ", key, ".\n");
    }
    else {
      vm->set_error("Struct get. Field ", key, " doesn't exists.\n", err.message());
    }

    return err;
  }

  return {};
}

} // namespace zs.
