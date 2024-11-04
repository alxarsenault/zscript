
// MIT License
//
// Copyright (c) 2024 Alexandre Arsenault
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <zscript/zscript.h>
#include "zvirtual_machine.h"

namespace zs {

struct parameter_stream {
  inline parameter_stream(vm_ref vm)
      : _vm(vm)
      , _it(vm->stack().stack_base_pointer())
      , _end(vm->stack().stack_end_pointer()) {}

  inline parameter_stream(vm_ref vm, bool skip_base)
      : _vm(vm)
      , _it(vm->stack().stack_base_pointer() + (int)skip_base)
      , _end(vm->stack().stack_end_pointer()) {}

  template <class ObjectParameterStreamer, class... Args>
  inline zs::error_result check(bool output_error, Args&... args) {

    if (!is_valid()) {
      if (output_error) {
        _vm.set_error("Invalid parameter count");
      }
      return zs::errc::invalid_parameter_count;
    }

    return ObjectParameterStreamer::parse(*this, output_error, args...);
  }

  template <class ObjectParameterStreamer, class... Args>
  inline zs::error_result require(Args&... args) {

    if (!is_valid()) {
      _vm.set_error("Invalid parameter count");
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

  inline bool is_user_data_with_uid(const object& uid) const noexcept {
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
  static zs::error_result parse(parameter_stream& s, bool output_error, zs::table_object*& value);
  static zs::error_result parse(parameter_stream& s, bool output_error, const zs::table_object*& value);
};

struct array_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, zs::array_object*& value);
};

struct number_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, float_t& value);
  static zs::error_result parse(parameter_stream& s, bool output_error, int_t& value);
};

struct float_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, float_t& value);
};

struct integer_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, int_t& value);
};

struct string_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, std::string_view& value);

  static zs::error_result parse(parameter_stream& s, bool output_error, const char*& value);
};

// struct error_code_parameter {
//   static zs::error_result parse(parameter_stream& s, bool output_error, zs::error_code& value) {
//     if (!s->is_error()) {
//       s.set_opt_error(output_error, "Invalid error type.");
//       return zs::errc::invalid_type;
//     }
//
//     value = s->_atom.error;
//     ++s;
//     return {};
//   }
// };

} // namespace zs.
