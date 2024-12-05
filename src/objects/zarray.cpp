#include <zscript.h>

namespace zs {
array_object::array_object(zs::engine* eng) noexcept
    : delegate_object(eng, zs::object_type::k_array)
    , vector_type(zs::allocator<object>(eng, memory_tag::nt_array)) {

  //  _delegate = eng->get_type_delegate(object_type::k_array);
}

array_object* array_object::create(zs::engine* eng, int_t sz) noexcept {
  array_object* arr = zs_new<memory_tag::nt_array, array_object>(eng, eng);

  if (sz) {
    arr->resize(sz);
  }

  return arr;
}

zs::error_result array_object::get(int_t idx, object& dst) const noexcept {

  const int_t sz = vector_type::size();
  zbase_assert(sz, "call array::get in an empty vector");

  if (idx < 0) {
    idx += sz;
    idx %= sz;
  }

  if (idx >= sz) {
    return zs::error_code::out_of_bounds;
  }

  dst = vector_type::operator[](idx);
  return {};
}

zs::error_result array_object::set(int_t idx, const object& obj) noexcept {

  const int_t sz = vector_type::size();
  zbase_assert(sz, "call array::set in an empty vector");

  if (idx < 0) {
    idx += sz;
    idx %= sz;
  }

  if (idx >= sz) {
    return zs::error_code::out_of_bounds;
  }

  vector_type::operator[](idx) = obj;
  return {};
}

zs::error_result array_object::set(int_t idx, object&& obj) noexcept {

  const int_t sz = vector_type::size();
  zbase_assert(sz, "call array::set in an empty vector");

  if (idx < 0) {
    idx += sz;
    idx %= sz;
  }

  if (idx >= sz) {
    return zs::error_code::out_of_bounds;
  }

  vector_type::operator[](idx) = std::move(obj);
  return {};
}

zs::error_result array_object::push(const object& obj) noexcept {
  vector_type::push_back(obj);
  return {};
}

zs::error_result array_object::push(object&& obj) noexcept {
  vector_type::push_back(std::move(obj));
  return {};
}

array_object* array_object::clone() const noexcept {
  array_object* arr = array_object::create(reference_counted_object::_engine, 0);
  ((vector_type&)*arr) = *this;
  //  arr->insert(arr->begin(), this->begin(), this->end());
  return arr;
}

bool array_object::is_number_array() const noexcept { return is_type_mask_array(object_base::k_number_mask); }

bool array_object::is_string_array() const noexcept { return is_type_mask_array(object_base::k_string_mask); }

bool array_object::is_type_array(object_type t) const noexcept {
  const size_t sz = vector_type::size();
  for (size_t i = 0; i < sz; i++) {
    if (!vector_type::operator[](i).is_type(t)) {
      return false;
    }
  }

  return true;
}

bool array_object::is_type_mask_array(uint32_t tflags) const noexcept {
  const size_t sz = vector_type::size();
  for (size_t i = 0; i < sz; i++) {
    if (!vector_type::operator[](i).has_type_mask(tflags)) {
      return false;
    }
  }

  return true;
}
uint32_t array_object::get_array_type_mask() const noexcept {
  uint32_t tflags = 0;
  const size_t sz = vector_type::size();
  for (size_t i = 0; i < sz; i++) {
    tflags |= get_object_type_mask(vector_type::operator[](i).get_type());
  }

  return tflags;
}

zs::error_result array_object::serialize_to_json(zs::engine* eng, std::ostream& stream, int idt) {

  size_t sz = this->size();

  if (sz == 0) {
    stream << "[]";
    return {};
  }

  if (this->is_number_array() or this->is_string_array()) {
    stream << "[";

    for (size_t i = 0; i < sz - 1; i++) {
      zs::serialize_to_json(eng, stream, (*this)[i], idt);
      stream << ", ";
    }
    zs::serialize_to_json(eng, stream, this->back(), idt);
    stream << "]";
    return {};
  }
  else {
    stream << "[\n";
    idt++;
    stream << zb::indent_t(idt, 4);
    for (size_t i = 0; i < sz - 1; i++) {

      zs::serialize_to_json(eng, stream, (*this)[i], idt);
      stream << ",\n";
      stream << zb::indent_t(idt, 4);
    }

    zs::serialize_to_json(eng, stream, this->back(), idt);
    idt--;
    stream << "\n" << zb::indent_t(idt, 4) << "]";
    return {};
  }

  //
  //
  //
  //
  ///
  ///

  //  bool is_small_array = true;
  //
  //  stream << "[";
  //
  //  if (const size_t sz = this->size()) {
  //    for (const auto& item :*this) {
  //      if (item.is_ref_counted()) {
  //        is_small_array = false;
  //        break;
  //      }
  //    }
  //
  //    if (!is_small_array) {
  //      stream << "\n";
  //      idt++;
  //
  //    }
  //
  //    for (size_t i = 0; i < sz - 1; i++) {
  //      stream << zb::indent_t(idt, 4);
  ////      stream << zs::serializer(s.type, arr[i], indent);
  //      zs::serialize_to_json( eng, stream, (*this)[i],idt ) ;
  //      //       if (is_small_array) {
  //      stream << ",";
  //      //       }
  //      //       else {
  //      //         stream << ",";
  //      //         //          stream << ",\n" << zb::indent_t(indent);
  //      //       }
  //    }
  //
  //
  //    stream << zb::indent_t(idt, 4);
  //    zs::serialize_to_json( eng, stream, this->back(),idt ) ;
  ////    stream << zs::serializer(s.type, this->back(), idt);
  //  }
  //
  //
  //
  //       if (is_small_array) {
  //  stream << "]";
  //         return {};
  //       }
  //
  //  idt--;
  //        stream << "\n" << zb::indent_t(idt, 4) << "]";
  return {};
}
} // namespace zs.
