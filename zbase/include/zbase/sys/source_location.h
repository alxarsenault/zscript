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

#include <zbase/zbase.h>

#if ZBASE_HAS_INCLUDE("source_location")
#include <source_location>
#define ZBASE_HAS_SOURCE_LOCATION 1
#define ZB_CURRENT_SOURCE_LOCATION() std::source_location::current()
#else
#define ZBASE_HAS_SOURCE_LOCATION 0
#define ZB_CURRENT_SOURCE_LOCATION() \
  zb::source_location { __FILE__, __FUNCTION__, __LINE__, 0 }
#endif // ZBASE_HAS_INCLUDE(source_location).

ZBASE_BEGIN_NAMESPACE

#if ZBASE_HAS_SOURCE_LOCATION
using source_location = std::source_location;
#else

struct source_location {
  std::string_view _filename;
  std::string_view _function_name;
  std::uint_least32_t _line;
  std::uint_least32_t _column;

  ZB_CK_INLINE std::string_view file_name() const noexcept { return _filename; }

  ZB_CK_INLINE std::string_view function_name() const noexcept { return _function_name; }

  ZB_CK_INLINE std::uint_least32_t line() const noexcept { return _line; }

  ZB_CK_INLINE std::uint_least32_t column() const noexcept { return _column; }
};

#endif // ZBASE_HAS_SOURCE_LOCATION.

ZBASE_END_NAMESPACE
