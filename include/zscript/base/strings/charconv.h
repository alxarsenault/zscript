#pragma once

#include <zscript/base/zbase.h>
#include <zscript/base/detail/fast_float/float_common.h>
#include <zscript/base/detail/ryu/ryu.h>
#include <charconv>

ZBASE_BEGIN_SUB_NAMESPACE(fast_float)

template <class T>
using enable_if_supported_float_type = std::enable_if_t<is_supported_float_type<T>(), int>;

template <class T>
using enable_if_unsupported_float_type = std::enable_if_t<!is_supported_float_type<T>(), int>;

/// This function parses the character sequence [first,last) for a number. It
/// parses floating-point numbers expecting a locale-indepent format equivalent
/// to what is used by std::strtod in the default ("C") locale. The resulting
/// floating-point value is the closest floating-point values (using either float
/// or double), using the "round to even" convention for values that would
/// otherwise fall right in-between two values. That is, we provide exact parsing
/// according to the IEEE standard.
///
/// Given a successful parse, the pointer (`ptr`) in the returned value is set to
/// point right after the parsed number, and the `value` referenced is set to the
/// parsed value. In case of error, the returned `ec` contains a representative
/// error, otherwise the default (`std::errc()`) value is stored.
///
/// The implementation does not throw and does not allocate memory (e.g., with
/// `new` or `malloc`).
///
/// Like the C++17 standard, the `fast_float::from_chars` functions take an
/// optional last argument of the type `fast_float::chars_format`. It is a bitset
/// value: we check whether `fmt & fast_float::chars_format::fixed` and `fmt &
/// fast_float::chars_format::scientific` are set to determine whether we allow
/// the fixed point and scientific notation respectively. The default is
/// `fast_float::chars_format::general` which allows both `fixed` and
/// `scientific`.
template <class T, class _CharT = char, class = enable_if_supported_float_type<T>>
constexpr from_chars_result_t<_CharT> from_chars(
    const _CharT* first, const _CharT* last, T& value, chars_format fmt = chars_format::general) noexcept;

/// Like from_chars, but accepts an `options` argument to govern number parsing.
template <class T, class _CharT = char>
constexpr from_chars_result_t<_CharT> from_chars_advanced(
    const _CharT* first, const _CharT* last, T& value, parse_options_t<_CharT> options) noexcept;

/// from_chars for integer types.
template <class T, class _CharT = char, class = enable_if_unsupported_float_type<T>>
constexpr from_chars_result_t<_CharT> from_chars(
    const _CharT* first, const _CharT* last, T& value, int base = 10) noexcept;

ZBASE_END_SUB_NAMESPACE(fast_float)

#include <zscript/base/detail/fast_float/parse_number.h>

ZBASE_BEGIN_NAMESPACE

using from_chars_result = __zb::fast_float::from_chars_result_t<char>;
using parse_options = __zb::fast_float::parse_options_t<char>;
using chars_format = __zb::fast_float::chars_format;

/// This function parses the character sequence [first,last) for a number. It
/// parses floating-point numbers expecting a locale-indepent format equivalent
/// to what is used by std::strtod in the default ("C") locale. The resulting
/// floating-point value is the closest floating-point values (using either float
/// or double), using the "round to even" convention for values that would
/// otherwise fall right in-between two values. That is, we provide exact parsing
/// according to the IEEE standard.
///
/// Given a successful parse, the pointer (`ptr`) in the returned value is set to
/// point right after the parsed number, and the `value` referenced is set to the
/// parsed value. In case of error, the returned `ec` contains a representative
/// error, otherwise the default (`zb::errc()`) value is stored.
///
/// The implementation does not throw and does not allocate memory (e.g., with
/// `new` or `malloc`).
///
/// Like the C++17 standard, the `zb::from_chars` functions take an
/// optional last argument of the type `zb::chars_format`. It is a bitset
/// value: we check whether `fmt & zb::chars_format::fixed` and `fmt &
/// fast_float::chars_format::scientific` are set to determine whether we allow
/// the fixed point and scientific notation respectively. The default is
/// `fast_float::chars_format::general` which allows both `fixed` and
/// `scientific`.
template <class T, class = __zb::fast_float::enable_if_supported_float_type<T>>
constexpr __zb::from_chars_result from_chars(const char* first, const char* last, T& value,
    __zb::chars_format fmt = __zb::chars_format::general) noexcept {
  return __zb::fast_float::from_chars(first, last, value, fmt);
}

