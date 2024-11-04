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
#include <zbase/utility/scoped.h>

namespace zs {

enum class parse_op : uint8_t;

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

  //  struct struct_parser;

  //
  // MARK: Members
  //
  struct macro {
    zs::object name;
    zs::object params;
    zs::object content;
  };

  zs::string _error_message;
  zs::error_stack _errors;
  zs::object _compile_time_consts;
  zs::virtual_machine* _vm = nullptr;
  expr_state _estate;
  scope _scope;
  int_t _scope_id_counter = 0;
  int_t _enum_counter = 0;
  zs::vector<macro> _macros;
  bool _add_line_info = true;
  int _is_header = 0;
  bool _has_doc_block = false;
  //  zs::object _doc_block;
  zs::vector<zs::object> _doc_blocks;

  template <opcode Op, class... Args>
  ZS_INLINE void add_instruction(Args... args) {
    _ccs->add_instruction<Op>(std::forward<Args>(args)...);

#if ZS_DEBUG
    _ccs->add_debug_line_info(_lexer->_last_line_info);
#endif
  }

  template <opcode Op, class... Args>
  ZB_INLINE void replace_last_instruction(Args... args) {
    _ccs->replace_last_instruction<Op>(std::forward<Args>(args)...);
  }

  template <opcode Op, class... Args>
  ZS_INLINE void add_new_target_instruction(Args... args) {
    add_instruction<Op>(_ccs->new_target(), std::forward<Args>(args)...);
  }

  template <opcode Op, class... Args>
  ZS_INLINE void add_top_target_instruction(Args... args) {
    add_instruction<Op>(_ccs->top_target(), std::forward<Args>(args)...);
  }

  inline void add_struct_doc_instruction(target_t idx) {
    add_string_instruction(_doc_blocks.back());
    add_instruction<op_set_struct_doc>(idx, pop_target());
    _doc_blocks.pop_back();
  }

  inline void add_struct_doc_member_instruction(target_t idx, object name) {
    add_string_instruction(name.get_string_unchecked());
    add_string_instruction(_doc_blocks.back());

    target_t doc_idx = pop_target();
    target_t name_idx = pop_target();
    add_instruction<op_set_struct_member_doc>(idx, name_idx, doc_idx);
    _doc_blocks.pop_back();
  }

  template <auto Op, token_type... Tokens, class... Args>
  ZS_INLINE zs::error_result lex_and_parse_if(Args... args) noexcept {
    if (is(Tokens...)) {
      lex();
      return parse<Op>(args...);
    }

    return {};
  }

  zs::error_result add_small_string_instruction(std::string_view s, int_t target_idx);

  ZS_INLINE zs::error_result add_small_string_instruction(std::string_view s) {
    return add_small_string_instruction(s, _ccs->new_target());
  }

  void add_string_instruction(std::string_view s, int_t target_idx);
  void add_string_instruction(const object& sobj, int_t target_idx);

  ZS_INLINE void add_string_instruction(std::string_view s) { add_string_instruction(s, _ccs->new_target()); }

  ZS_INLINE void add_string_instruction(const object& sobj) {
    add_string_instruction(sobj, _ccs->new_target());
  }

  //  zs::error_result add_export_string_instruction(const object& var_name);
  //
  //  zs::error_result add_to_export_table(const object& var_name);

  zs::error_result handle_error(zs::error_code ec, const zs::line_info& linfo, std::string_view msg,
      const zs::developer_source_location& loc);

  template <class... Args>
  inline zs::error_result handle_error(zs::error_code ec, const zs::line_info& linfo,
      const zs::developer_source_location& loc, const Args&... args) {
    if constexpr (sizeof...(Args) == 1
        and zb::is_string_view_convertible_v<std::tuple_element_t<0, std::tuple<Args...>>>) {
      return handle_error(ec, linfo, std::string_view(args...), loc);
    }
    else {
      return handle_error(ec, linfo, std::string_view(zs::strprint(_engine, args...)), loc);
    }
  }
  void move_if_current_target_is_local();

  zs::error_result invoke_expr(zb::member_function_pointer<jit_compiler, zs::error_result> fct);

  template <class Fct>
  inline zs::error_result expr_call(Fct&& fct);

  template <class Fct>
  inline zs::error_result expr_call(Fct&& fct, expr_state e);

