#include <zscript.h>

namespace zs {
struct_object::struct_object(zs::engine* eng) noexcept
    : zs::reference_counted_object(eng, zs::object_type::k_struct)
    , vector_type(zs::allocator<struct_item>(eng))
    , _statics(zs::allocator<struct_item>(eng)) {}

struct_object* struct_object::create(zs::engine* eng, int_t sz) noexcept {
  struct_object* arr = internal::zs_new<memory_tag::nt_array, struct_object>(eng, eng);
  if (sz) {
    arr->resize(sz);
  }

  return arr;
}

zs::error_result struct_object::get(const object& name, object& dst) const noexcept {
  for (const auto& n : *this) {
    if (n.key == name) {
      dst = n.value;
      return {};
    }
  }

  for (const auto& n : _statics) {
    if (n.key == name) {
      dst = n.value;
      return {};
    }
  }

  return zs::error_code::not_found;
}

zs::error_result struct_object::set(const object& name, const object& obj) noexcept {
  for (auto& n : *this) {
    if (n.key == name) {

      if (n.is_const and _initialized) {
        return zs::error_code::cant_modify_const_member;
      }

      if (n.mask and !obj.has_type_mask(n.mask)) {
        return zs::error_code::invalid_type_assignment;
      }

      n.value = obj;
      return {};
    }
  }

  for (auto& n : _statics) {
    if (n.key == name) {

      if (n.is_const) {
        return zs::error_code::cant_modify_static_const;
      }

      if (n.mask and !obj.has_type_mask(n.mask)) {
        return zs::error_code::invalid_type_assignment;
      }

      n.value = obj;
      return {};
    }
  }

  return zs::error_code::inaccessible;
}

zs::error_result struct_object::set_static(const object& name, const object& obj) noexcept {
  for (auto& n : _statics) {
    if (n.key == name) {

      if (n.is_const) {
        return zs::error_code::cant_modify_static_const;
      }

      if (n.mask and !obj.has_type_mask(n.mask)) {
        return zs::error_code::invalid_type_assignment;
      }

      n.value = obj;
      return {};
    }
  }

  return zs::error_code::inaccessible;
}

zs::error_result struct_object::new_slot(
    const object& name, const object& obj, uint32_t mask, bool is_static, bool is_const) noexcept {

  if (contains(name)) {
    return zs::error_code::already_exists;
  }

  if (mask and !obj.has_type_mask(mask)) {
    return zs::error_code::invalid_type_assignment;
  }

  if (is_static) {
    _statics.emplace_back(name, obj, mask, is_const);
  }
  else {
    vector_type::emplace_back(name, obj, mask, is_const);
  }

  return {};
}

zs::error_result struct_object::new_slot(
    const object& name, uint32_t mask, bool is_static, bool is_const) noexcept {

  if (contains(name)) {
    return zs::error_code::already_exists;
  }

  if (is_static) {
    _statics.emplace_back(name, nullptr, mask, is_const);
  }
  else {
    vector_type::emplace_back(name, nullptr, mask, is_const);
  }

  return {};
}

bool struct_object::contains_member(const object& name) const noexcept {
  return this->find_if([&](const auto& n) { return n.key == name; }) != this->end();
}

bool struct_object::contains_static(const object& name) const noexcept {
  return _statics.find_if([&](const auto& n) { return n.key == name; }) != _statics.end();
}

bool struct_object::contains(const object& name) const noexcept {
  return contains_member(name) or contains_static(name);
}

struct_instance_object* struct_object::create_instance() const noexcept {
  using enum object_type;

  const zs::vector<struct_item>& this_vec = (*this);
  const size_t sz = this_vec.size();

  struct_instance_object* sobj = struct_instance_object::create(_engine, sz);
  sobj->_base = zs::object((reference_counted_object*)this, true);

  zb::span<object> ivec = sobj->get_span();

  for (size_t i = 0; i < sz; i++) {
    const struct_item& sitem = this_vec[i];

    switch (sitem.value.get_type()) {
    case k_long_string:
      ivec[i] = object(sitem.value.as_string().clone(), false);
      break;

    case k_mutable_string:
      ivec[i] = object(sitem.value.as_mutable_string().clone(), false);
      break;

    case k_closure:
      // No clone for now.
      ivec[i] = sitem.value;
      break;

    case k_native_closure:
      ivec[i] = object(sitem.value.as_native_closure().clone(), false);
      break;

    case k_class:
      ivec[i] = sitem.value;
      break;

    case k_weak_ref:
      ivec[i] = sitem.value;
      break;

    case k_function_prototype:
      ivec[i] = sitem.value;
      break;

    case k_table:
      ivec[i] = object(sitem.value.as_table().clone(), false);
      break;

    case k_array:
      ivec[i] = object(sitem.value.as_array().clone(), false);
      break;

    case k_native_array: {
      object_base obj = sitem.value;
      obj._native_array_interface = sitem.value.as_native_array_interface().clone();
      ivec[i] = object(obj, false);
      break;
    }

    case k_struct:
      ivec[i] = object(sitem.value.as_struct().clone(), false);
      break;

    case k_struct_instance:
      ivec[i] = object(sitem.value.as_struct_instance().clone(), false);
      break;

    case k_user_data:
      // No clone for now.
      ivec[i] = sitem.value;
      break;

    case k_instance:
      // No clone for now.
      ivec[i] = sitem.value;
      break;

    default:
      ivec[i] = sitem.value;
    }
  }

  return sobj;
}

struct_object* struct_object::clone() const noexcept {
  using enum object_type;

  struct_object* sobj = struct_object::create(_engine, 0);
  sobj->_constructor = _constructor;
  sobj->_statics = _statics;

  const zs::vector<struct_item>& this_vec = (*this);
  zs::vector<struct_item>& arr_vec = *sobj;

  const size_t sz = this_vec.size();

  arr_vec.resize(sz);

  for (size_t i = 0; i < sz; i++) {
    const struct_item& sitem = this_vec[i];
    struct_item& aitem = arr_vec[i];
    aitem.key = sitem.key;
    aitem.is_const = sitem.is_const;
    aitem.mask = sitem.mask;

    switch (sitem.value.get_type()) {
    case k_long_string:
      aitem.value = object(sitem.value.as_string().clone(), false);
      break;

    case k_mutable_string:
      aitem.value = object(sitem.value.as_mutable_string().clone(), false);
      break;

    case k_closure:
      // No clone for now.
      aitem.value = sitem.value;
      break;

    case k_native_closure:
      aitem.value = object(sitem.value.as_native_closure().clone(), false);
      break;

    case k_class:
      aitem.value = sitem.value;
      break;

    case k_weak_ref:
      aitem.value = sitem.value;
      break;

    case k_function_prototype:
      aitem.value = sitem.value;
      break;

    case k_table:
      aitem.value = object(sitem.value.as_table().clone(), false);
      break;

    case k_array:
      aitem.value = object(sitem.value.as_array().clone(), false);
      break;

    case k_native_array: {
      object_base obj = sitem.value;
      obj._native_array_interface = sitem.value.as_native_array_interface().clone();
      aitem.value = object(obj, false);
      break;
    }

    case k_struct:
      aitem.value = object(sitem.value.as_struct().clone(), false);
      break;

    case k_struct_instance:
      aitem.value = object(sitem.value.as_struct_instance().clone(), false);
      break;

    case k_user_data:
      // No clone for now.
      aitem.value = sitem.value;
      break;

    case k_instance:
      // No clone for now.
      aitem.value = sitem.value;
      break;

    default:
      aitem.value = sitem.value;
    }
  }

  return sobj;
}

} // namespace zs.
