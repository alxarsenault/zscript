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

#include <zscript/base/zbase.h>
#include <zscript/base/sys/assert.h>
#include <zscript/base/utility/traits.h>
#include <string_view>

ZBASE_BEGIN_NAMESPACE
class string_view : public std::string_view {
public:
  using base = std::string_view;
  using base::base;
  using base::traits_type;
  using base::value_type;
  using base::size_type;
  using base::difference_type;
  using base::reference;
  using base::const_reference;
  using base::pointer;
  using base::const_pointer;
  using base::iterator;
  using base::const_iterator;
  using base::reverse_iterator;
  using base::const_reverse_iterator;
  using base::npos;
  using base::data;
  using base::contains;

  template <class... Args>
  inline constexpr string_view(Args&&... args) noexcept
      : base(std::forward<Args>(args)...) {}

  inline constexpr explicit operator bool() const noexcept { return !empty(); }

  ZB_CHECK ZB_INLINE const_pointer data(size_type index) const noexcept { return data() + index; }

  //  ZB_CHECK ZB_INLINE const_reference operator()(difference_type n) const noexcept {
  //    const size_t sz = size();
  //    zbase_assert(sz, "call operator[] in an empty string_view");
  //    return operator[]((n + sz) % sz);
  //  }

  inline constexpr string_view substr(size_type pos = 0, size_type __n = npos) const noexcept {
    size_type p = __zb::minimum(pos, size());
    return zb::string_view(data() + p, __zb::minimum(__n, size() - p));
  }

  //  template <class... Cs>
  //    requires std::is_convertible_v<std::common_type_t<Cs...>, char> and (sizeof...(Cs) > 1)
  //  inline constexpr bool contains(Cs... cs) const noexcept {
  //    for (char c : *this) {
  //      if (__zb::is_one_of(c, cs...)) {
  //        return true;
  //      }
  //    }
  //    return false;
  //  }
  inline constexpr string_view& operator<<=(size_t r_offset) noexcept {
    *this = this->substr(0, r_offset);
    return *this;
  }

  inline constexpr string_view& operator>>=(size_t l_offset) noexcept {
    *this = this->substr(l_offset);
    return *this;
  }

  inline constexpr string_view operator<<(size_t r_offset) const noexcept {
    return this->substr(0, r_offset);
  }

  inline constexpr string_view operator>>(size_t l_offset) const noexcept { return this->substr(l_offset); }

  template <auto, class... Ts>
  auto ext(Ts...);

private:
  using base::substr;
};

ZBASE_END_NAMESPACE
