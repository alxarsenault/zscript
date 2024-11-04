#include <zscript/zscript.h>

namespace zs {
struct_instance_object::struct_instance_object(zs::engine* eng) noexcept
    : zs::reference_counted_object(eng, zs::object_type::k_struct_instance) {}

struct_instance_object::~struct_instance_object() {
  zb::span<object> sp = get_span();

  const size_t sz = sp.size();
  for (size_t i = 0; i < sz; i++) {
    sp[i].~object();
  }
}

struct_instance_object* struct_instance_object::create(zs::engine* eng, int_t sz) noexcept {
  struct_instance_object* sobj = (struct_instance_object*)eng->allocate(
      sizeof(struct_instance_object) + sz * sizeof(object), (alloc_info_t)memory_tag::nt_string);
  sobj = zb_placement_new(sobj) struct_instance_object(eng);
  sobj->_size = sz;

  zb::span<object> sp = sobj->get_span();
  for (auto& obj : sp) {
    zb_placement_new(&obj) object();
  }

  return sobj;
}

zs::error_result struct_instance_object::get(
    const object& name, object& dst, bool can_access_private) const noexcept {
  ZS_ASSERT(_base.is_struct(), "Invalid struct instance base type.");

  const zs::struct_object& b = _base.as_struct();

  if (b.get_name() == name) {
    dst = _base;
    return {};
  }

  const size_t sz = b.size();

  if (name.is_integer()) {

    int_t ivalue = name._int;

    if (ivalue < 0 or ivalue >= sz) {
      return zs::errc::out_of_bounds;
    }

    if (!b[ivalue].is_private or can_access_private) {
      dst = data()[ivalue];
      return {};
    }

    return zs::errc::inaccessible_private;
  }

  for (size_t i = 0; i < sz; i++) {
    if (b[i].key == name) {
      if (!b[i].is_private or can_access_private) {
        dst = data()[i];
        return {};
      }

      return zs::errc::inaccessible_private;
    }
  }

  const zs::struct_object::static_member_vector& statics = b.get_static_members();
  for (const auto& n : statics) {
    if (n.key == name) {
      dst = n.value;
      return {};
    }
  }

  const zs::struct_object::method_vector& methods = b.get_methods();
  for (const auto& n : methods) {
    if (n.name == name) {

      if (!n.is_private or can_access_private) {
        dst = n.closure;
        return {};
      }

      return zs::errc::inaccessible_private;
    }
  }

  return zs::errc::not_found;
}

zs::error_result struct_instance_object::get_meta_method(const object& name, object& dst) const noexcept {
  ZS_ASSERT(_base.is_struct(), "Invalid struct instance base type.");
  ZS_ASSERT(name.is_string(), "Invalid meta method name parameter.");

  const zs::struct_object& b = _base.as_struct();

  for (const auto& n : b.get_methods()) {
    if (n.name == name) {
      dst = n.closure;
      return {};
    }
  }

  // TODO: Should we prevent meta method from being static?
  for (const auto& n : b.get_static_members()) {
    if (n.key == name) {
      dst = n.value;
      return {};
    }
  }

  // TODO: Should we prevent meta method from being declared as member value?
  const size_t sz = b.size();
  for (size_t i = 0; i < sz; i++) {
    if (b[i].key == name) {
      dst = data()[i];
      return {};
    }
  }

  return zs::errc::not_found;
}

const zs::object* struct_instance_object::get_meta_method(const object& name) const noexcept {
  ZS_ASSERT(_base.is_struct(), "Invalid struct instance base type.");
  ZS_ASSERT(name.is_string(), "Invalid meta method name parameter.");

  const zs::struct_object& b = _base.as_struct();

  for (const auto& n : b.get_methods()) {
    if (n.name == name) {
      return &n.closure;
    }
  }

  // TODO: Should we prevent meta method from being static?
  for (const auto& n : b.get_static_members()) {
    if (n.key == name) {
      return &n.value;
    }
  }

  // TODO: Should we prevent meta method from being declared as member value?
  const size_t sz = b.size();
  for (size_t i = 0; i < sz; i++) {
    if (b[i].key == name) {
      return &data()[i];
    }
  }

  return nullptr;
}

zs::error_result struct_instance_object::contains(
    const object& name, object& dst, bool can_access_private) const noexcept {
  ZS_ASSERT(_base.is_struct(), "Invalid struct instance base type.");

  const zs::struct_object& s = _base.as_struct();

  if (s.get_name() == name) {
    dst = true;
    return {};
  }

  const size_t sz = s.size();

  if (name.is_integer()) {

    int_t ivalue = name._int;

    if (ivalue < 0 or ivalue >= sz) {
      dst = false;
      return {};
    }

    if (!s[ivalue].is_private or can_access_private) {
      dst = true;
      return {};
    }

    dst = false;
    return {};
  }

  for (size_t i = 0; i < sz; i++) {
    if (s[i].key == name) {
      if (!s[i].is_private or can_access_private) {
        dst = true;
        return {};
      }

      dst = false;
      return {};
    }
  }

  const zs::struct_object::static_member_vector& statics = _base.as_struct().get_static_members();
  for (const auto& n : statics) {
    if (n.key == name) {
      dst = true;
      return {};
    }
  }

  const zs::struct_object::method_vector& methods = _base.as_struct().get_methods();
  for (const auto& n : methods) {
    if (n.name == name) {

      if (!n.is_private or can_access_private) {
        dst = true;
        return {};
      }

      dst = false;
      return {};
    }
  }

  dst = false;
  return {};
}

zs::error_result struct_instance_object::set(
    const object& name, const object& obj, bool can_access_private) noexcept {
  ZS_ASSERT(_base.is_struct(), "Invalid struct instance base type.");

  const zs::struct_object& s = _base.as_struct();
  const size_t sz = s.size();
  for (size_t i = 0; i < sz; i++) {
    if (s[i].key == name) {

      if (!s[i].is_private or can_access_private) {
        data()[i] = obj;
        return {};
      }
 
      return zs::errc::inaccessible_private;
    }
  }

  zs::struct_object::static_member_vector& statics = _base.as_struct().get_static_members();
  for (auto& n : statics) {
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

bool struct_instance_object::contains_member(const object& name) const noexcept {
  return _base.as_struct().contains_member(name);
}

bool struct_instance_object::contains_static(const object& name) const noexcept {
  return _base.as_struct().contains_static(name);
}

bool struct_instance_object::contains(const object& name) const noexcept {
  return contains_member(name) or contains_static(name);
}

zb::span<const struct_item> struct_instance_object::base_vector() const noexcept { return _base.as_struct(); }

const object& struct_instance_object::key(size_t index) const noexcept {
  return _base.as_struct()[index].key;
}

object& struct_instance_object::key(size_t index) noexcept { return _base.as_struct()[index].key; }

object struct_instance_object::clone() const noexcept {
  using enum object_type;
  struct_instance_object* sobj = (struct_instance_object*)this;
  sobj->retain();
  return object(sobj, false);}
} // namespace zs.
