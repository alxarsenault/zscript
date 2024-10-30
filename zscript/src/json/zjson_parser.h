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

#include "json/zjson_lexer.h"

namespace zs {

///
class json_parser : public engine_holder {
public:
  json_parser(zs::engine* eng);

  ~json_parser();

  zs::error_result parse(zs::virtual_machine* vm, std::string_view content, const object& table,
      object& output, zs::json_token_type* prepended_token = nullptr);

  //
  // MARK: Parser
  //

  ZB_CHECK zs::error_code expect(json_token_type tok) noexcept;
  ZB_CHECK zs::error_code expect_get(json_token_type tok, object& ret);

  ZB_CK_INLINE const zs::string& get_error() const noexcept { return _error_message; }

  ZB_CK_INLINE bool is_end_of_statement() const noexcept {
    using enum json_token_type;
    return (_lexer->last_token() == tok_endl) || is(tok_eof, tok_rcrlbracket);
  }

  //
  // MARK: Lexer
  //

  json_token_type lex();

  //
  // MARK: Token helpers
  //

  ZB_CK_INLINE bool is(json_token_type t) const noexcept { return _token == t; }

  template <class... Tokens>
  ZB_CK_INLINE bool is(Tokens... tokens) const noexcept {
    return zb::is_one_of(_token, tokens...);
  }

  template <class... Tokens>
  ZB_CK_INLINE bool is_not(Tokens... tokens) const noexcept {
    return !zb::is_one_of(_token, tokens...);
  }

private:
  struct helper;

  //
  // MARK: Members
  //

  zs::json_lexer* _lexer = nullptr;
  zs::json_token_type _token = json_token_type::tok_none;
  zs::vector<object> _stack;
  zs::string _error_message;
  zs::virtual_machine* _vm = nullptr;
  const zs::object* _table = nullptr;
  zs::error_result parse_value(zs::object& value);
  zs::error_result parse_table(zs::object& value);
  zs::error_result parse_array(zs::object& value);
  zs::error_result parse_table_value(zs::object& key, zs::object& value);
};
} // namespace zs.
