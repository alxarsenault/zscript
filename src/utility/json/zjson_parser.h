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
#include "zjson_lexer.h"

namespace zs {
class json_parser : public engine_holder {
public:
  json_parser(zs::engine* eng);

  ~json_parser();

  zs::error_result parse(zs::virtual_machine* vm, std::string_view content, const object& table,
      object& output, zs::json_token_type* prepended_token = nullptr);

  //
  // MARK: Parser
  //

  ZS_CHECK zs::error_code expect(json_token_type tok) noexcept;
  ZS_CHECK zs::error_code expect_get(json_token_type tok, object& ret);

  ZS_CK_INLINE const zs::string& get_error() const noexcept { return _error_message; }

  ZS_CHECK bool is_end_of_statement() const noexcept;

  //
  // MARK: Lexer
  //

  json_token_type lex();

  //
  // MARK: Token helpers
  //

  ZS_CK_INLINE bool is(json_token_type t) const noexcept { return _token == t; }

  template <class... Tokens>
  ZS_CK_INLINE bool is(Tokens... tokens) const noexcept {
    return zb::is_one_of(_token, tokens...);
  }

  template <class... Tokens>
  ZS_CK_INLINE bool is_not(Tokens... tokens) const noexcept {
    return !zb::is_one_of(_token, tokens...);
  }

private:
  struct helper;

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
