#ifndef ZS_VIRTUAL_MACHINE_CPP
#error This file should only be included in virtual_machine.cpp
#endif // ZS_VIRTUAL_MACHINE_CPP.

namespace zs {

template <>
inline error_result virtual_machine::proxy::raw_contains<object_type::k_array>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return obj.as_array().contains(key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_contains<object_type::k_table>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return obj.as_table().contains(key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_contains<object_type::k_struct>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return obj.as_struct().contains(key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_contains<object_type::k_user_data>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  dest = false;
  return {};
}

template <>
inline error_result virtual_machine::proxy::raw_contains<object_type::k_weak_ref>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return vm->raw_contains(obj.get_weak_ref_value(), key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_contains<object_type::k_long_string>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return string_raw_contains<object_type::k_long_string>(vm, obj, key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_contains<object_type::k_small_string>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return string_raw_contains<object_type::k_small_string>(vm, obj, key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_contains<object_type::k_string_view>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  return string_raw_contains<object_type::k_string_view>(vm, obj, key, dest);
}

template <>
inline error_result virtual_machine::proxy::raw_contains<object_type::k_struct_instance>(
    virtual_machine* vm, const object& obj, const object& key, object& dest) {
  zs::struct_instance_object& sobj = obj.as_struct_instance();
  return sobj.contains(key, dest, vm->_stack[0]._struct_instance == &sobj);
}

} // namespace zs.