/// Like from_chars, but accepts an `options` argument to govern number parsing.
template <class T>
constexpr __zb::from_chars_result from_chars_advanced(
    const char* first, const char* last, T& value, __zb::parse_options options) noexcept {
  return __zb::fast_float::from_chars_advanced(first, value, options);
}

/// from_chars for integer types.
template <class T, class = __zb::fast_float::enable_if_unsupported_float_type<T>>
constexpr __zb::from_chars_result from_chars(
    const char* first, const char* last, T& value, int base = 10) noexcept {
  return __zb::fast_float::from_chars(first, last, value, base);
}

template <class T, class = __zb::fast_float::enable_if_supported_float_type<T>>
constexpr __zb::from_chars_result from_chars(
    std::string_view s, T& value, __zb::chars_format fmt = __zb::chars_format::general) noexcept {
  return __zb::fast_float::from_chars(s.data(), s.data() + s.size(), value, fmt);
}

/// Like from_chars, but accepts an `options` argument to govern number parsing.
template <class T>
constexpr __zb::from_chars_result from_chars_advanced(
    std::string_view s, T& value, __zb::parse_options options) noexcept {
  return __zb::fast_float::from_chars_advanced(s.data(), value, options);
}

/// from_chars for integer types.
template <class T, class = __zb::fast_float::enable_if_unsupported_float_type<T>>
constexpr __zb::from_chars_result from_chars(std::string_view s, T& value, int base = 10) noexcept {
  return __zb::fast_float::from_chars(s.data(), s.data() + s.size(), value, base);
}

template <bool Fixed = true, class T>
inline __zb::optional_result<size_t> to_chars(
    char* buffer, size_t sz, T v, uint32_t precision = 2, bool add_endl = true) {
  if constexpr (std::is_same_v<T, float>) {

    if constexpr (Fixed) {
      double dv = v;
      int res = zb::ryu::d2fixed_buffered_n(dv, precision, buffer, sz);

      if (add_endl) {
        buffer[res] = 0;
      }
      return res == 0 ? __zb::optional_result<size_t>{ __zb::errc::invalid, 0 }
                      : __zb::optional_result<size_t>{ (size_t)res };
    }
    else {

      int res = zb::ryu::f2s_buffered_n(v, buffer, sz);

      if (add_endl) {
        buffer[res] = 0;
      }
      return res == 0 ? __zb::optional_result<size_t>{ __zb::errc::invalid, 0 }
                      : __zb::optional_result<size_t>{ (size_t)res };
    }
  }
  else if constexpr (std::is_same_v<T, double>) {
    int res = zb::ryu::d2fixed_buffered_n(v, precision, buffer, sz);

    if (add_endl) {
      buffer[res] = 0;
    }
    return res == 0 ? __zb::optional_result<size_t>{ __zb::errc::invalid, 0 }
                    : __zb::optional_result<size_t>{ (size_t)res };
  }
  else {
    auto res = std::to_chars(buffer, buffer + sz, v);

    if (add_endl) {
      res.ptr[0] = 0;
    }
    return std::make_error_code(res.ec) ? __zb::optional_result<size_t>{ __zb::errc::invalid, 0 }
                                        : __zb::optional_result<size_t>{ res.ptr - buffer };
  }
}

template <bool Fixed = true, class T>
inline __zb::optional_result<size_t> to_chars(
    std::string& buffer, size_t sz, T v, uint32_t precision = 2, bool add_endl = true) {

  auto res = __zb::to_chars(buffer.data(), buffer.size(), v, precision, add_endl);

  if (res) {
    buffer.resize(res.value());
  }

  return res;
}
template <bool Fixed = true, class T>
inline __zb::optional_result<size_t> to_chars(
    std::string& buffer, T v, uint32_t precision = 2, bool add_endl = true) {

  if (buffer.empty()) {
    buffer.resize(22, 0);
  }

  auto res = __zb::to_chars(buffer.data(), buffer.size(), v, precision, add_endl);

  if (res) {
    buffer.resize(res.value());
  }

  return res;
}
ZBASE_END_NAMESPACE
