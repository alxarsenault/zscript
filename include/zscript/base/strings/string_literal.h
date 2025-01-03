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
#include <zscript/base/utility/traits.h>
#include <ostream>
#include <string>
#include <string_view>

ZBASE_BEGIN_NAMESPACE

#define ZB_STRING_LITERAL_FROM_STRING_VIEW(sv) zb::string_literal<sv.size()>(sv)

/// string_literal.
template <size_t N>
class string_literal {
public:
  using value_type = char;
  using size_type = size_t;
  using string_type = std::string;
  using view_type = std::string_view;

  constexpr string_literal(const char (&s)[N]) noexcept {
    for (size_type i = 0; i < N; i++) {
      value[i] = s[i];
    }
  }

  constexpr string_literal(std::string_view s) noexcept {
    for (size_type i = 0; i < N; i++) {
      value[i] = s[i];
    }
  }

  ZB_CK_INLINE_CXPR static size_type size() noexcept { return N; }
  ZB_CK_INLINE_CXPR static bool empty() noexcept { return N == 0; }

  ZB_CK_INLINE_CXPR const char* c_str() const noexcept { return &value[0]; }

  ZB_CK_INLINE_CXPR view_type view() const noexcept { return view_type(&value[0], N - 1); }

  ZB_CK_INLINE string_type str() const noexcept { return string_type(&value[0], N - 1); }

  ZB_CK_INLINE_CXPR int compare(view_type sv) const noexcept { return view().compare(sv); }

  value_type value[N];

private:
  template <size_t Start, size_t Count, size_t... Indices>
  ZB_INLINE_CXPR string_literal<Count + 1> substr_impl(std::index_sequence<Indices...>) const noexcept {
    return string_literal<Count + 1>({ value[Start + Indices]..., 0 });
  }

public:
  template <size_t Start, size_t Count>
  ZB_INLINE_CXPR string_literal<Count + 1> substr() const noexcept {
    return substr_impl<Start, Count>(std::make_index_sequence<Count>());
  }
};

template <size_t N>
string_literal(const char (&str)[N]) -> string_literal<N>;

namespace detail {
template <string_literal A, string_literal B, size_t... AIndices, size_t... BIndices>
inline constexpr auto concat_impl(
    std::index_sequence<AIndices...>, std::index_sequence<BIndices...>) noexcept {
  return string_literal({ A.value[AIndices]..., B.value[BIndices]... });
}

template <string_literal Separator, string_literal A, string_literal B, size_t... SIndices,
    size_t... AIndices, size_t... BIndices>
inline constexpr auto join_impl(std::index_sequence<SIndices...>, std::index_sequence<AIndices...>,
    std::index_sequence<BIndices...>) noexcept {
  return string_literal({ A.value[AIndices]..., Separator.value[SIndices]..., B.value[BIndices]... });
}
} // namespace detail.

template <string_literal A, string_literal B, string_literal... Cs>
inline constexpr auto string_literal_concat() noexcept {

  constexpr auto ab = detail::concat_impl<A, B>(
      std::make_index_sequence<A.size() - 1>(), std::make_index_sequence<B.size()>());
  if constexpr (sizeof...(Cs) == 0) {
    return ab;
  }
  else {
    return string_literal_concat<ab, Cs...>();
  }
}

template <string_literal Separator, string_literal A, string_literal B, string_literal... Cs>
inline constexpr auto string_literal_join() noexcept {

  constexpr auto ab = detail::join_impl<Separator, A, B>(std::make_index_sequence<Separator.size() - 1>(),
      std::make_index_sequence<A.size() - 1>(), std::make_index_sequence<B.size()>());
  if constexpr (sizeof...(Cs) == 0) {
    return ab;
  }
  else {
    return string_literal_join<Separator, ab, Cs...>();
  }
}

template <size_t N1, size_t N2>
inline constexpr bool operator==(
    const __zb::string_literal<N1>& lhs, const __zb::string_literal<N2> rhs) noexcept {
  return lhs.view() == rhs.view();
}

template <size_t N>
inline constexpr bool operator==(const __zb::string_literal<N>& lhs, std::string_view rhs) noexcept {
  return lhs.view() == rhs;
}

template <size_t N>
inline constexpr bool operator==(std::string_view lhs, const __zb::string_literal<N>& rhs) noexcept {
  return lhs == rhs.view();
}

template <size_t N1, size_t N2>
inline constexpr bool operator!=(
    const __zb::string_literal<N1>& lhs, const __zb::string_literal<N2> rhs) noexcept {
  return lhs.view() != rhs.view();
}

template <size_t N>
inline constexpr bool operator!=(const __zb::string_literal<N>& lhs, std::string_view rhs) noexcept {
  return lhs.view() != rhs;
}

template <size_t N>
inline constexpr bool operator!=(std::string_view lhs, const __zb::string_literal<N>& rhs) noexcept {
  return lhs != rhs.view();
}

template <size_t N>
inline std::ostream& operator<<(std::ostream& stream, const __zb::string_literal<N>& s) {
  stream << s.view();
  return stream;
}

ZBASE_END_NAMESPACE
