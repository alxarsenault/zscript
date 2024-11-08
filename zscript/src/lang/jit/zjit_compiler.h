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

namespace zs {
///
class jit_compiler : public zs::engine_holder, jit::closure_compile_state_ref, zs::lexer_ref {
public:
  jit_compiler(zs::engine* eng);

  zs::error_result compile(std::string_view content, std::string_view filename, object& output,
      zs::virtual_machine* vm = nullptr, zs::token_type* prepended_token = nullptr, bool with_vargs = false);

  ZS_CK_INLINE const zs::string& get_error() const noexcept { return _error_message; }

private:
  using enum opcode;

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

  struct struct_parser;

  //
  // MARK: Members
  //

  zs::string _error_message;
  zs::object _compile_time_consts;
  zs::virtual_machine* _vm = nullptr;
  expr_state _estate;
  scope _scope;
  int_t _enum_counter = 0;

  ZS_CHECK zs::error_code expect(token_type tok) noexcept;
  ZS_CHECK zs::error_code expect_get(token_type tok, object& ret);

  template <opcode Op, class... Args>
  ZS_INLINE void add_instruction(Args... args) {
    _ccs->add_instruction<Op>(std::forward<Args>(args)...);

#if ZS_DEBUG
    _ccs->add_debug_line_info(_lexer->_last_line_info);
#endif
  }

  ZS_INLINE void add_move_instruction(int_t target) {
    add_instruction<opcode::op_move>(_ccs->new_target(), (uint8_t)target);
  }

  template <auto Op, class... Args>
  ZS_CHECK zs::error_result parse(Args... args);

  zs::error_result add_small_string_instruction(std::string_view s, int_t target_idx);

  ZS_INLINE zs::error_result add_small_string_instruction(std::string_view s) {
    return add_small_string_instruction(s, _ccs->new_target());
  }

  zs::error_result add_string_instruction(std::string_view s, int_t target_idx);
  zs::error_result add_string_instruction(const object& sobj, int_t target_idx);

  ZS_INLINE zs::error_result add_string_instruction(std::string_view s) {
    return add_string_instruction(s, _ccs->new_target());
  }

  ZS_INLINE zs::error_result add_string_instruction(const object& sobj) {
    return add_string_instruction(sobj, _ccs->new_target());
  }

  zs::error_result add_export_string_instruction(const object& var_name);

  zs::error_result add_to_export_table(const object& var_name);

  zs::error_result handle_error(zs::error_code ec, std::string_view msg, const zb::source_location& loc);

  void move_if_current_target_is_local();

  zs::error_result invoke_expr(zb::member_function_pointer<jit_compiler, zs::error_result> fct);

  template <class Fct>
  inline zs::error_result expr_call(Fct&& fct);

  template <class Fct>
  inline zs::error_result expr_call(Fct&& fct, expr_state e);

  inline bool needs_get() const noexcept;
  inline bool needs_get_no_assign() const noexcept;
  inline bool will_modify() const noexcept;

  template <opcode Op>
  ZS_CK_INLINE zs::error_result do_arithmetic_expr(
      zb::member_function_pointer<jit_compiler, zs::error_result> fct, std::string_view symbol);
};

template <class Fct>
inline zs::error_result jit_compiler::expr_call(Fct&& fct) {
  expr_state es = std::exchange(_estate, expr_state{ expr_type::e_expr, -1, false });
  zs::error_result res = fct();
  _estate = es;
  return res;
}

template <class Fct>
inline zs::error_result jit_compiler::expr_call(Fct&& fct, expr_state e) {
  expr_state es = std::exchange(_estate, e);
  zs::error_result res = fct();
  _estate = es;
  return res;
}

ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

inline bool jit_compiler::needs_get() const noexcept {
  using enum token_type;

  switch (_token) {
  case tok_eq:
  case tok_lbracket:
  case tok_add_eq:
  case tok_mul_eq:
  case tok_div_eq:
  case tok_minus_eq:
  case tok_exp_eq:
  case tok_mod_eq:
  case tok_lshift_eq:
  case tok_rshift_eq:
  case tok_inv_eq:
  case tok_bitwise_and_eq:
    return false;
  case tok_incr:
  case tok_decr:
    if (!is_end_of_statement()) {
      return false;
    }
    break;

  case tok_lt: {
    if (is_template_function_call()) {
      return false;
    }
    break;
  }
  }

  return (!_estate.no_get || (_estate.no_get && (_token == tok_dot || _token == tok_lsqrbracket)));
}

bool jit_compiler::needs_get_no_assign() const noexcept {
  using enum token_type;

  switch (_token) {
  case tok_lbracket:
    return false;
  case tok_incr:
  case tok_decr:
    if (!is_end_of_statement()) {
      return false;
    }
    break;

  case tok_lt: {
    if (is_template_function_call()) {
      return false;
    }
    break;
  }
  }

  return (!_estate.no_get || (_estate.no_get && (_token == tok_dot || _token == tok_lsqrbracket)));
}

bool jit_compiler::will_modify() const noexcept {
  using enum token_type;

  switch (_token) {
  case tok_eq:
  case tok_lbracket:
  case tok_add_eq:
  case tok_mul_eq:
  case tok_div_eq:
  case tok_minus_eq:
  case tok_exp_eq:
  case tok_mod_eq:
  case tok_lshift_eq:
  case tok_rshift_eq:
  case tok_inv_eq:
  case tok_bitwise_and_eq:
  case tok_incr:
  case tok_decr:
    return true;
  }

  return false;
}

template <opcode Op>
ZS_CK_INLINE zs::error_result jit_compiler::do_arithmetic_expr(
    zb::member_function_pointer<jit_compiler, zs::error_result> fct, std::string_view symbol) {
  lex();
  ZS_RETURN_IF_ERROR(invoke_expr(fct));

  int_t op2 = _ccs->pop_target();
  int_t op1 = _ccs->pop_target();

  add_instruction<Op>(_ccs->new_target(), (uint8_t)op1, (uint8_t)op2);
  _estate.type = expr_type::e_expr;

  return {};
}

ZBASE_PRAGMA_POP()
} // namespace zs.
