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

#if __ZBASE_APPLE__

#include <zscript/base/sys/apple/cf/type.h>
#include <zscript/base/sys/apple/cf/pointer.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)

class string_ref : public cf::pointer<CFStringRef> {
public:
  using base = cf::pointer<CFStringRef>;
  using base::base;

  inline string_ref() noexcept = default;
  inline string_ref(const string_ref&) noexcept = default;
  inline string_ref(string_ref&&) noexcept = default;

  string_ref(const char* str) noexcept;
  string_ref(std::string_view str) noexcept;
  string_ref(const std::string& str) noexcept;

  inline ~string_ref() noexcept = default;

  inline string_ref& operator=(const string_ref&) noexcept = default;
  inline string_ref& operator=(string_ref&&) noexcept = default;

  size_t size() const noexcept;

  inline size_t length() const noexcept { return this->size(); }

  uint16_t get_char(size_t index) const noexcept;

  std::string to_string() const;

  inline operator std::string() const { return this->to_string(); }
};

ZBASE_END_SUB_NAMESPACE(apple, cf)
#endif // __ZBASE_APPLE__
