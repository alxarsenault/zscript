#include "utility/zparameter_stream.h"
#include <zscript/std/zmutable_string.h>

namespace zs {

zs::error_result table_parameter::parse(parameter_stream& s, bool output_error, zs::table_object*& value) {
  if (!s->is_table()) {
    s.set_opt_error(output_error, "Invalid table type.");
    return zs::errc::not_a_table;
  }

  value = &s->as_table();
  ++s;
  return {};
}

zs::error_result table_parameter::parse(
    parameter_stream& s, bool output_error, const zs::table_object*& value) {
  if (!s->is_table()) {
    s.set_opt_error(output_error, "Invalid table type.");
    return zs::errc::not_a_table;
  }

  value = &s->as_table();
  ++s;
  return {};
}

zs::error_result array_parameter::parse(parameter_stream& s, bool output_error, zs::array_object*& value) {
  if (!s->is_array()) {
    s.set_opt_error(output_error, "Invalid array type.");
    return zs::errc::not_an_array;
  }

  value = &s->as_array();
  ++s;
  return {};
}

zs::error_result string_parameter::parse(parameter_stream& s, bool output_error, std::string_view& value) {
  if (zs::mutable_string::is_mutable_string(*s)) {
    value = s->as_udata().data_ref<mutable_string>();
    ++s;
    return {};
  }

  if (!s->is_string()) {
    s.set_opt_error(output_error, "Invalid string type.");
    return zs::errc::not_a_string;
  }

  value = s->get_string_unchecked();
  ++s;
  return {};
}

zs::error_result string_parameter::parse(parameter_stream& s, bool output_error, const char*& value) {
  if (zs::mutable_string::is_mutable_string(*s)) {
    value = s->as_udata().data_ref<mutable_string>().c_str();
    ++s;
    return {};
  }

  if (!s->is_cstring()) {
    s.set_opt_error(output_error, "Invalid cstring type.");
    return zs::errc::not_a_cstring;
  }

  value = s->get_cstring_unchecked();
  ++s;
  return {};
}

zs::error_result number_parameter::parse(parameter_stream& s, bool output_error, float_t& value) {
  if (!s->is_number()) {
    s.set_opt_error(output_error, "Invalid number type.");
    return zs::errc::not_a_number;
  }

  value = s->convert_to_float_unchecked();
  ++s;
  return {};
}

zs::error_result number_parameter::parse(parameter_stream& s, bool output_error, int_t& value) {
  if (!s->is_number()) {
    s.set_opt_error(output_error, "Invalid integer type.");
    return zs::errc::not_a_number;
  }

  value = s->convert_to_integer_unchecked();
  ++s;
  return {};
}

zs::error_result float_parameter::parse(parameter_stream& s, bool output_error, float_t& value) {
  if (!s->is_float()) {
    s.set_opt_error(output_error, "Invalid float type.");
    return zs::errc::not_a_float;
  }

  value = s->_float;
  ++s;
  return {};
}

zs::error_result integer_parameter::parse(parameter_stream& s, bool output_error, int_t& value) {
  if (!s->is_integer()) {
    s.set_opt_error(output_error, "Invalid integer type.");
    return zs::errc::not_an_integer;
  }

  value = s->_int;
  ++s;
  return {};
}
} // namespace zs.
