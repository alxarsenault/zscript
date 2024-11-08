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

#include <zscript/zscript.h>

#include "ztoken.h"

#include <zbase/strings/utf8_span_stream.h>
#include <string>
#include <string_view>
#include <unordered_map>

namespace zs {

class parser;
class jit_compiler;
class lexer : public engine_holder {
  ZS_CLASS_COMMON;
  friend class jit_compiler;
  friend class parser;

public:
  using enum token_type;

  lexer(zs::engine* eng);
  lexer(zs::engine* eng, std::string_view code);
  ~lexer();

  void init(std::string_view code);

  token_type lex();

  zs::error_result lex(std::span<token_type>& buffer);
  zs::error_result lex(std::span<token_type>& buffer, token_type tok);
  zs::error_result lex_rbracket(std::span<token_type>& buffer);
  zs::error_result lex_for_auto(std::span<token_type>& buffer);
  zs::error_result lex_to(token_type tok);
  zs::error_result lex_to(token_type tok, size_t nmax);
  zs::error_result lex_compare(std::span<const token_type> buffer);

  inline zs::error_result lex_compare(std::initializer_list<token_type> buffer) {
    return lex_compare(std::span<const token_type>(buffer));
  }

  token_type peek() const;
  zs::error_result peek(std::span<token_type>& buffer) const;

  ZS_CHECK bool is_template_function_call() const;

  ZS_CK_INLINE zs::line_info get_line_info() const noexcept {
    return { (int_t)_current_line, (int_t)_current_column };
  }

  ZS_CK_INLINE zs::line_info get_last_line_info() const noexcept { return _last_line_info; }

  zs::object get_value() const;
  zs::object get_debug_value() const;

  ZS_CK_INLINE int_t get_int_value() const noexcept { return _int_value; }
  ZS_CK_INLINE float_t get_float_value() const noexcept { return _float_value; }

  ZS_CK_INLINE std::string_view get_identifier_value() const noexcept { return _identifier; }
  ZS_CK_INLINE std::string_view get_string_value() const noexcept { return _string; }
  ZS_CK_INLINE const zs::string& get_escaped_string_value() const noexcept { return _escaped_string; }

  ZS_CK_INLINE zs::object get_identifier() const noexcept {
    return _is_string_view_identifier ? zs::_sv(_identifier) : zs::object(_engine, _identifier);
  }

  ZS_CK_INLINE token_type current_token() const noexcept { return _current_token; }
  ZS_CK_INLINE token_type last_token() const noexcept { return _last_token; }

private:
  struct helper;

  zb::utf8_span_stream _stream;
  token_type _current_token = tok_none;
  token_type _last_token = tok_none;

  uint64_t _current_line = 1;
  uint64_t _current_column = 1;
  uint64_t _last_token_line = 1;

  zs::line_info _last_line_info = { 0, 0 };
  const char* _last_endl_ptr = nullptr;
  std::string_view _identifier;
  std::string_view _string;
  zs::string _escaped_string;
  int_t _int_value;
  float_t _float_value;
  bool _is_string_view_identifier = false;
  bool _export_block_comments = false;

  inline void set_token(token_type t) { _last_token = std::exchange(_current_token, t); }

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
    _last_endl_ptr = _stream.ptr();
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

struct lexer_ref {
  using enum token_type;
  zs::lexer* _lexer = nullptr;
  zs::token_type _token = tok_none;
  bool _in_template = false;

  inline token_type lex() {
    _token = _lexer->lex();

    if (_in_template and _token == tok_gt) {
      _token = tok_rsqrbracket;
    }

    return _token;
  }

  ZS_CK_INLINE bool is(token_type t) const noexcept { return _token == t; }

  template <class... Tokens>
  ZS_CK_INLINE bool is(Tokens... tokens) const noexcept {
    return zb::is_one_of(_token, tokens...);
  }

  template <class... Tokens>
  ZS_CK_INLINE bool is_not(Tokens... tokens) const noexcept {
    return !is(tokens...);
  }

  template <class... Tokens>
  ZS_INLINE bool lex_if(Tokens... tokens) noexcept {
    if (is(tokens...)) {
      lex();
      return true;
    }
    return false;
  }

  ZS_CK_INLINE static bool is_var_decl_tok(token_type t) noexcept {
    switch (t) {
    case tok_const:
    case tok_var:
    case tok_array:
    case tok_table:
    case tok_string:
    case tok_char:
    case tok_int:
    case tok_bool:
    case tok_float:
      return true;

    default:
      return false;
    }
    return false;
  }

  ZS_CK_INLINE static bool is_var_decl_tok_no_const(token_type t) noexcept {
    switch (t) {
    case tok_var:
    case tok_array:
    case tok_table:
    case tok_string:
    case tok_char:
    case tok_int:
    case tok_bool:
    case tok_float:
      return true;

    default:
      return false;
    }
    return false;
  }

  ZS_CK_INLINE bool is_var_decl_tok() const noexcept { return is_var_decl_tok(_token); }

  ZS_CK_INLINE bool is_var_decl_tok_no_const() const noexcept { return is_var_decl_tok_no_const(_token); }

  ZS_CK_INLINE bool is_template_function_call() const noexcept { return _lexer->is_template_function_call(); }

  ZS_CK_INLINE bool is_end_of_statement() const noexcept {
    return (_lexer->last_token() == tok_endl) || is(tok_eof, tok_rcrlbracket, tok_semi_colon);
  }
};

} // namespace zs.
