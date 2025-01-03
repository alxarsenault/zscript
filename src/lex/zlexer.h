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
#include <zscript/base/strings/utf8_span_stream.h>
#include "ztoken.h"

namespace zs {

struct lexer_ref;
class parser;
class jit_compiler;

class lexer : public engine_holder {
  ZS_CLASS_COMMON;
  friend class jit_compiler;
  friend class parser;
  friend struct lexer_ref;

public:
  using enum token_type;

  lexer(zs::engine* eng) noexcept;
  lexer(zs::engine* eng, std::string_view code) noexcept;
  ~lexer() noexcept;

  void init(std::string_view code) noexcept;

  token_type lex(bool keep_endl = false) noexcept;

  zs::error_result lex(std::span<token_type>& buffer) noexcept;

  zs::error_result lex(std::span<token_type>& buffer, token_type tok) noexcept;

  zs::error_result lex_rbracket(std::span<token_type>& buffer) noexcept;

  zs::error_result lex_to_rctrlbracket() noexcept;

  zs::error_result lex_for_auto(std::span<token_type>& buffer) noexcept;

  zs::error_result lex_to(token_type tok) noexcept;
  zs::error_result lex_to(token_type tok, size_t nmax) noexcept;

  bool lex_compare(std::span<const token_type> buffer) noexcept;

  inline bool lex_compare(std::initializer_list<token_type> buffer) noexcept {
    return lex_compare(std::span<const token_type>(buffer));
  }

  token_type peek(bool keep_endl = false) const noexcept;

  zs::error_result peek(std::span<token_type>& buffer) const noexcept;

  bool peek_compare(std::span<const token_type> buffer) const noexcept;

  inline bool peek_compare(std::initializer_list<token_type> buffer) const noexcept {
    return peek_compare(std::span<const token_type>(buffer));
  }

  ZS_CHECK bool is_right_arrow_function() const noexcept;

  ZS_CK_INLINE zs::line_info get_line_info() const noexcept {
    return { (int_t)_current_line, (int_t)_current_column };
  }

  ZS_CK_INLINE zs::line_info get_last_line_info() const noexcept { return _last_line_info; }

  zs::object get_value() const noexcept;
  zs::object get_debug_value() const noexcept;

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

  zb::utf8_span_stream& stream() noexcept { return _stream; }

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

  ZS_INLINE void set_token(token_type t) noexcept { _last_token = std::exchange(_current_token, t); }

  ZS_INLINE token_type set_token_and_next(token_type t) noexcept {
    set_token(t);
    next();
    return _current_token;
  }

  ZS_INLINE token_type set_token_and_next(token_type t, size_t n) noexcept {
    set_token(t);
    next(n);
    return _current_token;
  }

  ZS_INLINE void next() noexcept {
    _last_line_info = get_line_info();
    _current_column++;
    _stream.incr();
  }

  ZS_INLINE void next(size_t n) noexcept {
    _last_line_info = get_line_info();
    _current_column += n;
    _stream.incr(n);
  }

  inline void set_identifier(std::string_view v, bool is_sv_string = false) noexcept {
    _identifier = v;
    _is_string_view_identifier = is_sv_string;
  }
};

struct lexer_ref {
  using enum token_type;
  zs::lexer* _lexer = nullptr;
  zs::token_type _token = tok_none;

  ZS_CK_INLINE const char* stream_ptr() const noexcept { return _lexer->stream().ptr(); }
  ZS_CK_INLINE zs::line_info get_line_info() const noexcept { return _lexer->get_line_info(); }

  ZS_CK_INLINE zs::line_info get_last_line_info() const noexcept { return _lexer->get_last_line_info(); }

  inline token_type lex(bool keep_endl = false) noexcept { return (_token = _lexer->lex(keep_endl)); }

  inline token_type lex_n(size_t n) noexcept {
    while (n--) {
      lex(false);
    }

    return _token;
  }

  ZS_CK_INLINE bool peek_compare(std::span<const token_type> buffer) const noexcept {
    return _lexer->peek_compare(buffer);
  }

  ZS_CK_INLINE bool peek_compare(std::initializer_list<token_type> buffer) const noexcept {
    return _lexer->peek_compare(std::span<const token_type>(buffer));
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

  ZS_CK_INLINE bool last_is(token_type t) const noexcept { return _lexer->_last_token == t; }

  template <class... Tokens>
  ZS_CK_INLINE bool last_is(Tokens... tokens) const noexcept {
    return zb::is_one_of(_lexer->_last_token, tokens...);
  }

  template <class... Tokens>
  ZS_CK_INLINE bool last_is_not(Tokens... tokens) const noexcept {
    return !last_is(tokens...);
  }

  ZS_CK_INLINE bool next_is(token_type t) const noexcept { return _lexer->peek() == t; }

  template <class... Tokens>
  ZS_CK_INLINE bool next_is(Tokens... tokens) const noexcept {
    return zb::is_one_of(_lexer->peek(), tokens...);
  }

  template <class... Tokens>
  ZS_CK_INLINE bool next_is_not(Tokens... tokens) const noexcept {
    return !next_is(tokens...);
  }

  template <class... Tokens>
  ZS_INLINE bool lex_if(Tokens... tokens) noexcept {
    if (is(tokens...)) {
      lex();
      return true;
    }
    return false;
  }

  ZS_CK_INLINE bool is_var_decl_tok() const noexcept { return zs::is_var_decl_tok(_token); }

  ZS_CK_INLINE bool is_var_decl_tok_no_const() const noexcept { return zs::is_var_decl_tok_no_const(_token); }

  ZS_CK_INLINE bool is_var_decl_prefix_token() const noexcept { return zs::is_var_decl_prefix_token(_token); }

  ZS_CK_INLINE bool is_end_of_statement() const noexcept {
    return last_is(tok_endl) or is(tok_eof, tok_rcrlbracket, tok_semi_colon);
  }

  ZS_CK_INLINE zs::object get_identifier() const noexcept { return _lexer->get_identifier(); }
};

} // namespace zs.
