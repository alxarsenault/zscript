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

#include <zscript.h>
#include <zbase/strings/utf8_span_stream.h>
#include "ztoken.h"

namespace zs {

enum class token_value_type {
  tv_none,
  tv_error,
  tv_token,

  // Numbers.
  tv_hex_number,
  tv_octal_number,
  tv_binary_number,
  tv_int_number,
  tv_float_number,

  // Strings.
  tv_string,
  tv_escaped_string,

  // Identifiers.
  tv_identifier
};

///
class token_stream {
public:
  using enum token_type;
  zb::utf8_span_stream _stream;

  uint64_t _last_token_line = 1;

  const char* _last_endl_ptr = nullptr;

  zs::line_info _current_line_info = { 0, 0 };
  zs::line_info _last_line_info = { 0, 0 };

  std::string_view _value;

  //  token_value_type _tok_vtype = tv_none;
  token_type _current_token = tok_none;
  token_type _last_token = tok_none;
};

class tokenizer : public engine_holder {
  ZS_CLASS_COMMON;

public:
  using enum token_type;
  using enum token_value_type;

  tokenizer(zs::engine* eng);
  tokenizer(zs::engine* eng, std::string_view code);
  ~tokenizer();

  void init(std::string_view code);

  zb::utf8_span_stream& stream() { return _stream; }

  ///
  ZS_CK_INLINE zs::line_info get_line_info() const noexcept { return _current_line_info; }

  ///
  ZS_CK_INLINE zs::line_info get_last_line_info() const noexcept { return _last_line_info; }

  zs::error_result parse_string(uint32_t end_char);
  zs::error_result parse_multi_line_string(uint32_t end_char);
  zs::error_result parse_single_line_comment();
  zs::error_result parse_multi_line_comment();
  zs::error_result parse_number();

  inline void next() {
    _last_line_info = get_line_info();
    _current_line_info.column++;
    _stream.incr();
  }

  inline void next(size_t n) {
    _last_line_info = get_line_info();
    _current_line_info.column += n;
    _stream.incr(n);
  }

  inline void new_line() {
    _last_line_info = get_line_info();
    _last_endl_ptr = _stream.ptr();
    _current_line_info.column = 1;
    _current_line_info.line++;
  }

  void skip_white_spaces();

private:
  zb::utf8_span_stream _stream;

  uint64_t _last_token_line = 1;

  const char* _last_endl_ptr = nullptr;

  zs::line_info _current_line_info = { 0, 0 };
  zs::line_info _last_line_info = { 0, 0 };

  std::string_view _value;

  token_value_type _tok_vtype = tv_none;
  token_type _current_token = tok_none;
  token_type _last_token = tok_none;

  inline void set_token(token_type t) { _last_token = std::exchange(_current_token, t); }
};
} // namespace zs.
