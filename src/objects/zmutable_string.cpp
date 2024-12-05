#include <zscript.h>

namespace zs {

mutable_string_object::mutable_string_object(zs::engine* eng) noexcept
    : delegate_object(eng, zs::object_type::k_mutable_string)
    , string_type((zs::string_allocator(eng))) {}

mutable_string_object::mutable_string_object(zs::engine* eng, zs::string&& s) noexcept
    : delegate_object(eng, zs::object_type::k_mutable_string)
    , string_type(std::move(s), (zs::string_allocator(eng))) {}

mutable_string_object* mutable_string_object::create(zs::engine* eng, std::string_view s) {
  mutable_string_object* mstr = internal::zs_new<memory_tag::nt_string, mutable_string_object>(eng, eng);
  mstr->assign(s);

  return mstr;
}

mutable_string_object* mutable_string_object::create(zs::engine* eng, zs::string&& s) {
  return internal::zs_new<memory_tag::nt_string, mutable_string_object>(eng, eng, std::move(s));
}

mutable_string_object* mutable_string_object::create(zs::engine* eng, size_t n) {
  mutable_string_object* mstr = internal::zs_new<memory_tag::nt_string, mutable_string_object>(eng, eng);

  if (n) {
    mstr->resize(n);
  }

  return mstr;
}

mutable_string_object* mutable_string_object::clone() const noexcept {
  return mutable_string_object::create(_engine, get_string());
}

} // namespace zs.
