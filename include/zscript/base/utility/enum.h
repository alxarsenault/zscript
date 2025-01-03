//
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
//

#pragma once

#include <zscript/base/zbase.h>
#include <zscript/base/detail/magic_enum/magic_enum.h>
#include <zscript/base/detail/magic_enum/magic_enum_switch.h>
#include <zscript/base/detail/magic_enum/magic_enum_utility.h>

ZBASE_BEGIN_NAMESPACE

//
// Enum.
//

using magic_enum::enum_cast;
using magic_enum::enum_contains;
using magic_enum::enum_count;
using magic_enum::enum_for_each;
using magic_enum::enum_name;
using magic_enum::enum_names;
using magic_enum::enum_switch;

///
template <class _Enum>
struct is_sequential_enum {
  using enum_type = _Enum;

  static constexpr bool value = []() {
    constexpr size_t count = __zb::enum_count<enum_type>();

    for (size_t i = 0; i < count; i++) {
      if (!__zb::enum_contains<enum_type>((int)i)) {
        return false;
      }
    }

    return true;
  }();
};

template <class _Enum>
inline constexpr bool is_sequential_enum_v = __zb::is_sequential_enum<_Enum>::value;

ZBASE_END_NAMESPACE
