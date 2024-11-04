
namespace zs {

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
