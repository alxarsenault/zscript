
// Copyright(c) 2024, Meta-Sonic.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.  See the file COPYING included with
// this distribution for more information.
//
// Alternatively, if you have a valid commercial licence for aulib obtained
// by agreement with the copyright holders, you may redistribute and/or modify
// it under the terms described in that licence.
//
// If you wish to distribute code using aulib under terms other than those of
// the GNU General Public License, you must obtain a valid commercial licence
// before doing so.

#pragma once

#include <zscript.h>
#include "zvirtual_machine.h"

namespace zs {

struct parameter_stream {
  inline parameter_stream(vm_ref vm)
      : _vm(vm)
      , _it(vm->stack().stack_base_pointer())
      , _end(vm->stack().stack_end_pointer()) {}

  template <class ObjectParameterStreamer, class... Args>
  inline zs::error_result require(Args&... args) {

    if (!is_valid()) {
      _vm->set_error("Invalid parameter count");
      return zs::errc::invalid_parameter_count;
    }

    return ObjectParameterStreamer::parse(*this, true, args...);
  }

  template <class ObjectParameterStreamer, class... Args>
  inline zs::error_result optional(Args&... args) {
    if (!is_valid()) {
      return zs::errc::invalid_parameter_count;
    }

    return ObjectParameterStreamer::parse(*this, false, args...);
  }

  inline const object& operator*() const noexcept { return *_it; }
  inline const object* operator->() const noexcept { return _it; }

  inline parameter_stream& operator++() noexcept {
    ++_it;
    return *this;
  }

  inline const object* operator++(int) noexcept {
    const object* it = _it;
    ++_it;
    return it;
  }

  inline bool is_valid() const noexcept { return _it < _end; }

  inline explicit operator bool() const noexcept { return _it < _end; }

  template <class... Args>
  inline void set_error(Args&&... args) {
    _vm->set_error(std::forward<Args>(args)...);
  }

  template <class... Args>
  inline void set_opt_error(bool output_error, Args&&... args) {
    if (output_error) {
      _vm->set_error(std::forward<Args>(args)...);
    }
  }

  inline bool is_user_data_with_uid(std::string_view uid) const noexcept {
    return _it->is_user_data() and _it->as_udata().get_uid() == uid;
  }

  inline bool is_struct_instance_with_name(std::string_view name) const noexcept {
    return _it->is_struct_instance() and _it->as_struct_instance().get_base().as_struct().get_name() == name;
  }

  inline bool is_array_with_size(int_t sz) const noexcept {
    return _it->is_array() and _it->as_array().size() == sz;
  }

  inline size_t size() const noexcept { return _end - _it; }

  inline vm_ref vm() const noexcept { return _vm; }

  vm_ref _vm;
  const object* _it;
  const object* _end;
};

struct table_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, zs::table_object*& value) {
    if (!s->is_table()) {
      s.set_opt_error(output_error, "Invalid table type.");
      return zs::errc::not_a_table;
    }

    value = &s->as_table();
    ++s;
    return {};
  }
};

struct array_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, zs::array_object*& value) {

    if (!s->is_array()) {
      s.set_opt_error(output_error, "Invalid array type.");
      return zs::errc::not_an_array;
    }

    value = &s->as_array();
    ++s;
    return {};
  }
};

struct number_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, float_t& value) {
    if (!s->is_number()) {
      s.set_opt_error(output_error, "Invalid number type.");
      return zs::errc::not_a_number;
    }

    value = s->convert_to_float_unchecked();
    ++s;
    return {};
  }

  static zs::error_result parse(parameter_stream& s, bool output_error, int_t& value) {
    if (!s->is_number()) {
      s.set_opt_error(output_error, "Invalid integer type.");
      return zs::errc::not_a_number;
    }

    value = s->convert_to_integer_unchecked();
    ++s;
    return {};
  }
};

struct float_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, float_t& value) {
    if (!s->is_float()) {
      s.set_opt_error(output_error, "Invalid float type.");
      return zs::errc::not_a_float;
    }

    value = s->_float;
    ++s;
    return {};
  }
};

struct integer_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, int_t& value) {

    if (!s->is_integer()) {
      s.set_opt_error(output_error, "Invalid integer type.");
      return zs::errc::not_an_integer;
    }

    value = s->_int;
    ++s;
    return {};
  }
};

struct string_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, std::string_view& value) {
    if (!s->is_string()) {
      s.set_opt_error(output_error, "Invalid string type.");
      return zs::errc::not_a_string;
    }

    value = s->get_string_unchecked();
    ++s;
    return {};
  }

  static zs::error_result parse(parameter_stream& s, bool output_error, zs::mutable_string_object*& value) {
    if (!s->is_mutable_string()) {
      s.set_opt_error(output_error, "Invalid mutable_string type.");
      return zs::errc::not_a_mutable_string;
    }

    value = &(s->as_mutable_string());
    ++s;
    return {};
  }
};

} // namespace zs.
