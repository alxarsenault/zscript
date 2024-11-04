#include <zscript/zscript.h>

namespace zs {
string_object* string_object::create(zs::engine* eng, std::string_view s) {
  string_object* sobj
      = (string_object*)eng->allocate(sizeof(string_object) + s.size(), (alloc_info_t)memory_tag::nt_string);
  sobj = zb_placement_new(sobj) string_object(eng);
  sobj->_size = s.size();
  memcpy(sobj->_str, s.data(), s.size());
  *(sobj->_str + s.size()) = 0;

  sobj->update_hash();

  return sobj;
}

string_object* string_object::create(zs::engine* eng, size_t n) {
  string_object* sobj
      = (string_object*)eng->allocate(sizeof(string_object) + n, (alloc_info_t)memory_tag::nt_string);
  sobj = zb_placement_new(sobj) string_object(eng);
  sobj->_size = n;
  *(sobj->_str + n) = 0;

  return sobj;
}

object string_object::clone() const noexcept {
  string_object* s = string_object::create(reference_counted_object::_engine, get_string());
  return object(s, false);
}

void string_object::update_hash() const noexcept { _hash = object_table_hash()(get_string()); }
} // namespace zs.
