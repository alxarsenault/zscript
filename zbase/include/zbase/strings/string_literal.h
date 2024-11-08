// Copyright(c) 2024, Meta-Sonic.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.  See the file COPYING included with
// this distribution for more information.
//
// Alternatively, if you have a valid commercial licence for aulib obtained
// by agreement with the copyright holders, you may redistribute and/or modify
// it under the terms described in that licence.
//
// If you wish to distribute code using aulib under terms other than those of
// the GNU General Public License, you must obtain a valid commercial licence
// before doing so.

#pragma once

#include <zbase/zbase.h>
#include <zbase/utility/traits.h>
#include <ostream>
#include <string>
#include <string_view>

ZBASE_BEGIN_NAMESPACE

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
