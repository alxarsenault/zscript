
namespace zs {

ZS_DECL_RT_ACTION(weak_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  using enum runtime_code;

  ZS_ASSERT(obj->is_weak_ref());

  object real_obj = obj->get_weak_ref_value();

  return this->get(real_obj, key, dest);
}

ZS_DECL_RT_ACTION(weak_set, objref_t obj, cobjref_t key, cobjref_t value) {
  ZS_ASSERT(obj->is_weak_ref());

  object real_obj = obj->get_weak_ref_value();

  return this->set(real_obj, key, value);
}
} // namespace zs.
