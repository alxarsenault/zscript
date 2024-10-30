
namespace zs {
namespace {

  template <class Fct>
  static inline int_t native_array_op(zs::vm_ref vm, const object& obj, Fct&& fct) {

    switch (obj._na_type) {
#define _X(name, str, type)     \
  case native_array_type::name: \
    return fct(obj.as_native_array<type>());

      ZS_NATIVE_ARRAY_TYPE_ENUM(_X);

#undef _X
    case native_array_type::n_invalid:
      ZBASE_NO_BREAK;
    default:
      vm.set_error("Invalid native_array type in native_array::size().");
      return -1;
    }
  }

  static inline int_t native_array_size_impl(zs::vm_ref vm) noexcept {
    if (vm.stack_size() != 1) {
      vm.set_error("Invalid number of arguments in native_array::size().");
      return -1;
    }

    const object& obj = vm[0];
    if (!obj.is_native_array()) {
      vm.set_error("Invalid native_array object in native_array::size().");
      return -1;
    }

    if (obj._na_type == native_array_type::n_invalid) {
      vm.set_error("Invalid native_array type in native_array::size().");
      return -1;
    }

    return native_array_op(vm, obj, [&](auto& arr) { return vm.push(arr.size()); });
  }
} // namespace.

zs::object create_native_array_default_delegate(zs::engine* eng) {
  object obj = object::create_table(eng);
  zs::table_object& t = obj.as_table();

  t["size"] = t["length"] = _nf(native_array_size_impl);
  return obj;
}
} // namespace zs.
