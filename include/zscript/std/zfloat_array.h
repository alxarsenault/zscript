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

struct float_array : zs::vector<float> {
  using vtype = zs::vector<float>;
  using vtype::vtype;

  static float_array& as_float_array(const object& obj) noexcept;
};

object create_float_array(zs::vm_ref vm) noexcept;
object create_float_array_lib(zs::engine* eng) noexcept;

int_t vm_create_float_array(zs::vm_ref vm);

/// Returns true if 'obj' is a float array.
bool is_float_array(const object& obj) noexcept;

/// Float array parameter parser.
struct float_array_parameter {
  static zs::error_result parse(zs::parameter_stream& s, bool output_error, zs::float_array*& value);
};

} // namespace zs.
