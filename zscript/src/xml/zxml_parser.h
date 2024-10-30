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

#include "xml/zxml_lexer.h"

namespace zs {

///
class xml_parser : public engine_holder {
public:
  xml_parser(zs::engine* eng);

  ~xml_parser();

  zs::error_result parse(zs::virtual_machine* vm, std::string_view content, const object& table,
      object& output, zs::xml_token_type* prepended_token = nullptr);

  zs::error_result parse(zs::virtual_machine* vm, std::string_view content, object& output,
      zs::xml_token_type* prepended_token = nullptr) {
    return parse(vm, content, nullptr, output, prepended_token);
  }

  //
  // MARK: Parser
  //

  ZB_CHECK zs::error_code expect(xml_token_type tok) noexcept;
  ZB_CHECK zs::error_code expect_get(xml_token_type tok, object& ret);

  ZB_CK_INLINE const zs::string& get_error() const noexcept { return _error_message; }

  ZB_CK_INLINE bool is_end_of_statement() const noexcept {
    using enum xml_token_type;
    return (_lexer->last_token() == tok_endl) || is(tok_eof, tok_rcrlbracket);
  }

  //
  // MARK: Lexer
  //

  xml_token_type lex(bool lazy = false);

  //
  // MARK: Token helpers
  //

  ZB_CK_INLINE bool is(xml_token_type t) const noexcept { return _token == t; }

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

  zs::xml_lexer* _lexer = nullptr;
  zs::xml_token_type _token = xml_token_type::tok_none;
  zs::vector<object> _stack;
  zs::string _error_message;
  zs::virtual_machine* _vm = nullptr;
  const zs::object* _table = nullptr;
  const char* _last_ptr = nullptr;

  bool is_node_end() const;
  zs::error_result parse_bom();
  zs::error_result parse_value(zs::object& value);
  zs::error_result parse_node(zs::object& value);
  zs::error_result parse_element(zs::object& value);
  zs::error_result parse_tag_or_attribute_name(zs::object& value);
  zs::error_result parse_attributes(zs::object& value);
  //  zs::error_result parse_attribute_name(zs::object& name);
  zs::error_result parse_node_contents(zs::object& value, const char* begin_ptr);
};
} // namespace zs.
