#include <zscript.h>
#include "objects/zfunction_prototype.h"
namespace zs {

table_object::table_object(zs::engine* eng)
    : delegate_object(eng, zs::object_type::k_table)
    , map_type((zs::unordered_map_allocator<object, object>(eng, zs::memory_tag::nt_table))) {

  //_delegate = eng->get_default_table_delegate();
}

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

zs::object table_object::get_opt(const object& key, const object& opt) const noexcept {
  auto it = map_type::find(key);

  if (it == map_type::end()) {
    return opt;
  }

  return it->second;
}

zs::error_result table_object::set(const object& key, const object& obj) noexcept {
  map_type::insert_or_assign(key, obj);
  return {};
}

zs::error_result table_object::set(const object& key, object&& obj) noexcept {
  map_type::insert_or_assign(key, std::move(obj));
  return {};
}

zs::error_result table_object::set(object&& key, const object& obj) noexcept {

  //  if(key == "__add" and obj.is_closure()) {
  //    if(obj.as_closure().get_proto()._parameter_names.size()==2) {
  //      obj.as_closure().get_proto()._parameter_names.push_back(zs::_ss("__delegate__"));
  //      obj.as_closure().get_proto()._default_params.push_back(0);
  //    }
  //  }
  map_type::insert_or_assign(std::move(key), obj);
  return {};
}

zs::error_result table_object::set(object&& key, object&& obj) noexcept {
  map_type::insert_or_assign(std::move(key), std::move(obj));
  return {};
}

table_object* table_object::clone() const noexcept {
  table_object* tbl = table_object::create(_engine);
  ((map_type&)*tbl) = *this;
  //  tbl->insert(this->begin(), this->end());
  return tbl;
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

      //      stream << zs::serializer(  it.first, idt);
      zs::serialize_to_json(eng, stream, it.first, idt);
    }

    stream << ": ";

    if (it.second.is_table() and this == it.second._table) {
      stream << "<RECURSION>";
    }
    else {

      //      stream << zs::serializer(  it.second, idt);
      zs::serialize_to_json(eng, stream, it.second, idt);
    }

    stream << ((++count == sz) ? "\n" : ",\n");
  }

  idt--;
  stream << zb::indent_t(idt, 4) << "}";

  return {};
}
} // namespace zs.
