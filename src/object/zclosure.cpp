#include <zscript/zscript.h>
#include "object/zfunction_prototype.h"

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
  return &function_prototype_object::as_proto(_function);
}

zs::function_prototype_object& closure_object::get_proto() const noexcept {
  return function_prototype_object::as_proto(_function);
}

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

bool closure_object::is_valid_parameters(
    zs::vm_ref vm, zb::span<const object> params, int_t& n_type_match) const noexcept {
  return get_proto().is_valid_parameters(vm, params, n_type_match);
}

object closure_object::clone() const noexcept {
  closure_object* cobj = closure_object::create(_engine, _function, _root);
  cobj->_base = _base;
  cobj->_module = _module;
  cobj->_this = _this;
  cobj->_default_params = _default_params;
  cobj->_captured_values = _captured_values;

  return object(cobj, false);
}

object closure_object::copy_with_binded_this(const zs::object& env) const noexcept {
  object cobj = clone();
  cobj.as_closure(). _this = env;
  return cobj;
}
} // namespace zs.
