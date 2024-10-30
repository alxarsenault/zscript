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
#include <zbase/utility/traits.h>
#include <zbase/strings/string_literal.h>
#include <zbase/utility/enum.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <source_location>
#include <filesystem>

ZBASE_BEGIN_NAMESPACE

#define ZBASE_VNAME(variable) #variable, variable

#define __ZB_PRINT_IMPL_LAST(name) ZBASE_VNAME(name)
#define __ZB_PRINT_IMPL(name) ZBASE_VNAME(name),
#define ZB_NPRINT(...) zb::print(ZBASE_FOR_EACH_WITH_LAST(__ZB_PRINT_IMPL, __ZB_PRINT_IMPL_LAST, __VA_ARGS__))
 
template <class _CharT = char, _CharT SpaceChar = ' '>
struct indent_t {

  inline constexpr indent_t(int indent, int spaces = 2) noexcept
      : _indent(indent)
      , _spaces(spaces) {}

  int _indent;
  int _spaces;

  inline indent_t& operator +=(int v) noexcept {
    _indent += v;
    return *this;
  }
  
  inline indent_t& operator -=(int v) noexcept {
    _indent -= v;
    return *this;
  }
  
  
  inline indent_t& operator++(int) noexcept {
    indent_t t = *this;
    ++_indent;
    return t;
  }

  inline indent_t& operator--(int) noexcept {
    indent_t t = *this;
    --_indent;
    return t;
  }

  inline indent_t operator++() noexcept {
    ++_indent;
    return *this;
  }

  inline indent_t operator--() noexcept {
     --_indent;
    return *this;
  }

  inline friend std::basic_ostream<_CharT>& operator<<(
      std::basic_ostream<_CharT>& stream, const indent_t& indent) {

    if (indent._indent) {
      auto flags = stream.flags();
      stream << std::left << std::setw(indent._indent * indent._spaces) << SpaceChar;
      stream.setf(flags);
    }

    return stream;
  }
};

template <class _CharT = char>
struct fill_t {

  inline constexpr fill_t(_CharT c, int count) noexcept
      : _char(c)
      , _count(count) {}

  _CharT _char;
  int _count;

  inline friend std::basic_ostream<_CharT>& operator<<(std::basic_ostream<_CharT>& stream, const fill_t& ft) {

    if (ft._count) {
      auto flags = stream.flags();
      stream << std::left << std::setfill(ft._char) << std::setw(ft._count) << ft._char;
      stream.setf(flags);
    }

    return stream;
  }
};

template <class _CharT = char>
struct sfill_t {

  inline constexpr sfill_t(std::string_view s, int count) noexcept
      : _s(s)
      , _count(count) {}

  std::string_view _s;
  int _count;

  inline sfill_t& operator++(int) noexcept {
    ++_count;
    return *this;
  }

  inline sfill_t& operator--(int) noexcept {
    --_count;
    return *this;
  }

  inline sfill_t operator++() noexcept {
    sfill_t t = *this;
    ++_count;
    return t;
  }

  inline sfill_t operator--() noexcept {
    sfill_t t = *this;
    --_count;
    return t;
  }

  inline friend std::basic_ostream<_CharT>& operator<<(
      std::basic_ostream<_CharT>& stream, const sfill_t& ft) {

    if (ft._count) {

      int count = ft._count;
      while (count--) {
        stream << ft._s;
      }
    }

    return stream;
  }
};
namespace detail {

template <__zb::string_literal Separator, class _CharT, class T>
inline std::basic_ostream<_CharT>& print_element(
    std::basic_ostream<_CharT>& stream, const T& t, bool is_last = false);

template <__zb::string_literal Separator, class _CharT, class... Args>
inline std::basic_ostream<_CharT>& print_elements(std::basic_ostream<_CharT>& stream, const Args&... args);

/// Default.
template <class T, class = void>
struct element_printer {
  template <__zb::string_literal Separator, class _CharT>
  static inline std::basic_ostream<_CharT>& print_element(std::basic_ostream<_CharT>& stream, const T& t) {
    stream << t;
    return stream;
  }
};

/// Container.
template <class T>
struct element_printer<T, std::enable_if_t<__zb::is_iterable_container_not_string_v<T> and !__zb::is_streamable<std::basic_ostream<char>, T>::value>> {

  template <__zb::string_literal Separator, class _CharT>
  static inline std::basic_ostream<_CharT>& print_element(std::basic_ostream<_CharT>& stream, const T& t) {

    const size_t sz = t.size();

    if (!sz) {
      stream << "{}";
      return stream;
    }

    stream.put('{');
    stream.put(' ');

    auto it = t.begin();

    for (; it != t.end(); ++it) {
      if (auto dt = it; ++dt == t.end()) {
        break;
      }

      __zb::detail::print_element<", ">(stream, *it);
    }

    // Print last element.
    __zb::detail::print_element<" }">(stream, *it);
    return stream;
  }
};

/// uint8_t.
template <class T>
struct element_printer<T, std::enable_if_t<std::is_same_v<T, uint8_t>>> {

  template <__zb::string_literal Separator, class _CharT>
  static inline std::basic_ostream<_CharT>& print_element(std::basic_ostream<_CharT>& stream, const T& t) {
    stream << (int)t;
    return stream;
  }
};

/// Pair.
template <class T>
struct element_printer<T, std::enable_if_t<__zb::is_pair_v<T>>> {

  template <__zb::string_literal Separator, class _CharT>
  static inline std::basic_ostream<_CharT>& print_element(std::basic_ostream<_CharT>& stream, const T& t) {
    stream.put('{');
    stream.put(' ');
    __zb::detail::print_element<", ">(stream, t.first);
    __zb::detail::print_element<" }">(stream, t.second);
    return stream;
  }
};

/// Tuple.
template <class T>
struct element_printer<T, std::enable_if_t<__zb::is_tuple_v<T>>> {

