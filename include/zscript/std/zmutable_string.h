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

namespace zs {

/// Mutable string user data content.
struct mutable_string : zs::string {
  using stype = zs::string;
  using stype::stype;

  
  /// Creates an empty mutable string.
  static object create (zs::vm_ref vm) noexcept;

  /// Creates a mutable string of size 'n'.
  static  object create (zs::vm_ref vm, size_t n) noexcept;

  /// Creates a mutable string by moving the zs::string 's'.
  static object create (zs::vm_ref vm, zs::string&& s) noexcept;

  /// Creates a mutable string by moving the zs::mutable_string 's'.
  static   inline object create (zs::vm_ref vm, zs::mutable_string&& s) noexcept {
    return create (vm, (zs::string&&)std::move(s));
  }

  /// Creates a mutable string from the string_view 's'.
  static  object create (zs::vm_ref vm, std::string_view s) noexcept;

  /// Creates a mutable string from the string 's'.
  /// Same as creating a mutable string from a string_view.
  static inline object create (zs::vm_ref vm, const char* s) noexcept {
    return create (vm, std::string_view(s));
  }

  /// Creates a mutable string from the string 's'.
  /// Same as creating a mutable string from a string_view.
  static  inline object create (zs::vm_ref vm, const std::string& s) noexcept {
    return create (vm, std::string_view(s));
  }

  /// Creates a mutable string from the string 's'.
  /// Same as creating a mutable string from a string_view.
  static inline object create (zs::vm_ref vm, const zs::mutable_string& s) noexcept {
    return create (vm, std::string_view(s));
  }
  
  static mutable_string& as_mutable_string(const object& obj);
  
  /// Returns true if 'obj' is a mutable string.
  static bool is_mutable_string(const object_base& obj) noexcept;

  
  ZS_CK_INLINE int_t compare(std::string_view s) const noexcept { return ::strncmp(this->data(), s.data(), this->size()); }
  ZS_CK_INLINE const char* end_ptr() const noexcept { return this->data() + this->size(); }

  ZS_CK_INLINE bool is_ptr_in_range(const char* ptr) const noexcept {
    return ptr >= this->data() and ptr < this->data() + this->size();
  }
  
  object clone()const noexcept;
};


/// Creates a mutable string from a vm function call.
int_t vm_create_mutable_string(zs::vm_ref vm);
 
/// Mutable string parameter parser.
struct mutable_string_parameter {
  static zs::error_result parse(zs::parameter_stream& s, bool output_error, zs::mutable_string*& value);
  static zs::error_result parse(zs::parameter_stream& s, bool output_error, const zs::mutable_string*& value);
};

const zs::object& get_mutable_string_delegate(zs::engine* eng);

int_t mutable_string_meta_get_impl(zs::vm_ref vm) noexcept;
} // namespace zs.
