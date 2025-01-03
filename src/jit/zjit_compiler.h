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
#include "lex/zlexer.h"
#include "jit/zclosure_compile_state.h"
#include <zscript/base/utility/scoped.h>

namespace zs {

enum class parse_op : uint8_t {
  // 2**n
  p_exponential,

  // '*', '/', '%'
  p_mult,

  // '+', '-'.
  p_plus,

  // '<<', '>>'.
  p_shift,

  // '>', '<', '>=', '<=', 'in'.
  p_compare,

  // '==', '!=', '<=>', '==='.
  p_eq_compare,

  // '&'.
  p_bitwise_and,

  // 'xor'.
  p_bitwise_xor,

  // '|'.
  p_bitwise_or,

  // '&&'.
  p_and,

  // '||'
  p_or,

  // '|||'
  p_triple_or,

  count
};

///
class jit_compiler : public zs::engine_holder, jit::closure_compile_state_ref, zs::lexer_ref {
public:
  using target_type_info_t = closure_compile_state::target_type_info_t;

  jit_compiler(zs::engine* eng);

  zs::error_result compile(std::string_view content, object filename, object& output,
      zs::virtual_machine* vm = nullptr, zs::token_type* prepended_token = nullptr, bool with_vargs = false,
      bool add_line_info = true);

  template <class String>
    requires zb::is_string_view_convertible_v<String>
  ZS_CK_INLINE zs::error_result compile(std::string_view content, String&& filename, object& output,
      zs::virtual_machine* vm = nullptr, zs::token_type* prepended_token = nullptr, bool with_vargs = false,
      bool add_line_info = true) {
    return compile(
        content, zs::_s(_engine, filename), output, vm, prepended_token, with_vargs, add_line_info);
  }

  ZS_CHECK zs::string get_error() const noexcept;
  ZS_CK_INLINE const zs::error_stack& get_errors() const noexcept { return _errors; }

private:
  struct proxy;
  using enum opcode;
  using enum arithmetic_op;
  using enum arithmetic_uop;
  using enum error_code;

  enum class expr_type { e_expr, e_object, e_base, e_local, e_capture };

  struct scope {
    int_t n_captures;
    int_t stack_size;
    int_t scope_id;
  };

  struct expr_state {
    /// Expression's type.
    expr_type type = expr_type::e_expr;

    /// Expression's location on stack (-1 for e_object and e_base).
    int_t pos = 0;

    /// Signal not to deref the next value.
    bool no_get = false;

    bool no_assign = false;
    bool no_new_set = false;
  };

  //
  // MARK: Members
  //
  zs::string _error_message;
  zs::error_stack _errors;
  zs::virtual_machine* _vm = nullptr;
  expr_state _estate;
  scope _scope;
  int_t _scope_id_counter = 0;
  bool _add_line_info = true;

  template <opcode Op, class... Args>
  ZS_INLINE void add_instruction(Args&&... args) noexcept;

  template <opcode Op, class... Args>
  ZS_INLINE void add_new_target_instruction(Args... args) noexcept;

  template <opcode Op, class... Args>
  ZS_INLINE void add_top_target_instruction(Args... args) noexcept;

  ZS_INLINE void add_new_target_move_this_instruction() noexcept;

  void add_string_instruction(std::string_view s, int_t target_idx = k_invalid_target) noexcept;
  ZS_INLINE void add_string_instruction(const object& sobj, int_t target_idx = k_invalid_target) noexcept;
  ZS_INLINE void add_string_instruction(const char* s, int_t target_idx = k_invalid_target) noexcept;

  /// Add a new named stack variable.
  ZS_CHECK zs::error_result add_stack_variable(const object& name, int_t* ret_pos = nullptr,
      uint32_t mask = 0, uint64_t custom_mask = 0, bool is_const = false);

  zs::error_result handle_error(
      zs::error_code ec, const zs::line_info& linfo, std::string_view msg, const zb::source_location& loc);

  template <class... Args>
  inline zs::error_result handle_error(
      zs::error_code ec, const zs::line_info& linfo, const zb::source_location& loc, const Args&... args);

  void move_if_current_target_is_local();

  inline bool needs_get() const noexcept;

  ZS_CK_INLINE int_t scope_id() const noexcept { return _scope.scope_id; }

  ZS_CK_INLINE scope start_new_scope() noexcept;

  ZS_CK_INLINE auto start_new_auto_scope() noexcept {
    return zb::scoped([&, previous_scope = start_new_scope()]() { _scope = previous_scope; });
  }

  ZS_CK_INLINE auto start_new_auto_scope_with_close_capture() noexcept {
    return zb::scoped([&, previous_scope = start_new_scope()]() {
      close_capture_scope();
      _scope = previous_scope;
    });
  }

  ZS_CK_INLINE auto new_auto_state() noexcept {
    return zb::scoped([&, previous_expr_state = _estate]() { _estate = previous_expr_state; });
  }

  ZS_CK_INLINE auto new_auto_scoped_closure_compile_state(zs::closure_compile_state* new_ccs) noexcept {
    return zb::scoped(
        [&, previous_compile_state = std::exchange(_ccs, new_ccs)]() { _ccs = previous_compile_state; });
  }
  ZS_CK_INLINE auto new_auto_scoped_closure_compile_state_with_set_stack_zero(
      zs::closure_compile_state* new_ccs) noexcept {
    return zb::scoped([&, previous_compile_state = std::exchange(_ccs, new_ccs)]() {
      _ccs->set_stack_size(0);
      _ccs = previous_compile_state;
    });
  }

  void close_capture_scope();