  template <__zb::string_literal Separator, class _CharT>
  static inline std::basic_ostream<_CharT>& print_element(std::basic_ostream<_CharT>& stream, const T& t) {
    stream << "{ ";
    std::apply([&](const auto&... args) { __zb::detail::print_elements<", ">(stream, args...); }, t);
    stream << " }";
    return stream;
  }
};

/// Tuple.
template <class T>
struct element_printer<T, std::enable_if_t<std::is_enum_v<T>>> {

  template <__zb::string_literal Separator, class _CharT>
  static inline std::basic_ostream<_CharT>& print_element(std::basic_ostream<_CharT>& stream, const T& t) {

    if (std::string_view ename = __zb::enum_name(t); !ename.empty()) {
      stream << ename;
    }
    else {
      stream << (std::underlying_type_t<T>)t;
    }

    return stream;
  }
};

template <class T>
struct element_printer<T, std::enable_if_t<std::is_same_v<T, std::source_location>>> {

  template <__zb::string_literal Separator, class _CharT>
  static inline std::basic_ostream<_CharT>& print_element(std::basic_ostream<_CharT>& stream, const T& t) {
    stream << t.function_name() << " " << t.line() << ":" << t.column() << " "
           << std::filesystem::path(t.file_name()).filename().c_str();
    return stream;
  }
};

template <__zb::string_literal Separator, class _CharT, class T>
inline std::basic_ostream<_CharT>& print_element(
    std::basic_ostream<_CharT>& stream, const T& t, bool is_last) {
  __zb::detail::element_printer<T>::template print_element<Separator>(stream, t);

  if constexpr (!Separator.empty()) {
    if (!is_last) {
      stream << Separator;
    }
  }

  return stream;
}

template <__zb::string_literal Separator, class _CharT, typename... Args, size_t... Is>
void print_elements_impl(
    std::basic_ostream<_CharT>& stream, std::index_sequence<Is...>, const Args&... args) {
  (__zb::detail::print_element<Separator>(stream, args, Is == sizeof...(Args) - 1), ...);
}

template <__zb::string_literal Separator, class _CharT, class... Args>
inline std::basic_ostream<_CharT>& print_elements(std::basic_ostream<_CharT>& stream, const Args&... args) {
  if constexpr (sizeof...(Args)) {
    __zb::detail::print_elements_impl<Separator>(stream, std::index_sequence_for<Args...>{}, args...);
  }

  return stream;
}

} // namespace detail.

template <__zb::string_literal Separator = "", class _CharT, class... Args>
inline std::basic_ostream<_CharT>& basic_stream_print(
    std::basic_ostream<_CharT>& stream, const Args&... args) {
  return __zb::detail::print_elements<Separator, _CharT>(stream, args...);
}

template <__zb::string_literal Separator = "", class... Args>
inline std::ostream& stream_print(std::ostream& stream, const Args&... args) {
  return __zb::detail::print_elements<Separator, char>(stream, args...);
}

template <__zb::string_literal Separator, class _CharT, class... Args>
inline std::basic_ostream<_CharT>& basic_print(std::basic_ostream<_CharT>& stream, const Args&... args) {
  __zb::detail::print_elements<Separator>(stream, args...);
  stream << std::endl;
  return stream;
}

template <__zb::string_literal Separator = " ", class... Args>
inline std::ostream& print(const Args&... args) {
  return __zb::basic_print<Separator>(std::cout, args...);
}

template <__zb::string_literal Separator = " ", class _CharT, class... Args>
inline std::basic_string<_CharT> basic_strprint(const Args&... args) {
  std::basic_ostringstream<_CharT> stream;
  __zb::detail::print_elements<Separator, _CharT>(stream, args...);
  return stream.str();
}

template <__zb::string_literal Separator = " ", class... Args>
inline std::string strprint(const Args&... args) {
  return basic_strprint<Separator, char>(args...);
}

namespace detail {
//template <class T, __zb::string_literal Quote = "\"">
//struct quoted_t {
//
//  inline constexpr quoted_t(const T& t) noexcept
//      : _value(t) {}
//
//  const T& _value;
//
//  template <class _CharT>
//  inline friend std::basic_ostream<_CharT>& operator<<(
//      std::basic_ostream<_CharT>& stream, const quoted_t& v) {
//
//    stream << Quote;
//    __zb::detail::print_element<"">(stream, v._value, false) << Quote;
//
//    return stream;
//  }
//};
template <class T, __zb::string_literal LQuote = "\"", __zb::string_literal RQuote = "\"">
struct  quoted_t {

  inline constexpr  quoted_t(const T& t) noexcept
      : _value(t) {}

  const T& _value;

  template <class _CharT>
  inline friend std::basic_ostream<_CharT>& operator<<(
      std::basic_ostream<_CharT>& stream, const  quoted_t& v) {

    stream << LQuote;
    __zb::detail::print_element<"">(stream, v._value, false) << RQuote;

    return stream;
  }
};
} // namespace detail.

template <__zb::string_literal LQuote = "\"", __zb::string_literal RQuote = "\"", class T>
inline detail::quoted_t<T, LQuote, RQuote>  quoted(const T& t) {
  return detail::quoted_t<T, LQuote, RQuote>(t);
}
//template <__zb::string_literal Quote = "\"", class T>
//inline detail::quoted_t<T, Quote> quoted(const T& t) {
//  return detail::quoted_t<T, Quote>(t);
//}
ZBASE_END_NAMESPACE
