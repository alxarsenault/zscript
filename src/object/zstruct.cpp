#include <zscript/zscript.h>
#include "object/zfunction_prototype.h"

namespace zs {
struct_object::struct_object(zs::engine* eng) noexcept
    : zs::reference_counted_object(eng, object_type::k_struct)
    , vector_type(zs::allocator<struct_item>(eng))
    , _statics(zs::allocator<struct_item>(eng))
    , _methods(zs::allocator<struct_method>(eng)) {}

struct_object* struct_object::create(zs::engine* eng, int_t sz) noexcept {
  struct_object* arr = internal::zs_new<memory_tag::nt_struct, struct_object>(eng, eng);
  if (sz) {
    arr->resize(sz);
  }

  return arr;
}

void struct_object::destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept {
  struct_object* sobj = (struct_object*)obj;

  zs_delete(eng, sobj);
}

object struct_object::clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept {
  struct_object* sobj = (struct_object*)obj;

  using enum object_type;

  struct_object* out_sobj = struct_object::create(eng, 0);
  out_sobj->_constructors = sobj->_constructors;
  out_sobj->_statics = sobj->_statics;

  const zs::vector<struct_item>& this_vec = *sobj;
  zs::vector<struct_item>& arr_vec = *out_sobj;

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
      aitem.value = sitem.value.as_string().clone();
      break;

    case k_closure:
      // No clone for now.
      aitem.value = sitem.value;
      break;

    case k_native_closure:
      aitem.value = sitem.value.as_native_closure().clone();
      break;

    case k_weak_ref:
      aitem.value = sitem.value;
      break;

    case k_table:
      aitem.value = sitem.value.as_table().clone();
      break;

    case k_array:
      aitem.value = sitem.value.as_array().clone();
      break;

    case k_struct:
      aitem.value = sitem.value.as_struct().clone();
      break;

    case k_struct_instance:
      aitem.value = sitem.value.as_struct_instance().clone();
      break;

    case k_user_data:
      // No clone for now.
      aitem.value = sitem.value;
      break;

    default:
      aitem.value = sitem.value;
    }
  }

  return object(out_sobj, false);
}

object struct_object::create_object(zs::engine* eng, int_t sz) noexcept {
  return object(struct_object::create(eng, sz), false);
}

zs::error_result struct_object::get(const object& name, object& dst) const noexcept {
  //  for (const auto& n : *this) {
  //    if (n.key == name) {
  //      dst = n.value;
  //      return {};
  //    }
  //  }

  for (const auto& n : _statics) {
    if (n.key == name) {
      dst = n.value;
      return {};
    }
  }

  //  for (const auto& n : _methods) {
  //    if (n.name == name) {
  //      dst = n.closure;
  //      return {};
  //    }
  //  }

  for (const auto& n : _methods) {
    if (n.name == name and n.is_static) {
      dst = n.closure;
      return {};
    }
  }

  return zs::error_code::not_found;
}

