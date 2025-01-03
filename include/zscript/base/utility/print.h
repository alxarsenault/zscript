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
#include <zscript/base/utility/traits.h>
#include <zscript/base/strings/string_literal.h>
#include <zscript/base/utility/enum.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <zscript/base/sys/source_location.h>

#define ZBASE_VNAME(variable) #variable, variable

#define __ZB_PRINT_IMPL_LAST(name) ZBASE_VNAME(name)
#define __ZB_PRINT_IMPL(name) ZBASE_VNAME(name),
#define ZB_NPRINT(...) zb::print(ZBASE_FOR_EACH_WITH_LAST(__ZB_PRINT_IMPL, __ZB_PRINT_IMPL_LAST, __VA_ARGS__))

ZBASE_BEGIN_NAMESPACE

enum class print_options {
  none = 0,
  endl_each_vector_item = 1,
};

template <size_t N>
struct separator {
  ZB_INLINE_CXPR separator(const char (&s)[N], print_options opts = print_options::none) noexcept
      : str(s)
      , options(opts) {}

  template <class _CharT>
  inline friend std::basic_ostream<_CharT>& operator<<(
      std::basic_ostream<_CharT>& stream, const separator& sep) {

    if constexpr (N) {
      stream << sep.str.view();
    }

    return stream;
  }

  __zb::string_literal<N> str;
  print_options options;
};

template <size_t N>
separator(const char (&str)[N], print_options opts = print_options::none) -> separator<N>;

template <class _CharT = char, _CharT SpaceChar = ' '>
struct indent_t {

  template <class Indent>
    requires std::is_integral_v<Indent>
  inline constexpr indent_t(Indent indent, int spaces = 1) noexcept
      : _indent(zb::maximum(static_cast<int>(indent), 0))
      , _spaces(spaces) {}

  int _indent;
  int _spaces;

  inline indent_t& operator+=(int v) noexcept {
    _indent += v;
    return *this;
  }

  inline indent_t& operator-=(int v) noexcept {
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

    int count = ft._count;
    while (count--) {
      stream << ft._s;
    }

    return stream;
  }
};

template <class T, class _CharT = char>
struct padded_number_t {
  inline constexpr padded_number_t(const T& value, int count = 2, _CharT c = '0') noexcept
      : _value(value)
      , _count(count)
      , _char(c) {}

  padded_number_t(const padded_number_t&) = delete;
  padded_number_t(padded_number_t&&) = delete;

  padded_number_t& operator=(const padded_number_t&) = delete;
  padded_number_t& operator=(padded_number_t&&) = delete;

  const T& _value;
  int _count;
  _CharT _char;

  inline friend std::basic_ostream<_CharT>& operator<<(
      std::basic_ostream<_CharT>& stream, const padded_number_t& pnum) {
    auto flags = stream.flags();
    stream << std::right << std::setfill(pnum._char) << std::setw(pnum._count) << pnum._value;
    stream.setf(flags);
    return stream;
  }
};

template <class T, class _CharT = char>
struct left_aligned_t {
  inline constexpr left_aligned_t(const T& value, int count, _CharT c = ' ') noexcept
      : _value(value)
      , _count(count)
      , _char(c) {}

  left_aligned_t(const left_aligned_t&) = delete;
  left_aligned_t(left_aligned_t&&) = delete;

  left_aligned_t& operator=(const left_aligned_t&) = delete;
  left_aligned_t& operator=(left_aligned_t&&) = delete;

  const T& _value;
  int _count;
  _CharT _char;

  inline friend std::basic_ostream<_CharT>& operator<<(
      std::basic_ostream<_CharT>& stream, const left_aligned_t& l) {
    auto flags = stream.flags();
    stream << std::left << std::setfill(l._char) << std::setw(l._count) << l._value;
    stream.setf(flags);
    return stream;
  }
};

