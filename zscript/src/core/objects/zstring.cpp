#include <zscript/core/zcore.h>

namespace zs {
string_object* string_object::create(zs::engine* eng, std::string_view s) {
  string_object* sobj
      = (string_object*)eng->allocate(sizeof(string_object) + s.size(), (alloc_info_t)memory_tag::nt_string);
  sobj = zb_placement_new(sobj) string_object(eng);
  sobj->_size = s.size();
  memcpy(sobj->_str, s.data(), s.size());
  *(sobj->_str + s.size()) = 0;

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

string_object* string_object::clone() const noexcept {
  string_object* s = string_object::create(reference_counted_object::_engine, get_string());
  return s;
}

} // namespace zs.
