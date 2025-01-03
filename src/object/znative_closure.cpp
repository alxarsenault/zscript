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
    : reference_counted_object(eng, object_type::k_native_closure)
    , _callback(cb)
    , _default_params(zs::allocator<zs::object>(eng))
    , _parameter_names(zs::allocator<zs::named_variable_type_info>(eng))
    , _restricted_types(zs::allocator<zs::object>(eng)) {}

void native_closure_object::destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept {
  native_closure_object* nc = (native_closure_object*)obj;

  if (nc->_release_hook) {
    (*nc->_release_hook)(eng, nc->_user_pointer);
  }

  if (nc->_callback.ctype == closure_type::obj) {
    nc->_callback.closure->release();
  }

  zs_delete(eng, nc);
}

object native_closure_object::clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept {
  native_closure_object* nc = (native_closure_object*)obj;

  if (nc->_callback.ctype == closure_type::obj) {
    native_closure_object* out_nc = native_closure_object::create(eng, nc->_callback.closure);
    out_nc->_callback.closure->retain();
    return object(out_nc, false);
  }

  native_closure_object* out_nc = native_closure_object::create(eng, nc->_callback.fct);
  return object(out_nc, false);
}

int_t native_closure_object::call(vm_ref vm) {
  if (_callback.ctype == closure_type::fct) {
    return (*_callback.fct)(vm);
  }
  else {
    return _callback.closure->call(vm);
  }
}

function_parameter_interface native_closure_object::get_parameter_interface() const noexcept {
  return function_parameter_interface{ _default_params, &_this, get_parameters_count(),
    get_default_parameters_count(), has_variadic_parameters() };
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
