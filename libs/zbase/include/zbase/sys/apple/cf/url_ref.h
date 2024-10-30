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

#if __ZBASE_APPLE__
#include <zbase/sys/apple/cf/type.h>
#include <zbase/sys/apple/cf/pointer.h>
#include <zbase/sys/apple/cf/string_ref.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)

class url_ref : public cf::pointer<CFURLRef> {
public:
  using base = cf::pointer<CFURLRef>;
  using base::base;

  inline url_ref() noexcept = default;
  inline url_ref(const url_ref&) noexcept = default;
  inline url_ref(url_ref&&) noexcept = default;

  url_ref(const char* str, bool is_dir = false) noexcept;
  url_ref(std::string_view str, bool is_dir = false) noexcept;
  url_ref(const std::string& str, bool is_dir = false) noexcept;

  url_ref(const cf::string_ref& str_ref) noexcept;

  url_ref(const cf::string_ref& str_ref, const cf::url_ref& base) noexcept;

  url_ref(const char* str, bool is_dir, const cf::url_ref& base) noexcept;

  inline ~url_ref() noexcept = default;

  inline url_ref& operator=(const url_ref&) noexcept = default;
  inline url_ref& operator=(url_ref&&) noexcept = default;

  std::string to_string() const;

  cf::string_ref to_string_ref() const;

  inline operator std::string() const { return this->to_string(); }

  url_ref get_base() const;

  bool has_directory() const noexcept;
};

ZBASE_END_SUB_NAMESPACE(apple, cf)

#endif // __ZBASE_APPLE__
