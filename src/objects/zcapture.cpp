#include <zscript.h>

namespace zs {

capture_object::capture_object(zs::engine* eng, object* ptr)
    : zs::reference_counted_object(eng, zs::object_type::k_capture)
    , _ptr(ptr) {}

capture_object* capture_object::create(zs::engine* eng, object* ptr) noexcept {
  capture_object* cap = internal::zs_new<memory_tag::nt_capture, capture_object>(eng, eng, ptr);

  return cap;
}

capture_object* capture_object::clone() noexcept {
  retain();
  return this;
}

} // namespace zs.
