
namespace zs {

ZS_DECL_RT_ACTION(weak_set, objref_t obj, cobjref_t key, cobjref_t value) {
  ZS_ASSERT(obj->is_weak_ref());

  object real_obj = obj->get_weak_ref_value();

  return this->set(real_obj, key, value);
}
} // namespace zs.
