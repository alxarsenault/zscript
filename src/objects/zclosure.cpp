#include <zscript.h>
#include "objects/zfunction_prototype.h"

namespace zs {

closure_object::closure_object(zs::engine* eng, const zs::object& fpo)
    : reference_counted_object(eng, zs::object_type::k_closure)
    , _function(fpo)
    , _default_params(zs::allocator<object>(eng))
    , _captured_values(zs::allocator<object>(eng)) {}

closure_object::closure_object(zs::engine* eng, zs::object&& fpo)
    : reference_counted_object(eng, zs::object_type::k_closure)
    , _function(std::move(fpo))
    , _default_params(zs::allocator<object>(eng))
    , _captured_values(zs::allocator<object>(eng)) {}

closure_object* closure_object::create(zs::engine* eng, const zs::object& fpo, const zs::object& root) {
  closure_object* cobj = internal::zs_new<closure_object>(eng, eng, fpo);
  cobj->_root = root;

  return cobj;
}

closure_object* closure_object::create(zs::engine* eng, zs::object&& fpo, const zs::object& root) {
  closure_object* cobj = internal::zs_new<closure_object>(eng, eng, std::move(fpo));
  cobj->_root = root;

  return cobj;
}

zs::function_prototype_object* closure_object::get_function_prototype() const noexcept {
  return _function._fproto;
}

zs::function_prototype_object& closure_object::get_proto() const noexcept { return _function.as_proto(); }

int_t closure_object::get_parameters_count() const noexcept { return get_proto().get_parameters_count(); }

int_t closure_object::get_default_parameters_count() const noexcept {
  return get_proto().get_parameters_count();
}

int_t closure_object::get_minimum_required_parameters_count() const noexcept {
  return get_proto().get_minimum_required_parameters_count();
}

bool closure_object::is_possible_parameter_count(size_t sz) const noexcept {
  return get_proto().is_possible_parameter_count(sz);
}
} // namespace zs.
