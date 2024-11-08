#include <zscript.h>

namespace zs {

closure_object::closure_object(zs::engine* eng, const zs::object& fpo)
    : reference_counted_object(eng, zs::object_type::k_closure)
    , _function(fpo)
    , _default_params(zs::allocator<object>(eng))
    , _capture_values(zs::allocator<object>(eng)) {}

closure_object* closure_object::create(zs::engine* eng, const zs::object& fpo, const zs::object& root) {
  closure_object* cobj = internal::zs_new<closure_object>(eng, eng, fpo);
  cobj->_root = root;

  return cobj;
}

zs::function_prototype_object* closure_object::get_function_prototype() const noexcept {
  return _function._fproto;
}

zs::function_prototype_object& closure_object::get_proto() const noexcept { return _function.as_proto(); }
} // namespace zs.
