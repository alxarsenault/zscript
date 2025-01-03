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

#include <zscript/base/zbase.h>

namespace zs {
#define __ZS_TOK_IMPL_LAST(name) zs::token_type::tok_##name
#define __ZS_TOK_IMPL(name) zs::token_type::tok_##name,
#define ZS_TOK(...) ZBASE_FOR_EACH_WITH_LAST(__ZS_TOK_IMPL, __ZS_TOK_IMPL_LAST, __VA_ARGS__)

enum class token_type : uint8_t {
#define ZS_DECL_TOKEN(name) tok_##name,
#include "lex/ztoken_def.h"
#undef ZS_DECL_TOKEN
};

inline constexpr const char* token_to_string(token_type token) noexcept {
  switch (token) {
#define ZS_DECL_TOKEN(name)    \
  case token_type::tok_##name: \
    return #name;
#include "lex/ztoken_def.h"
#undef ZS_DECL_TOKEN
  }

  return "unknown";
}

ZS_CK_INLINE_CXPR bool is_var_decl_tok_no_const(token_type t) noexcept {
  using enum token_type;
  switch (t) {
  case tok_var:
  case tok_array:
  case tok_table:
  case tok_string:
  case tok_char:
  case tok_int:
  case tok_bool:
  case tok_float:
  case tok_atom:
  case tok_number:
    return true;

  default:
    return false;
  }
  return false;
}

ZS_CK_INLINE_CXPR bool is_var_decl_tok(token_type t) noexcept {
  using enum token_type;
  return t == tok_const or zs::is_var_decl_tok_no_const(t);
}

ZS_CK_INLINE_CXPR bool is_var_decl_prefix_token(token_type t) noexcept {
  using enum token_type;
  switch (t) {
  case tok_const:
  case tok_static:
  case tok_private:
    return true;

  default:
    return false;
  }
  return false;
}

} // namespace zs.
