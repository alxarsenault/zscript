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

#if ZBASE_HAS_INCLUDE("source_location")

#include <source_location>
#define ZBASE_HAS_SOURCE_LOCATION 1

ZBASE_BEGIN_NAMESPACE
using source_location = std::source_location;
ZBASE_END_NAMESPACE

#else

#define ZBASE_HAS_SOURCE_LOCATION 0

ZBASE_BEGIN_NAMESPACE

class source_location {
public:
  // An explicit value should never be provided.
  static consteval source_location current(const char* file = __builtin_FILE(),
      const char* fct = __builtin_FUNCTION(), uint32_t l = __builtin_LINE()) noexcept {
    return { file, fct, l };
  }

  ZB_INLINE_CXPR source_location() noexcept = default;

  ZB_INLINE_CXPR source_location(const char* file, const char* fct, uint32_t l) noexcept
      : _filename(file)
      , _function_name(fct)
      , _line(l) {}

  ZB_CK_INLINE_CXPR uint32_t line() const noexcept { return _line; }
  ZB_CK_INLINE_CXPR uint32_t column() const noexcept { return 0; }

  ZB_CK_INLINE_CXPR const char* file_name() const noexcept { return _filename; }

  ZB_CK_INLINE_CXPR const char* function_name() const noexcept { return _function_name; }

private:
  const char* _filename;
  const char* _function_name;
  uint32_t _line;
};
ZBASE_END_NAMESPACE

#endif // ZBASE_HAS_INCLUDE.
