#include <zscript/zscript.h>
#include "object/zfunction_prototype.h"

namespace zs {
struct_object::struct_object(zs::engine* eng) noexcept
    : zs::reference_counted_object(eng, zs::object_type::k_struct)
    , vector_type(zs::allocator<struct_item>(eng))
    , _methods(zs::allocator<struct_method>(eng))
    , _statics(zs::allocator<struct_item>(eng)) {}

struct_object* struct_object::create(zs::engine* eng, int_t sz) noexcept {
  struct_object* arr = internal::zs_new<memory_tag::nt_array, struct_object>(eng, eng);
  if (sz) {
    arr->resize(sz);
  }

  return arr;
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

  struct_instance_object* sobj = struct_instance_object::create(_engine, sz);
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

object struct_object::clone() const noexcept {
  using enum object_type;

  struct_object* sobj = struct_object::create(_engine, 0);
  sobj->_constructors = _constructors;
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

  return object(sobj, false);
}

void struct_object::set_doc(const object& doc) {

  if (!_doc.is_table()) {
    _doc = zs::_t(_engine);
  }

  _doc.as_table()["description"] = doc;

  if (_name.is_string()) {
    _doc.as_table()["name"] = _name;
  }
}

void struct_object::set_member_doc(const object& name, const object& doc) {

  if (!_doc.is_table()) {
    _doc = zs::_t(_engine);
  }

  if (auto it = this->find_if([&](const auto& n) { return n.key == name; }); it != this->end()) {
    auto key = zs::_ss("members");
    if (!_doc.as_table().contains(key)) {
      _doc.as_table()[key] = zs::_a(_engine, 0);
    }

    _doc.as_table()[key].as_array().push_back(
        zs::_t(_engine, { { zs::_ss("name"), name }, { zs::_ss("description"), doc } }));
  }

  else if (auto it = _statics.find_if([&](const auto& n) { return n.key == name; }); it != _statics.end()) {
    auto key = zs::_ss("static-members");
    if (!_doc.as_table().contains(key)) {
      _doc.as_table()[key] = zs::_a(_engine, 0);
    }

    _doc.as_table()[key].as_array().push_back(
        zs::_t(_engine, { { zs::_ss("name"), name }, { zs::_ss("description"), doc } }));
  }
  else if (auto it = _methods.find_if([&](const auto& n) { return n.name == name; }); it != _methods.end()) {

    auto key = it->is_static ? zs::_ss("static-methods") : zs::_ss("methods");

    if (!_doc.as_table().contains(key)) {
      _doc.as_table()[key] = zs::_a(_engine, 0);
    }

    _doc.as_table()[key].as_array().push_back(
        zs::_t(_engine, { { zs::_ss("name"), name }, { zs::_ss("description"), doc } }));

    if (it->closure.is_closure()) {
      const auto& param_names = it->closure.as_closure().get_proto()._parameter_names;

      object params = zs::_a(_engine, 0);

      for (const auto& n : param_names) {
        params.as_array().push_back(zs::_t(_engine, { { zs::_ss("name"), n } }));

        if (const zs::local_var_info_t* loc = it->closure.as_closure().get_proto().find_local(n)) {

          if (loc->is_const()) {
            params.as_array().back().as_table()["is_const"] = loc->is_const();
          }

          if (loc->mask) {
            zs::ostringstream ss = zs::create_string_stream(_engine);
            ss << zs::object_type_mask_printer{ loc->mask };

            params.as_array().back().as_table()["mask"] = zs::_s(_engine, ss.str());
          }
        }
      }
      auto& ff = _doc.as_table()[key].as_array().back();
      ff.as_table()["parameters"] = std::move(params);
    }
  }
}

} // namespace zs.
