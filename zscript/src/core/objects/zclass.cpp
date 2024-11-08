#include <zscript/zscript.h>

namespace zs {

class_object::class_object(zs::engine* eng)
    : delegate_object(eng, zs::object_type::k_class)
    , map_type((zs::unordered_object_map_allocator(eng))) {}

class_object* class_object::create(zs::engine* eng) noexcept {
  class_object* cls = internal::zs_new<memory_tag::nt_class, class_object>(eng, eng);

  return cls;
}

object class_object::create_instance() {
  object self_obj;
  self_obj._type = object_type::k_class;
  self_obj._class = this;
  this->retain();

  class_instance_object* cls_instance = class_instance_object::create(_engine, self_obj);

  object instance;
  instance._type = object_type::k_instance;
  instance._instance = cls_instance;
  return instance;
}

const zs::object* class_object::get(const object& key) const noexcept {
  auto it = map_type::find(key);
  return it == map_type::end() ? nullptr : &it->second;
}

zs::object* class_object::get(const object& key) noexcept {
  auto it = map_type::find(key);
  return it == map_type::end() ? nullptr : &it->second;
}

zs::error_result class_object::get(const object& key, object& dst) const noexcept {
  auto it = map_type::find(key);

  if (it == map_type::end()) {
    return zs::error_code::not_found;
  }

  dst = it->second;
  return {};
}

zs::object class_object::get_opt(const object& key, const object& opt) const noexcept {
  auto it = map_type::find(key);

  if (it == map_type::end()) {
    return opt;
  }

  return it->second;
}

zs::error_result class_object::set(const object& key, const object& obj) noexcept {
  map_type::insert_or_assign(key, obj);
  return {};
}

zs::error_result class_object::set(const object& key, object&& obj) noexcept {
  map_type::insert_or_assign(key, std::move(obj));
  return {};
}

zs::error_result class_object::set(object&& key, const object& obj) noexcept {
  map_type::insert_or_assign(std::move(key), obj);
  return {};
}

zs::error_result class_object::set(object&& key, object&& obj) noexcept {
  map_type::insert_or_assign(std::move(key), std::move(obj));
  return {};
}

// table_object* table_object::clone() const noexcept {
//   table_object* tbl = table_object::create(_engine);
//   ((map_type&)*tbl) = *this;
//   //  tbl->insert(this->begin(), this->end());
//   return tbl;
// }

class_instance_object::class_instance_object(zs::engine* eng, zs::object cls)
    : delegate_object(eng, zs::object_type::k_instance)
    , map_type((zs::unordered_map_allocator<object, object>(eng))) {
  _cls = cls;
  set_delegate(_cls);
}

class_instance_object* class_instance_object::create(zs::engine* eng, zs::object cls) noexcept {
  class_instance_object* cls_instance
      = internal::zs_new<memory_tag::nt_class, class_instance_object>(eng, eng, cls);
  return cls_instance;
}

const zs::object* class_instance_object::get(const object& key) const noexcept {
  auto it = map_type::find(key);
  return it == map_type::end() ? nullptr : &it->second;
}

zs::object* class_instance_object::get(const object& key) noexcept {
  auto it = map_type::find(key);
  return it == map_type::end() ? nullptr : &it->second;
}

zs::error_result class_instance_object::get(const object& key, object& dst) const noexcept {
  auto it = map_type::find(key);

  if (it == map_type::end()) {
    return zs::error_code::not_found;
  }

  dst = it->second;
  return {};
}

zs::object class_instance_object::get_opt(const object& key, const object& opt) const noexcept {
  auto it = map_type::find(key);

  if (it == map_type::end()) {
    return opt;
  }

  return it->second;
}

zs::error_result class_instance_object::set(const object& key, const object& obj) noexcept {
  map_type::insert_or_assign(key, obj);
  return {};
}

zs::error_result class_instance_object::set(const object& key, object&& obj) noexcept {
  map_type::insert_or_assign(key, std::move(obj));
  return {};
}

zs::error_result class_instance_object::set(object&& key, const object& obj) noexcept {
  map_type::insert_or_assign(std::move(key), obj);
  return {};
}

zs::error_result class_instance_object::set(object&& key, object&& obj) noexcept {
  map_type::insert_or_assign(std::move(key), std::move(obj));
  return {};
}

} // namespace zs.
