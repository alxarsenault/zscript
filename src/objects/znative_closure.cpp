#include <zscript.h>

namespace zs {

native_closure_object* native_closure_object::create(zs::engine* eng, zs::native_closure* closure) {
  native_closure_object* nc
      = internal::zs_new<memory_tag::nt_native_closure, native_closure_object>(eng, eng, closure);

  return nc;
}

native_closure_object* native_closure_object::create(zs::engine* eng, zs::function_t fct) {
  native_closure_object* nc
      = internal::zs_new<memory_tag::nt_native_closure, native_closure_object>(eng, eng, fct);

  return nc;
}

native_closure_object::native_closure_object(zs::engine* eng, zs::native_closure* closure) noexcept
    : reference_counted_object(eng, zs::object_type::k_native_closure)
    , _callback{ .closure = closure }
    , _type_check(zs::allocator<uint32_t>(eng))
    , _ctype(closure_type::obj) {

  //      _callback.closure = closure;
}

native_closure_object::native_closure_object(zs::engine* eng, zs::function_t fct) noexcept
    : reference_counted_object(eng, zs::object_type::k_native_closure)
    , _callback{ .fct = fct }
    , _type_check(zs::allocator<uint32_t>(eng))
    , _ctype(closure_type::fct) {}

native_closure_object::~native_closure_object() {

  if (_release_hook) {
    (*_release_hook)(_engine, _user_pointer);
  }

  if (_ctype == closure_type::obj) {
    _callback.closure->release();
  }
}

int_t native_closure_object::call(vm_ref vm) {
  if (_ctype == closure_type::fct) {
    return (*_callback.fct)(vm);
  }
  else {
    return _callback.closure->call(vm);
  }
}

native_closure_object* native_closure_object::clone() {

  if (_ctype == closure_type::obj) {
    native_closure_object* nc
        = native_closure_object::create(reference_counted_object::_engine, _callback.closure);
    nc->_callback.closure->retain();
    return nc;
  }

  native_closure_object* nc = native_closure_object::create(reference_counted_object::_engine, _callback.fct);
  return nc;
}

zs::error_result native_closure_object::parse_type_check(int_t n_param_check, std::string_view typemask) {
  //    zs::small_vector<uint32_t, 8> _type_check;
  //    int_t _n_param_check = 0;
  _type_check.clear();
  //    _type_check.resize(n_param_check, 0);

  const size_t sz = typemask.size();

  //    size_t index = 0;
  bool is_or = false;

  for (size_t i = 0; i < sz; i++) {
    const char c = typemask[i];

    switch (c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      break;

    case '|':
      if (is_or) {
        // TODO: ERROR;
        //            zs::throw_compiler_exception("invalid typecheck");
        return zs::error_code::invalid;
      }

      is_or = true;
      break;

#define SINGLE_TYPE(letter, type)                              \
  case letter:                                                 \
    if (is_or) {                                               \
      _type_check.back() |= (uint32_t)object_type_mask::type;  \
      is_or = false;                                           \
    }                                                          \
    else {                                                     \
      _type_check.push_back((uint32_t)object_type_mask::type); \
    }                                                          \
    break

#define DOUBLE_TYPE(letter, type1, type2)                                                           \
  case letter:                                                                                      \
    if (is_or) {                                                                                    \
      _type_check.back() |= (uint32_t)object_type_mask::type1 | (uint32_t)object_type_mask::type2;  \
      is_or = false;                                                                                \
    }                                                                                               \
    else {                                                                                          \
      _type_check.push_back((uint32_t)object_type_mask::type1 | (uint32_t)object_type_mask::type2); \
    }                                                                                               \
    break

      SINGLE_TYPE('o', k_null);
      SINGLE_TYPE('i', k_integer);
      SINGLE_TYPE('f', k_float);
      SINGLE_TYPE('b', k_bool);
      SINGLE_TYPE('t', k_table);
      SINGLE_TYPE('a', k_array);
      SINGLE_TYPE('u', k_user_data);
      SINGLE_TYPE('p', k_raw_pointer);
      SINGLE_TYPE('x', k_instance);
      SINGLE_TYPE('y', k_class);
      DOUBLE_TYPE('n', k_float, k_integer);
      DOUBLE_TYPE('s', k_long_string, k_small_string);
      DOUBLE_TYPE('c', k_closure, k_native_closure);

    case '.':
      if (is_or) {
        _type_check.back() |= (uint32_t)std::numeric_limits<uint32_t>::max();
        is_or = false;
      }
      else {
        _type_check.push_back((uint32_t)std::numeric_limits<uint32_t>::max());
      }
      break;
    }
  }

  _n_param_check = n_param_check;
  return {};
}

} // namespace zs.