namespace detail {

template <class _CharT>
using bstream = std::basic_ostream<_CharT>;

template <__zb::separator Separator, bool IsLast, class _CharT, class T>
inline bstream<_CharT>& print_element(bstream<_CharT>& stream, const T& t);

//
template <__zb::print_options Options = __zb::print_options::none, class _CharT, class T>
inline bstream<_CharT>& print_element(bstream<_CharT>& stream, const T& t);

template <__zb::separator Separator, class _CharT, class... Args>
inline bstream<_CharT>& print_elements(bstream<_CharT>& stream, const Args&... args);

/// Default.
template <class T, class = void>
struct item_printer {
  template <__zb::print_options Options, class _CharT>
  static inline bstream<_CharT>& print_item(bstream<_CharT>& stream, const T& t) {
    if constexpr (__zb::is_streamable<std::basic_ostream<_CharT>, T>::value) {
      return stream << t;
    }
    else {
      return stream << "unknown " << typeid(T).name();
    }
  }
};

/// Container.
template <class T>
struct item_printer<T,
    std::enable_if_t<__zb::is_iterable_container_not_string_v<T>
        and !__zb::is_streamable<std::basic_ostream<char>, T>::value>> {

  template <__zb::print_options Options, class _CharT>
  static inline bstream<_CharT>& print_item(bstream<_CharT>& stream, const T& t) {

    if (t.empty()) {
      return stream << "{}";
    }

    if constexpr (Options == __zb::print_options::endl_each_vector_item) {
      auto it = t.begin();
      print_element(stream << "{\n  ", *it);
      while (++it != t.end()) {
        print_element<Options>(stream << ",\n  ", *it);
      }

      return stream << "\n}";
    }
    else {
      auto it = t.begin();
      print_element(stream << "[ ", *it);
      while (++it != t.end()) {
        print_element<Options>(stream << ", ", *it);
      }

      return stream << " ]";
    }
  }
};

/// uint8_t.
template <class T>
struct item_printer<T, std::enable_if_t<std::is_same_v<T, uint8_t>>> {
  template <__zb::print_options Options, class _CharT>
  static inline bstream<_CharT>& print_item(bstream<_CharT>& stream, const T& t) {
    return stream << (int)t;
  }
};

/// bool.
template <class T>
struct item_printer<T, std::enable_if_t<std::is_same_v<T, bool>>> {
  template <__zb::print_options Options, class _CharT>
  static inline bstream<_CharT>& print_item(bstream<_CharT>& stream, const T& t) {
    return stream << (t ? "true" : "false");
  }
};

/// Pair.
template <class T>
struct item_printer<T, std::enable_if_t<__zb::is_pair_v<T>>> {
  template <__zb::print_options Options, class _CharT>
  static inline bstream<_CharT>& print_item(bstream<_CharT>& stream, const T& t) {
    return print_elements<__zb::separator{ ", ", Options }>(stream << "{ ", t.first, t.second) << " }";
  }
};

/// Tuple.
template <class T>
struct item_printer<T, std::enable_if_t<__zb::is_tuple_v<T>>> {
  template <__zb::print_options Options, class _CharT>
  static inline bstream<_CharT>& print_item(bstream<_CharT>& stream, const T& t) {
    stream << "{ ";
    std::apply(
        [&](const auto&... args) { print_elements<__zb::separator{ ", ", Options }>(stream, args...); }, t);
    return stream << " }";
  }
};

/// Enum.
template <class T>
struct item_printer<T, std::enable_if_t<std::is_enum_v<T>>> {
  template <__zb::print_options Options, class _CharT>
  static inline bstream<_CharT>& print_item(bstream<_CharT>& stream, const T& t) {
    if constexpr (__zb::is_streamable<bstream<_CharT>, T>::value) {
      stream << t;
    }
    else {
      if (std::string_view ename = __zb::enum_name(t); !ename.empty()) {
        stream << ename;
      }
      else {
        stream << (std::underlying_type_t<T>)t;
      }
    }

    return stream;
  }
};

template <class T>
struct item_printer<T, std::enable_if_t<std::is_same_v<T, zb::source_location>>> {
  template <__zb::print_options Options, class _CharT>
  static inline bstream<_CharT>& print_item(bstream<_CharT>& stream, const T& t) {
    stream << t.function_name() << " " << t.line() << ":" << t.column() << " "
           << std::filesystem::path(t.file_name()).filename().c_str();
    return stream;
  }
};

template <__zb::print_options Options, class _CharT, class T>
inline bstream<_CharT>& print_element(bstream<_CharT>& stream, const T& t) {
  return item_printer<T>::template print_item<Options>(stream, t);
}

template <__zb::separator Separator, bool IsLast, class _CharT, class T>
inline bstream<_CharT>& print_element(bstream<_CharT>& stream, const T& t) {
  item_printer<T>::template print_item<Separator.options>(stream, t);
  if constexpr (!IsLast) {
    stream << Separator;
  }
  return stream;
}

template <__zb::separator Separator, class _CharT, typename... Args, size_t... Is>
void print_elements_impl(bstream<_CharT>& stream, std::index_sequence<Is...>, const Args&... args) {
  (print_element<Separator, Is == sizeof...(Args) - 1>(stream, args), ...);
}

template <__zb::separator Separator, class _CharT, class... Args>
inline bstream<_CharT>& print_elements(bstream<_CharT>& stream, const Args&... args) {
  if constexpr (sizeof...(Args)) {
    print_elements_impl<Separator>(stream, std::index_sequence_for<Args...>{}, args...);
  }

  return stream;
}
} // namespace detail.

