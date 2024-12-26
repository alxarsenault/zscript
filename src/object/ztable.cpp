#include <zscript/zscript.h>
#include "object/zfunction_prototype.h"
namespace zs {

table_object::table_object(zs::engine* eng)
    : delegable_object(eng, zs::object_type::k_table)
    , map_type((zs::unordered_map_allocator<object, object>(eng, zs::memory_tag::nt_table))) {}

table_object* table_object::create(zs::engine* eng) noexcept {
  table_object* tbl = internal::zs_new<memory_tag::nt_table, table_object>(eng, eng);
  return tbl;
}

const zs::object* table_object::get(const object& key) const noexcept {
  auto it = map_type::find(key);
  return it == map_type::end() ? nullptr : &it->second;
}

zs::object* table_object::get(const object& key) noexcept {
  auto it = map_type::find(key);
  return it == map_type::end() ? nullptr : &it->second;
}

zs::error_result table_object::get(const object& key, object& dst) const noexcept {
  auto it = map_type::find(key);

  if (it == map_type::end()) {
    return zs::error_code::not_found;
  }

  dst = it->second;
  return {};
}

zs::error_result table_object::contains(const object& key, object& dst) const noexcept {
  dst = map_type::contains(key);
  return {};
}

bool table_object::contains(const object& key) const noexcept { return map_type::contains(key); }

zs::object table_object::get_opt(const object& key, const object& opt) const noexcept {
  auto it = map_type::find(key);

  if (it == map_type::end()) {
    return opt;
  }

  return it->second;
}

zs::error_result table_object::erase(const object& key) noexcept {
  map_type::erase(key);
  return {};
}

object table_object::clone() const noexcept {
  table_object* tbl = table_object::create(_engine);
  ((map_type&)*tbl) = *this;

  object obj(tbl, false);
  copy_delegate(obj);
  return obj;
}

zs::error_result table_object::serialize_to_json(zs::engine* eng, std::ostream& stream, int idt) {

  stream << "{\n";

  const size_t sz = this->size();
  size_t count = 0;
  idt++;
  for (auto it : *this) {

    stream << zb::indent_t(idt, 4);

    if (it.first.is_table() and this == it.first._table) {
      stream << "<RECURSION>";
    }
    else {
      zs::serialize_to_json(eng, stream, it.first, idt);
    }

    stream << ": ";

    if (it.second.is_table() and this == it.second._table) {
      stream << "<RECURSION>";
    }
    else {
      zs::serialize_to_json(eng, stream, it.second, idt);
    }

    stream << ((++count == sz) ? "\n" : ",\n");
  }

  idt--;
  stream << zb::indent_t(idt, 4) << "}";

  return {};
}
} // namespace zs.
