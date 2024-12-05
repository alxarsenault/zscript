#include <zscript.h>
#include <zbase/strings/charconv.h>

namespace zs {

zs::error_result object_base::get_bool(bool_t& res) const noexcept {
  if (_type == k_bool) {
    //      res = _bool._value;
    res = _bool;
    return {};
  }

  return zs::error_code::invalid_type;
}

zs::error_result object_base::get_integer(int_t& res) const noexcept {
  if (_type == k_integer) {
    res = _int;
    return {};
  }
  else if (_type == k_float) {
    res = (int_t)_float;
    return {};
  }

  return zs::error_code::invalid_type;
}

zs::error_result object_base::get_float(float_t& res) const noexcept {
  switch (_type) {
  case k_integer:
    res = (float_t)_int;
    return {};

  case k_float:
    res = _float;
    return {};

  default:
    return zs::error_code::invalid_type;
  }
}

zs::error_result object_base::convert_to_bool(bool_t& v) const noexcept {
  switch (_type) {
  case k_null:
    v = false;
    return {};

  case k_bool:
    v = _bool;
    return {};

  case k_integer:
    v = (bool_t)_int;
    return {};

  case k_float: {
    v = (bool_t)_float;
    return {};
  }

  default:
    return zs::error_code::conversion_error;
  }
}

bool_t object_base::convert_to_bool_unchecked() const noexcept {
  switch (_type) {
  case k_bool:
    return _bool;

  case k_integer:
    return (bool_t)_int;

  case k_float:
    return (bool_t)_float;

  default:
    return false;
  }
}

int_t object_base::convert_to_integer_unchecked() const noexcept {
  switch (_type) {
  case k_bool:
    return (int_t)_bool;

  case k_integer:
    return _int;

  case k_float:
    return (int_t)_float;

  default:
    return 0;
  }
}

float_t object_base::convert_to_float_unchecked() const noexcept {
  switch (_type) {
  case k_null:
    return (float_t)0;

  case k_bool:
    return (float_t)_bool;

  case k_integer:
    return (float_t)_int;

  case k_float:
    return _float;

  default:
    return (float_t)0;
  }
}

zs::error_result object_base::get_string(std::string_view& res) const noexcept {
  const object_type type = get_type();

  switch (type) {
  case object_type::k_long_string: {
    res = _lstring->get_string();
    return {};
  }

  case object_type::k_mutable_string: {
    res = _mstring->get_string();
    return {};
  }

  case object_type::k_small_string: {
    const char* s = (const char*)&_value;
    res = std::string_view(s);
    return {};
  }

  case object_type::k_string_view: {
    res = std::string_view(this->_sview, this->_sview_size);
    return {};
  }

  default:
    return zs::error_code::invalid_type;
  }

  return zs::error_code::invalid_type;
}

std::string_view object_base::get_long_string_unchecked() const noexcept { return _lstring->get_string(); }
std::string_view object_base::get_mutable_string_unchecked() const noexcept { return _mstring->get_string(); }

std::string_view object_base::get_string_view_unchecked() const noexcept {
  return std::string_view(this->_sview, this->_sview_size);
}

std::string_view object_base::get_string_unchecked() const noexcept {

  switch (_type) {
  case k_long_string: {
    return get_long_string_unchecked();
  }

  case object_type::k_small_string: {
    return get_small_string_unchecked();
  }

  case object_type::k_string_view: {
    return get_string_view_unchecked();
  }

  case object_type::k_mutable_string: {
    return get_mutable_string_unchecked();
  }

  default:
    zbase_error("not a string");
    return {};
  }
}

const char* object_base::get_cstring_unchecked() const noexcept {

  switch (_type) {
  case k_long_string:
    return get_long_string_unchecked().data();

  case object_type::k_small_string:
    return get_small_string_unchecked().data();

  case object_type::k_mutable_string:
    return get_mutable_string_unchecked().data();

  default:
    zbase_error("not a valid string");
    return nullptr;
  }
}
//
// zs::error_result object::convert_to_bool(bool_t& v) const noexcept {
//  switch (_type) {
//  case k_null:
//    v = false;
//    return {};
//
//  case k_bool:
//    v = _bool;
//    return {};
//
//  case k_integer:
//    v = (bool_t)_int;
//    return {};
//
//  case k_float: {
//    v = (bool_t)_float;
//    return {};
//  }
//
//  default:
//    return zs::error_code::conversion_error;
//  }
//}

zs::error_result object_base::convert_to_integer(int_t& v) const noexcept {
  switch (_type) {
  case k_null:
    v = 0;
    return {};

  case k_bool:
    v = _bool;
    return {};

  case k_integer:
    v = _int;
    return {};

  case k_float: {
    v = (int_t)_float;
    return {};
  }

  case k_small_string: {
    std::string_view s = get_string_unchecked<k_small_string>();
    if (zb::from_chars_result res = zb::from_chars(s.data(), s.data() + s.size(), v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  case k_long_string: {
    std::string_view s = get_string_unchecked<k_long_string>();
    if (zb::from_chars_result res = zb::from_chars(s.data(), s.data() + s.size(), v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  case k_mutable_string: {
    std::string_view s = get_string_unchecked<k_mutable_string>();
    if (zb::from_chars_result res = zb::from_chars(s.data(), s.data() + s.size(), v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  case k_string_view: {
    std::string_view s = get_string_unchecked<k_string_view>();
    if (zb::from_chars_result res = zb::from_chars(s.data(), s.data() + s.size(), v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  default:
    return zs::error_code::conversion_error;
  }
}

zs::error_result object_base::convert_to_float(float_t& v) const noexcept {
  switch (_type) {
  case k_null:
    v = 0.0;
    return {};

  case k_bool:
    v = _bool;
    return {};

  case k_integer:
    v = (float_t)_int;
    return {};

  case k_float: {
    v = _float;
    return {};
  }

  case k_small_string: {
    std::string_view s = get_string_unchecked<k_small_string>();
    if (zb::from_chars_result res = zb::from_chars(s.data(), s.data() + s.size(), v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  case k_long_string: {
    std::string_view s = get_string_unchecked<k_long_string>();
    if (zb::from_chars_result res = zb::from_chars(s.data(), s.data() + s.size(), v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  case k_mutable_string: {
    std::string_view s = get_string_unchecked<k_mutable_string>();
    if (zb::from_chars_result res = zb::from_chars(s.data(), s.data() + s.size(), v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  case k_string_view: {
    std::string_view s = get_string_unchecked<k_string_view>();
    if (zb::from_chars_result res = zb::from_chars(s.data(), s.data() + s.size(), v)) {
      return {};
    }

    return zs::error_code::conversion_error;
  }

  default:
    return zs::error_code::conversion_error;
  }
}
//
// int_t object::convert_to_integer_unchecked() const noexcept {
//  switch (_type) {
//  case k_bool:
//    return (int_t)_bool;
//
//  case k_integer:
//    return _int;
//
//  case k_float:
//    return (int_t)_float;
//
//  default:
//    return 0;
//  }
//}
//
// float_t object::convert_to_float_unchecked() const noexcept {
//  switch (_type) {
//  case k_null:
//    return (float_t)0;
//
//  case k_bool:
//    return (float_t)_bool;
//
//  case k_integer:
//    return (float_t)_int;
//
//  case k_float:
//    return _float;
//
//  default:
//    return (float_t)0;
//  }
//}

size_t object_base::get_ref_count() const noexcept {
  return is_ref_counted() ? _ref_counted->ref_count() : 0;
}

} // namespace zs.
