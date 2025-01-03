/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "jit/zjit_compiler_include_guard.h"

namespace zs {

struct parse_function_parameter {};

template <>
auto jit_compiler::create_local_lambda<parse_function_parameter>() {
  return [this](zs::closure_compile_state* fct_state, int_t& def_params) -> error_result {
    closure_compile_state& fs = *fct_state;

    if (fs.has_vargs_params()) {
      return ZS_COMPILER_ERROR(
          invalid_token, "A function can only have one variadic '...' parameter as last parameter.");
    }

    variable_type_info vinfo;
    zs::object param_name;

    // (...)
    if ((fs._has_vargs_params = lex_if(tok_triple_dots))) {
      param_name = zs::_ss("vargv");
    }
    else {
      ZS_RETURN_IF_ERROR(parse_variable(vinfo));

      if (is(tok_identifier)) {
        param_name = _lexer->get_value();
        lex();
      }
      else if (is(tok_triple_dots)) {
        return ZS_COMPILER_ERROR(invalid_token, "Variadic '...' goes after identifier.");
      }
      else {
        return ZS_COMPILER_ERROR(invalid_token, "Expected identifier in function parameter.");
      }
    }

    if (auto err = fs.add_parameter(param_name, vinfo.mask, vinfo.custom_mask, vinfo.is_const())) {
      return ZS_COMPILER_ERROR(err, "Could not add parameter.");
    }

    if (lex_if(tok_triple_dots)) {
      if (fs._has_vargs_params) {
        return ZS_COMPILER_ERROR(invalid_argument, "Too many variadic ...");
      }

      fs._has_vargs_params = true;
    }

    if (lex_if(tok_eq)) {
      if (lex_if(tok_triple_dots)) {
        if (fs._has_vargs_params) {
          return ZS_COMPILER_ERROR(invalid_token, "Too many variadic ...");
        }

        fs._has_vargs_params = true;

        // Add a default null parameter for 'vargv'.
        add_new_target_instruction<op_load_null>();
      }
      else if (auto err = parse_expression()) {
        return ZS_COMPILER_ERROR(err, "Could not parse function parameter default value expression.");
      }

      fs.add_default_param(top_target());

      // Increment the default parameters count.
      def_params++;

      return {};
    }

    if (fs._has_vargs_params) {
      // Add a default null parameter for 'vargv'.
      add_new_target_instruction<op_load_null>();
      fs.add_default_param(top_target());

      // Increment the default parameters count.
      def_params++;

      return {};
    }

    // If a default parameter was defined, all of them (from that point) needs
    // to have one too.
    if (def_params > 0) {
      return ZS_COMPILER_ERROR(invalid_token, "expected '=' after a default paramter definition");
    }

    return {};
  };
}

struct parse_function_parameters {};

template <>
auto jit_compiler::create_local_lambda<parse_function_parameters>() {
  return [this](zs::closure_compile_state* fct_state, int_t& def_params) -> error_result {
    closure_compile_state& fs = *fct_state;

    if (auto err = fs.add_parameter(zs::_ss("this"))) {
      return ZS_COMPILER_ERROR(err, "Could not add 'this' parameter to function state.");
    }

    while (is_not(tok_rbracket)) {
      if (auto err = call_local_lambda<parse_function_parameter>(fct_state, def_params)) {
        return ZS_COMPILER_ERROR(err, "Could not parse function paramaters.");
      }

      if (!lex_if(tok_comma) and is_not(tok_rbracket)) {
        return ZS_COMPILER_ERROR(invalid_token, "expected ')' or ',' at the end of function declaration");
      }
    }

    ZS_COMPILER_EXPECT(tok_rbracket);
    return {};
  };
}

zs::error_result jit_compiler::parse_function_statement() {
  lex();
  zs::object fct_name;
  ZS_COMPILER_EXPECT_GET(tok_identifier, fct_name);
  ZS_RETURN_IF_ERROR(parse_function(fct_name, false));

  pop_target();
  ZS_COMPILER_RETURN_IF_ERROR(add_stack_variable(fct_name), "Duplicated local variable name '",
      fct_name.get_string_unchecked(), "'.\n");

  return {};
}

zs::error_result jit_compiler::parse_arrow_lamda() {

  ZS_COMPILER_EXPECT(tok_lbracket);

  zs::closure_compile_state cc_state(_engine, *_ccs, zs::object());

  int_t def_params = 0;
  ZS_RETURN_IF_ERROR(call_local_lambda<parse_function_parameters>(&cc_state, def_params));

  for (int_t n = 0; n < def_params; n++) {
    pop_target();
  }

  ZS_COMPILER_EXPECT(tok_right_arrow);

  if (zb::scoped auto_scoped_ccs = new_auto_scoped_closure_compile_state_with_set_stack_zero(&cc_state)) {
    if (is_not(tok_lcrlbracket)) {
      ZS_RETURN_IF_ERROR(parse_expression());
      add_instruction<op_return>(_ccs->pop_target(), true);
      lex_if(tok_semi_colon);
    }
    else {
      ZS_RETURN_IF_ERROR(parse_statement(false));
    }
  }

  get_functions().push_back(cc_state.build_function_prototype());

  add_new_target_instruction<op_new_closure>((uint32_t)(_ccs->_functions.size() - 1), 0);

  return {};
}

zs::error_result jit_compiler::parse_function_call_args(bool is_table_call) {

  // this as first arg.
  uint8_t nargs = 1;

  if (is_table_call) {
    if (auto err = parse_expression()) {
      return err;
    }

    move_if_current_target_is_local();
    nargs++;
  }
  else {
    while (is_not(tok_rbracket)) {

      if (auto err = parse_expression()) {
        return err;
      }

      move_if_current_target_is_local();
      nargs++;

      if (is(tok_comma)) {
        lex();

        if (is(tok_rbracket)) {
          return ZS_COMPILER_ERROR(invalid_token, "expression expected, found ')'");
        }
      }
    }
  }

  lex();

  for (int_t i = 0; i < (nargs - 1); i++) {
    pop_target();
  }

  target_t stack_base = pop_target();
  target_t closure = pop_target();

  add_new_target_instruction<op_call>(closure, stack_base, (uint8_t)nargs);

  return {};
}

zs::error_result jit_compiler::parse_function(
    const object& name, bool is_lamda, bool parse_bound_target_and_add_op_new_closure_instruction) {

  int_t bound_target = k_invalid_target;

  if (parse_bound_target_and_add_op_new_closure_instruction) {
    if (lex_if(tok_lsqrbracket)) {
      ZS_RETURN_IF_ERROR(parse_expression());
      bound_target = top_target();
      ZS_COMPILER_EXPECT(tok_rsqrbracket);
    }
  }

  ZS_COMPILER_EXPECT(tok_lbracket);

  zs::closure_compile_state cc_state(_engine, *_ccs, name);
  int_t def_params = 0;
  ZS_RETURN_IF_ERROR(call_local_lambda<parse_function_parameters>(&cc_state, def_params));

  pop_target_if(bound_target != k_invalid_target);
  pop_n_target(def_params);

  if (zb::scoped auto_scoped_ccs = new_auto_scoped_closure_compile_state_with_set_stack_zero(&cc_state)) {
    if (is_not(tok_lcrlbracket) and is_lamda) {
      ZS_RETURN_IF_ERROR(parse_statement(false));
      lex_if(tok_semi_colon);
    }
    else {
      if (auto err = parse_statement(false)) {
        return err;
      }
    }
  }

  get_functions().push_back(cc_state.build_function_prototype());

  if (parse_bound_target_and_add_op_new_closure_instruction) {
    add_new_target_instruction<op_new_closure>(get_last_function_index(), (uint8_t)bound_target);
  }

  return {};
}

} // namespace zs.
