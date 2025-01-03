#include <zscript/zscript.h>

namespace zs {
static_assert(!std::is_polymorphic_v<array_object>, "array_object should not be polymorphic.");

array_object::array_object(zs::engine* eng) noexcept
    : delegable_object(eng, object_type::k_array)
    , _vec(nullptr) {}

array_object* array_object::create(zs::engine* eng, int_t sz) noexcept {
  array_object* arr = (array_object*)eng->allocate(
      sizeof(array_object) + sizeof(vector_type), (alloc_info_t)memory_tag::nt_array);
  zb_placement_new(arr) array_object(eng);

  arr->_vec = (vector_type*)(arr->_data);
  zb_placement_new(arr->_vec) vector_type(zs::allocator<object>(eng, memory_tag::nt_array));

  if (sz) {
    arr->_vec->resize(sz);
  }

  return arr;
}

void array_object::destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept {
  array_object* aobj = (array_object*)obj;

  if (aobj->_vec) {
    aobj->_vec->~vector_type();
    aobj->_vec = nullptr;
  }

  zs_delete(eng, aobj);
}

object array_object::clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept {
  array_object* aobj = (array_object*)obj;

  array_object* arr = array_object::create(eng, 0);
  *arr->_vec = *aobj->_vec;
  arr->set_delegate(aobj->get_delegate(), aobj->get_delegate_flags());

  return object(arr, false);
}

object array_object::create_object(zs::engine* eng, int_t sz) noexcept {
  return object(array_object::create(eng, sz), false);
}

zs::error_result array_object::get(int_t idx, object& dst) const noexcept {

  const int_t sz = _vec->size();
  zbase_assert(sz, "call array::get on an empty vector");

  if (idx < 0) {
    idx += sz;
  }

  if (idx < 0 or idx >= sz) {
    return errc::out_of_bounds;
  }

  dst = _vec->operator[](idx);
  return {};
}

zs::error_result array_object::get(const object& key, object& dst) const noexcept {
  if (key.is_integer()) {
    return get(key._int, dst);
  }

  return errc::inaccessible;
}

zs::error_result array_object::contains(const object& key, object& dst) const noexcept {

  if (key.is_integer()) {
    const int_t sz = _vec->size();
    if (!sz) {
      dst = false;
      return {};
    }

    int_t idx = key._int;

    if (idx < 0) {
      idx += sz;
    }

    dst = idx >= 0 && idx < sz;
    return {};
  }

  dst = false;
  return {};
}

zs::error_result array_object::set(int_t idx, const object& obj) noexcept {

  const int_t sz = _vec->size();
  zbase_assert(sz, "call array::set in an empty vector");

  if (idx < 0) {
    idx += sz;
    idx %= sz;
  }

  if (idx >= sz) {
    return zs::error_code::out_of_bounds;
  }

  _vec->operator[](idx) = obj;
  return {};
}

zs::error_result array_object::set(int_t idx, object&& obj) noexcept {

  const int_t sz = _vec->size();
  zbase_assert(sz, "call array::set in an empty vector");

  if (idx < 0) {
    idx += sz;
    idx %= sz;
  }

  if (idx >= sz) {
    return zs::error_code::out_of_bounds;
  }

  _vec->operator[](idx) = std::move(obj);
  return {};
}

zs::error_result array_object::push(const object& obj) noexcept {
  _vec->push_back(obj);
  return {};
}

zs::error_result array_object::push(object&& obj) noexcept {
  _vec->push_back(std::move(obj));
  return {};
}

bool array_object::is_number_array() const noexcept { return is_type_mask_array(object_base::k_number_mask); }

bool array_object::is_number_array(bool& has_float) const noexcept {
  const size_t sz = _vec->size();
  for (size_t i = 0; i < sz; i++) {
    if (!_vec->operator[](i).has_type_mask(object_base::k_number_mask)) {
      return false;
    }

    has_float = has_float or _vec->operator[](i).is_float();
  }

  return true;
}

bool array_object::is_string_array() const noexcept { return is_type_mask_array(object_base::k_string_mask); }

bool array_object::is_type_array(object_type t) const noexcept {
  const size_t sz = _vec->size();
  for (size_t i = 0; i < sz; i++) {
    if (!_vec->operator[](i).is_type(t)) {
      return false;
    }
  }

  return true;
}

bool array_object::is_type_mask_array(uint32_t tflags) const noexcept {
  const size_t sz = _vec->size();
  for (size_t i = 0; i < sz; i++) {
    if (!_vec->operator[](i).has_type_mask(tflags)) {
      return false;
    }
  }

  return true;
}

uint32_t array_object::get_array_type_mask() const noexcept {
  uint32_t tflags = 0;
  const size_t sz = _vec->size();
  for (size_t i = 0; i < sz; i++) {
    tflags |= get_object_type_mask(_vec->operator[](i).get_type());
  }

  return tflags;
}

zs::error_result array_object::serialize_to_json(zs::engine* eng, std::ostream& stream, int idt) {

  size_t sz = _vec->size();

  if (sz == 0) {
    stream << "[]";
    return {};
  }

  if (this->is_number_array() or this->is_string_array()) {
    stream << "[";

    for (size_t i = 0; i < sz - 1; i++) {
      zs::serialize_to_json(eng, stream, (*_vec)[i], idt);
      stream << ", ";
    }
    zs::serialize_to_json(eng, stream, _vec->back(), idt);
    stream << "]";
    return {};
  }
  else {
    stream << "[\n";
    idt++;
    stream << zb::indent_t(idt, 4);
    for (size_t i = 0; i < sz - 1; i++) {

      zs::serialize_to_json(eng, stream, (*_vec)[i], idt);
      stream << ",\n";
      stream << zb::indent_t(idt, 4);
    }

    zs::serialize_to_json(eng, stream, _vec->back(), idt);
    idt--;
    stream << "\n" << zb::indent_t(idt, 4) << "]";
    return {};
  }
  return {};
}
} // namespace zs.
