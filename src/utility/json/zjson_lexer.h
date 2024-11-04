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

#include <zscript/zscript.h>

#include "zjson_token.h"
#include <zbase/strings/utf8_span_stream.h>
#include <string>
#include <string_view>
#include <unordered_map>

namespace zs {
class json_parser;

class json_lexer : public engine_holder {
  ZS_CLASS_COMMON;
  friend class json_parser;

public:
  using enum json_token_type;

  json_lexer(zs::engine* eng);
  ~json_lexer();

  void init(std::string_view code);

  json_token_type lex();

  ZS_CK_INLINE zs::line_info get_line_info() const noexcept {
    return { (int_t)_current_line, (int_t)_current_column };
  }

  ZS_CK_INLINE zs::line_info get_last_line_info() const noexcept { return _last_line_info; }

  zs::object get_value() const;
  zs::object get_debug_value() const;

  inline int_t get_int_value() const noexcept { return _int_value; }
  inline float_t get_float_value() const noexcept { return _float_value; }

  inline std::string_view get_identifier_value() const noexcept { return _identifier; }
  inline std::string_view get_string_value() const noexcept { return _string; }
  inline const zs::string& get_escaped_string_value() const noexcept { return _escaped_string; }

  inline zs::object get_identifier() const noexcept {
    return _is_string_view_identifier ? zs::_sv(_identifier) : zs::object(_engine, _identifier);
  }

  inline json_token_type current_token() const noexcept { return _current_token; }
  inline json_token_type last_token() const noexcept { return _last_token; }

private:
  struct helper;

  zb::utf8_span_stream _stream;
  json_token_type _current_token = tok_none;
  json_token_type _last_token = tok_none;

  uint64_t _current_line = 1;
  uint64_t _current_column = 1;
  uint64_t _last_token_line = 1;

  zs::line_info _last_line_info = { 0, 0 };

  std::string_view _identifier;
  std::string_view _string;
  zs::string _escaped_string;
  int_t _int_value;
  float_t _float_value;
  bool _is_string_view_identifier = false;

  inline void set_token(json_token_type t) { _last_token = std::exchange(_current_token, t); }

  inline json_token_type set_token_and_next(json_token_type t) {
    set_token(t);
    next();
    return _current_token;
  }

  inline json_token_type set_token_and_next(json_token_type t, size_t n) {
    set_token(t);
    next(n);
    return _current_token;
  }

  inline void next() {
    _last_line_info = get_line_info();
    _current_column++;
    _stream.incr();
  }

  inline void next(size_t n) {
    _last_line_info = get_line_info();
    _current_column += n;
    _stream.incr(n);
  }

  inline void new_line() {
    _last_line_info = get_line_info();
    _current_column = 1;
    _current_line++;
  }

  inline void skip_white_spaces() {
    _last_line_info = get_line_info();

    while (zb::is_one_of(*_stream, ' ', '\t')) {
      _current_column++;
      _stream.incr();
    }
  }

  inline void set_identifier(std::string_view v, bool is_sv_string = false) {
    _identifier = v;
    _is_string_view_identifier = is_sv_string;
  }
};
} // namespace zs.
