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
#include <zscript/base/utility/enum.h>
#include <array>
#include <string_view>

ZBASE_BEGIN_NAMESPACE

///
template <class T, class _Enum>
struct enum_array : public std::array<T, __zb::enum_count<_Enum>()> {

  static_assert(__zb::is_sequential_enum_v<_Enum>, "_Enum is not a sequential enum");

  using enum_type = _Enum;
  using array_type = std::array<T, __zb::enum_count<_Enum>()>;
  using value_type = typename array_type::value_type;
  using reference = typename array_type::reference;
  using const_reference = typename array_type::const_reference;

  template <enum_type E>
  constexpr reference at() noexcept {
    return std::get<size_t(E)>(*this);
  }

  template <enum_type E>
  constexpr const_reference at() const noexcept {
    return std::get<size_t(E)>(*this);
  }

  using array_type::at;
  constexpr reference at(enum_type e) noexcept { return array_type::at(size_t(e)); }
  constexpr const_reference at(enum_type e) const noexcept { return array_type::at(size_t(e)); }

  using array_type::operator[];
  constexpr reference operator[](enum_type e) noexcept { return array_type::operator[](size_t(e)); }
  constexpr const_reference operator[](enum_type e) const noexcept {
    return array_type::operator[](size_t(e));
  }

  inline reference operator[](std::string_view name) {
    return const_cast<reference>(const_cast<const enum_array*>(this)->operator[](name));
  }

  inline const_reference operator[](std::string_view name) const {
    static constexpr auto names = __zb::enum_names<enum_type>();
    for (size_t i = 0; i < names.size(); i++) {
      if (names[i] == name) {
        return array_type::operator[](i);
      }
    }

    throw std::invalid_argument(std::string(name));
  }
};
ZBASE_END_NAMESPACE
