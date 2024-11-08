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
#include <zbase/sys/error_code.h>
#include <zbase/strings/unicode.h>
#include <zbase/strings/parse_utils.h>
#include <zbase/utility/traits.h>
#include <span>

ZBASE_BEGIN_NAMESPACE

class utf8_span_stream {
public:
  using value_type = char;
  using span_type = std::span<const value_type>;
  using pointer = typename span_type::pointer;
  using const_pointer = typename span_type::const_pointer;
  using reference = typename span_type::reference;
  using const_reference = typename span_type::const_reference;
  using iterator = typename span_type::iterator;
  using difference_type = typename span_type::difference_type;

  enum class number_format {
    invalid,
    floating_point,
    scientific_float,
    integer,
    hex_integer,
    binary_integer,
    octal_integer
  };

  inline utf8_span_stream() noexcept = default;

  inline utf8_span_stream(span_type s) noexcept
      : _data(s)
      , _it(s.begin()) {}

  inline auto begin() noexcept { return _data.begin(); }
  inline auto begin() const noexcept { return _data.begin(); }

  inline auto end() noexcept { return _data.end(); }
  inline auto end() const noexcept { return _data.end(); }

  ZB_CHECK ZB_INLINE bool is_valid() const noexcept { return _it != _data.end(); }

  ZB_CHECK ZB_INLINE explicit operator bool() const noexcept { return is_valid(); }

  ZB_CHECK ZB_INLINE bool is_next_valid() const noexcept {
    return _it + __zb::unicode::sequence_length(static_cast<uint8_t>(*_it)) < _data.end();
  }

  ZB_CHECK ZB_INLINE bool is_end() const noexcept { return _it == _data.end(); }

  ZB_CHECK ZB_INLINE bool is_next_end() const noexcept {
    return _it + __zb::unicode::sequence_length(static_cast<uint8_t>(*_it)) >= _data.end();
  }

  inline utf8_span_stream& incr() noexcept {
    _it += __zb::unicode::sequence_length(static_cast<uint8_t>(*_it));
    return *this;
  }

  inline utf8_span_stream& incr(size_t n) noexcept {
    for (size_t i = 0; i < n; i++) {
      _it += __zb::unicode::sequence_length(static_cast<uint8_t>(*_it));
    }

    return *this;
  }

  ZB_CHECK ZB_INLINE uint32_t get_next() const noexcept {
    iterator it = _it + __zb::unicode::sequence_length(static_cast<uint8_t>(*_it));
    return __zb::unicode::next_u8_to_u32(it);
  }

  ZB_CHECK ZB_INLINE uint32_t safe_get_next() const noexcept {
    iterator it = _it + __zb::unicode::sequence_length(static_cast<uint8_t>(*_it));
    return it < _data.end() ? __zb::unicode::next_u8_to_u32(it) : 0;
  }

  inline uint32_t get_next(size_t n) const noexcept {
    iterator it = _it + __zb::unicode::sequence_length(static_cast<uint8_t>(*_it));

    while (n--) {
      it += __zb::unicode::sequence_length(static_cast<uint8_t>(*it));
    }

    return __zb::unicode::next_u8_to_u32(it);
  }

  inline utf8_span_stream& goto_next_line_end() noexcept {

    while ((_it + __zb::unicode::sequence_length(static_cast<uint8_t>(*_it))) < _data.end()) {
      if (__zb::unicode::next_u8_to_u32(_it) == '\n') {
        return *this;
      }
    }

    return *this;
  }

  inline utf8_span_stream& operator++() noexcept {
    incr();
    return *this;
  }

  inline utf8_span_stream operator++(int) noexcept {
    utf8_span_stream s = *this;
    incr();
    return s;
  }

  ZB_CHECK ZB_INLINE uint32_t operator*() const noexcept {
    iterator it = _it;
    return __zb::unicode::next_u8_to_u32(it);
  }

  ZB_CHECK ZB_INLINE uint32_t safe_get() const noexcept {
    iterator it = _it;
    return it != _data.end() ? __zb::unicode::next_u8_to_u32(it) : 0;
  }

  inline uint32_t operator[](difference_type n) const noexcept {
    iterator it = _it;

    while (n--) {
      it += __zb::unicode::sequence_length(static_cast<uint8_t>(*it));
    }

    return __zb::unicode::next_u8_to_u32(it);
  }

