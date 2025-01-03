#include <zscript/zscript.h>

namespace zs {
static_assert(!std::is_polymorphic_v<weak_ref_object>, "weak_ref_object should not be polymorphic.");

weak_ref_object* weak_ref_object::create(zs::engine* eng, const object_base& obj) {
  zbase_assert(obj.is_ref_counted(), "invalid object");
  weak_ref_object* wobj = internal::zs_new<memory_tag::nt_weak_ptr, weak_ref_object>(eng, eng, obj);

  return wobj;
}

weak_ref_object::weak_ref_object(zs::engine* eng, const object_base& obj) noexcept
    : reference_counted_object(eng, object_type::k_weak_ref)
    , _obj(obj) {}

void weak_ref_object::destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept {
  weak_ref_object* wobj = (weak_ref_object*)obj;

  if (wobj->_obj.is_ref_counted()) {
    wobj->_obj._ref_counted->_weak_ref_object = nullptr;
  }

  zs_delete(eng, wobj);
}

object weak_ref_object::clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept {
  weak_ref_object* wobj = (weak_ref_object*)obj;
  if (wobj->_obj.is_ref_counted()) {
    return wobj->_obj.as_ref_counted().clone();
  }
  return object(wobj->_obj, true);
}

object weak_ref_object::get_object() const noexcept { return object(_obj, true); }

} // namespace zs.
