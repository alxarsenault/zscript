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

  zs::object var_name;

  //  bool is_global = is(tok_global);
  //
  //  if (is_global) {
  //    lex();
  //    ZS_COMPILER_EXPECT(tok_dot);
  //    ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  //
  //    add_new_target_instruction<op_load_global>();
  //    add_string_instruction(var_name);
  //
  //    if (is(tok_dot)) {
  //      int_t key_idx = _ccs->pop_target();
  //      int_t table_idx = _ccs->pop_target();
  //      add_new_target_instruction<op_get>(
  //          (uint8_t)table_idx, (uint8_t)key_idx, get_op_flags_t::gf_look_in_root);
  //    }
  //
  //    while (lex_if(tok_dot)) {
  //      ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  //      add_string_instruction(var_name);
  //
  //      if (is(tok_dot)) {
  //        int_t key_idx = _ccs->pop_target();
  //        int_t table_idx = _ccs->pop_target();
  //        add_new_target_instruction<op_get>(
  //            (uint8_t)table_idx, (uint8_t)key_idx, get_op_flags_t::gf_look_in_root);
  //      }
  //    }
  //  }
  //  else {
  ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  //  }

  ZS_RETURN_IF_ERROR(parse_function(var_name, false));

  //  if (is_global) {
  //    int_t value_idx = _ccs->pop_target();
  //    int_t key_idx = _ccs->pop_target();
  //    int_t table_idx = _ccs->top_target();
  //
  //    add_instruction<op_set>((uint8_t)-1, (uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx, true);
  //
  //    _estate.type = expr_type::e_object;
  //    _estate.pos = table_idx;
  //  }
  //  else {

  _ccs->pop_target();
  ZS_COMPILER_RETURN_IF_ERROR(
      add_stack_variable(var_name), "Duplicated local variable name ", var_name, ".\n");
  //  }

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_arrow_lamda) {

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

ZS_JIT_COMPILER_PARSE_OP(p_function_call_args, bool rawcall, bool table_call) {

  int_t nargs = 1; // this.

  if (table_call) {
    if (auto err = parse_expression()) {
      return err;
    }

    move_if_current_target_is_local();
    nargs++;
  }
  else {
    while (is_not(tok_rbracket)) {

      if (is(tok_identifier) and _lexer->peek() == tok_left_arrow) {
        zs::print("DSLKDJSKDJSKJDKLSJDLS");
        lex_n(2);
      }

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

  // Rawcall.
  if (rawcall) {
    ZS_TODO("Implement");
    if (nargs < 3) {
      return ZS_COMPILER_ERROR(invalid_argument, "rawcall requires at least 2 parameters (callee and this)");
    }

    nargs -= 2; // removes callee and this from count
  }

  //  zb::print("NARGS", nargs, _ccs->top_target());

  for (int_t i = 0; i < (nargs - 1); i++) {
    pop_target();
  }

  int_t stack_base = pop_target();
  int_t closure = pop_target();

  add_new_target_instruction<op_call>((uint8_t)closure, // closure_idx.
      (uint8_t)stack_base, // this_idx.
      (uint8_t)nargs, // n_params.
      (uint64_t)stack_base // stack_base.
  );

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_member_function_call_args) {

  int_t nargs = 1; // this.

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

  lex();

  for (int_t i = 0; i < (nargs - 1); i++) {
    pop_target();
  }

  int_t stack_base = pop_target();
  int_t closure = pop_target();

  add_new_target_instruction<op_call>((uint8_t)closure, // closure_idx.
      (uint8_t)stack_base, // this_idx.
      (uint8_t)nargs, // n_params.
      (uint64_t)stack_base // stack_base.
  );

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_function_call_args_template, std::string_view meta_code) {

  int_t nargs = 1; // this.

  {
    zs::lexer* last_lexer = _lexer;

    zs::lexer lexer(_engine, meta_code);
    _lexer = &lexer;
    _in_template = true;

    {
      uint8_t array_target = _ccs->new_target();
      add_instruction<op_new_obj>(array_target, object_type::k_array);
      add_instruction<op_set_meta_argument>(array_target);
      lex();

      while (is_not(tok_rsqrbracket)) {
        if (auto err = parse_expression()) {
          zb::print("ERRRO");
          return err;
        }

        if (_token == tok_comma) {
          lex();
        }

        int_t val = _ccs->pop_target();
        add_instruction<op_array_append>(array_target, (uint8_t)val);
      }

      lex();
    }

    _in_template = false;

    if (_token == tok_lex_error) {
      last_lexer->_current_token = _lexer->_current_token;
      last_lexer->_last_token = _lexer->_last_token;
      _lexer = last_lexer;
      return invalid;
    }

    _lexer = last_lexer;
    _token = _lexer->_current_token;
    nargs++;

    move_if_current_target_is_local();
  }

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

  lex();

  for (int_t i = 0; i < (nargs - 1); i++) {
    _ccs->pop_target();
  }

  int_t stack_base = _ccs->pop_target();

  int_t closure = _ccs->pop_target();

  add_new_target_instruction<op_call>((uint8_t)closure, // closure_idx.
      (uint8_t)stack_base, // this_idx.
      (uint8_t)nargs, // n_params.
      (uint64_t)stack_base // stack_base.
  );

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

// ZS_JIT_COMPILER_PARSE_OP(p_export_function_statement) {
//   _ccs->push_export_target();
//
//   zs::object var_name;
//   ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
//   ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
//
//   ZS_RETURN_IF_ERROR(parse_function(var_name, false));
//
//   int_t value_idx = _ccs->pop_target();
//   int_t key_idx = _ccs->pop_target();
//   int_t table_idx = _ccs->pop_target();
//   add_instruction<op_set>(k_invalid_target, (uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx,
//   true);
//
//   _estate.type = expr_type::e_object;
//   _estate.pos = table_idx;
//   return {};
// }

// ZS_JIT_COMPILER_PARSE_OP(p_global_function_statement) {
//
//   lex();
//
//   add_new_target_instruction<op_load_global>();
//
//   zs::object var_name;
//   ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
//   add_string_instruction(var_name);
//
//   ZS_RETURN_IF_ERROR(parse_function(var_name, false));
//
//   int_t value_idx = _ccs->pop_target();
//   int_t key_idx = _ccs->pop_target();
//   int_t table_idx = _ccs->top_target();
//
//   add_instruction<op_set>((uint8_t)-1, (uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx, true);
//
//   _estate.type = expr_type::e_object;
//   _estate.pos = table_idx;
//
//   return {};
// }
} // namespace zs.
