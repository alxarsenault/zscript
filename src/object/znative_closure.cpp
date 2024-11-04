#include <zscript/zscript.h>
#include "zvirtual_machine.h"

namespace zs {

native_closure_object* native_closure_object::create(zs::engine* eng, zs::native_closure* closure) {
  return zs_new<memory_tag::nt_native_closure, native_closure_object>(eng, eng, closure);
}

native_closure_object* native_closure_object::create(zs::engine* eng, zs::function_t fct) {
  return zs_new<memory_tag::nt_native_closure, native_closure_object>(eng, eng, fct);
}

native_closure_object::native_closure_object(zs::engine* eng, callback_type cb) noexcept
    : reference_counted_object(eng, zs::object_type::k_native_closure)
    , _restricted_types(zs::allocator<zs::object>(eng))
    , _default_params(zs::allocator<zs::object>(eng))
    , _parameter_names(zs::allocator<zs::named_variable_type_info>(eng))
    , _callback(cb) {}

native_closure_object::~native_closure_object() {

  if (_release_hook) {
    (*_release_hook)(_engine, _user_pointer);
  }

  if (_callback.ctype == closure_type::obj) {
    _callback.closure->release();
  }
}

int_t native_closure_object::call(vm_ref vm) {
  if (_callback.ctype == closure_type::fct) {
    return (*_callback.fct)(vm);
  }
  else {
    return _callback.closure->call(vm);
  }
}

object  native_closure_object::clone() const noexcept{

  if (_callback.ctype == closure_type::obj) {
    native_closure_object* nc
        = native_closure_object::create(reference_counted_object::_engine, _callback.closure);
    nc->_callback.closure->retain();
    return object(nc, false);
  }

  native_closure_object* nc = native_closure_object::create(reference_counted_object::_engine, _callback.fct);
  return object(nc, false);
}

bool native_closure_object::is_valid_parameters(
    zs::vm_ref vm, zb::span<const object> params, int_t& n_type_match) const noexcept {

  if (!has_parameter_info()) {
    return true;
  }

  const size_t n_params = params.size();

  if (!is_possible_parameter_count(n_params)) {
    return false;
  }

  n_type_match = 0;
  for (size_t k = 0; k < n_params; k++) {
    const zs::named_variable_type_info* vinfo = &_parameter_names[k];
    if (!vinfo or !vinfo->mask) {
      continue;
    }

    const object& param_obj = params[k];

    if (!param_obj.has_type_mask(vinfo->mask)) {
      return false;
    }

    if (!vinfo->custom_mask) {
      n_type_match++;
      continue;
    }

    zs::object typeobj;
    if (auto err = vm->type_of(param_obj, typeobj)) {
      return false;
    }

    int_t r_sz = _restricted_types.size();
    bool found = false;

    for (int i = 0; i < r_sz; i++) {
      if ((vinfo->custom_mask & (1 << i)) and typeobj == _restricted_types[i]) {
        found = true;
        break;
      }
    }

    if (!found) {
      return false;
    }

    n_type_match++;
  }

  return true;
}

} // namespace zs.