  inline bool needs_get() const noexcept;
  inline bool needs_get_no_assign() const noexcept;
  inline bool will_modify() const noexcept;

  ZS_CK_INLINE int_t scope_id() const noexcept { return _scope.scope_id; }

  inline scope start_new_scope() noexcept {
    return std::exchange(
        _scope, { (int_t)_ccs->_n_capture, (int_t)_ccs->get_stack_size(), ++_scope_id_counter });
  }

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

  inline void close_capture_scope() {
    const int_t previous_n_capture = _ccs->_n_capture;
    if (_ccs->get_stack_size() != _scope.stack_size) {
      _ccs->set_stack_size(_scope.stack_size);
      if (previous_n_capture != (int_t)_ccs->_n_capture) {
        add_instruction<op_close>((u32)_scope.stack_size);
      }
    }
  }

  /// Add a new named stack variable.
  ZS_CHECK zs::error_result add_stack_variable(const object& name, int_t* ret_pos = nullptr,
      uint32_t mask = 0, uint64_t custom_mask = 0, bool is_const = false);

  ZS_CK_INLINE zs::error_result do_arithmetic_expr(
      arithmetic_op aop, zb::member_function_pointer<jit_compiler, zs::error_result> fct);

  template <parse_op P>
  ZS_CK_INLINE zs::error_result do_arithmetic_expr(arithmetic_op aop);

  template <auto Op, class... Args>
  ZS_CHECK zs::error_result parse(Args... args);

  template <class Tag>
  auto create_local_lambda();

  template <class Tag, class... Args>
  ZS_CK_INLINE zs::error_result call_local_lambda(Args&&... args) {
    return create_local_lambda<Tag>()(std::forward<Args>(args)...);
  }

  //
  //
  //
  zs::error_result parse_statement(bool close_frame);
  zs::error_result parse_expression();
  zs::error_result parse_comma();
  zs::error_result parse_semi_colon();
  zs::error_result parse_if();

  //
  //
  //

  zs::error_result parse_factor(object* name);

  /// '.', '[', '++', '--', '('.
  zs::error_result parse_prefixed();
  zs::error_result parse_variable_declaration();
  zs::error_result parse_variable_prefix(variable_type_info& vinfo);
  zs::error_result parse_variable(variable_type_info& vinfo);

  static zs::error_result check_compile_time_mask(
      zs::opcode last_op, const variable_type_info& vinfo, bool& procesed);
  //
  //
  //
  zs::error_result parse_struct_statement();
  zs::error_result parse_struct(const object* struct_name);

  //
  // Functions.
  //
  zs::error_result parse_function_statement();

  /// Parsing function definition: `(parameters) { content }`.
  zs::error_result parse_function(
      const object& name, bool is_lamda, bool parse_bound_target_and_add_op_new_closure_instruction = true);

  //
  //
  //

  inline zs::error_result gen_arith_eq(arithmetic_op aop);

  zs::error_result parse_arith_eq(token_type op);
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

  switch (_token) {
  case tok_eq:
  case tok_lbracket:

    ///////////////////////////
  case tok_lcrlbracket:

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
  switch (_token) {
  case tok_eq:
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

ZS_CK_INLINE zs::error_result jit_compiler::do_arithmetic_expr(
    arithmetic_op aop, zb::member_function_pointer<jit_compiler, zs::error_result> fct) {
  lex();
  ZS_RETURN_IF_ERROR(invoke_expr(fct));

  target_t op2 = pop_target();
  target_t op1 = pop_target();

  add_new_target_instruction<opcode::op_arith>(op1, op2, aop);
  _estate.type = expr_type::e_expr;

  return {};
}

template <parse_op P>
zs::error_result jit_compiler::do_arithmetic_expr(arithmetic_op aop) {
  lex();

  const expr_state es = std::exchange(_estate, expr_state{ expr_type::e_expr, -1, false, false, false });
  ZS_RETURN_IF_ERROR(parse<P>());
  _estate = es;
  _estate.type = expr_type::e_expr;

  target_t op2 = pop_target();
  target_t op1 = pop_target();
  add_new_target_instruction<opcode::op_arith>(op1, op2, aop);
  return {};
}

ZBASE_PRAGMA_POP()
} // namespace zs.
