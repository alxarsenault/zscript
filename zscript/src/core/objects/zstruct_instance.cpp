
//
#include <zscript/core/zcore.h>

namespace zs {
struct_instance_object::struct_instance_object(zs::engine* eng) noexcept
    : zs::reference_counted_object(eng) {}

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

zs::error_result struct_instance_object::get(const object& name, object& dst) const noexcept {

  const zs::struct_object& s = _base.as_struct();
  const size_t sz = s.size();
  for (size_t i = 0; i < sz; i++) {
    if (s[i].key == name) {
      dst = get_span()[i];
      return {};
    }
  }

  const zs::struct_object::static_member_vector& statics = _base.as_struct().get_static_members();
  for (const auto& n : statics) {
    if (n.key == name) {
      dst = n.value;
      return {};
    }
  }

  return zs::error_code::not_found;
}

zs::error_result struct_instance_object::set(const object& name, const object& obj) noexcept {
  const zs::struct_object& s = _base.as_struct();
  const size_t sz = s.size();
  for (size_t i = 0; i < sz; i++) {
    if (s[i].key == name) {

      if (s[i].is_const and _initialized) {
        return zs::error_code::cant_modify_const_member;
      }

      if (s[i].mask and !obj.has_type_mask(s[i].mask)) {
        return zs::error_code::invalid_value_type_assignment;
      }

      data()[i] = obj;
      return {};
    }
  }

  zs::struct_object::static_member_vector& statics = _base.as_struct().get_static_members();
  for (auto& n : statics) {
    if (n.key == name) {

      if (n.is_const) {
        return zs::error_code::cant_modify_static_const;
      }

      if (n.mask and !obj.has_type_mask(n.mask)) {
        return zs::error_code::invalid_value_type_assignment;
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

struct_instance_object* struct_instance_object::clone() const noexcept {
  using enum object_type;
  struct_instance_object* sobj = (struct_instance_object*)this;
  sobj->retain();
  return sobj;
}
} // namespace zs.
