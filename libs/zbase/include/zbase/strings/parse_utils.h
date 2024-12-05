///
/// BSD 3-Clause License
///
/// Copyright (c) 2020, Alexandre Arsenault
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// * Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
/// * Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
/// * Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
/// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
/// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
/// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
/// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
/// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
/// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
/// POSSIBILITY OF SUCH DAMAGE.
///

#pragma once
#include <zbase/zbase.h>
#include <zbase/sys/assert.h>
#include <zbase/memory/memory.h>
#include <string_view>

ZBASE_BEGIN_NAMESPACE

inline constexpr char distance_between_lower_and_upper_case() { return 'a' - 'A'; }

// inline constexpr bool is_char(char c) { return c >= 0 && c <= 127; }
inline constexpr bool is_char(char c) { return c >= 0; }

inline constexpr bool is_null(char c) { return c == 0; }

inline constexpr bool is_space(char c) { return c == ' '; }

inline constexpr bool is_tab(char c) { return c == '\t'; }

inline constexpr bool is_space_or_tab(char c) { return is_space(c) || is_tab(c); }

inline constexpr bool is_digit(char c) { return c >= '0' && c <= '9'; }

inline constexpr bool is_letter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }

inline constexpr bool is_lower_case_letter(char c) { return c >= 'a' && c <= 'z'; }

inline constexpr bool is_upper_case_letter(char c) { return c >= 'A' && c <= 'Z'; }

inline constexpr bool is_alphanumeric(char c) { return is_digit(c) || is_letter(c); }

inline constexpr bool is_letter_or_underscore(char c) { return is_letter(c) || c == '_'; }

inline constexpr bool is_alphanumeric_or_underscore(char c) { return is_alphanumeric(c) || c == '_'; }

inline constexpr bool is_alphanumeric_or_minus_or_underscore(char c) {
  return is_alphanumeric(c) || c == '_' || c == '-';
}

inline constexpr bool is_operator(char c) noexcept { return c == '<' || c == '>' || c == '='; }

inline constexpr bool is_dot(char c) noexcept { return c == '.'; }

inline constexpr bool is_logical_or(char c) noexcept { return c == '|'; }

inline constexpr bool is_hyphen(char c) noexcept { return c == '-'; }

