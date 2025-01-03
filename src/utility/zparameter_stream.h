
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
  ZS_INLINE parameter_stream(vm_ref vm) noexcept
      : _vm(vm)
      , _it(vm->stack().stack_base_pointer())
      , _end(vm->stack().stack_end_pointer()) {}

  ZS_INLINE parameter_stream(vm_ref vm, bool skip_base) noexcept
      : _vm(vm)
      , _it(vm->stack().stack_base_pointer() + (int)skip_base)
      , _end(vm->stack().stack_end_pointer()) {}

  template <class ObjectParameterStreamer, class... Args>
  ZS_CK_INLINE zs::error_result require(Args&... args) noexcept {
    if (!is_valid()) {
      _vm.set_error("Invalid parameter count");
      return errc::invalid_parameter_count;
    }

    return ObjectParameterStreamer::parse(*this, true, args...);
  }

  template <class ObjectParameterStreamer, class... Args>
  ZS_CK_INLINE zs::error_result require_if_valid(Args&... args) noexcept {
    if (!is_valid()) {
      return {};
    }

    return ObjectParameterStreamer::parse(*this, true, args...);
  }

  template <class ObjectParameterStreamer, class... Args>
  ZS_CK_INLINE zs::error_result optional(Args&... args) noexcept {
    if (!is_valid()) {
      return errc::invalid_parameter_count;
    }

    return ObjectParameterStreamer::parse(*this, false, args...);
  }

  template <class ObjectParameterStreamer, class... Args>
  ZS_CK_INLINE zs::error_result check(bool output_error, Args&... args) noexcept {
    if (!is_valid()) {
      set_opt_error(output_error, "Invalid parameter count");
      return errc::invalid_parameter_count;
    }

    return ObjectParameterStreamer::parse(*this, output_error, args...);
  }

  ZS_CK_INLINE const object& operator*() const noexcept {
    ZS_ASSERT(is_valid(), "Trying to get an invalid parameter.");
    return is_valid() ? *_it : object::invalid_object();
  }

  ZS_INLINE const object* operator->() const noexcept { return _it; }

  ZS_INLINE parameter_stream& operator++() noexcept {
    ++_it;
    return *this;
  }

  ZS_INLINE const object* operator++(int) noexcept {
    const object* it = _it;
    ++_it;
    return it;
  }

  ZS_CK_INLINE bool is_valid() const noexcept { return _it < _end; }

  ZS_CK_INLINE explicit operator bool() const noexcept { return _it < _end; }

  ZS_CK_INLINE size_t size() const noexcept { return _end - _it; }

  ZS_CK_INLINE bool has_type(object_type t) const noexcept {
    for (const object* it = _it; it < _end; ++it) {
      if (it->is_type(t)) {
        return true;
      }
    }

    return false;
  }

  ZS_CK_INLINE bool is_user_data_with_uid(std::string_view uid) const noexcept {
    return _it->is_user_data() and _it->as_udata().get_uid() == uid;
  }

  ZS_CK_INLINE bool is_user_data_with_uid(const object& uid) const noexcept {
    return _it->is_user_data() and _it->as_udata().get_uid() == uid;
  }

  ZS_CK_INLINE bool is_struct_instance_with_name(std::string_view name) const noexcept {
    return _it->is_struct_instance() and _it->as_struct_instance().get_base().as_struct().get_name() == name;
  }

  ZS_CK_INLINE bool is_array_with_size(int_t sz) const noexcept {
    return _it->is_array() and _it->as_array().size() == sz;
  }

  template <class... Args>
  inline void set_error(Args&&... args) noexcept {
    _vm.set_error(std::forward<Args>(args)...);
  }

  template <class... Args>
  inline void set_opt_error(bool output_error, Args&&... args) noexcept {
    if (output_error) {
      _vm.set_error(std::forward<Args>(args)...);
    }
  }

  ZS_CK_INLINE vm_ref vm() const noexcept { return _vm; }

private:
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

struct function_parameter {
  static zs::error_result parse(parameter_stream& s, bool output_error, object& value);

  static zs::error_result parse(parameter_stream& s, bool output_error, zs::closure_object*& value);
  static zs::error_result parse(parameter_stream& s, bool output_error, const zs::closure_object*& value);

  static zs::error_result parse(parameter_stream& s, bool output_error, zs::native_closure_object*& value);
  static zs::error_result parse(
      parameter_stream& s, bool output_error, const zs::native_closure_object*& value);

  static zs::error_result parse(parameter_stream& s, bool output_error, zs::function_t& value);
};

} // namespace zs.
