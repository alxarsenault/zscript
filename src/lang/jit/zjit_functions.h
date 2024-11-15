
//
// MARK: - Functions.
//

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
    ZS_RETURN_IF_ERROR(add_string_instruction(var_name));

    if (is(tok_dot)) {
      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->pop_target();
      add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, true);
    }

    while (lex_if(tok_dot)) {
      ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
      ZS_RETURN_IF_ERROR(add_string_instruction(var_name));

      if (is(tok_dot)) {
        int_t key_idx = _ccs->pop_target();
        int_t table_idx = _ccs->pop_target();
        add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, true);
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

    add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);

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
    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
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

ZS_JIT_COMPILER_PARSE_OP(p_function_call_args, bool rawcall) {

  using enum token_type;

  int_t nargs = 1; // this.

  while (is_not(tok_rbracket)) {
    if (auto err = parse<p_expression>()) {
      return err;
    }

    //        MoveIfCurrentTargetIsLocal();

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

  ZS_TODO("Implement");
  // Rawcall.
  if (rawcall) {

    if (nargs < 3) {

      return ZS_COMPILER_ERROR(
          zs::error_code::invalid_argument, "rawcall requires at least 2 parameters (callee and this)");
    }

    nargs -= 2; // removes callee and this from count
  }

  //  zb::print("NARGS", nargs, _ccs->top_target());

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

ZS_JIT_COMPILER_PARSE_OP(p_function_call_args_template, std::string_view meta_code) {

  using enum token_type;
  using enum opcode;

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

ZS_JIT_COMPILER_PARSE_OP(
    p_create_function, zb::ref_wrapper<const object> name, int_t boundtarget, bool lambda) {
  using enum token_type;
  using enum object_type;
  using enum opcode;

  zs::closure_compile_state* fct_state = _ccs->push_child_state();
  fct_state->name = name;
  //  fct_state->source_name = _ccs->_tdata->source_name;

  ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("this")));

  int_t def_params = 0;

  // Parsing function parameters: `function (parameters)`.
  while (is_not(tok_rbracket)) {
    if (is(tok_triple_dots)) {
      // TODO: Named triple dots?

      if (def_params > 0) {
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
      const bool is_const = is(tok_const);
      bool has_type = false;
      token_type variable_type = _token;

      if (is_const) {
        lex();

        switch (_token) {
        case tok_var:
        case tok_array:
        case tok_table:
        case tok_string:
        case tok_char:
        case tok_int:
        case tok_bool:
        case tok_float:
          variable_type = _token;
          has_type = true;
          lex();
          break;

        case tok_identifier:
          break;
        default:
          return zs::error_code::invalid_token;
        }
      }

      else if (is(tok_var, tok_array, tok_table, tok_string, tok_char, tok_int, tok_bool, tok_float)) {
        variable_type = _token;
        has_type = true;
        lex();
      }

      uint32_t mask = 0;
      uint64_t custom_mask = 0;

      if (has_type) {
        switch (variable_type) {
        case tok_char:
        case tok_int:
          mask = zs::get_object_type_mask(k_integer);
          break;
        case tok_float:
          mask = zs::get_object_type_mask(k_float);
          break;
        case tok_string:
          mask = zs::object_base::k_string_mask;
          break;
        case tok_array:
          mask = zs::get_object_type_mask(k_array);
          break;
        case tok_table:
          mask = zs::get_object_type_mask(k_table);
          break;
        case tok_bool:
          mask = zs::get_object_type_mask(k_bool);
          break;

        case tok_exttype:
          mask = zs::get_object_type_mask(k_extension);
          break;

        case tok_null:
          mask = zs::get_object_type_mask(k_null);
          break;

        case tok_var:
          // Parsing a typed var (var<type1, type2, ...>).
          if (is(tok_lt)) {
            if (auto err = parse<p_variable_type_restriction>(REF(mask), REF(custom_mask))) {
              return ZS_COMPILER_ERROR(err, "parsing variable type restriction `var<....>`");
            }
          }
          break;
        }
      }

      zs::object param_name;
      ZS_COMPILER_EXPECT_GET(tok_identifier, param_name);
      ZS_RETURN_IF_ERROR(fct_state->add_parameter(param_name, mask, custom_mask, is_const));

      if (is(tok_eq)) {
        lex();
        ZS_RETURN_IF_ERROR(parse<p_expression>());
        fct_state->add_default_param(_ccs->top_target());
        def_params++;
      }

      // If a default parameter was defined, all of them (from that point) needs
      // to have one too.
      else if (def_params > 0) {
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
  //  //

  ZS_COMPILER_EXPECT(tok_rbracket);

  if (boundtarget != 0xFF) {
    _ccs->pop_target();
    //      _fs->pop_target();
  }
  //
  for (int_t n = 0; n < def_params; n++) {
    _ccs->pop_target();
  }
  //
  zs::closure_compile_state* curr_chunk = std::exchange(_ccs, fct_state);
  //  _ccs = fct_state;

  if (is_not(tok_lcrlbracket) and lambda) {
    ZS_RETURN_IF_ERROR(parse<p_expression>());
  }
  else {
    ZS_RETURN_IF_ERROR(parse<p_statement>(false));
  }

  //  if (lambda) {
  //    ZS_RETURN_IF_ERROR(parse<p_expression>());
  //    //      _fs->AddInstruction(_OP_RETURN, 1, _fs->PopTarget());
  //  }
  //  else {
  //    ZS_RETURN_IF_ERROR(parse<p_statement>(false));
  //  }

  //  //
  //  //  fct_state->AddLineInfos(
  //  //      _lex._prevtoken == _SC('\n') ? _lex._lasttokenline :
  //  _lex._currentline, _lineinfo, true);
  //      fct_state->AddInstruction(_OP_RETURN, -1);
  //      fct_state->SetStackSize(0);
  //  //
  //      SQFunctionProto* func = funcstate->BuildProto();
  //  // #ifdef _DEBUG_DUMP
  //  //  funcstate->Dump(func);
  //  // #endif
  //  _fs = curr_chunk;

  //  _fs->pop_child_state();

  //  fct_state->set_stack_size(stack_size);
  //  add_instruction<op_return>(0, false);
  fct_state->set_stack_size(0);

  zs::function_prototype_object* fpo = fct_state->build_function_prototype();

  object output;
  ::memset(&output, 0, sizeof(object));
  output._type = object_type::k_function_prototype;
  output._fproto = fpo;

  _ccs = curr_chunk;
  _ccs->_functions.push_back(output);
  _ccs->pop_child_state();

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

  add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);

  _estate.type = expr_type::e_object;
  _estate.pos = table_idx;

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_global_function_statement) {
  using enum token_type;
  using enum opcode;

  lex();

  add_new_target_instruction<op_load_global>();

  zs::object var_name;
  ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  ZS_RETURN_IF_ERROR(add_string_instruction(var_name));

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

  add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);

  _estate.type = expr_type::e_object;
  _estate.pos = table_idx;

  //    if (is_global) {
  //      lex();
  //      ZS_COMPILER_EXPECT(tok_dot);
  //      ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  //
  //      add_instruction<op_load_global>(_ccs->new_target());
  //      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));
  //
  //      if(is(tok_dot)) {
  //        int_t key_idx = _ccs->pop_target();
  //        int_t table_idx = _ccs->pop_target();
  //        add_instruction<op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);
  //      }
  //
  //      while (is(tok_dot)) {
  //        lex();
  //        ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  //        ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));
  //
  //        if(is(tok_dot)) {
  //          int_t key_idx = _ccs->pop_target();
  //          int_t table_idx = _ccs->pop_target();
  //          add_instruction<op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);
  //        }
  //      }
  //
  //    }
  //    else {
  //      ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  //    }
  //
  //    int_t bound_target = 0xFF;
  //
  //    if (is(tok_lsqrbracket)) {
  //      ZS_RETURN_IF_ERROR(parse<p_bind_env>(std::ref(bound_target)));
  //    }
  //
  //    ZS_COMPILER_EXPECT(tok_lbracket);
  //
  //    ZS_RETURN_IF_ERROR(parse<p_create_function>(std::cref(var_name), bound_target, false));
  //
  //    add_instruction<op_new_closure>(
  //        (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);
  //
  //    if (is_global) {
  //      int_t value_idx = _ccs->pop_target();
  //      int_t key_idx = _ccs->pop_target();
  //      int_t table_idx = _ccs->top_target();
  //
  //      add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
  //
  //      _estate.type = expr_type::e_object;
  //      _estate.pos = table_idx;
  //    }
  //    else {
  //      _ccs->pop_target();
  //      ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
  //    }

  return {};
}
} // namespace zs.
