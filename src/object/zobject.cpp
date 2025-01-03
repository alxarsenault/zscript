#include <zscript/zscript.h>
#include "zvirtual_machine.h"

namespace zs {

// null = 0x1000000000000
// none= 0x2000000000000
// bool = 0x3000000000000
// int = 0x4000000000000
// floay = 0x5000000000000
// table = 0xf000000000000
// array = 0x10000000000000
// udata = 0x11000000000000
inline constexpr zs::object_base k_null_object = { ._lvalue = 0ULL, ._rvalue = 0x1000000000000ULL };

object::object(reference_counted_object* ref_obj, bool should_retain) noexcept {
  ::memset(this, 0, sizeof(object_base));
  _type = ref_obj->get_object_type();
  _ref_counted = ref_obj;

  if (should_retain) {
    _ref_counted->retain();
  }
}

object::object(zs::engine* eng, std::string_view v)
    : object() {

  if (v.size() <= constants::k_small_string_max_size) {
    _type = k_small_string;
    zb::memcpy(&_lvalue, v.data(), v.size());
  }
  else {
    _type = k_long_string;
    _lstring = string_object::create(eng, v);
  }
}

object& object::operator=(const object& obj) {
  if (&obj == this) {
    return *this;
  }

  if (is_ref_counted()) {
    _ref_counted->release();
  }

  base() = obj;

  if (is_ref_counted()) {
    _ref_counted->retain();
  }

  return *this;
}

object& object::operator=(object&& obj) {
  if (&obj == this) {
    return *this;
  }

  if (is_ref_counted()) {
    _ref_counted->release();
  }

  base() = std::exchange(obj.base(), k_null_object);
  return *this;
}

object object::create_concat_string(zs::engine* eng, std::string_view s1, std::string_view s2) {
  size_t sz = s1.size() + s2.size();

  if (sz <= constants::k_small_string_max_size) {
    object s;
    s._type = k_small_string;
    zb::memcpy((char*)&s._lvalue, s1.data(), s1.size());
    zb::memcpy(((char*)&s._lvalue) + s1.size(), s2.data(), s2.size());
    return s;
  }
  else {
    object s;
    s._type = k_long_string;
    string_object*& sobj = s._lstring;
    sobj = string_object::create(eng, sz);

    zb::memcpy((char*)sobj->data(), s1.data(), s1.size());
    zb::memcpy(((char*)sobj->data()) + s1.size(), s2.data(), s2.size());

    sobj->update_hash();
    return s;
  }
}

object object::create_array(zs::engine* eng, size_t sz) { return array_object::create_object(eng, sz); }

object object::create_struct(zs::engine* eng, size_t sz) { return struct_object::create_object(eng, sz); }

object object::create_table(zs::engine* eng) { return table_object::create_object(eng); }

object object::create_table_with_delegate(zs::engine* eng, const zs::object& delegate) {
  object obj = table_object::create_object(eng);
  if (auto err = obj._table->set_delegate(delegate)) {
    zs::print("ERROR", err);
  }
  return obj;
}

object object::create_table_with_delegate(
    zs::engine* eng, const zs::object& delegate, delegate_flags_t flags) {
  object obj = table_object::create_object(eng);
  if (auto err = obj._table->set_delegate(delegate, flags)) {
    zs::print("ERROR", err);
  }
  return obj;
}

object object::create_native_closure(zs::engine* eng, zs::function_t nc) {
  object obj;
  obj._type = object_type::k_native_closure;
  obj._native_closure = native_closure_object::create(eng, nc);
  return obj;
}

object object::create_native_closure(zs::engine* eng, zs::native_closure* nc) {
  object obj;
  obj._type = object_type::k_native_closure;
  obj._native_closure = native_closure_object::create(eng, nc);
  return obj;
}

object object::create_closure(zs::engine* eng, const object& fct_prototype, const object& root) {
  return object(zs::closure_object::create(eng, fct_prototype, root), false);
}

object object::create_closure(zs::engine* eng, object&& fct_prototype, const object& root) {
  return object(zs::closure_object::create(eng, std::move(fct_prototype), root), false);
}

object object::create_user_data(zs::engine* eng, size_t size) {
  object obj;
  obj._type = object_type::k_user_data;
  zs::user_data_object* uobj = zs::user_data_object::create(eng, size);
  obj._udata = uobj;
  return obj;
}

bool object::is_if_true() const noexcept {
  switch (_type) {

  case k_null:
    return false;
  case k_bool:
  case k_integer:
    return (bool)_int;
  case k_float:
    return (bool)_float;
  case k_small_string:
    return !get_small_string_unchecked().empty();
  case k_string_view:
    return !get_string_view_unchecked().empty();
  case k_long_string:
    return !get_long_string_unchecked().empty();

  case k_atom:
    return true;

  default:
    return true;
  }

  return true;
}

bool object::is_double_question_mark_true() const noexcept {
  switch (_type) {

  case k_null:
  case k_none:
    return false;
  case k_bool:
  case k_integer:
    return (bool)_int;
  case k_float:
    return (bool)_float;
  case k_small_string:
    return !get_small_string_unchecked().empty();
  case k_string_view:
    return !get_string_view_unchecked().empty();
  case k_long_string:
    return !get_long_string_unchecked().empty();
  case k_array:
    return !as_array().empty();

  case k_atom:
    return true;

  default:
    return true;
  }

  return true;
}

void object::reset() noexcept {
  if (is_ref_counted()) {
    _ref_counted->release();
  }

  base() = k_null_object;
}

object& object::with_delegate(object delegate) noexcept {
  if (ZBASE_UNLIKELY(!delegate.is_type(k_table, k_null))) {
    return *this;
  }

  if (ZBASE_UNLIKELY(!is_delegable())) {
    return *this;
  }

  as_delegable().set_delegate(delegate);
  return *this;
}

zs::error_result object::set_delegate(object delegate) noexcept {

  if (ZBASE_UNLIKELY(!is_delegable())) {
    return errc::inaccessible;
  }

  if (ZBASE_UNLIKELY(!delegate.is_type(k_null, k_none, k_table))) {
    return errc::invalid_delegate_type;
  }

  as_delegable().set_delegate(delegate);
  return {};
}

function_parameter_interface object::get_parameter_interface() const noexcept {
  if (is_closure()) {
    return as_closure().get_parameter_interface();
  }
  else if (is_native_closure()) {
    return as_native_closure().get_parameter_interface();
  }
  return {};
}

object object::get_weak_ref() const noexcept {
  if (is_ref_counted() and !is_weak_ref()) {
    return _ref_counted->get_weak_ref(*this);
  }

  return *this;
}

object object::get_weak_ref_value() const noexcept {
  zbase_assert(is_weak_ref());

  if (!is_weak_ref()) {
    return {};
  }

  return _weak_ref->get_object();
}

object object_base::get_exposed_type_name() const noexcept {
  return zs::_sv(zs::get_exposed_object_type_name(get_type()));
}

} // namespace zs.