  inline iterator get_iterator() noexcept { return _it; }

  inline pointer ptr() const noexcept { return &(*_it); }

  inline size_t rsize() const noexcept { return std::distance(_it, _data.end()); }

  inline __zb::error_result parse_number(
      std::string_view& output, number_format& nfmt, bool* is_neg = nullptr) noexcept {
    utf8_span_stream& s = *this;

    const char* beg = ptr();
    bool dot_found = false;
    bool e_found = false;
    bool post_e_sign_found = false;
    bool post_e_digit_found = false;
    //
    //  // TODO: Fix negative.
    bool is_negative = false;
    //
    // Check for hexadecimal number.
    if (*s == '0' and zb::is_one_of(get_next(), 'x', 'X')) {
      incr(2);
      while (s) {
        const uint32_t t = *s;

        if (zb::is_digit(t) or zb::is_one_of(t, 'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F')) {
          incr();
        }
        else {
          break;
        }
      }

      const size_t sz = std::distance(beg, ptr());
      std::string_view svalue(beg, sz);
      output = svalue;
      nfmt = number_format::hex_integer;

      if (is_neg) {
        *is_neg = is_negative;
      }
      return {};
    }

    // Check for binary number.
    if (*s == '0' and get_next() == 'b') {
      incr(2);
      while (s) {
        const uint32_t t = *s;

        if (zb::is_digit(t) or zb::is_one_of(t, '0', '1')) {
          incr();
        }
        else {
          break;
        }
      }

      const size_t sz = std::distance(beg, ptr());
      std::string_view svalue(beg, sz);

      output = svalue;
      nfmt = number_format::binary_integer;

      if (is_neg) {
        *is_neg = is_negative;
      }
      return {};
    }

    // Check for octal number.
    if (*s == '0' and get_next() == 'h') {
      incr(2);
      while (s) {
        const uint32_t t = *s;

        if (zb::is_digit(t) or zb::is_one_of(t, '0', '1', '2', '3', '4', '5', '6', '7')) {
          incr();
        }
        else {
          break;
        }
      }

      const size_t sz = std::distance(beg, s.ptr());
      std::string_view svalue(beg, sz);

      output = svalue;
      nfmt = number_format::octal_integer;

      if (is_neg) {
        *is_neg = is_negative;
      }
      return {};
    }

    while (s.is_valid()) {
      const uint32_t t = *s;

      if ('.' == t) {

        if (dot_found) {
          // Multiple dots.
          return __zb::error_code::invalid;
        }

        dot_found = true;
        incr();
        continue;
      }

      // TODO: Fix this.
      if ('-' == t) {
        incr();
        is_negative = true;
        continue;
      }

      // TODO: This never happens.
      else if ('+' == *s) {
        incr();
        is_negative = false;
        continue;
      }

      else if (zb::is_one_of(t, 'e', 'E')) {
        char c = s.get_next();

        if (s.is_next_end()) {
          //            l->set_token(tok_lex_error);
          return __zb::error_code::invalid;
        }
        else if (('+' != c) && ('-' != c) && !zb::is_digit(c)) {
          //            l->set_token(tok_lex_error);
          return __zb::error_code::invalid;
        }

        e_found = true;
        incr();
        continue;
      }

      //
      else if (e_found && zb::is_sign(*s) && !post_e_digit_found) {
        if (post_e_sign_found) {
          return __zb::error_code::invalid;
        }

        post_e_sign_found = true;
        incr();
        continue;
      }

      //
      else if (e_found && zb::is_digit(*s)) {
        post_e_digit_found = true;
        incr();
        continue;
      }

      //
      else if (('.' != t) && !__zb::is_digit(*s)) {
        break;
      }

      //
      else {
        incr();
      }
    }

    const size_t sz = std::distance(beg, s.ptr());
    std::string_view svalue(beg, sz);

    if (is_negative) {
      // TODO: Handle negative.
      svalue = svalue.substr(1);
    }

    if (dot_found || e_found) {

      output = svalue;

      nfmt = e_found ? number_format::scientific_float : number_format::floating_point;

      if (is_neg) {
        *is_neg = is_negative;
      }
      return {};
    }

    output = svalue;
    nfmt = number_format::integer;

    if (is_neg) {
      *is_neg = is_negative;
    }
    return {};
  }

  std::span<const value_type> _data;
  iterator _it = {};
};
ZBASE_END_NAMESPACE
