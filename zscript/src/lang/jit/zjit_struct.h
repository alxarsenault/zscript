
//
// MARK: - Struct.
//

namespace zs {

ZS_JIT_COMPILER_PARSE_OP(p_struct_statement) {
  using enum token_type;
  using enum object_type;
  using enum opcode;
  using ps = parse_op;

  lex();
  expr_state es = _estate;
  _estate.no_get = true;

  // Check if the class is declared as `class var_name {` or `class something.var_name`.
  const bool is_local = is(tok_identifier) and _lexer->peek() == tok_lcrlbracket;

  if (is_local) {
    zs::object var_name;
    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
    ZS_RETURN_IF_ERROR(parse<ps::p_struct>());
    _ccs->pop_target();
    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
  }
  else {
    ZS_RETURN_IF_ERROR(parse<ps::p_prefixed>());

    switch (_estate.type) {
    case expr_type::e_expr:
      return handle_error(
          zs::error_code::invalid_operation, "Invalid class name", ZB_CURRENT_SOURCE_LOCATION());

    case expr_type::e_base:
      ZBASE_NO_BREAK;
    case expr_type::e_object: {
      ZS_RETURN_IF_ERROR(parse<ps::p_struct>());

      int_t val = _ccs->pop_target();
      int_t key = _ccs->pop_target();
      int_t table = _ccs->pop_target();

      add_instruction<op_new_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val);

      break;
    }

    case expr_type::e_local:
      ZBASE_NO_BREAK;
    case expr_type::e_capture:

      return handle_error(zs::error_code::invalid_operation,
          "Cannot create a class in a local with the syntax(class <local>)", ZB_CURRENT_SOURCE_LOCATION());
    }
  }

  _estate = es;
  return {};
}

struct jit_compiler::struct_parser {
  inline struct_parser(zs::engine* eng)
      : names((zs::allocator<zs::object>(eng))) {}

  inline zs::error_result add_member(const zs::object& name) {
    if (names.contains(name)) {
      return zs::error_code::already_exists;
    }

    names.push_back(name);
    return {};
  }

  zs::small_vector<zs::object, 8> names;
};

ZS_JIT_COMPILER_PARSE_OP(p_struct) {
  using enum token_type;
  using enum object_type;
  using enum opcode;
  using ps = parse_op;

  ZS_RETURN_IF_ERROR(expect(tok_lcrlbracket));
  add_instruction<op_new_obj>(_ccs->new_target(), k_struct);

  struct_parser sparser(_engine);

  [[maybe_unused]] int_t nitems = 0;

  while (!is(tok_rcrlbracket)) {
    ZS_RETURN_IF_ERROR(parse<ps::p_struct_content>(&sparser));
    nitems++;
  }

  lex();

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_struct_member_type, uint32_t* obj_type_mask, bool* is_static, bool* is_const) {
  using enum token_type;
  using enum opcode;
  using enum object_type;

  if (is(tok_static)) {
    *is_static = true;
    lex();
  }

  if (is(tok_const)) {
    *is_const = true;
    lex();
  }

  if (!is_var_decl_tok_no_const()) {
    if (is(tok_identifier, tok_constructor)) {
      return {};
    }

    return zs::error_code::invalid_token;
  }

  switch (_token) {
  case tok_char:
    ZBASE_NO_BREAK;
  case tok_int:
    *obj_type_mask |= zs::get_object_type_mask(k_integer);
    lex();
    break;

  case tok_float:
    *obj_type_mask |= zs::get_object_type_mask(k_float);
    lex();
    break;

  case tok_bool:
    *obj_type_mask |= zs::get_object_type_mask(k_bool);
    lex();
    break;

  case tok_array:
    *obj_type_mask |= zs::get_object_type_mask(k_array);
    lex();
    break;

  case tok_table:
    *obj_type_mask |= zs::get_object_type_mask(k_table);
    lex();
    break;

  case tok_string:
    *obj_type_mask |= zs::object_base::k_string_mask;
    lex();
    break;

  case tok_exttype:
    *obj_type_mask |= zs::get_object_type_mask(k_extension);
    lex();
    break;

  case tok_null:
    *obj_type_mask |= zs::get_object_type_mask(k_null);
    lex();
    break;

  case tok_var:
    lex();

    // Parsing a typed var (var<type1, type2, ...>).
    if (is(tok_lt)) {
      return ZS_COMPILER_HANDLE_ERROR_STREAM(
          zs::error_code::unimplemented, "struct typed variable var<...> is unimplemented.\n");
    }
    break;

  default:
    return zs::error_code::invalid_token;
  }

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_struct_content, struct_parser* sparser) {
  using enum token_type;
  using enum object_type;
  using enum opcode;
  using ps = parse_op;

  uint32_t obj_type_mask = 0;

  bool is_static = false;
  bool is_const = false;
  ZS_RETURN_IF_ERROR(parse<ps::p_struct_member_type>(&obj_type_mask, &is_static, &is_const));

  const bool is_constructor = is(tok_constructor);

  if (is_constructor) {
    lex();

    zs::object var_name = zs::_ss("constructor");

    ZS_COMPILER_RETURN_IF_ERROR_STREAM(
        sparser->add_member(var_name), "struct constructor() already exists.\n");

    ZS_RETURN_IF_ERROR(expect(tok_lbracket));

    int_t bound_target = 0xFF;
    ZS_RETURN_IF_ERROR(parse<ps::p_create_function>(CREF(var_name), bound_target, false));

    add_instruction<op_new_closure>(
        (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

    int_t val = _ccs->pop_target();
    int_t table = _ccs->top_target();
    add_instruction<op_new_struct_constructor>((uint8_t)table, (uint8_t)val);

    if (is(tok_semi_colon)) {
      lex();
    }
  }
  else {
    zs::object identifier;
    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, identifier));

    ZS_COMPILER_RETURN_IF_ERROR_STREAM(
        sparser->add_member(identifier), "struct member variable", identifier, "already exists.\n");

    ZS_RETURN_IF_ERROR(add_string_instruction(identifier));

    int_t val = -1;

    // Default value?
    if (is(tok_eq)) {
      lex();
      ZS_RETURN_IF_ERROR(parse<ps::p_expression>());
      val = _ccs->pop_target();
    }
    else if (is_static) {
      return ZS_COMPILER_HANDLE_ERROR_STREAM(
          zs::error_code::invalid, "struct static variable requires a default value.\n");
    }

    int_t key = _ccs->pop_target();
    int_t table = _ccs->top_target();
    add_instruction<op_new_struct_slot>(
        (uint8_t)table, (uint8_t)key, (uint8_t)val, obj_type_mask, is_static, val != -1, is_const);

    ZS_RETURN_IF_ERROR(expect(tok_semi_colon));
  }

  return {};
}

} // namespace zs.
