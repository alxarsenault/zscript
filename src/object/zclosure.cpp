#include <zscript/zscript.h>
#include "object/zfunction_prototype.h"

namespace zs {

closure_object::closure_object(zs::engine* eng, const zs::object& fpo) noexcept
    : reference_counted_object(eng, object_type::k_closure)
    , _function(fpo)
    , _default_params(zs::allocator<object>(eng))
    , _captured_values(zs::allocator<object>(eng)) {}

closure_object::closure_object(zs::engine* eng, zs::object&& fpo) noexcept
    : reference_counted_object(eng, object_type::k_closure)
    , _function(std::move(fpo))
    , _default_params(zs::allocator<object>(eng))
    , _captured_values(zs::allocator<object>(eng)) {}

closure_object* closure_object::create(
    zs::engine* eng, const zs::object& fpo, const zs::object& root) noexcept {
  closure_object* cobj = internal::zs_new<closure_object>(eng, eng, fpo);
  cobj->_root = root;

  return cobj;
}

closure_object* closure_object::create(zs::engine* eng, zs::object&& fpo, const zs::object& root) noexcept {
  closure_object* cobj = internal::zs_new<closure_object>(eng, eng, std::move(fpo));
  cobj->_root = root;

  return cobj;
}

void closure_object::destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept {
  closure_object* cobj = (closure_object*)obj;
  zs_delete(eng, cobj);
}

object closure_object::clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept {
  closure_object* cobj = (closure_object*)obj;

  closure_object* out_cobj = closure_object::create(eng, cobj->_function, cobj->_root);
  out_cobj->_this = cobj->_this;
  out_cobj->_default_params = cobj->_default_params;
  out_cobj->_captured_values = cobj->_captured_values;

  return object(out_cobj, false);
}

zs::function_prototype_object* closure_object::get_function_prototype() const noexcept {
  return &function_prototype_object::as_proto(_function);
}

zs::function_prototype_object& closure_object::get_proto() const noexcept {
  return function_prototype_object::as_proto(_function);
}

function_parameter_interface closure_object::get_parameter_interface() const noexcept {

  return function_parameter_interface{ _default_params, &_this, get_parameters_count(),
    get_default_parameters_count(), has_variadic_parameters() };
}

int_t closure_object::get_parameters_count() const noexcept { return get_proto().get_parameters_count(); }

int_t closure_object::get_default_parameters_count() const noexcept {
  return get_proto().get_default_parameters_count();
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

bool closure_object::has_variadic_parameters() const noexcept {
  return get_proto().has_variadic_parameters();
}

object closure_object::copy_with_binded_this(const zs::object& env) const noexcept {
  object cobj = clone();
  cobj.as_closure()._this = env;
  return cobj;
}
} // namespace zs.
