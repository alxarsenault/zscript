/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "lang/jit/zjit_compiler_include_guard.h"

namespace zs {

ZS_JIT_COMPILER_PARSE_OP(p_function_statement) {
  using enum token_type;
  using enum opcode;

  lex();

  zs::object var_name;

  //    bool is_global = is(tok_global, tok_double_colon);
  bool is_global = is(tok_global);

  if (is_global) {
    lex();
    ZS_COMPILER_EXPECT(tok_dot);
    ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);

    add_new_target_instruction<op_load_global>();
    add_string_instruction(var_name);

    if (is(tok_dot)) {
      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->pop_target();
      add_new_target_instruction<op_get>(
          (uint8_t)table_idx, (uint8_t)key_idx, get_op_flags_t::gf_look_in_root);
    }

    while (lex_if(tok_dot)) {
      ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
      add_string_instruction(var_name);

      if (is(tok_dot)) {
        int_t key_idx = _ccs->pop_target();
        int_t table_idx = _ccs->pop_target();
        add_new_target_instruction<op_get>(
            (uint8_t)table_idx, (uint8_t)key_idx, get_op_flags_t::gf_look_in_root);
      }
    }
  }
  else {
    ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);

    //    add_instruction<op_load_root>(_ccs->new_target());
    //    _ccs->push_target(0);
    //    ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));
    //
  }

  int_t bound_target = 0xFF;

  if (is(tok_lsqrbracket)) {
    ZS_RETURN_IF_ERROR(parse<p_bind_env>(REF(bound_target)));
  }

  ZS_COMPILER_EXPECT(tok_lbracket);

  ZS_RETURN_IF_ERROR(parse<p_create_function>(CREF(var_name), bound_target, false));

  add_new_target_instruction<op_new_closure>((uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

  if (is_global) {
    int_t value_idx = _ccs->pop_target();
    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->top_target();

    add_instruction<op_set>((uint8_t)-1, (uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx, true);

    _estate.type = expr_type::e_object;
    _estate.pos = table_idx;
  }
  else {

    //      int_t value_idx = _ccs->pop_target();
    //      int_t key_idx = _ccs->pop_target();
    //      int_t table_idx = _ccs->pop_target();
    //
    //
    //    if(_ccs->get_parent()) {
    //      add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
    //    }
    //    else {
    //      add_instruction<op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
    //    }
    //
    //
    //
    //    _estate.type = expr_type::e_expr;
    //    _estate.pos = -1;
    //    _estate.no_get = false;
    //    _estate.no_assign = false;

    //    _estate.type = expr_type::e_object;
    //    _estate.pos = _ccs->top_target();
    //    _estate.no_assign = false;

    //      _estate.type = expr_type::e_object;
    //      _estate.pos = table_idx;

    //    add_instruction<op_load_global>(_ccs->new_target());
    //    ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));

    //    add_instruction<op_load_global>(_ccs->new_target());
    //
    //    lex();
    //    _estate.type = expr_type::e_object;
    //    _estate.pos = _ccs->top_target();
    //    _estate.no_assign = false;

    _ccs->pop_target();
    ZS_COMPILER_RETURN_IF_ERROR(
        add_stack_variable(var_name), "Duplicated local variable name ", var_name, ".\n");
  }

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_function, bool lambda) {
  using enum token_type;
  using enum opcode;

  lex();

  int_t bound_target = 0xFF;

  if (is(tok_lsqrbracket)) {
    ZS_RETURN_IF_ERROR(parse<p_bind_env>(REF(bound_target)));
  }

  ZS_COMPILER_EXPECT(tok_lbracket);

  object dummy;
  ZS_RETURN_IF_ERROR(parse<p_create_function>(CREF(dummy), bound_target, lambda));

  add_new_target_instruction<op_new_closure>((uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_arrow_lamda) {
  using enum token_type;
  using enum opcode;

  ZS_COMPILER_EXPECT(tok_lbracket);

  zs::closure_compile_state cc_state(_engine, *_ccs, zs::object());

  int_t def_params = 0;
  ZS_COMPILER_PARSE(p_function_parameters, &cc_state, &def_params);

  for (int_t n = 0; n < def_params; n++) {
    pop_target();
  }

  ZS_COMPILER_EXPECT(tok_right_arrow);

  if (zb::scoped auto_scoped_ccs = new_auto_scoped_closure_compile_state_with_set_stack_zero(&cc_state)) {

    if (is_not(tok_lcrlbracket)) {
      ZS_COMPILER_PARSE(p_expression);
      add_instruction<op_return>(_ccs->pop_target(), true);
      lex_if(tok_semi_colon);
    }
    else {
      ZS_COMPILER_PARSE(p_statement, false);
    }
  }

  get_functions().emplace_back(cc_state.build_function_prototype(), false);

  add_new_target_instruction<op_new_closure>((uint32_t)(_ccs->_functions.size() - 1), 0);

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_function_call_args, bool rawcall) {

  int_t nargs = 1; // this.

  while (is_not(tok_rbracket)) {
    if (auto err = parse<p_expression>()) {
      return err;
    }

    move_if_current_target_is_local();
    nargs++;

    if (is(tok_comma)) {
      lex();

      if (is(tok_rbracket)) {
        return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expression expected, found ')'");
      }
    }
  }

  lex();

  // Rawcall.
  if (rawcall) {
    ZS_TODO("Implement");
    if (nargs < 3) {
      return ZS_COMPILER_ERROR(
          zs::error_code::invalid_argument, "rawcall requires at least 2 parameters (callee and this)");
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
        if (auto err = parse<p_expression>()) {
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
      return zs::error_code::invalid;
    }

    _lexer = last_lexer;
    _token = _lexer->_current_token;
    nargs++;

    move_if_current_target_is_local();
  }

  while (is_not(tok_rbracket)) {
    if (auto err = parse<p_expression>()) {
      return err;
    }

    move_if_current_target_is_local();
    nargs++;

    if (is(tok_comma)) {
      lex();

      if (is(tok_rbracket)) {
        return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expression expected, found ')'");
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

ZS_JIT_COMPILER_PARSE_OP(p_function_parameters, zs::closure_compile_state* fct_state, int_t* def_params) {

  ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("this")));

  while (is_not(tok_rbracket)) {
    if (is(tok_triple_dots)) {
      // TODO: Named triple dots?

      if (*def_params > 0) {
        return ZS_COMPILER_ERROR(zs::error_code::invalid_argument,
            "function with default parameters cannot have variable number of parameters");
      }

      ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("vargv")));
      fct_state->_vargs_params = true;
      lex();

      if (is_not(tok_rbracket)) {
        return ZS_COMPILER_ERROR(
            zs::error_code::invalid_token, "expected ')' after a variadic (...) parameter");
      }

      break;
    }
    else {
      uint32_t obj_type_mask = 0;
      uint64_t custom_mask = 0;
      bool is_static = false;
      bool is_const = false;

      ZS_COMPILER_PARSE(p_variable, &obj_type_mask, &custom_mask, &is_static, &is_const);

      zs::object param_name;
      ZS_COMPILER_EXPECT_GET(tok_identifier, param_name);
      ZS_RETURN_IF_ERROR(fct_state->add_parameter(param_name, obj_type_mask, custom_mask, is_const));

      if (lex_if(tok_eq)) {
        ZS_RETURN_IF_ERROR(parse<p_expression>());
        fct_state->add_default_param(_ccs->top_target());
        (*def_params)++;
      }

      // If a default parameter was defined, all of them (from that point) needs
      // to have one too.
      else if (*def_params > 0) {
        return ZS_COMPILER_ERROR(
            zs::error_code::invalid_token, "expected '=' after a default paramter definition");
      }

      if (is(tok_comma)) {
        lex();
      }
      else if (is_not(tok_rbracket)) {
        return ZS_COMPILER_ERROR(
            zs::error_code::invalid_token, "expected ')' or ',' at the end of function declaration");
      }
    }
  }

  ZS_COMPILER_EXPECT(tok_rbracket);
  return {};
}

/// Parsing function definition: `parameters) { content }`.
ZS_JIT_COMPILER_PARSE_OP(p_create_function, cobjref_t name, int_t boundtarget, bool lambda) {
  zs::closure_compile_state cc_state(_engine, *_ccs, name);

  int_t def_params = 0;
  ZS_COMPILER_PARSE(p_function_parameters, &cc_state, &def_params);

  if (boundtarget != k_invalid_target) {
    pop_target();
  }

  for (int_t n = 0; n < def_params; n++) {
    pop_target();
  }

  if (zb::scoped auto_scoped_ccs = new_auto_scoped_closure_compile_state_with_set_stack_zero(&cc_state)) {

    if (is_not(tok_lcrlbracket) and lambda) {
      ZS_COMPILER_PARSE(p_statement, false);
      if (!lex_if(tok_semi_colon)) {
        return ZS_COMPILER_ERROR(
            zs::errc::invalid_token, "Expected ';' after one-liner lamda '$(...) return ...;'.");
      }
    }
    else {
      ZS_COMPILER_PARSE(p_statement, false);
    }
  }

  get_functions().emplace_back(cc_state.build_function_prototype(), false);
  return {};
}

/// Parsing function definition: `parameters) { content }`.
ZS_JIT_COMPILER_PARSE_OP(p_create_normal_function, cobjref_t name) {
  zs::closure_compile_state cc_state(_engine, *_ccs, name);

  int_t def_params = 0;
  ZS_COMPILER_PARSE(p_function_parameters, &cc_state, &def_params);

  for (int_t n = 0; n < def_params; n++) {
    pop_target();
  }

  if (zb::scoped auto_scoped_ccs = new_auto_scoped_closure_compile_state_with_set_stack_zero(&cc_state)) {

    if (is_not(tok_lcrlbracket)) {
      return ZS_COMPILER_ERROR(errc::invalid_token, "invalid token ",
          zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
          zb::quoted<"'">(zs::token_to_string(tok_lcrlbracket)));
    }

    ZS_COMPILER_PARSE(p_statement, false);
  }

  get_functions().emplace_back(cc_state.build_function_prototype(), false);
  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_export_function_statement) {
  using enum token_type;
  using enum opcode;

  //  lex();

  _ccs->push_export_target();

  //  _ccs->push_target(0);
  //  add_instruction<op_load_global>(_ccs->new_target());

  zs::object var_name;
  ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);

  ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
  //  _ccs->_exported_names.push_back(var_name);
  //  ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));

  int_t bound_target = 0xFF;

  if (is(tok_lsqrbracket)) {
    ZS_RETURN_IF_ERROR(parse<p_bind_env>(REF(bound_target)));
  }

  ZS_COMPILER_EXPECT(tok_lbracket);

  ZS_RETURN_IF_ERROR(parse<p_create_function>(CREF(var_name), bound_target, false));

  add_new_target_instruction<op_new_closure>((uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

  int_t value_idx = _ccs->pop_target();
  int_t key_idx = _ccs->pop_target();
  int_t table_idx = _ccs->pop_target();

  add_instruction<op_set>((uint8_t)-1, (uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx, true);

  _estate.type = expr_type::e_object;
  _estate.pos = table_idx;

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_global_function_statement) {

  lex();

  add_new_target_instruction<op_load_global>();

  zs::object var_name;
  ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  add_string_instruction(var_name);

  int_t bound_target = 0xFF;

  if (is(tok_lsqrbracket)) {
    ZS_RETURN_IF_ERROR(parse<p_bind_env>(REF(bound_target)));
  }

  ZS_COMPILER_EXPECT(tok_lbracket);

  ZS_RETURN_IF_ERROR(parse<p_create_function>(CREF(var_name), bound_target, false));

  add_new_target_instruction<op_new_closure>((uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

  int_t value_idx = _ccs->pop_target();
  int_t key_idx = _ccs->pop_target();
  int_t table_idx = _ccs->top_target();

  add_instruction<op_set>((uint8_t)-1, (uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx, true);

  _estate.type = expr_type::e_object;
  _estate.pos = table_idx;

  return {};
}
} // namespace zs.