inline constexpr bool is_hex(char c) {
  return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

inline constexpr bool is_control(char c) { return (c >= 0 && c <= 31) || c == 127; }

inline constexpr bool is_end_of_line(char c) { return c == '\n' || c == '\r'; }

inline constexpr bool is_sign(char c) noexcept { return ('+' == c) || ('-' == c); }

constexpr inline unsigned char hex_to_char(char c) {
  if (is_digit(c)) {
    return c - '0';
  }

  if (c >= 'a' && c <= 'f') {
    return 10 + c - 'a';
  }

  if (c >= 'A' && c <= 'F') {
    return 10 + c - 'A';
  }
  return 0;
}

inline constexpr bool is_special(char c) {
  // ! " # $ % & ' ( ) * + - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~
  return (c >= 32 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) || (c >= 123 && c <= 126);
}

inline constexpr unsigned int to_digit(char c) { return c - '0'; }

inline constexpr char to_upper_case(char c) {
  return is_lower_case_letter(c) ? (c - distance_between_lower_and_upper_case()) : c;
}

inline constexpr char to_lower_case(char c) {
  return is_upper_case_letter(c) ? (c + distance_between_lower_and_upper_case()) : c;
}

inline constexpr char to_lower(char c) noexcept { return to_lower_case(c); }

inline constexpr char to_upper(char c) noexcept { return to_upper_case(c); }

inline std::string_view to_string_view_n(const char* s, size_t max_len) {
  return std::string_view(s, __zb::strnlen(s, max_len));
}

inline constexpr bool is_upper_case(std::string_view s) {
  for (size_t i = 0; i < s.size(); i++) {
    if (__zb::is_letter(s[i]) && !__zb::is_upper_case_letter(s[i])) {
      return false;
    }
  }

  return true;
}

inline constexpr bool is_lower_case(std::string_view s) {
  for (size_t i = 0; i < s.size(); i++) {
    if (__zb::is_letter(s[i]) && !__zb::is_lower_case_letter(s[i])) {
      return false;
    }
  }

  return true;
}

inline constexpr bool is_alphanumeric(std::string_view s) {
  for (size_t i = 0; i < s.size(); i++) {
    if (!__zb::is_alphanumeric(s[i])) {
      return false;
    }
  }

  return true;
}

inline constexpr bool is_alphanumeric_with_spaces(std::string_view s) {
  for (size_t i = 0; i < s.size(); i++) {
    if (!(__zb::is_alphanumeric(s[i]) || __zb::is_space(s[i]))) {
      return false;
    }
  }

  return true;
}

inline constexpr bool is_alphanumeric_or_underscore(std::string_view s) {
  for (size_t i = 0; i < s.size(); i++) {
    if (!__zb::is_alphanumeric_or_underscore(s[i])) {
      return false;
    }
  }

  return true;
}

inline constexpr bool has_leading_spaces(std::string_view s) {
  return s.empty() ? false : __zb::is_space(s[0]);
}

inline constexpr bool has_trailing_spaces(std::string_view s) {
  return s.empty() ? false : __zb::is_space(s.back());
}

inline constexpr bool has_end_of_line(std::string_view s) {
  for (size_t i = 0; i < s.size(); i++) {
    if (__zb::is_end_of_line(s[i])) {
      return true;
    }
  }

  return false;
}

inline constexpr std::string_view strip_leading_spaces(std::string_view s) {
  const size_t b = s.find_first_not_of(' ');
  if (b == std::string_view::npos) {
    return std::string_view();
  }

  return s.substr(b);
}

inline constexpr std::string_view strip_leading_tabs(std::string_view s) {
  const size_t b = s.find_first_not_of('\t');
  if (b == std::string_view::npos) {
    return std::string_view();
  }

  return s.substr(b);
}

inline constexpr std::string_view strip_leading_endlines(std::string_view s) {
  const size_t b = s.find_first_not_of('\n');
  if (b == std::string_view::npos) {
    return std::string_view();
  }

  return s.substr(b);
}

inline constexpr std::string_view strip_trailing_endlines(std::string_view s) {
  const size_t b = s.find_last_not_of('\n');
  if (b == std::string_view::npos) {
    return std::string_view();
  }

  return s.substr(0, b + 1);
}

inline constexpr std::string_view strip_leading_and_trailing_endlines(std::string_view s) {
  return strip_trailing_endlines(strip_leading_endlines(s));
}

inline constexpr std::string_view strip_leading_spaces_and_tabs(std::string_view s) {
  return strip_leading_tabs(strip_leading_spaces(s));
}

inline constexpr std::string_view strip_trailing_spaces(std::string_view s) {
  const size_t b = s.find_last_not_of(' ');
  if (b == std::string_view::npos) {
    return std::string_view();
  }

  return s.substr(0, b + 1);
}

inline constexpr std::string_view strip_leading_and_trailing_spaces(std::string_view s) {
  return strip_trailing_spaces(strip_leading_spaces(s));
}

inline constexpr std::string_view strip_all(std::string_view s) {
  s = strip_leading_and_trailing_spaces(strip_leading_and_trailing_endlines(s));
  return strip_leading_and_trailing_spaces(strip_leading_and_trailing_endlines(s));
}

inline constexpr std::string_view strip_quotes(std::string_view s) {
  while (s.starts_with('"')) {
    s = s.substr(1);
  }
  while (s.ends_with('"')) {
    s = s.substr(0, s.size() - 1);
  }

  return s;
}

ZB_CHECK inline constexpr uint8_t hex_to_byte(uint32_t ch) noexcept {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  }

  if (ch >= 'A' && ch <= 'F') {
    return (ch - 'A') + 10;
  }

  if (ch >= 'a' && ch <= 'f') {
    return (ch - 'a') + 10;
  }

  return 0;
}

