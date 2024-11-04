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
#include "lang/zlexer.h"
#include "lang/jit/zclosure_compile_state.h"

// #define ZS_COMPILER_USE_HANDLER 1
// #define ZS_COMPILER_DEV 1

namespace zs {

///
class jit_compiler : public engine_holder {
public:
  jit_compiler(zs::engine* eng);

  ~jit_compiler();

  zs::error_result compile(std::string_view content, std::string_view filename, object& output,
      zs::virtual_machine* vm = nullptr, zs::token_type* prepended_token = nullptr, bool with_vargs = false);

  //
  // MARK: Parser
  //

  ZS_CHECK zs::error_code expect(token_type tok) noexcept;
  ZS_CHECK zs::error_code expect_get(token_type tok, object& ret);

  ZS_CK_INLINE const zs::string& get_error() const noexcept { return _error_message; }

  ZS_CK_INLINE bool is_end_of_statement() const noexcept {
    using enum token_type;
    return (_lexer->last_token() == tok_endl) || is(tok_eof, tok_rcrlbracket, tok_semi_colon);
  }

  //
  // MARK: Lexer
  //

  token_type lex();

  //
  // MARK: Token helpers
  //

  ZS_CK_INLINE bool is(token_type t) const noexcept { return _token == t; }

  template <class... Tokens>
  ZS_CK_INLINE bool is(Tokens... tokens) const noexcept {
    return zb::is_one_of(_token, tokens...);
  }

  template <class... Tokens>
  ZS_CK_INLINE bool is_not(Tokens... tokens) const noexcept {
    return !zb::is_one_of(_token, tokens...);
  }

  template <class... Tokens>
  ZS_INLINE bool lex_if(Tokens... tokens) noexcept {
    if (zb::is_one_of(_token, tokens...)) {
      lex();
      return true;
    }
    return false;
  }

  ZS_CK_INLINE bool is_var_decl_tok() const noexcept {
    switch (_token) {
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
  
  
  ZS_CK_INLINE static bool is_var_decl_tok(token_type t)   noexcept {
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

  ZS_CK_INLINE bool is_var_decl_tok_no_const() const noexcept {
    switch (_token) {
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

private:
  struct helper;
  using enum token_type;

  enum class expr_type { e_expr, e_object, e_base, e_local, e_capture };

  struct scope {
    int_t n_captures;
    int_t stack_size;
  };

  struct expr_state {
    /// Expression's type.
    expr_type type = expr_type::e_expr;

    /// Expression's location on stack (-1 for e_object and e_base).
    int_t pos = 0;

    /// Signal not to deref the next value.
    bool no_get = false;

    bool no_assign = false;
  };

  enum class parser_type { pt_default };

  enum class parse_op : uint8_t;

  struct struct_parser;

  //
  // MARK: Members
  //

  zs::lexer* _lexer = nullptr;
  zs::token_type _token = token_type::tok_none;
  zs::closure_compile_state* _ccs = nullptr;
  zs::string _error_message;
  zs::object _compile_time_consts;
  zs::virtual_machine* _vm = nullptr;
  expr_state _estate;
  scope _scope;
  int_t _enum_counter = 0;
  bool _in_template = false;

  template <parse_op Op>
  ZS_CK_INLINE_CXPR static parse_op next_parse_op() noexcept {
    return static_cast<parse_op>(uint8_t(Op) + 1);
  }

  template <parse_op Op, class... Args>
  ZS_CHECK zs::error_result parse(Args... args);

  enum class action_type { act_move_if_current_target_is_local, act_invoke_expr };

  template <action_type Action, class... Args>
  ZS_CHECK zs::error_result action(Args... args);

  template <opcode Op, class... Args>
  ZS_INLINE void add_instruction(Args... args) {
    _ccs->add_instruction<Op>(std::forward<Args>(args)...);

#if ZS_DEBUG
    _ccs->add_debug_line_info(_lexer->_last_line_info);
#endif
  }

  zs::error_result parse_include_or_import_statement(token_type tok);
};
} // namespace zs.
