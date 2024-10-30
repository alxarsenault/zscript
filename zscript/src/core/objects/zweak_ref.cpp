#include <zscript/core/zcore.h>
namespace zs {

weak_ref_object* weak_ref_object::create(zs::engine* eng, const object_base& obj) {
  zbase_assert(obj.is_ref_counted(), "invalid object");
  return internal::zs_new<memory_tag::nt_weak_ptr, weak_ref_object>(eng, eng, obj);
}

weak_ref_object::weak_ref_object(zs::engine* eng, const object_base& obj) noexcept
    : reference_counted_object(eng)
    , _obj(obj) {}

weak_ref_object::~weak_ref_object() {
  if (_obj.is_ref_counted()) {
    _obj._ref_counted->_weak_ref_object = nullptr;
  }
}

object weak_ref_object::get_object() const noexcept { return object(_obj, true); }

} // namespace zs.
