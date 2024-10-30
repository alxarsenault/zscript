#include <zscript/core/zcore.h>

namespace zs {

mutable_string_object::mutable_string_object(zs::engine* eng) noexcept
    : reference_counted_object(eng)
    , string_type((zs::string_allocator(eng))) {}

mutable_string_object* mutable_string_object::create(zs::engine* eng, std::string_view s) {
  mutable_string_object* mstr = internal::zs_new<memory_tag::nt_string, mutable_string_object>(eng, eng);
  mstr->assign(s);
  return mstr;
}

mutable_string_object* mutable_string_object::create(zs::engine* eng, size_t n) {
  mutable_string_object* mstr = internal::zs_new<memory_tag::nt_string, mutable_string_object>(eng, eng);

  if (n) {
    mstr->resize(n);
  }

  return mstr;
}

mutable_string_object* mutable_string_object::clone() const noexcept {
  mutable_string_object* s = mutable_string_object::create(_engine, get_string());
  return s;
}

} // namespace zs.