ZB_CHECK inline constexpr uint8_t octal_to_byte(uint32_t ch) noexcept {
  if (ch >= '0' && ch <= '7')
    return ch - '0';
  return 0;
}

//
ZB_CHECK inline constexpr uint64_t hex_to_int(std::string_view hex) noexcept {
  const size_t sz = hex.size();

  uint64_t value = 0;
  for (size_t i = sz, j = 0; i > 0 && hex[i - 1] != 'x'; --i, j += 4) {
    value += hex_to_byte(hex[i - 1]) * (1 << (j));
  }

  return value;
}

ZB_CHECK inline constexpr uint64_t octal_to_int(std::string_view hex) noexcept {
  const size_t sz = hex.size();
  uint64_t value = 0;
  for (size_t i = sz, j = 0; i > 0; --i, j += 3) {
    value += octal_to_byte(hex[i - 1]) * (1 << (j));
  }

  return value;
}

ZB_CHECK inline constexpr uint64_t binary_to_int(std::string_view binary) noexcept {
  uint64_t value = 0;
  uint64_t index = 0;

  auto rend = binary.rend();
  for (auto it = binary.rbegin(); it != rend; ++it, ++index) {
    value |= (uint64_t(*it - '0') << index);
  }
  return value;
}

// A table of all two-digit numbers. This is used to speed up decimal digit
// generation by copying pairs of digits into the final output.
inline constexpr std::array<char, 200> k_digit_table
    = { '0', '0', '0', '1', '0', '2', '0', '3', '0', '4', '0', '5', '0', '6', '0', '7', '0', '8', '0', '9',
        '1', '0', '1', '1', '1', '2', '1', '3', '1', '4', '1', '5', '1', '6', '1', '7', '1', '8', '1', '9',
        '2', '0', '2', '1', '2', '2', '2', '3', '2', '4', '2', '5', '2', '6', '2', '7', '2', '8', '2', '9',
        '3', '0', '3', '1', '3', '2', '3', '3', '3', '4', '3', '5', '3', '6', '3', '7', '3', '8', '3', '9',
        '4', '0', '4', '1', '4', '2', '4', '3', '4', '4', '4', '5', '4', '6', '4', '7', '4', '8', '4', '9',
        '5', '0', '5', '1', '5', '2', '5', '3', '5', '4', '5', '5', '5', '6', '5', '7', '5', '8', '5', '9',
        '6', '0', '6', '1', '6', '2', '6', '3', '6', '4', '6', '5', '6', '6', '6', '7', '6', '8', '6', '9',
        '7', '0', '7', '1', '7', '2', '7', '3', '7', '4', '7', '5', '7', '6', '7', '7', '7', '8', '7', '9',
        '8', '0', '8', '1', '8', '2', '8', '3', '8', '4', '8', '5', '8', '6', '8', '7', '8', '8', '8', '9',
        '9', '0', '9', '1', '9', '2', '9', '3', '9', '4', '9', '5', '9', '6', '9', '7', '9', '8', '9', '9' };

inline constexpr std::array<uint8_t, 256> k_modulo_10_table = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, //
  0, 1, 2, 3, 4, 5 //
};

inline constexpr int u8_mod_10(uint8_t v) noexcept { return k_modulo_10_table[v]; }

inline constexpr std::array<uint64_t, 20> k_10_exp_table = {
  1, //
  10, //
  100, //
  1000, //
  10000, //
  100000, //
  1000000, //
  10000000, //
  100000000, //
  1000000000, //
  10000000000ull, //
  100000000000ull, //
  1000000000000ull, //
  10000000000000ull, //
  100000000000000ull, //
  1000000000000000ull, //
  10000000000000000ull, //
  100000000000000000ull, //
  1000000000000000000ull, //
  10000000000000000000ull //
};

/// Returns 10^v or pow(10, v).
/// v must be [0, 19].
inline constexpr uint64_t exp10(uint64_t v) noexcept {
  zbase_assert(v < 20, "out of range");
  return k_10_exp_table[v];
}
ZBASE_END_NAMESPACE
