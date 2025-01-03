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

/// Stream user data content.
struct stream_object {
  using string_stream_type = std::basic_stringstream<char, std::char_traits<char>, zs::string_allocator>;

  static object create(zs::vm_ref vm, std::istream* istream, std::ostream* ostream) noexcept;
  static object create(zs::vm_ref vm) noexcept;

  static stream_object& as_stream(const object& obj);

  /// Returns true if 'obj' is a stream object.
  static bool is_stream(const object_base& obj) noexcept;

  stream_object(zs::engine* eng);

  ZS_CK_INLINE bool is_input_stream() const noexcept { return (bool)_istream; }

  ZS_CK_INLINE bool is_output_stream() const noexcept { return (bool)_ostream; }

  ZS_CK_INLINE bool is_inout_stream() const noexcept { return (bool)_ostream and (bool) _istream; }

  std::ostream* _ostream;
  std::istream* _istream;
  string_stream_type _sstream;
};

/// Creates a stream object from a vm function call.
int_t vm_create_stream(zs::vm_ref vm);

/// Stream object parameter parser.
struct stream_parameter {
  static zs::error_result parse(zs::parameter_stream& s, bool output_error, zs::stream_object*& value);
  static zs::error_result parse(zs::parameter_stream& s, bool output_error, const zs::stream_object*& value);
};

const zs::object& get_stream_delegate(zs::engine* eng);

zs::object create_io_lib(zs::vm_ref vm);
} // namespace zs.
