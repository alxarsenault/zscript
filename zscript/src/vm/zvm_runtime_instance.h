
namespace zs {

ZS_DECL_RT_ACTION(instance_get, cobjref_t obj, cobjref_t key, objref_t dest) {
  zbase_assert(obj->is_instance(), "should be an instance");

  zs::class_instance_object& inst = obj->as_instance();

  // Start by looking directly in the instance.
  if (!inst.get(key, dest)) {
    return {};
  }
  //
  object meta_get;
  //  zb::print("SALKSAJKSAL", inst.has_delegate());

  if (inst.has_delegate()) {
    object& delegate_obj = inst.get_delegate();

    if (zs::error_result err
        = runtime_action<runtime_code::delegate_get>(obj, CREF(delegate_obj), key, dest)) {
      return err;
    }

    //
    //      if (zs::error_result err = helper::delegate_get(vm, obj,
    //      delegate_obj, key, dest)) {
    //        return err;
    //      }

    // A value was returned. All good.
    if (!dest->is_none()) {
      return {};
    }
  }
  else if (const object& class_obj = inst.get_class(); class_obj.is_class()) {

    if (zs::error_result err = runtime_action<runtime_code::class_get>(obj, CREF(class_obj), key, dest)) {
      return err;
    }

    //
    //      if (zs::error_result err = helper::delegate_get(vm, obj,
    //      delegate_obj, key, dest)) {
    //        return err;
    //      }

    // A value was returned. All good.
    if (!dest->is_none()) {
      return {};
    }
  }

  return zs::error_code::inaccessible;
}
} // namespace zs.