zs::error_result struct_object::contains(const object& name, object& dst) const noexcept {
  for (const auto& n : _statics) {
    if (n.key == name) {
      dst = true;
      return {};
    }
  }

  for (const auto& n : _methods) {
    if (n.name == name and n.is_static) {
      dst = true;
      return {};
    }
  }

  dst = false;
  return {};
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

zs::error_result struct_object::new_slot(const object& name, const object& value, uint32_t mask,
    bool is_static, bool is_private, bool is_const) noexcept {

  if (contains(name)) {
    return zs::error_code::already_exists;
  }

  if (mask and !value.has_type_mask(mask)) {
    return zs::error_code::invalid_type_assignment;
  }

  if (is_static) {
    _statics.emplace_back(name, value, mask, is_private, is_const);
  }
  else {
    vector_type::emplace_back(name, value, mask, is_private, is_const);
  }

  return {};
}

zs::error_result struct_object::new_slot(
    const object& name, uint32_t mask, bool is_static, bool is_private, bool is_const) noexcept {

  if (contains(name)) {
    return zs::error_code::already_exists;
  }

  if (is_static) {
    _statics.emplace_back(name, nullptr, mask, is_private, is_const);
  }
  else {
    vector_type::emplace_back(name, nullptr, mask, is_private, is_const);
  }

  return {};
}

zs::error_result struct_object::new_slot(
    const object& name, const object& value, uint32_t mask, zs::variable_attribute_t vflags) noexcept {

  if (contains(name)) {
    return zs::error_code::already_exists;
  }

  if (mask and !value.has_type_mask(mask)) {
    return zs::error_code::invalid_type_assignment;
  }

  if (zb::has_flag<variable_attribute_t::va_static>(vflags)) {
    _statics.emplace_back(name, value, mask, zb::has_flag<variable_attribute_t::va_private>(vflags),
        zb::has_flag<variable_attribute_t::va_const>(vflags));
  }
  else {
    vector_type::emplace_back(name, value, mask, zb::has_flag<variable_attribute_t::va_private>(vflags),
        zb::has_flag<variable_attribute_t::va_const>(vflags));
  }

  return {};
}

zs::error_result struct_object::new_slot(
    const object& name, uint32_t mask, zs::variable_attribute_t vflags) noexcept {

  if (contains(name)) {
    return zs::error_code::already_exists;
  }

  if (zb::has_flag<variable_attribute_t::va_static>(vflags)) {
    _statics.emplace_back(name, nullptr, mask, zb::has_flag<variable_attribute_t::va_private>(vflags),
        zb::has_flag<variable_attribute_t::va_const>(vflags));
  }
  else {
    vector_type::emplace_back(name, nullptr, mask, zb::has_flag<variable_attribute_t::va_private>(vflags),
        zb::has_flag<variable_attribute_t::va_const>(vflags));
  }

  return {};
}

zs::error_result struct_object::new_method(
    const object& name, const object& closure, bool is_private, bool is_const) noexcept {
  if (contains(name)) {
    return zs::error_code::already_exists;
  }

  _methods.emplace_back(name, closure, is_private, is_const);
  return {};
}

zs::error_result struct_object::new_static_method(
    const object& name, const object& closure, bool is_private, bool is_const) noexcept {
  if (contains(name)) {
    return zs::error_code::already_exists;
  }

  _methods.emplace_back(name, closure, is_private, is_const);
  _methods.back().is_static = true;
  return {};
}

bool struct_object::contains_member(const object& name) const noexcept {
  return this->find_if([&](const auto& n) { return n.key == name; }) != this->end();
}

bool struct_object::contains_method(const object& name) const noexcept {
  return _methods.find_if([&](const auto& m) { return m.name == name; }) != _methods.end();
}

bool struct_object::contains_static(const object& name) const noexcept {
  return _statics.find_if([&](const auto& n) { return n.key == name; }) != _statics.end();
}

bool struct_object::contains(const object& name) const noexcept {
  return contains_member(name) or contains_static(name) or contains_method(name);
}

size_t struct_object::get_constructors_count() const noexcept {
  if (_constructors.is_array()) {
    return _constructors.as_array().size();
  }

  return (size_t)_constructors.is_function();
}

struct_instance_object* struct_object::create_instance() const noexcept {
  using enum object_type;

  const zs::vector<struct_item>& this_vec = (*this);
  const size_t sz = this_vec.size();

  struct_instance_object* sobj = struct_instance_object::create(get_engine(), sz);
  sobj->_base = zs::object((reference_counted_object*)this, true);

  zb::span<object> ivec = sobj->get_span();

  for (size_t i = 0; i < sz; i++) {
    const struct_item& sitem = this_vec[i];

    switch (sitem.value.get_type()) {
    case k_long_string:
      ivec[i] = sitem.value.as_string().clone();
      break;

    case k_closure:
      // No clone for now.
      ivec[i] = sitem.value;
      break;

    case k_native_closure:
      ivec[i] = sitem.value.as_native_closure().clone();
      break;

    case k_weak_ref:
      ivec[i] = sitem.value;
      break;

    case k_table:
      ivec[i] = sitem.value.as_table().clone();
      break;

    case k_array:
      ivec[i] = sitem.value.as_array().clone();
      break;

    case k_struct:
      ivec[i] = sitem.value.as_struct().clone();
      break;

    case k_struct_instance:
      ivec[i] = sitem.value.as_struct_instance().clone();
      break;

    case k_user_data:
      // No clone for now.
      ivec[i] = sitem.value;
      break;

    default:
      ivec[i] = sitem.value;
    }
  }

  return sobj;
}

struct constructor_info {
  const object* obj;
  int_t n_type_match;

  inline const zs::closure_object& get_closure() const noexcept { return obj->as_closure(); }

  inline const object& get_object() const noexcept { return *obj; }
};

inline static bool is_closure_with_invalid_param_count(const object& obj, int_t n_params) {
  return obj.is_closure() and !obj.as_closure().is_possible_parameter_count(n_params);
}

zs::error_result struct_object::resolve_constructor(
    zs::vm_ref vm, zs::parameter_list params, zs::object& constructor) {
  const int_t n_params = params.size();

  // Call default constructor.
  if (n_params == 1 and has_default_constructor()) {
    return {};
  }

  // If the struct has no constructor, we could still call the default constructor,
  // only if there is no parameters other than 'this'.
  if (!has_constructors()) {
    return n_params == 1 ? errc::success : errc::invalid_parameter_count;
  }

  // The struct has only one constructor, strct_obj._constructors is a function.
  if (has_single_constructor()) {
    const zs::object& constructor_obj = _constructors;

    if (constructor_obj.is_closure()) {

      int_t n_type_match = -1;
      if (!constructor_obj.as_closure().is_valid_parameters(vm, params, n_type_match)) {
        return zs::errc::invalid_parameters;
      }
    }

    constructor = constructor_obj;
    return {};
  }

  // If 'has_multi_constructors' returns false, there's no constructor.
  ZS_ASSERT(has_multi_constructors());

  // An array of constructors.
  const zs::array_object& strc_ctors = _constructors.as_array();

  zs::small_vector<constructor_info, 8> potential_constructors(
      (zs::allocator<constructor_info>(vm.get_engine())));

  for (const object& obj : strc_ctors) {
    if (!is_closure_with_invalid_param_count(obj, n_params) and obj.is_function()) {
      potential_constructors.push_back({ &obj, -1 });
    }
  }

  for (auto it = potential_constructors.begin(); it != potential_constructors.end();) {
    if (!it->obj->is_closure()) {
      ++it;
      continue;
    }

    int_t n_type_match = -1;
    if (!it->get_closure().is_valid_parameters(vm, params, n_type_match)) {
      it = potential_constructors.erase(it);
      continue;
    }

    it->n_type_match = n_type_match;
    ++it;
  }

  if (potential_constructors.size() == 1) {
    constructor = potential_constructors.back().get_object();
    return {};
  }

  if (potential_constructors.empty()) {
    return zs::errc::invalid_operation;
  }

  return zs::errc::invalid;
}
} // namespace zs.