template <__zb::separator Separator = "", __zb::separator Endl = "", class _CharT, class... Args>
inline std::basic_ostream<_CharT>& basic_stream_print(
    std::basic_ostream<_CharT>& stream, const Args&... args) {
  return __zb::detail::print_elements<Separator, _CharT>(stream, args...) << Endl;
}

template <__zb::separator Separator = "", __zb::separator Endl = "", class... Args>
inline std::ostream& stream_print(std::ostream& stream, const Args&... args) {
  return basic_stream_print<Separator, Endl>(stream, args...);
}

template <__zb::separator Separator = " ", __zb::separator Endl = "\n", class... Args>
inline std::ostream& print(const Args&... args) {
  return stream_print<Separator, Endl>(std::cout, args...);
}

template <__zb::separator Separator = " ", class _CharT, class... Args>
inline std::basic_string<_CharT> basic_strprint(const Args&... args) {
  std::basic_ostringstream<_CharT> stream;
  return ((std::basic_ostringstream<_CharT>&)basic_stream_print<Separator>(stream, args...)).str();
}

template <__zb::separator Separator = " ", class... Args>
inline std::string strprint(const Args&... args) {
  return basic_strprint<Separator, char>(args...);
}

namespace detail {
template <class T>
struct stringifier {
  const T& value;

  inline constexpr stringifier(const T& t) noexcept
      : value(t) {}

  stringifier(const stringifier&) = delete;
  stringifier(stringifier&&) = delete;

  stringifier& operator=(const stringifier&) = delete;
  stringifier& operator=(stringifier&&) = delete;

  template <class _CharT>
  inline friend std::basic_ostream<_CharT>& operator<<(
      std::basic_ostream<_CharT>& stream, const stringifier& item) {
    return __zb::basic_stream_print(stream, item.value);
  }
};
} // namespace detail.

template <class T>
inline auto stringifier(const T& t) {
  if constexpr (__zb::is_streamable<std::ostream, T>::value) {
    return t;
  }
  else {
    return detail::stringifier(t);
  }
}

namespace detail {
template <class T, __zb::string_literal Quote = "\"">
struct quoted_t {
  inline constexpr quoted_t(const T& t) noexcept
      : _value(t) {}

  quoted_t(const quoted_t&) = delete;
  quoted_t(quoted_t&&) = delete;

  quoted_t& operator=(const quoted_t&) = delete;
  quoted_t& operator=(quoted_t&&) = delete;

  const T& _value;

  template <class _CharT>
  inline friend std::basic_ostream<_CharT>& operator<<(
      std::basic_ostream<_CharT>& stream, const quoted_t& v) {
    return __zb::detail::print_element(stream << Quote, v._value) << Quote;
  }
};

template <class T, __zb::string_literal Left = "\"", __zb::string_literal Right = "\"">
struct double_quoted_t {
  inline constexpr double_quoted_t(const T& t) noexcept
      : _value(t) {}

  double_quoted_t(const double_quoted_t&) = delete;
  double_quoted_t(double_quoted_t&&) = delete;

  double_quoted_t& operator=(const double_quoted_t&) = delete;
  double_quoted_t& operator=(double_quoted_t&&) = delete;

  const T& _value;

  template <class _CharT>
  inline friend std::basic_ostream<_CharT>& operator<<(
      std::basic_ostream<_CharT>& stream, const double_quoted_t& v) {
    return __zb::detail::print_element(stream << Left, v._value) << Right;
  }
};
} // namespace detail.

template <__zb::string_literal Quote = "\"", class T>
inline detail::quoted_t<T, Quote> quoted(const T& t) {
  return detail::quoted_t<T, Quote>(t);
}

template <__zb::string_literal Left = "\"", __zb::string_literal Right = "\"", class T>
inline detail::double_quoted_t<T, Left, Right> double_quoted(const T& t) {
  return detail::double_quoted_t<T, Left, Right>(t);
}

template <class T>
inline detail::quoted_t<T, "\""> dquoted(const T& t) {
  return detail::quoted_t<T, "\"">(t);
}

template <class T>
inline detail::quoted_t<T, "'"> squoted(const T& t) {
  return detail::quoted_t<T, "'">(t);
}
ZBASE_END_NAMESPACE
