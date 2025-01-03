#include <zscript/zscript.h>
#include "object/zfunction_prototype.h"

namespace zs {

table_object::table_object(zs::engine* eng)
    : delegable_object(eng, object_type::k_table)
    , _map(nullptr) {}

table_object* table_object::create(zs::engine* eng) noexcept {

  table_object* tbl = (table_object*)eng->allocate(
      sizeof(table_object) + sizeof(map_type), (alloc_info_t)memory_tag::nt_table);
  zb_placement_new(tbl) table_object(eng);

  tbl->_map = (map_type*)(tbl->_data);
  zb_placement_new(tbl->_map)
      map_type((zs::unordered_map_allocator<object, object>(eng, zs::memory_tag::nt_table)));

  return tbl;
}

void table_object::destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept {
  table_object* tbl = (table_object*)obj;

  if (tbl->_map) {
    tbl->_map->~map_type();
    tbl->_map = nullptr;
  }

  zs_delete(eng, tbl);
}

object table_object::clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept {
  table_object* tbl = (table_object*)obj;

  table_object* out_tbl = table_object::create(eng);
  *out_tbl->_map = *tbl->_map;
  out_tbl->set_delegate(tbl->get_delegate(), tbl->get_delegate_flags());
  return object(out_tbl, false);
}

object table_object::create_object(zs::engine* eng) noexcept {
  return object(table_object::create(eng), false);
}

zs::error_result table_object::get(const object& key, object& dst) const noexcept {
  if (auto it = _map->find(key); it != _map->end()) {
    dst = it->second;
    return {};
  }

  dst = nullptr;
  return errc::not_found;
}

bool table_object::contains_all(const table_object& tbl) const noexcept {
  for (const auto& item : tbl) {
    if (!this->contains(item.first)) {
      return false;
    }
  }

  return true;
}

zs::error_result table_object::erase(const object& key) noexcept {
  _map->erase(key);
  return {};
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