  template <auto Op, class... Args>
  ZS_CHECK zs::error_result parse(Args... args);

  template <opcode Op, class Fct, class... Args>
  inline zs::error_result do_binary_expr(Fct&& fct, Args&&... args);

  template <opcode Op, parse_op P, class... Args>
  inline zs::error_result do_binary_expr(Args&&... args);

  template <class Tag>
  auto create_local_lambda() {
    zb_static_error("This method should never be called.");
  }

  template <class Tag, class... Args>
  ZS_CK_INLINE zs::error_result call_local_lambda(Args&&... args) {
    return create_local_lambda<Tag>()(std::forward<Args>(args)...);
  }

  //
  // MARK: Parse.
  //

  zs::error_result parse_statement(bool close_frame);
  zs::error_result parse_expression();
  zs::error_result parse_comma();
  zs::error_result parse_semi_colon();
  zs::error_result parse_if();
  zs::error_result parse_arrow_lamda();
  zs::error_result parse_function_call_args(bool table_call);
  zs::error_result parse_table();
  zs::error_result parse_for();
  zs::error_result parse_for_auto(std::span<zs::token_type> sp);
  zs::error_result parse_factor(object* name);
  zs::error_result parse_prefixed();
  zs::error_result parse_variable_declaration();
  zs::error_result parse_variable_prefix(variable_type_info& vinfo);
  zs::error_result parse_variable(variable_type_info& vinfo);
  zs::error_result parse_arith_eq(token_type op);
  zs::error_result parse_struct_statement();
  zs::error_result parse_struct(const object* struct_name);
  zs::error_result parse_function_statement();

  /// Parsing function definition: `(parameters) { content }`.
  zs::error_result parse_function(
      const object& name, bool is_lamda, bool parse_bound_target_and_add_op_new_closure_instruction = true);

  static zs::error_result check_compile_time_mask(
      zs::opcode last_op, const variable_type_info& vinfo, bool& procesed);
};

ZBASE_PRAGMA_PUSH_NO_MISSING_SWITCH_WARNING()

template <opcode Op, class... Args>
void jit_compiler::add_instruction(Args&&... args) noexcept {
  _ccs->add_instruction<Op>(std::forward<Args>(args)...);

#if ZS_DEBUG
  _ccs->add_debug_line_info(_lexer->get_last_line_info());
#endif
}

template <opcode Op, class... Args>
void jit_compiler::add_new_target_instruction(Args... args) noexcept {
  add_instruction<Op>(new_target(), std::forward<Args>(args)...);
}

template <opcode Op, class... Args>
void jit_compiler::add_top_target_instruction(Args... args) noexcept {
  add_instruction<Op>(top_target(), std::forward<Args>(args)...);
}

void jit_compiler::add_new_target_move_this_instruction() noexcept { add_new_target_instruction<op_move>(0); }

void jit_compiler::add_string_instruction(const object& sobj, int_t target_idx) noexcept {
  ZS_ASSERT(sobj.is_string(), "Invalid string type");
  add_string_instruction(sobj.get_string_unchecked(), target_idx);
}

void jit_compiler::add_string_instruction(const char* s, int_t target_idx) noexcept {
  add_string_instruction(std::string_view(s), target_idx);
}

jit_compiler::scope jit_compiler::start_new_scope() noexcept {
  return std::exchange(
      _scope, { (int_t)_ccs->_n_capture, (int_t)_ccs->get_stack_size(), ++_scope_id_counter });
}

inline bool jit_compiler::needs_get() const noexcept {

  switch (_token) {
  case tok_eq:
  case tok_lbracket: // Function call.
  case tok_lcrlbracket: // Table function call.
  case tok_add_eq:
  case tok_mul_eq:
  case tok_div_eq:
  case tok_sub_eq:
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
  }

  return (!_estate.no_get || (_estate.no_get && (_token == tok_dot || _token == tok_lsqrbracket)));
}

template <opcode Op, class Fct, class... Args>
inline zs::error_result jit_compiler::do_binary_expr(Fct&& fct, Args&&... args) {
  lex();

  const expr_state es = std::exchange(_estate, expr_state{ expr_type::e_expr, -1, false, false, false });
  ZS_RETURN_IF_ERROR(fct());
  _estate = es;
  _estate.type = expr_type::e_expr;

  target_t rhs = pop_target();
  add_new_target_instruction<Op>(std::forward<Args>(args)..., pop_target(), rhs);
  _estate.type = expr_type::e_expr;
  return {};
}

template <opcode Op, parse_op P, class... Args>
inline zs::error_result jit_compiler::do_binary_expr(Args&&... args) {
  lex();

  const expr_state es = std::exchange(_estate, expr_state{ expr_type::e_expr, -1, false, false, false });
  ZS_RETURN_IF_ERROR(parse<P>());
  _estate = es;
  _estate.type = expr_type::e_expr;

  target_t rhs = pop_target();
  add_new_target_instruction<Op>(std::forward<Args>(args)..., pop_target(), rhs);
  return {};
}

template <class... Args>
zs::error_result jit_compiler::handle_error(
    zs::error_code ec, const zs::line_info& linfo, const zb::source_location& loc, const Args&... args) {
  if constexpr (sizeof...(Args) == 1
      and zb::is_string_view_convertible_v<std::tuple_element_t<0, std::tuple<Args...>>>) {
    return handle_error(ec, linfo, std::string_view(args...), loc);
  }
  else {
    return handle_error(ec, linfo, std::string_view(zs::strprint(_engine, args...)), loc);
  }
}

ZBASE_PRAGMA_POP()
} // namespace zs.
