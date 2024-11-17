
ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

///
#define ZS_COMPILER_EXPECT(...) \
  ZBASE_DEFER(ZBASE_CONCAT(__ZS_COMPILER_EXPECT_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)

#define ZS_COMPILER_EXPECT_WITH_MESSAGE(tok, ...)                                                            \
  if (!lex_if(tok)) {                                                                                        \
    return handle_error(zs::error_code::invalid_token,                                                       \
        zs::strprint(_engine, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)), ", expected ", \
            zb::quoted<"'">(zs::token_to_string(tok)), "\n", __VA_ARGS__),                                   \
        ZB_CURRENT_SOURCE_LOCATION());                                                                       \
  }

#define ZS_COMPILER_ERROR(err, ...) \
  ZBASE_DEFER(ZBASE_CONCAT(__ZS_COMPILER_ERROR_, ZBASE_NARG_BINARY(__VA_ARGS__)), err, __VA_ARGS__)

#define ZS_COMPILER_RETURN_IF_ERROR(X, ...)     \
  if (zs::error_result err = X) {               \
    return ZS_COMPILER_ERROR(err, __VA_ARGS__); \
  }

#define ZS_COMPILER_EXPECT_GET(tok, ret)                                      \
  if (is_not(tok)) {                                                          \
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token ", \
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",          \
        zb::quoted<"'">(zs::token_to_string(tok)));                           \
  }                                                                           \
  else {                                                                      \
    ret = _lexer->get_value();                                                \
    lex();                                                                    \
  }

#define __ZS_COMPILER_ERROR_1(err, msg) handle_error(err, msg, ZB_CURRENT_SOURCE_LOCATION())

#define __ZS_COMPILER_ERROR_MULTIPLE(err, ...) \
  handle_error(err, zs::sstrprint(_engine, __VA_ARGS__), ZB_CURRENT_SOURCE_LOCATION())

#define ZS_COMPILER_PARSE(exprname, ...) ZS_RETURN_IF_ERROR(parse<exprname>(__VA_ARGS__))

#define ZS_COMPILER_EXPECT_ERROR_MESSAGE_WITH_CODE(tok, ec)                                            \
  ZS_COMPILER_ERROR(ec, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)), ", expected ", \
      zb::quoted<"'">(zs::token_to_string(tok)))

#define ZS_COMPILER_EXPECT_ERROR_MESSAGE(tok) \
  ZS_COMPILER_EXPECT_ERROR_MESSAGE_WITH_CODE(tok, zs::error_code::invalid_token)

#define __ZS_COMPILER_EXPECT_1(tok) __ZS_COMPILER_EXPECT_2(tok, zs::error_code::invalid_token)

#define __ZS_COMPILER_EXPECT_2(tok, ec)                                                        \
  if (!lex_if(tok)) {                                                                          \
    return handle_error(ec,                                                                    \
        zs::sstrprint(_engine, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)), \
            ", expected ", zb::quoted<"'">(zs::token_to_string(tok))),                         \
        ZB_CURRENT_SOURCE_LOCATION());                                                         \
  }

namespace zs {

ZS_CK_INLINE static bool is_small_string_identifier(const object& identifier) noexcept {
  return identifier.get_string_unchecked().size() <= constants::k_small_string_max_size;
}

using objref_t = zb::ref_wrapper<object>;
using cobjref_t = zb::ref_wrapper<const object>;

#define REF(...) zb::wref(__VA_ARGS__)
#define CREF(...) zb::wcref(__VA_ARGS__)
enum class parse_op : uint8_t {

  //
  p_preprocessor,
  p_statement,
  p_expression,
  p_function_statement,
  p_global_function_statement,
  p_export_function_statement,
  p_function,
  p_function_call_args,
  p_function_call_args_template,
  p_create_function,
  p_comma,
  p_semi_colon,
  p_decl_var,
  p_decl_var_internal,
  p_decl_var_internal_2,

  p_include_or_import_statement,
  p_export,
  p_export_table,

  p_decl_enum,
  p_enum_table,
  p_variable_type_restriction,
  p_table,
  p_table_or_class,

  p_module,
  p_class_statement,
  p_class,

  p_struct,
  p_struct_content,
  p_struct_statement,
  p_struct_member_type,

  p_if,
  p_if_block,
  p_factor,
  p_for,
  p_for_auto,
  p_for_each,
  p_factor_identifier,
  p_factor_at,
  p_bind_env,
  p_define,

  p_as_table,
  p_load_json_file,
  p_as_string,
  p_as_value,

  p_macro,
  p_macro_call,

  // '.', '[', '++', '--', '('.
  p_prefixed,
  p_prefixed_incr,
  p_prefixed_lbracket,
  p_prefixed_lbracket_template,

  // 2^n
  p_exponential,

  // '*', '/', '%'
  p_mult,

  // '+', '-'.
  p_plus,

  // '<<', '>>'.
  p_shift,

  // '>', '<', '>=', '<='.
  p_compare,

  // '==', '!=', '===', '<-->', '<==>'.
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

  count
};

using enum parse_op;
using enum object_type;

#define ZS_JIT_COMPILER_PARSE_OP(name, ...) \
  template <>                               \
  zs::error_result jit_compiler::parse<zs::name>(__VA_ARGS__)

//
// MARK: Parse forward declare.
//

ZS_JIT_COMPILER_PARSE_OP(p_statement, bool close_frame);
ZS_JIT_COMPILER_PARSE_OP(p_include_or_import_statement, token_type tok);
ZS_JIT_COMPILER_PARSE_OP(p_table_or_class, token_type separator, token_type terminator);
ZS_JIT_COMPILER_PARSE_OP(p_table);
ZS_JIT_COMPILER_PARSE_OP(p_semi_colon);
ZS_JIT_COMPILER_PARSE_OP(p_prefixed);
ZS_JIT_COMPILER_PARSE_OP(p_macro, token_type);
ZS_JIT_COMPILER_PARSE_OP(p_macro_call);
ZS_JIT_COMPILER_PARSE_OP(p_decl_var_internal_2, bool is_export, bool is_const);
ZS_JIT_COMPILER_PARSE_OP(p_struct);
ZS_JIT_COMPILER_PARSE_OP(p_struct_member_type, uint32_t* obj_type_mask, bool* is_static, bool* is_const);
ZS_JIT_COMPILER_PARSE_OP(p_struct_statement);
ZS_JIT_COMPILER_PARSE_OP(p_struct_content, struct_parser* sparser);
ZS_JIT_COMPILER_PARSE_OP(p_class_statement);
ZS_JIT_COMPILER_PARSE_OP(p_variable_type_restriction, zb::ref_wrapper<uint32_t>, zb::ref_wrapper<uint64_t>);

ZS_JIT_COMPILER_PARSE_OP(p_expression);
ZS_JIT_COMPILER_PARSE_OP(p_bind_env, zb::ref_wrapper<int_t> target);
ZS_JIT_COMPILER_PARSE_OP(p_factor);
ZS_JIT_COMPILER_PARSE_OP(p_factor_identifier);
ZS_JIT_COMPILER_PARSE_OP(p_factor_at);
ZS_JIT_COMPILER_PARSE_OP(p_as_table);
ZS_JIT_COMPILER_PARSE_OP(p_as_string);
ZS_JIT_COMPILER_PARSE_OP(p_as_value);
ZS_JIT_COMPILER_PARSE_OP(p_load_json_file);
ZS_JIT_COMPILER_PARSE_OP(p_if_block);
ZS_JIT_COMPILER_PARSE_OP(p_if);

// Functions.
ZS_JIT_COMPILER_PARSE_OP(p_function_statement);
ZS_JIT_COMPILER_PARSE_OP(p_function, bool lambda);
ZS_JIT_COMPILER_PARSE_OP(p_function_call_args, bool rawcall);
ZS_JIT_COMPILER_PARSE_OP(p_function_call_args_template, std::string_view meta_code);
ZS_JIT_COMPILER_PARSE_OP(
    p_create_function, zb::ref_wrapper<const object> name, int_t boundtarget, bool lambda);

ZS_JIT_COMPILER_PARSE_OP(p_export_function_statement);
ZS_JIT_COMPILER_PARSE_OP(p_global_function_statement);
ZS_JIT_COMPILER_PARSE_OP(p_module);

ZS_JIT_COMPILER_PARSE_OP(p_export);
ZS_JIT_COMPILER_PARSE_OP(p_export_table);

//
// MARK: Compiler.
//

jit_compiler::jit_compiler(zs::engine* eng)
    : engine_holder(eng)
    , _error_message(zs::allocator<char>(eng))
    , _compile_time_consts(zs::object::create_table(eng))
    , _macros(zs::allocator<macro>(eng)) {
  _scope.n_captures = 0;
  _scope.stack_size = 0;
}

static std::string_view get_line_content(const zb::utf8_span_stream& stream, const zs::line_info& linfo) {

  const char* begin = &(*stream._data.begin());
  const char* end = &(*stream._data.end());

  const char* it_line_begin = stream.ptr() - 1;
  while (it_line_begin > begin) {
    if (*it_line_begin == '\n') {
      ++it_line_begin;
      break;
    }

    --it_line_begin;
  }

  const char* it_line_end = stream.ptr();
  while (it_line_end < end) {
    if (*it_line_end == '\n') {
      break;
    }

    ++it_line_end;
  }

  std::string_view line_content(it_line_begin, std::distance(it_line_begin, it_line_end));
  return line_content;
}

zs::error_result jit_compiler::handle_error(
    zs::error_code ec, std::string_view msg, const zb::source_location& loc) {
  zs::line_info linfo = _lexer->get_last_line_info();
  std::string_view line_content = get_line_content(_lexer->stream(), linfo);

  _error_message += zs::strprint(_engine, "\nError: ", linfo, "\n'''", line_content, "\n",
      zb::indent_t(linfo.column - 1), "^\n'''\n\nfrom function : '", loc.function_name(),
      "'\n     file     : '", loc.file_name(), "'\n     line     : ", loc.line(), "\n\nMessage:\n", msg);

  return ec;
}

zs::error_result jit_compiler::compile(std::string_view content, std::string_view filename, object& output,
    zs::virtual_machine* vm, zs::token_type* prepended_token, bool with_vargs, bool add_line_info) {
  using enum token_type;
  using enum opcode;

  _add_line_info = add_line_info;
  _vm = vm;
  zs::lexer lexer(_engine);

  _lexer = &lexer;

  //  _shared_state = &shared_state;
  _lexer->init(content);

  // Init expression state.
  //  _exp_state.etype = zs::expression_type::expr;
  //  _exp_state.epos = 0;
  //  _exp_state.do_not_get = true;

  jit::shared_state_data sdata(_engine);
  zs::closure_compile_state c_compile_state(_engine, sdata);
  c_compile_state.name = zs::object(_engine, "main");
  c_compile_state._sdata._source_name = zs::object(_engine, filename);

  _ccs = &c_compile_state;
  ZS_RETURN_IF_ERROR(_ccs->add_parameter(object::create_small_string("this")));

  if (with_vargs) {
    ZS_RETURN_IF_ERROR(_ccs->add_parameter(object::create_small_string("vargs")));
    add_new_target_instruction<op_load_null>();
    _ccs->add_default_param(_ccs->top_target());
    _ccs->pop_target();
  }

  int_t stack_size = _ccs->get_stack_size();

  if (prepended_token) {
    _token = *prepended_token;
  }
  else {
    lex();
  }

  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    ZS_COMPILER_PARSE(p_statement, true);

    if (!zb::is_one_of(_lexer->last_token(), tok_rcrlbracket, tok_semi_colon)) {
      ZS_COMPILER_PARSE(p_semi_colon);
    }
  }

  if (_token == tok_lex_error) {
    return zs::error_code::invalid;
  }

  if (_ccs->is_top_level() and _ccs->has_export()) {
    _ccs->push_export_target();
    add_instruction<op_return_export>(_ccs->pop_target());
  }

  _ccs->set_stack_size(stack_size);

  if (_add_line_info) {
    _ccs->add_line_infos(_lexer->get_line_info());
  }

  _ccs->set_stack_size(0);

  zs::function_prototype_object* fpo = _ccs->build_function_prototype();
  ::memset(&output, 0, sizeof(object));
  output._type = object_type::k_function_prototype;
  output._fproto = fpo;
  //  object_proxy::as_function_prototype(output) = fpo;

  //  output._value
  //  _ccs->set_stack_size(0);
  //  zs::instruction_vector& ivector = _ccs->_instructions;
  //  for(auto it = ivector.begin(); it != ivector.end(); ++it) {
  //    zb::print(it.get_opcode());
  //  }

  return {};
}

void jit_compiler::move_if_current_target_is_local() {
  if (_ccs->is_local(_ccs->top_target())) {
    // Pops the target and moves it.
    add_new_target_instruction<op_move>(_ccs->pop_target());
  }
}

zs::error_result jit_compiler::invoke_expr(zb::member_function_pointer<jit_compiler, zs::error_result> fct) {

  const expr_state es = _estate;
  _estate.type = expr_type::e_expr;
  _estate.pos = -1;
  _estate.no_get = false;
  _estate.no_assign = false;
  ZS_RETURN_IF_ERROR((this->*fct)());
  _estate = es;

  return {};
}

// class tmp_string_view {
// public:
//   inline tmp_string_view(zs::engine* eng)
//       : _buffer((zs::string_allocator(eng))) {}
//
//   inline tmp_string_view(zs::engine* eng, std::string_view s)
//       : _buffer((zs::string_allocator(eng)))
//       , _str(s) {}
//
//   zs::error_result assign(const object& obj) { return object_char_or_string_to_string(obj, _buffer, _str);
//   }
//
//   inline bool empty() const noexcept { return _str.empty(); }
//
//   friend inline std::ostream& operator<<(std::ostream& stream, const tmp_string_view& s) {
//     if (!s.empty()) {
//       return stream << s._str;
//     }
//     return stream;
//   }
//   zs::string _buffer;
//   std::string_view _str;
// };

zs::error_result jit_compiler::add_small_string_instruction(std::string_view s, int_t target_idx) {
  if (s.size() > zs::constants::k_small_string_max_size) {
    return ZS_COMPILER_ERROR(
        zs::error_code::invalid_token, "invalid string size in add_small_string_instruction.\n");
  }

  add_instruction<op_load_small_string>(target_idx, zs::small_string_instruction_data::create(s));
  return {};
}

zs::error_result jit_compiler::add_string_instruction(const object& sobj, int_t target_idx) {
  if (std::string_view s = sobj.get_string_unchecked(); s.size() <= zs::constants::k_small_string_max_size) {
    return add_small_string_instruction(s, target_idx);
  }

  add_instruction<op_load_string>(target_idx, (uint32_t)_ccs->get_literal(sobj));
  return {};
}

zs::error_result jit_compiler::add_string_instruction(std::string_view s, int_t target_idx) {
  if (s.size() > zs::constants::k_small_string_max_size) {
    add_instruction<op_load_string>(
        target_idx, (uint32_t)_ccs->get_literal(object::create_string(_engine, s)));
    return {};
  }

  return add_small_string_instruction(s, target_idx);
}

zs::error_result jit_compiler::add_export_string_instruction(const object& var_name) {

  if (auto err = _ccs->add_exported_name(var_name)) {
    return ZS_COMPILER_ERROR(err, "duplicated value keys in export statement.\n");
  }

  return add_string_instruction(var_name);
}

zs::error_result jit_compiler::add_to_export_table(const object& var_name) {

  int_t table_idx = _ccs->find_local_variable(zs::_ss("__exports__"));

  std::string_view s = var_name.get_string_unchecked();
  if (s.size() <= zs::constants::k_small_string_max_size) {
    int_t value_idx = _ccs->pop_target();

    struct uint64_t_pair {
      uint64_t value_1;
      uint64_t value_2;
    } spair = {};

    ::memcpy(&spair, s.data(), s.size());
    ZS_COMPILER_RETURN_IF_ERROR(
        _ccs->add_exported_name(var_name), "duplicated value keys in export statement.\n");

    add_instruction<op_rawsets>((uint8_t)table_idx, (uint8_t)value_idx, spair.value_1, spair.value_2);
  }
  else {

    ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));

    int_t key_idx = _ccs->pop_target();
    int_t value_idx = _ccs->pop_target();
    add_instruction<op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
  }

  return {};
}
//
// MARK: Parse.
//

template <>
zs::error_result jit_compiler::parse<p_prefixed_incr>(bool is_incr) {
  using enum token_type;

  lex();
  expr_state es = _estate;
  _estate.no_get = true;

  ZS_COMPILER_PARSE(p_prefixed);

  switch (_estate.type) {
  case expr_type::e_expr:
    return ZS_COMPILER_ERROR(zs::error_code::invalid_operation, "Can't '++' or '--' an expression");

  case expr_type::e_base:
    return ZS_COMPILER_ERROR(zs::error_code::invalid_operation, "Can't '++' or '--' a base");

  case expr_type::e_object: {
    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->pop_target();

    add_new_target_instruction<op_pobjincr>((uint8_t)table_idx, (uint8_t)key_idx, is_incr);

    break;
    ;
  }

  case expr_type::e_local: {
    int_t src = _ccs->top_target();
    add_instruction<op_pincr>((uint8_t)src, (uint8_t)src, is_incr);
    //      SQInteger src = _fs->TopTarget();
    //      _fs->AddInstruction(_OP_INCL, src, src, 0, diff);
    break;
  }

  case expr_type::e_capture:
    //      SQInteger tmp = _fs->PushTarget();
    //      _fs->AddInstruction(_OP_GETOUTER, tmp, _es.epos);
    //      _fs->AddInstruction(_OP_INCL, tmp, tmp, 0, diff);
    //      _fs->AddInstruction(_OP_SETOUTER, tmp, _es.epos, tmp);
    return zs::error_code::unimplemented;
  }

  _estate = es;
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_prefixed_lbracket>() {
  using enum token_type;
  using enum opcode;

  bool is_member_call = false;
  //  if (pos != _estate.pos and (pos == -1 or _estate.pos == -1)) {
  //    zb::print("klklk---", pos, _estate.pos, _token);
  //  }

  switch (_estate.type) {
  case expr_type::e_object: {

    // The table is the root table?
    if (_estate.pos == -1) {
      //      if (pos == -1) {
      // Nothing to do here other than pushing it on the stack.
      add_new_target_instruction<op_move>(0);
    }
    else {
      is_member_call = true;

      // We need to call a function from a table e.g. `table.fct();`.
      // The get wasn't done in the `case tok_dot:` above especially for
      // this.

      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->top_target();

      // Get the item at the given `key_idx`, from the table at `table_idx`.
      add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, true);

      // To prepare for a function call, we now have the closure on top of
      // the stack. Since we want this table as first arg, we need to push
      // it on the stack after the closure.
      _estate.type = expr_type::e_object;
      _estate.pos = _ccs->new_target();
      //      add_instruction<op_move>(pos, (uint8_t)table_idx);
      add_instruction<op_move>(_estate.pos, (uint8_t)table_idx);
    }
    break;
  }

  case expr_type::e_base:
    zb::print("MISSED EXPR");
    // Emit2ArgsOP(_OP_GET);
    //            _fs->AddInstruction(_OP_MOVE, _fs->PushTarget(), 0);
    break;

  case expr_type::e_capture: {
    //          zb::print("MISSED EXPR", _estate.pos);
    //            _fs->AddInstruction(_OP_GETOUTER, _ccs->new_target(),
    //            _es.epos); _fs->AddInstruction(_OP_MOVE,
    //            _ccs->new_target(), 0);

    if (_estate.pos == -1) {
      //      zb::print("DSLJDSKJDLKS capture");
      // Nothing to do here other than pushing it on the stack.
      //            add_instruction<op_move>(_ccs->new_target(),
      //            (uint8_t)0);
    }
    else {
      // TODO: Fix this.
      //      zb::print("DSLJDSKJDLKS CAPTURE");

      // Push the captured closure.
      add_new_target_instruction<op_get_capture>((uint32_t)_estate.pos);
      _estate.pos = _ccs->top_target();

      // TODO: Was should we put here???
      // Push the root table.
      add_new_target_instruction<op_move>(0);
    }

    break;
  }

  default:
    add_new_target_instruction<op_move>(0);
  }

  _estate.type = expr_type::e_expr;
  lex();

  ZS_COMPILER_PARSE(p_function_call_args, false);

  // When `is_member_call` is true, we are stuck with a table below the
  // result that we need to remove. TRo solve this, we pop them both, and
  // push the result back on top.
  if (is_member_call) {
    // Pop result.
    int_t result_idx = _ccs->pop_target();

    // Pop table.
    _ccs->pop_target();

    // Move the result back on top.
    add_new_target_instruction<op_move>(result_idx);
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_prefixed_lbracket_template>() {
  using enum token_type;
  using enum opcode;

  ZS_ASSERT(is(tok_lt));

  const char* meta_code_begin = _lexer->_stream.ptr();
  ZS_RETURN_IF_ERROR(_lexer->lex_to(tok_gt));
  std::string_view meta_code(meta_code_begin, std::distance(meta_code_begin, _lexer->_stream.ptr()));

  lex();

  bool is_member_call = false;

  switch (_estate.type) {
  case expr_type::e_object: {

    // The table is the root table?
    if (_estate.pos == -1) {
      // Nothing to do here other than pushing it on the stack.
      add_new_target_instruction<op_move>(0);
    }
    else {
      is_member_call = true;

      // We need to call a function from a table e.g. `table.fct();`.
      // The get wasn't done in the `case tok_dot:` above especially for
      // this.
      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->top_target();

      // Get the item at the given `key_idx`, from the table at `table_idx`.
      add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, false);

      // To prepare for a function call, we now have the closure on top of
      // the stack. Since we want this table as first arg, we need to push
      // it on the stack after the closure.
      _estate.type = expr_type::e_object;
      _estate.pos = _ccs->new_target();
      add_instruction<op_move>(_estate.pos, (uint8_t)table_idx);
    }
    break;
  }

  case expr_type::e_base:
    zb::print("MISSED EXPR");
    // Emit2ArgsOP(_OP_GET);
    //            _fs->AddInstruction(_OP_MOVE, _fs->PushTarget(), 0);
    break;

  case expr_type::e_capture: {
    //          zb::print("MISSED EXPR", _estate.pos);
    //            _fs->AddInstruction(_OP_GETOUTER, _ccs->new_target(),
    //            _es.epos); _fs->AddInstruction(_OP_MOVE,
    //            _ccs->new_target(), 0);

    if (_estate.pos == -1) {

      // Nothing to do here other than pushing it on the stack.
      //            add_instruction<op_move>(_ccs->new_target(),
      //            (uint8_t)0);
    }
    else {
      // TODO: Fix this.

      // Push the captured closure.
      add_new_target_instruction<op_get_capture>((uint32_t)_estate.pos);
      _estate.pos = _ccs->top_target();

      // TODO: Was should we put here???
      // Push the root table.
      add_new_target_instruction<op_move>(0);
    }

    break;
  }

  default:
    add_new_target_instruction<op_move>(0);
  }

  _estate.type = expr_type::e_expr;
  lex();

  ZS_COMPILER_PARSE(p_function_call_args_template, meta_code);

  // When `is_member_call` is true, we are stuck with a table below the
  // result that we need to remove. TRo solve this, we pop them both, and
  // push the result back on top.
  if (is_member_call) {
    // Pop result.
    int_t result_idx = _ccs->pop_target();

    // Pop table.
    _ccs->pop_target();

    // Move the result back on top.
    add_new_target_instruction<op_move>(result_idx);
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_exponential>() {
  using enum token_type;

  ZS_COMPILER_PARSE(p_prefixed);

  zb_loop() {
    switch (_token) {
    case tok_exp: {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_exp>(&jit_compiler::parse<p_exponential>, "^"));
      break;
    }

    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_mult>() {
  using enum token_type;

  ZS_COMPILER_PARSE(p_exponential);

  zb_loop() {
    switch (_token) {

    case tok_mul: {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_mul>(&jit_compiler::parse<p_exponential>, "*"));
    } break;

    case tok_div: {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_div>(&jit_compiler::parse<p_exponential>, "/"));
    } break;

    case tok_mod: {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_mod>(&jit_compiler::parse<p_exponential>, "%"));
    } break;

    default:
      return {};
    }
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_plus>() {
  static constexpr parse_op next_op = p_mult;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    switch (_token) {
    case tok_add: {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_add>(&jit_compiler::parse<next_op>, "+"));
      break;
    }

    case tok_minus: {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_sub>(&jit_compiler::parse<next_op>, "-"));
      break;
    }

    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_shift>() {
  using enum token_type;
  static constexpr parse_op next_op = p_plus;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    switch (_token) {
    case tok_lshift:
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_lshift>(&jit_compiler::parse<next_op>, "<<"));
      break;
    case tok_rshift:
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_rshift>(&jit_compiler::parse<next_op>, ">>"));
      break;
      break;
    default:
      return {};
    }
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_compare>() {
  static constexpr parse_op next_op = p_shift;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    switch (_token) {
    case tok_gt: {
      lex();

      ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_new_target_instruction<op_cmp>(compare_op::gt, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_lt: {
      lex();

      ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_new_target_instruction<op_cmp>(compare_op::lt, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_gt_eq: {
      lex();

      ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_new_target_instruction<op_cmp>(compare_op::ge, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_lt_eq: {
      lex();

      ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_new_target_instruction<op_cmp>(compare_op::le, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_in:
      //      helper::binary_exp(this, op_cmp,
      //      &compiler::parse<p_shift>);
      // binary_exp(_OP_EXISTS, &SQCompiler::ShiftExp);
      break;

    // case TK_INSTANCEOF:
    //   binary_exp(_OP_INSTANCEOF, &SQCompiler::ShiftExp);
    //   break;
    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_eq_compare>() {
  using enum token_type;
  static constexpr parse_op next_op = p_compare;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    switch (_token) {

    case tok_eq_eq: {
      lex();

      ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_new_target_instruction<op_eq>((uint8_t)op1, (uint8_t)op2, false);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_not_eq: {
      lex();

      ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_new_target_instruction<op_ne>((uint8_t)op1, (uint8_t)op2, false);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_three_way_compare: {
      lex();

      ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_new_target_instruction<op_cmp>(compare_op::tw, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_double_arrow: {
      lex();

      ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_new_target_instruction<op_cmp>(compare_op::double_arrow, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_double_arrow_eq: {
      lex();

      ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_new_target_instruction<op_cmp>(compare_op::double_arrow_eq, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    default:
      return {};
    }
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_bitwise_and>() {
  using enum token_type;

  ZS_COMPILER_PARSE(p_eq_compare);

  zb_loop() {
    if (is(tok_bitwise_and)) {

      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_bitwise_and>(&jit_compiler::parse<p_eq_compare>, "&"));
    }

    //      helper::binary_exp(this, op_bitw,
    //      &compiler::parse<p_bitwise_and>);

    //        binary_exp(_OP_BITW, &SQCompiler::BitwiseAndExp, BW_OR);

    else {
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_bitwise_xor>() {
  using enum token_type;

  ZS_COMPILER_PARSE(p_bitwise_and);

  zb_loop() {
    if (is(tok_xor)) {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_bitwise_xor>(&jit_compiler::parse<p_bitwise_and>, "xor"));
    }
    else {
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_bitwise_or>() {
  using enum token_type;

  ZS_COMPILER_PARSE(p_bitwise_xor);

  zb_loop() {
    if (is(tok_bitwise_or)) {

      ZS_RETURN_IF_ERROR(do_arithmetic_expr<op_bitwise_or>(&jit_compiler::parse<p_bitwise_xor>, "|"));
    }

    //      helper::binary_exp(this, op_bitw,
    //      &compiler::parse<p_bitwise_and>);

    //        binary_exp(_OP_BITW, &SQCompiler::BitwiseAndExp, BW_OR);

    else {
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_and>() {
  using enum token_type;

  ZS_COMPILER_PARSE(p_bitwise_or);

  zb_loop() {
    switch (_token) {
    case tok_and: {
      //        SQInteger first_exp = _fs->PopTarget();
      //        SQInteger trg = _fs->PushTarget();
      //        _fs->AddInstruction(_OP_AND, trg, 0, first_exp, 0);
      //        SQInteger jpos = _fs->GetCurrentPos();
      //        if (trg != first_exp) {
      //          _fs->AddInstruction(_OP_MOVE, trg, first_exp);
      //        }
      lex();
      ZS_COMPILER_PARSE(p_and);
      //        invoke_exp(&SQCompiler::LogicalAndExp);
      //        _fs->SnoozeOpt();
      //        SQInteger second_exp = _fs->PopTarget();
      //        if (trg != second_exp) {
      //          _fs->AddInstruction(_OP_MOVE, trg, second_exp);
      //        }
      //        _fs->SnoozeOpt();
      //        _fs->SetInstructionParam(jpos, 1, (_fs->GetCurrentPos() -
      //        jpos)); _es.etype = EXPR;
      break;
    }

    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_or>() {
  using enum token_type;

  ZS_COMPILER_PARSE(p_and);

  zb_loop() {
    if (is(tok_or)) {

      lex();
      ZS_COMPILER_PARSE(p_or);
    }
    break;
  }
  //    for (;;)
  //      if (_token == TK_OR) {
  //        SQInteger first_exp = _fs->PopTarget();
  //        SQInteger trg = _fs->PushTarget();
  //        _fs->AddInstruction(_OP_OR, trg, 0, first_exp, 0);
  //        SQInteger jpos = _fs->GetCurrentPos();
  //        if (trg != first_exp) {
  //          _fs->AddInstruction(_OP_MOVE, trg, first_exp);
  //        }
  //        Lex();
  //        invoke_exp(&SQCompiler::LogicalOrExp);
  //        _fs->SnoozeOpt();
  //        SQInteger second_exp = _fs->PopTarget();
  //        if (trg != second_exp) {
  //          _fs->AddInstruction(_OP_MOVE, trg, second_exp);
  //        }
  //        _fs->SnoozeOpt();
  //        _fs->SetInstructionParam(jpos, 1, (_fs->GetCurrentPos() - jpos));
  //        _es.etype = EXPR;
  //        break;
  //      }
  //      else
  //        return;

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_expression>() {
  using enum token_type;
  using enum opcode;
  using enum object_type;

  expr_state es = _estate;
  _estate.type = expr_type::e_expr;
  _estate.pos = -1;
  _estate.no_get = false;

  zb::scoped expr_state_cache = [&]() { _estate = es; };

  ZS_COMPILER_PARSE(p_or);

  if (is(tok_question_mark)) {
    ZS_TODO("Implement");
    //    _estate = es;
    return zs::error_code::unimplemented;
  }

  // TODO: Add question_mark?
  if (is_not(tok_eq, tok_minus_eq, tok_add_eq, tok_mul_eq, tok_div_eq, tok_exp_eq, tok_mod_eq, tok_lshift_eq,
          tok_rshift_eq, tok_bitwise_or_eq, tok_bitwise_and_eq, tok_double_arrow_eq)) {
    //    _estate = es;
    return {};
  }

  zs::token_type op = _token;
  expr_type es_type = _estate.type;

  switch (es_type) {
  case expr_type::e_expr:
    return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "Can't assign an expression");

  case expr_type::e_base:
    return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "'base' cannot be modified");
  }

  lex();

  // Is setting function?
  //

  ZS_COMPILER_PARSE(p_expression);

  switch (op) {
    // Assign.
  case tok_eq: {
    //    zb::print(tok_eq, es_type, ZB_CURRENT_SOURCE_LOCATION());

    ////////////////////////////////////////////////////////////////////
    switch (es_type) {
    case expr_type::e_local: {
      int_t src = _ccs->pop_target();
      int_t dst = _ccs->top_target();

      variable_type_info_t type_info = _ccs->top_target_type_info();
      uint32_t mask = type_info.mask;
      uint64_t custom_mask = type_info.custom_mask;
      //      const variable_mask_t mask = type_info.get_vmask();
      //      variable_mask_t mask = _ccs->top_target_mask();
      //      _estate.type = expr_type::e_local;
      //      _estate.pos = pos;

      if (type_info.is_const) {
        return ZS_COMPILER_ERROR(
            zs::error_code::invalid_type_assignment, "trying to assign to a const value");
      }

      bool skip_mask = false;

      if (mask) {
        zs::opcode last_op
            = zs::instruction_iterator(&_ccs->_instructions._data[_ccs->get_instruction_index()])
                  .get_opcode();

        switch (last_op) {
        case op_load_int:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_integer)))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask", k_integer,
                "expected", zs::object_type_mask_printer{ mask });
          }
          break;

        case op_load_float:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_float)))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask", k_float,
                "expected", zs::object_type_mask_printer{ mask });
          }
          break;

        case op_load_bool:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_bool)))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask", k_bool,
                "expected", zs::object_type_mask_printer{ mask });
          }
          break;

        case op_load_small_string:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_small_string)))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask",
                k_small_string, "expected", zs::object_type_mask_printer{ mask });
          }
          break;

        case op_load_string:
          if (!(skip_mask = (mask & zs::object_base::k_string_mask))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask",
                k_long_string, "expected", zs::object_type_mask_printer{ mask });
          }
          break;
        }
      }

      //
      //             if (!skip_mask) {
      //               if (custom_mask) {
      //                 add_instruction<op_check_custom_type_mask>((uint8_t)_ccs->top_target(),
      //                 mask, custom_mask);
      //               }
      //               else if (mask) {
      //                 add_instruction<op_check_type_mask>((uint8_t)_ccs->top_target(),
      //                 mask);
      //               }
      //             }

      add_instruction<op_move>((uint8_t)dst, (uint8_t)src);

      if (!skip_mask) {
        if (custom_mask) {
          add_instruction<op_check_custom_type_mask>((uint8_t)dst, mask, custom_mask);
        }
        else if (mask) {
          add_instruction<op_check_type_mask>((uint8_t)dst, mask);
        }
      }
      break;
    }

    case expr_type::e_object: {
      if (_ccs->_target_stack.size() < 3) {
        return ZS_COMPILER_ERROR(
            zs::error_code::invalid_operation, "wrong type mask", k_long_string, "expected");
      }

      int_t value_idx = _ccs->pop_target();
      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->top_target();

      if (table_idx == 0 and _ccs->is_top_level()) {
        add_instruction<op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
      }
      else {
        add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
      }

      _estate.type = expr_type::e_object;
      _estate.pos = table_idx;
      break;
    }

    case expr_type::e_capture:
      return ZS_COMPILER_ERROR(zs::error_code::invalid_operation, "Can't assign a value to a capture.\n");

    default:

      return ZS_COMPILER_ERROR(zs::error_code::invalid_operation, "invalid invalid_operation");
      //      return zs::error_code::unimplemented;
    }
  } break;

  case tok_add_eq: {
    expr_type es_type = _estate.type;

    switch (es_type) {
    case expr_type::e_local: {
      int_t src = _ccs->pop_target();
      int_t target = _ccs->top_target();
      add_instruction<op_add_eq>((uint8_t)target, (uint8_t)src);
      break;
    }

    default: {
      return zs::error_code::unimplemented;
    }
    }
    break;
  }

  case tok_mul_eq: {
    expr_type es_type = _estate.type;

    switch (es_type) {
    case expr_type::e_local: {
      int_t src = _ccs->pop_target();
      int_t target = _ccs->top_target();
      add_instruction<op_mul_eq>((uint8_t)target, (uint8_t)src);
      break;
    }

    default:
      return zs::error_code::unimplemented;
    }
    break;
  }
  case tok_minus_eq:
  case tok_div_eq:
  case tok_exp_eq:
  case tok_mod_eq:
  case tok_lshift_eq:
  case tok_rshift_eq:
  case tok_bitwise_or_eq:
  case tok_bitwise_and_eq:
  case tok_double_arrow_eq:
    return zs::error_code::unimplemented;

  default:
    break;
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_comma>() {
  using enum token_type;

  ZS_COMPILER_PARSE(p_expression);

  while (_token == tok_comma) {
    lex();
    ZS_COMPILER_PARSE(p_comma);
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_semi_colon>() {
  using enum token_type;

  if (is(tok_semi_colon)) {
    lex();
    return {};
  }

  if (!is_end_of_statement()) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token");
  }

  return {};
}
//
// template <>
// zs::error_result jit_compiler::parse<p_decl_var_internal_2>(bool is_export, bool
// is_const) {
//  using enum token_type;
//  using enum opcode;
//  using enum object_type;
//
//  uint32_t mask = 0;
//  uint64_t custom_mask = 0;
//
//  if(is_not(tok_identifier)) {
//    token_type last_token = _token;
//    lex();
//
//    switch (last_token) {
//    case tok_char:
//    case tok_int:
//      mask = zs::get_object_type_mask(k_integer);
//      break;
//    case tok_float:
//      mask = zs::get_object_type_mask(k_float);
//      break;
//    case tok_string:
//      mask = zs::object_base::k_string_mask;
//      break;
//    case tok_array:
//      mask = zs::get_object_type_mask(k_array);
//      break;
//    case tok_table:
//      mask = zs::get_object_type_mask(k_table);
//      break;
//    case tok_bool:
//      mask = zs::get_object_type_mask(k_bool);
//      break;
//    case tok_exttype:
//      mask = zs::get_object_type_mask(k_extension);
//      break;
//
//    case tok_null:
//      mask = zs::get_object_type_mask(k_null);
//      break;
//
//    case tok_var:
//      // Parsing a typed var (var<type1, type2, ...>).
//      if (is(tok_lt)) {
//        if (auto err = parse<p_variable_type_restriction>(std::ref(mask), std::ref(custom_mask)))
//        {
//          return helper::handle_error(
//              this, err, "parsing variable type restriction `var<....>`", ZB_CURRENT_SOURCE_LOCATION());
//        }
//      }
//      break;
//    }
//  }
//
//  zb_loop() {
//    if (is_not(tok_identifier)) {
//      return helper::handle_error(
//          this, zs::error_code::identifier_expected, "expected identifier", ZB_CURRENT_SOURCE_LOCATION());
//    }
//
//    object var_name(_engine, _lexer->get_identifier_value());
//
//    lex();
//
//    // @code `var name = ...;`
//    if (is(tok_eq)) {
//      lex();
//      ZS_COMPILER_PARSE(p_expression);
//
//      int_t src = _ccs->pop_target();
//      int_t dest = _ccs->new_target(mask, custom_mask, is_const);
//
//      zs::opcode last_op
//          =
//          zs::instruction_iterator(&_ccs->_instructions._data[_ccs->get_instruction_index()]).get_opcode();
//
//      bool skip_mask = false;
//      if (mask) {
//        switch (last_op) {
//        case op_load_int:
//          if (!(skip_mask = (mask & zs::get_object_type_mask(k_integer)))) {
//            return handle_error(zs::error_code::invalid_value_type_assignment,
//                zs::strprint(
//                    _engine, "wrong type mask", k_integer, "expected", zs::object_type_mask_printer{ mask
//                    }),
//                ZB_CURRENT_SOURCE_LOCATION());
//          }
//          break;
//
//        case op_load_float:
//          if (!(skip_mask = (mask & zs::get_object_type_mask(k_float)))) {
//            return handle_error(zs::error_code::invalid_value_type_assignment,
//                zs::strprint(
//                    _engine, "wrong type mask", k_float, "expected", zs::object_type_mask_printer{ mask }),
//                ZB_CURRENT_SOURCE_LOCATION());
//          }
//          break;
//
//        case op_load_bool:
//          if (!(skip_mask = (mask & zs::get_object_type_mask(k_bool)))) {
//            return handle_error(zs::error_code::invalid_value_type_assignment,
//                zs::strprint(
//                    _engine, "wrong type mask", k_bool, "expected", zs::object_type_mask_printer{ mask }),
//                ZB_CURRENT_SOURCE_LOCATION());
//          }
//          break;
//
//        case op_load_small_string:
//          if (!(skip_mask = (mask & zs::get_object_type_mask(k_small_string)))) {
//            return handle_error(zs::error_code::invalid_value_type_assignment,
//                zs::strprint(_engine, "wrong type mask", k_small_string, "expected",
//                    zs::object_type_mask_printer{ mask }),
//                ZB_CURRENT_SOURCE_LOCATION());
//          }
//          break;
//
//        case op_load_string:
//          if (!(skip_mask = (mask & zs::object_base::k_string_mask))) {
//            return handle_error(zs::error_code::invalid_value_type_assignment,
//                zs::strprint(_engine, "wrong type mask", k_long_string, "expected",
//                    zs::object_type_mask_printer{ mask }),
//                ZB_CURRENT_SOURCE_LOCATION());
//          }
//          break;
//        }
//      }
//
//      if (dest != src) {
//        //                          if (_fs->IsLocal(src)) {
//        //                              _fs->SnoozeOpt();
//        //                          }
//        add_instruction<op_move>((uint8_t)dest, (uint8_t)src);
//      }
//
//      if (!skip_mask) {
//        if (custom_mask) {
//          add_instruction<op_check_custom_type_mask>((uint8_t)_ccs->top_target(), mask,
//          custom_mask);
//        }
//        else if (mask) {
//          add_instruction<op_check_type_mask>((uint8_t)_ccs->top_target(), mask);
//        }
//      }
//    }
//
//    // @code `var name;`
//    else {
//      add_instruction<op_load_null>(_ccs->new_target());
//    }
//
//    if(is_export) {
//
//          _ccs->push_target(0);
//          ZS_RETURN_IF_ERROR(add_string_instruction(var_name, _ccs->new_target()));
//
//
//          int_t key_idx = _ccs->pop_target();
//          int_t table_idx = _ccs->pop_target();
//      int_t value_idx = _ccs->pop_target();
//
//      //
//      //
//        if(_ccs->get_parent()) {
//
//            return handle_error(zs::error_code::invalid_operation,
//                "static variable declaration is only allowed in the top level scope.\n",
//                ZB_CURRENT_SOURCE_LOCATION());
////          add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
//        }
//          add_instruction<op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
//
//
////      _ccs->pop_target();
////      ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name, nullptr, mask, custom_mask, is_const));
//
//      if (is_not(tok_comma)) {
//        break;
//      }
//
//      lex();
//    }
//    else {
//      _ccs->pop_target();
//      ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name, nullptr, mask, custom_mask, is_const));
//
//      if (is_not(tok_comma)) {
//        break;
//      }
//
//      lex();
//    }
//
//    //    _ccs->push_target(0);
//    //    ZS_RETURN_IF_ERROR(add_string_instruction(var_name, _ccs->new_target()));
//    ////
//    //
//    //    int_t value_idx = _ccs->pop_target();
//    //    int_t key_idx = _ccs->pop_target();
//    //    int_t table_idx = _ccs->pop_target();
//    //
//    //
//    //  if(_ccs->get_parent()) {
//    //    add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
//    //  }
//    //  else {
//    //    add_instruction<op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
//    //  }
//    //
//    //
//    //
//    //  _estate.type = expr_type::e_expr;
//    //  _estate.pos = -1;
//    //  _estate.no_get = false;
//    //  _estate.no_assign = false;
//
//
//
//
//
//
//
////    _ccs->pop_target();
////    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name, nullptr, mask, custom_mask, is_const));
////
////    if (is_not(tok_comma)) {
////      break;
////    }
////
////    lex();
//  }
//
//  return {};
//}

template <>
zs::error_result jit_compiler::parse<p_decl_var_internal>(bool is_export) {
  using enum token_type;
  using enum opcode;
  using enum object_type;

  const bool is_const = is(tok_const);

  if (is_const) {
    lex();
  }

  return parse<p_decl_var_internal_2>(is_export, is_const);
}

template <>
zs::error_result jit_compiler::parse<p_export>() {
  using enum token_type;
  using enum opcode;
  using enum object_type;

  ZS_ASSERT(is(tok_export), "Invalid token");

  if (!_ccs->is_top_level()) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "export can only be called on top level.\n");
  }

  ZS_RETURN_IF_ERROR(_ccs->create_export_table());

  lex();

  if (is(tok_function)) {
    lex();
    return parse<p_export_function_statement>();
  }

  if (is_var_decl_tok()) {
    return parse<p_decl_var_internal>(true);
  }

  if (is(tok_struct)) {
    lex();

    //    const int_t export_target_idx = _ccs->push_export_target();

    zs::object var_name;
    ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);

    //    ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));

    ZS_COMPILER_PARSE(p_struct);

    //    int_t struct_val = _ccs->pop_target();
    //    int_t key = _ccs->pop_target();
    //    int_t table = _ccs->pop_target();
    //    add_instruction<op_new_slot>((uint8_t)table, (uint8_t)key, (uint8_t)struct_val);

    add_to_export_table(var_name);
    return {};
  }

  if (is(tok_lcrlbracket)) {
    lex();
    const int_t export_target_idx = _ccs->push_export_target();
    ZS_COMPILER_PARSE(p_export_table);

    // Pop the export table.
    if (_ccs->pop_target() != export_target_idx) {
      return ZS_COMPILER_ERROR(zs::error_code::compile_stack_error, "export target error");
    }

    return {};
  }

  if (is(tok_identifier)) {
    const token_type next_token = _lexer->peek();

    if (!zb::is_one_of(next_token, tok_endl, tok_semi_colon)) {
      return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token after export");
    }

    object var_name = _lexer->get_identifier();
    //    std::string_view s = var_name.get_string_unchecked();
    ZS_COMPILER_PARSE(p_expression);
    add_to_export_table(var_name);
    //    if(s.size() <= zs::constants::k_small_string_max_size) {
    //        int_t table_idx=  _ccs->find_local_variable(zs::_ss("__exports__"));
    //
    //      ZS_COMPILER_PARSE(p_expression);
    //        int_t value_idx = _ccs->pop_target();
    //
    //        {
    //          struct uint64_t_pair {
    //            uint64_t value_1;
    //            uint64_t value_2;
    //          } spair = {};
    //
    //          ::memcpy(&spair, s.data(), s.size());
    //          if(!_ccs->_exported_names.insert(var_name).second) {
    //            zb::print("duplicated export");
    //          }
    //          add_instruction<op_rawsets>((uint8_t)table_idx,  (uint8_t)value_idx, spair.value_1,
    //          spair.value_2);
    //        }
    //      }
    //      else {
    //        int_t table_idx=  _ccs->find_local_variable(zs::_ss("__exports__"));
    //        ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
    //        ZS_COMPILER_PARSE(p_expression);
    //        int_t key_idx = _ccs->pop_target();
    //        int_t value_idx = _ccs->pop_target();
    //        add_instruction<op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
    //      }
    //    const int_t export_target_idx = _ccs->push_export_target();
    //    ZS_RETURN_IF_ERROR(add_export_string_instruction(_lexer->get_identifier()));
    //
    //    ZS_COMPILER_PARSE(p_expression);
    //
    //    int_t key = _ccs->pop_target();
    //    int_t exported_value_idx = _ccs->pop_target();
    //    int_t table = _ccs->pop_target();
    //
    //    add_instruction<op_new_slot>(
    //        (uint8_t)table, (uint8_t)key, (uint8_t)exported_value_idx);
    return {};
  }
  //
  return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token after export");
}

ZS_JIT_COMPILER_PARSE_OP(p_export_table) {
  using enum token_type;
  using enum opcode;

  while (is_not(tok_rcrlbracket)) {
    switch (_token) {
    case tok_function: {
      int_t bound_target = 0xFF;
      lex();

      zs::object var_name;
      ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
      ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));

      if (is(tok_lsqrbracket)) {
        ZS_COMPILER_PARSE(p_bind_env, REF(bound_target));
      }

      ZS_COMPILER_EXPECT(tok_lbracket);
      ZS_COMPILER_PARSE(p_create_function, CREF(var_name), bound_target, false);

      add_new_target_instruction<op_new_closure>(
          (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);
      break;
    }

    case tok_lsqrbracket: {
      lex();
      ZS_COMPILER_PARSE(p_comma);
      ZS_COMPILER_EXPECT(tok_rsqrbracket);
      ZS_COMPILER_EXPECT(tok_eq);
      ZS_COMPILER_PARSE(p_expression);
      break;
    }

    case tok_string_value:
    case tok_escaped_string_value: {
      zs::object value;
      ZS_COMPILER_EXPECT_GET(tok_string_value, value);
      ZS_RETURN_IF_ERROR(add_export_string_instruction(value));
      ZS_COMPILER_EXPECT(tok_colon);
      ZS_COMPILER_PARSE(p_expression);
      break;
    }

    case tok_identifier: {
      const token_type next_token = _lexer->peek();

      if (!zb::is_one_of(next_token, tok_eq, tok_endl, tok_comma, tok_rcrlbracket)) {
        return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token after export");
      }

      zs::object var_name = _lexer->get_identifier();
      ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));

      if (next_token == tok_eq) {
        lex();
        lex();
      }

      ZS_COMPILER_PARSE(p_expression);
      break;
    }

    default:
      return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token after export");
    }

    if (_token == tok_comma) {
      // optional comma.
      lex();
    }

    int_t val = _ccs->pop_target();
    int_t key = _ccs->pop_target();

    //<<BECAUSE OF THIS NO COMMON EMIT FUNC IS POSSIBLE.
    int_t table = _ccs->top_target();

    add_instruction<op_new_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val);
  }

  lex();

  return {};
}

// TODO: Prevent from declaring empty const variable.
template <>
zs::error_result jit_compiler::parse<p_decl_var>() {
  return parse<p_decl_var_internal>(false);

  //  using enum token_type;
  //  using enum opcode;
  //  using enum object_type;
  //
  //  token_type variable_type = _token;
  //
  //  lex();
  //
  ////  // var function name().
  ////  // TODO: Let's forget about this one for now.
  ////  if (is(tok_function)) {
  ////    ZS_TODO("Implement function declaration.");
  ////    int_t bound_target = 0xFF;
  ////    lex();
  ////
  ////    zs::object var_name;
  ////    ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  ////
  ////    //    if (_token == tok_lcrlbracket) {
  ////    //            boundtarget = ParseBindEnv();
  ////    //    }
  ////
  ZS_COMPILER_EXPECT(tok_lbracket);
  ////
  ////    ZS_COMPILER_PARSE(p_create_function, std::cref(var_name), bound_target, false);
  ////
  ////    //      CreateFunction(varname, 0xFF, false);
  ////
  ////    add_instruction<op_new_closure>(
  ////        (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);
  ////    _ccs->pop_target();
  ////    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
  ////    return {};
  ////  }
  //
  //  //
  //  //
  //  //
  //    const bool is_static = variable_type == tok_static;
  //  if(is_static) {
  //
  //    variable_type = _token;
  //    lex();
  //
  ////
  //    zs::print(_token);
  //  }
  //
  //
  //  const bool is_const = variable_type == tok_const;
  //
  //
  //  if (is_const) {
  //    switch (_token) {
  //    case tok_var:
  //    case tok_array:
  //    case tok_table:
  //    case tok_string:
  //    case tok_char:
  //    case tok_int:
  //    case tok_bool:
  //    case tok_float:
  //      variable_type = _token;
  //      lex();
  //      break;
  //
  //    case tok_identifier:
  //      break;
  //    default:
  //      return zs::error_code::invalid_token;
  //    }
  //  }
  //
  //
  //  uint32_t mask = 0;
  //  uint64_t custom_mask = 0;
  //
  //  switch (variable_type) {
  //  case tok_char:
  //  case tok_int:
  //    mask = zs::get_object_type_mask(k_integer);
  //    break;
  //  case tok_float:
  //    mask = zs::get_object_type_mask(k_float);
  //    break;
  //  case tok_string:
  //    mask = zs::object_base::k_string_mask;
  //    break;
  //  case tok_array:
  //    mask = zs::get_object_type_mask(k_array);
  //    break;
  //  case tok_table:
  //    mask = zs::get_object_type_mask(k_table);
  //    break;
  //  case tok_bool:
  //    mask = zs::get_object_type_mask(k_bool);
  //    break;
  //  case tok_exttype:
  //    mask = zs::get_object_type_mask(k_extension);
  //    break;
  //
  //  case tok_null:
  //    mask = zs::get_object_type_mask(k_null);
  //    break;
  //
  //  case tok_var:
  //    // Parsing a typed var (var<type1, type2, ...>).
  //    if (is(tok_lt)) {
  //      if (auto err = parse<p_variable_type_restriction>(std::ref(mask), std::ref(custom_mask)))
  //      {
  //        return helper::handle_error(
  //            this, err, "parsing variable type restriction `var<....>`", ZB_CURRENT_SOURCE_LOCATION());
  //      }
  //    }
  //    break;
  //  }
  //
  //  zb_loop() {
  //    if (is_not(tok_identifier)) {
  //      return helper::handle_error(
  //          this, zs::error_code::identifier_expected, "expected identifier", ZB_CURRENT_SOURCE_LOCATION());
  //    }
  //
  //    object var_name(_engine, _lexer->get_identifier_value());
  //
  //    lex();
  //
  //    // @code `var name = ...;`
  //    if (is(tok_eq)) {
  //      lex();
  //      ZS_COMPILER_PARSE(p_expression);
  //
  //      int_t src = _ccs->pop_target();
  //      int_t dest = _ccs->new_target(mask, custom_mask, is_const);
  //
  //      zs::opcode last_op
  //          =
  //          zs::instruction_iterator(&_ccs->_instructions._data[_ccs->get_instruction_index()]).get_opcode();
  //
  //      bool skip_mask = false;
  //      if (mask) {
  //        switch (last_op) {
  //        case op_load_int:
  //          if (!(skip_mask = (mask & zs::get_object_type_mask(k_integer)))) {
  //            return handle_error(zs::error_code::invalid_value_type_assignment,
  //                zs::strprint(
  //                    _engine, "wrong type mask", k_integer, "expected", zs::object_type_mask_printer{ mask
  //                    }),
  //                ZB_CURRENT_SOURCE_LOCATION());
  //          }
  //          break;
  //
  //        case op_load_float:
  //          if (!(skip_mask = (mask & zs::get_object_type_mask(k_float)))) {
  //            return handle_error(zs::error_code::invalid_value_type_assignment,
  //                zs::strprint(
  //                    _engine, "wrong type mask", k_float, "expected", zs::object_type_mask_printer{ mask
  //                    }),
  //                ZB_CURRENT_SOURCE_LOCATION());
  //          }
  //          break;
  //
  //        case op_load_bool:
  //          if (!(skip_mask = (mask & zs::get_object_type_mask(k_bool)))) {
  //            return handle_error(zs::error_code::invalid_value_type_assignment,
  //                zs::strprint(
  //                    _engine, "wrong type mask", k_bool, "expected", zs::object_type_mask_printer{ mask }),
  //                ZB_CURRENT_SOURCE_LOCATION());
  //          }
  //          break;
  //
  //        case op_load_small_string:
  //          if (!(skip_mask = (mask & zs::get_object_type_mask(k_small_string)))) {
  //            return handle_error(zs::error_code::invalid_value_type_assignment,
  //                zs::strprint(_engine, "wrong type mask", k_small_string, "expected",
  //                    zs::object_type_mask_printer{ mask }),
  //                ZB_CURRENT_SOURCE_LOCATION());
  //          }
  //          break;
  //
  //        case op_load_string:
  //          if (!(skip_mask = (mask & zs::object_base::k_string_mask))) {
  //            return handle_error(zs::error_code::invalid_value_type_assignment,
  //                zs::strprint(_engine, "wrong type mask", k_long_string, "expected",
  //                    zs::object_type_mask_printer{ mask }),
  //                ZB_CURRENT_SOURCE_LOCATION());
  //          }
  //          break;
  //        }
  //      }
  //
  //      if (dest != src) {
  //        //                          if (_fs->IsLocal(src)) {
  //        //                              _fs->SnoozeOpt();
  //        //                          }
  //        add_instruction<op_move>((uint8_t)dest, (uint8_t)src);
  //      }
  //
  //      if (!skip_mask) {
  //        if (custom_mask) {
  //          add_instruction<op_check_custom_type_mask>((uint8_t)_ccs->top_target(), mask,
  //          custom_mask);
  //        }
  //        else if (mask) {
  //          add_instruction<op_check_type_mask>((uint8_t)_ccs->top_target(), mask);
  //        }
  //      }
  //    }
  //
  //    // @code `var name;`
  //    else {
  //      add_instruction<op_load_null>(_ccs->new_target());
  //    }
  //
  //    if(is_static) {
  //
  //          _ccs->push_target(0);
  //          ZS_RETURN_IF_ERROR(add_string_instruction(var_name, _ccs->new_target()));
  //
  //
  //          int_t key_idx = _ccs->pop_target();
  //          int_t table_idx = _ccs->pop_target();
  //      int_t value_idx = _ccs->pop_target();
  //
  //      //
  //      //
  //        if(_ccs->get_parent()) {
  //
  //            return handle_error(zs::error_code::invalid_operation,
  //                "static variable declaration is only allowed in the top level scope.\n",
  //                ZB_CURRENT_SOURCE_LOCATION());
  ////          add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
  //        }
  //          add_instruction<op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
  //
  //
  ////      _ccs->pop_target();
  ////      ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name, nullptr, mask, custom_mask, is_const));
  //
  //      if (is_not(tok_comma)) {
  //        break;
  //      }
  //
  //      lex();
  //    }
  //    else {
  //      _ccs->pop_target();
  //      ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name, nullptr, mask, custom_mask, is_const));
  //
  //      if (is_not(tok_comma)) {
  //        break;
  //      }
  //
  //      lex();
  //    }
  //
  //    //    _ccs->push_target(0);
  //    //    ZS_RETURN_IF_ERROR(add_string_instruction(var_name, _ccs->new_target()));
  //    ////
  //    //
  //    //    int_t value_idx = _ccs->pop_target();
  //    //    int_t key_idx = _ccs->pop_target();
  //    //    int_t table_idx = _ccs->pop_target();
  //    //
  //    //
  //    //  if(_ccs->get_parent()) {
  //    //    add_instruction<op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
  //    //  }
  //    //  else {
  //    //    add_instruction<op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
  //    //  }
  //    //
  //    //
  //    //
  //    //  _estate.type = expr_type::e_expr;
  //    //  _estate.pos = -1;
  //    //  _estate.no_get = false;
  //    //  _estate.no_assign = false;
  //
  //
  //
  //
  //
  //
  //
  ////    _ccs->pop_target();
  ////    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name, nullptr, mask, custom_mask, is_const));
  ////
  ////    if (is_not(tok_comma)) {
  ////      break;
  ////    }
  ////
  ////    lex();
  //  }
  //
  //  return {};
}

template <>
zs::error_result jit_compiler::parse<p_enum_table>() {
  using enum token_type;

  while (is_not(tok_rcrlbracket)) {
    switch (_token) {
    case tok_lsqrbracket:
      return ZS_COMPILER_ERROR(zs::error_code::invalid_operation, "Enum keys can only be regular identifier");

    case tok_string_value:
    case tok_escaped_string_value:
      return ZS_COMPILER_ERROR(zs::error_code::invalid_operation,
          "Enum keys can only be regular identifier i.e. no json style identifier)");

    case tok_identifier: {
      zs::object identifier;
      ZS_COMPILER_EXPECT_GET(tok_identifier, identifier);
      ZS_RETURN_IF_ERROR(add_string_instruction(identifier));

      // No value enum field.
      if (is(tok_comma, tok_rcrlbracket)) {
        add_new_target_instruction<op_load_none>();
      }
      else {
        ZS_COMPILER_EXPECT(tok_eq);
        ZS_COMPILER_PARSE(p_expression);
      }
      break;
    }

    default:
      return ZS_COMPILER_ERROR(
          zs::error_code::invalid_operation, "Enum can only contain integers, floats, bools and strings.");
    }

    lex_if(tok_comma);

    int_t val = _ccs->pop_target();
    int_t key = _ccs->pop_target();
    int_t table = _ccs->top_target();
    add_instruction<op_new_enum_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val);
  }

  lex();

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_decl_enum>() {
  using enum token_type;
  using enum opcode;
  using enum object_type;

  zbase_assert(_token == tok_enum);

  lex();

  //
  //
  //

  if (is_not(tok_identifier)) {
    return ZS_COMPILER_ERROR(zs::error_code::identifier_expected, "expected identifier");
  }

  object var_name(_engine, _lexer->get_identifier_value());

  lex();

  // Optional `=`.
  lex_if(tok_eq);

  if (is_not(tok_lcrlbracket)) {
    return ZS_COMPILER_EXPECT_ERROR_MESSAGE(tok_lcrlbracket);
    //    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token ",
    //        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
    //        zb::quoted<"'">(zs::token_to_string(tok_lcrlbracket)));
  }

  _enum_counter = 0;
  add_new_target_instruction<op_new_obj>(object_type::k_table);
  lex();
  ZS_COMPILER_PARSE(p_enum_table);

  int_t src = _ccs->pop_target();

  if (int_t dest = _ccs->new_target(); dest != src) {
    add_instruction<op_move>((uint8_t)dest, (uint8_t)src);
  }

  add_instruction<op_close_enum>((uint8_t)_ccs->top_target());

  _ccs->pop_target();

  ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
  return {};
}

static inline void replace_all(zs::string& str, std::string_view from, std::string_view to) {
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != zs::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

ZS_JIT_COMPILER_PARSE_OP(p_macro_call, token_type last_token) {
  using enum token_type;
  using enum object_type;

  if (is_not(tok_identifier)) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "macro cannot have default parameters");
  }
  zs::object identifier = _lexer->get_identifier();

  size_t macro_index = _macros.ifind_if([&](const macro& m) { return m.name == identifier; });

  if (macro_index == zs::vector<macro>::npos) {
    zb::print("macro_index", macro_index);
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "macro doesn't exists");
  }

  lex();

  if (is_not(tok_lbracket)) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "macro cannot have default parameters");
  }
  const char* param_begin = _lexer->_stream.ptr();
  //  lex();
  int_t nargs = 0; // this.

  int_t brack_count = 0;
  int_t ctrlbrack_count = 0;
  int_t sqbrack_count = 0;
  zs::object params = zs::_a(_engine, 0);
  bool first = true;

  const char* params_last_end = _lexer->_stream.ptr();
  while (!zb::is_one_of(_token, tok_rbracket, tok_eof, tok_lex_error)) {
    const char* params_end = _lexer->_stream.ptr();
    params_last_end = params_end;

    lex();

    brack_count += _token == tok_lbracket;
    ctrlbrack_count += _token == tok_lcrlbracket;
    sqbrack_count += _token == tok_lsqrbracket;

    if (_token == tok_rcrlbracket) {
      if (!ctrlbrack_count) {
        zb::print("KLKLKLK");
      }
      ctrlbrack_count--;
    }
    else if (_token == tok_rsqrbracket) {
      if (!sqbrack_count) {
        zb::print("KLKLKLK");
      }
      sqbrack_count--;
    }

    else if (_token == tok_rbracket) {
      if (!brack_count and !ctrlbrack_count and !sqbrack_count) {
        continue;
      }

      brack_count--;

      _token = tok_none;
    }

    if (_token == tok_comma and !brack_count and !ctrlbrack_count and !sqbrack_count) {

      std::string_view param(param_begin, std::distance(param_begin, params_end));
      //       zb::print(param );
      param = zb::strip_all(param);
      params.as_array().push(zs::_s(_engine, param));
      //
      param_begin = _lexer->_stream.ptr();
    }
  }

  std::string_view param(param_begin, std::distance(param_begin, params_last_end));

  param = zb::strip_all(param);

  if (!param.empty() and param[0] != ')') {
    params.as_array().push(zs::_s(_engine, param));
  }

  zb::print(params);
  zb::print(params.as_array().size());
  if (is_not(tok_rbracket)) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_parameter_type, "macro cannot have default parameters");
  }

  const macro& m = _macros[macro_index];

  const zs::array_object& in_params = params.as_array();
  const zs::array_object& macro_params = m.params.as_array();

  if (macro_params.size() != in_params.size()) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_parameter_count, "macro invalid number of parameters");
  }

  zs::string output_code(m.content.get_string_unchecked(), zs::allocator<char>(_engine));

  for (size_t i = 0; i < in_params.size(); i++) {
    std::string_view from = macro_params[i].get_string_unchecked();
    std::string_view to = in_params[i].get_string_unchecked();
    size_t start_pos = 0;
    while ((start_pos = output_code.find(from, start_pos)) != zs::string::npos) {
      output_code.replace(start_pos, from.length(), to);
      start_pos += to.length();
    }
  }
  zb::print(output_code);

  //  m.content;

  //  scope previous_scope = _scope;
  //  _scope.n_captures = _ccs->_n_capture;
  //  _scope.stack_size = _ccs->get_stack_size();
  //
  //  //
  //  //
  //  lex();
  zs::lexer* last_lexer = _lexer;
  zs::lexer lexer(_engine, output_code);
  _lexer = &lexer;
  _token = tok_none;
  lex();
  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    ZS_COMPILER_PARSE(p_statement, true);

    if (!zb::is_one_of(_lexer->last_token(), tok_rcrlbracket, tok_semi_colon)) {
      ZS_COMPILER_PARSE(p_semi_colon);
    }
  }

  if (_token == tok_lex_error) {

    last_lexer->_current_token = _lexer->_current_token;
    last_lexer->_last_token = _lexer->_last_token;
    _lexer = last_lexer;

    return zs::error_code::invalid;
  }

  last_lexer->_current_token = _lexer->_current_token;
  last_lexer->_last_token = _lexer->_last_token;

  _lexer = last_lexer;
  //  lex();

  //  zb::print("DSLKDJSKLDJSLD", _token);
  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_macro) {
  using enum token_type;
  using enum object_type;

  ZS_COMPILER_EXPECT(tok_macro);

  if (is_not(tok_identifier)) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "macro cannot have default parameters");
  }

  zs::object identifier = _lexer->get_identifier();

  //    ZS_COMPILER_EXPECT_GET(tok_identifier, identifier);

  const char* params_begin = _lexer->_stream.ptr();
  lex();

  ZS_COMPILER_EXPECT(tok_lbracket);

  //    zs::closure_compile_state* fct_state = _ccs->push_child_state();
  //    fct_state->name = identifier;
  int_t def_params = 0;

  //    zs::closure_compile_state* curr_chunk = std::exchange(_ccs, fct_state);

  //  fct_state->source_name = _ccs->_tdata->source_name;

  //    ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("this")));

  zs::object param_names = zs::_a(_engine, 0);

  // Parsing function parameters: `function (parameters)`.
  while (is_not(tok_rbracket)) {
    if (is(tok_triple_dots)) {
      // TODO: Named triple dots?

      if (def_params > 0) {
        return ZS_COMPILER_ERROR(zs::error_code::invalid_argument,
            "function with default parameters cannot have variable number of parameters");
      }

      //        ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("vargv")));
      //        fct_state->_vargs_params = true;
      lex();

      if (is_not(tok_rbracket)) {
        return ZS_COMPILER_ERROR(
            zs::error_code::invalid_token, "expected ')' after a variadic (...) parameter");
      }

      break;
    }
    else {

      zs::object param_name;
      ZS_COMPILER_EXPECT_GET(tok_identifier, param_name);
      //        ZS_RETURN_IF_ERROR(fct_state->add_parameter(param_name ));
      param_names.as_array().push(
          zs::object::create_concat_string(_engine, "$", param_name.get_string_unchecked()));
      if (is(tok_eq)) {
        return ZS_COMPILER_ERROR(
            zs::error_code::invalid_parameter_type, "macro cannot have default parameters");
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
  const char* params_end = _lexer->_stream.ptr();
  const char* content_begin = _lexer->_stream.ptr();

  ZS_COMPILER_EXPECT(tok_rbracket);

  std::string_view params(params_begin, std::distance(params_begin, params_end));
  zb::print(params, "---------------------------");

  int_t brack_count = 0;
  while (!zb::is_one_of(_token, tok_rcrlbracket, tok_eof, tok_lex_error)) {

    lex();

    brack_count += _token == tok_lcrlbracket;
    //      if(_token == tok_lcrlbracket) {
    //        brack_count++;
    //      }
    if (_token == tok_rcrlbracket and brack_count) {
      brack_count--;
      _token = tok_none;
    }
  }

  if (is_not(tok_rcrlbracket)) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_parameter_type, "macro cannot have default parameters");
  }

  const char* content_end = _lexer->_stream.ptr();

  lex();
  std::string_view content(content_begin, std::distance(content_begin, content_end));
  zb::print(content, "---------------------------");

  content = zb::strip_all(content);
  content = content.substr(1);
  content = content.substr(0, content.size() - 1);
  macro m = { identifier, param_names, zs::_s(_engine, content) };

  zb::print(m.params, m.content);
  _macros.push_back(std::move(m));
  _lexer->_last_token = tok_rcrlbracket;

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_define>() {
  using enum token_type;

  ZS_COMPILER_EXPECT(tok_define);

  zs::object identifier;
  ZS_COMPILER_EXPECT_GET(tok_identifier, identifier);

  ZS_COMPILER_EXPECT(tok_eq);

  zs::object value;

  switch (_token) {
  case tok_string_value: {
    std::string_view svalue = _lexer->get_string_value();
    value = zs::_s(_engine, svalue);

    lex();
    break;
  }

  case tok_escaped_string_value: {
    std::string_view svalue = _lexer->get_escaped_string_value();
    value = zs::_s(_engine, svalue);

    lex();
    break;
  }

  case tok_null:
    value.reset();
    lex();
    break;

  case tok_none:
    value = zs::object::create_none();
    lex();
    break;

  case tok_integer_value:
    value = _lexer->get_int_value();

    lex();
    break;

  case tok_float_value:
    value = _lexer->get_float_value();
    lex();
    break;

  case tok_true:
    value = true;
    lex();
    break;

  case tok_false:
    value = false;
    lex();
    break;

  default:
    return zs::error_code::invalid_token;
  }

  return _compile_time_consts._table->set(identifier, std::move(value));
}

template <>
zs::error_result jit_compiler::parse<p_preprocessor>() {
  using enum token_type;

  ZS_COMPILER_EXPECT(tok_hastag);

  switch (_token) {
  case tok_include:
    return parse<p_include_or_import_statement>(tok_include);

  case tok_import:
    return parse<p_include_or_import_statement>(tok_import);

  case tok_define:
    return parse<p_define>();

  default:
    _error_message += zs::strprint(_engine, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)),
        ", expected {'include', 'import', 'macro', 'define', 'if', 'elif', "
        "'else'}",
        _lexer->get_line_info());
    return zs::error_code::invalid_token;
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_if_block>() {
  using enum token_type;
  if (is(tok_lcrlbracket)) {
    //       BEGIN_SCOPE();

    scope previous_scope = _scope;
    _scope.n_captures = _ccs->_n_capture;
    _scope.stack_size = _ccs->get_stack_size();

    lex();

    while (is_not(tok_rcrlbracket, tok_default, tok_case)) {
      ZS_COMPILER_PARSE(p_statement, true);

      if (_lexer->_last_token != tok_rcrlbracket && _lexer->_last_token != tok_semi_colon) {
        //            OptionalSemicolon();
        ZS_COMPILER_PARSE(p_semi_colon);
      }
    }

    ZS_COMPILER_EXPECT(tok_rcrlbracket);

    {
      int_t previous_n_capture = _ccs->_n_capture;

      if (_ccs->get_stack_size() != _scope.stack_size) {
        _ccs->set_stack_size(_scope.stack_size);
        if (previous_n_capture != (int_t)_ccs->_n_capture) {
          add_instruction<op_close>((uint32_t)_scope.stack_size);
        }
      }
      _scope = previous_scope;
    }
  }
  else {
    ZS_COMPILER_PARSE(p_statement, true);

    if (_lexer->_last_token != tok_rcrlbracket && _lexer->_last_token != tok_semi_colon) {
      //            OptionalSemicolon();
      ZS_COMPILER_PARSE(p_semi_colon);
    }

    //       Statement();
    //       if (_lex._prevtoken != _SC('}') && _lex._prevtoken != _SC(';'))
    //         OptionalSemicolon();
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_if>() {
  using enum token_type;
  using enum opcode;

  zs::instruction_vector& ivec = _ccs->_instructions;

  ZS_COMPILER_EXPECT(tok_if);
  ZS_COMPILER_EXPECT(tok_lbracket);
  ZS_COMPILER_PARSE(p_comma);
  ZS_COMPILER_EXPECT(tok_rbracket);

  add_instruction<op_jz>(0, _ccs->pop_target());
  const int_t jz_inst_idx = _ccs->get_instruction_index();

  ZS_COMPILER_PARSE(p_if_block);

  if (is_not(tok_else)) {
    ivec.get<op_jz>(jz_inst_idx)->offset = (int32_t)(_ccs->get_next_instruction_index() - jz_inst_idx);
    return {};
  }

  add_instruction<op_jmp>(0);
  const int_t jmp_inst_idx = _ccs->get_instruction_index();
  const int_t jmp_end_idx = _ccs->get_next_instruction_index();

  lex();
  ZS_COMPILER_PARSE(p_if_block);

  ivec.get<op_jmp>(jmp_inst_idx)->offset = (int32_t)(_ccs->get_next_instruction_index() - jmp_inst_idx);
  ivec.get<op_jz>(jz_inst_idx)->offset = (int32_t)(jmp_end_idx - jz_inst_idx);

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_for_each>() {
  using enum token_type;

  // foreach(var i : arr)
  // ^
  lex();

  // foreach(var i : arr)
  //        ^
  ZS_COMPILER_EXPECT(tok_lbracket);
  ZS_COMPILER_EXPECT(tok_var);

  zs::object var_name;
  zs::object idx_name = zs::_ss("index");
  zs::object iter_name = zs::_ss("iter");

  ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  ZS_RETURN_IF_ERROR(_ccs->push_local_variable(idx_name));
  ZS_COMPILER_EXPECT(tok_colon);

  zb::print(var_name);

  scope previous_scope = _scope;
  _scope.n_captures = _ccs->_n_capture;
  _scope.stack_size = _ccs->get_stack_size();
  ZS_COMPILER_PARSE(p_expression);

  ZS_COMPILER_EXPECT(tok_rbracket);
  //      // put the table in the stack(evaluate the table expression)
  //      Expression();
  //      Expect(_SC(')'));
  int_t container = _ccs->top_target();
  //      // push the index local var
  int_t indexpos = _ccs->find_local_variable(idx_name);
  add_instruction<op_load_null>(indexpos);

  //      // push the value local var
  int_t valuepos = _ccs->find_local_variable(var_name);
  add_instruction<op_load_null>(valuepos);
  //      // push reference index
  int_t itrpos = _ccs->find_local_variable(iter_name); // use invalid id to make it inaccessible
  add_instruction<op_load_null>(itrpos);

  const int_t cond_inst_start_idx = _ccs->get_next_instruction_index();
  //      SQInteger jmppos = _fs->GetCurrentPos();
  //      _fs->AddInstruction(_OP_FOREACH, container, 0, indexpos);
  //      SQInteger foreachpos = _fs->GetCurrentPos();
  //      _fs->AddInstruction(_OP_POSTFOREACH, container, 0, indexpos);
  //      // generate the statement code
  //      BEGIN_BREAKBLE_BLOCK()
  //      Statement();
  //      _fs->AddInstruction(_OP_JMP, 0, jmppos - _fs->GetCurrentPos() - 1);
  //      _fs->SetInstructionParam(foreachpos, 1, _fs->GetCurrentPos() - foreachpos);
  //      _fs->SetInstructionParam(foreachpos + 1, 1, _fs->GetCurrentPos() - foreachpos);
  //      END_BREAKBLE_BLOCK(foreachpos - 1);
  //      // restore the local variable stack(remove index,val and ref idx)
  //      _fs->PopTarget();
  //      END_SCOPE();
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_for_auto>(std::span<zs::token_type> sp) {
  using enum token_type;

  //  zb::print(sp);

  object val_name;
  object key_name;
  const char* colon_ptr = nullptr;
  const char* type_list_begin = nullptr;
  const char* type_list_end = nullptr;
  const char* input_code_begin = _lexer->_stream.ptr();

  //  if(sp[0] == tok_int) {
  //    zb::print("DSKDJSKJDLKSDJKLSDDHHAJKSKJA");
  //  }

  size_t type_restriction_begin_token_index = 0;
  size_t type_restriction_end_token_index = 0;

  if (zb::is_one_of(sp[1], tok_int, tok_char, tok_bool, tok_float, tok_string, tok_table)) {
    type_restriction_begin_token_index = 0;
    type_restriction_end_token_index = 1;
  }
  else if (sp[1] == tok_auto) {
  }
  else if (sp[1] == tok_var and sp[2] == tok_lt) {

    type_restriction_begin_token_index = 2;

    for (size_t i = 2; i < sp.size() - 1; i++) {
      if (sp[i] == tok_gt) {
        type_restriction_end_token_index = i - 1;
        break;
      }
    }
  }

  else if (sp[1] == tok_identifier) {

    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expected var or type");
  }

  bool has_key = false;
  if (type_restriction_end_token_index) {
    for (size_t i = 0; i < sp.size() - 1; i++) {
      if (i == type_restriction_begin_token_index) {
        type_list_begin = _lexer->_stream.ptr();
      }

      if (i == type_restriction_end_token_index) {
        type_list_end = _lexer->_stream.ptr();
      }

      if (is(tok_identifier) and i > type_restriction_end_token_index and val_name.is_null()) {
        val_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          has_key = true;

          //          return handle_error(zs::error_code::invalid_token, "expected ':' in for
          //          loop",
          //              ZB_CURRENT_SOURCE_LOCATION());
        }
      }
      else if (is(tok_identifier) and i > type_restriction_end_token_index and key_name.is_null()
          and has_key) {
        key_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expected ':' in for loop");
        }
      }

      else if (is(tok_colon, tok_in)) {
        if (colon_ptr) {
          return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "multiple ':' in for loop");
        }
        colon_ptr = _lexer->_stream.ptr();
      }

      lex();
    }
  }
  else {
    for (size_t i = 0; i < sp.size() - 1; i++) {

      if (is(tok_identifier) and val_name.is_null()) {
        val_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          has_key = true;
          //          return handle_error(zs::error_code::invalid_token, "expected ':' in for
          //          loop",
          //              ZB_CURRENT_SOURCE_LOCATION());
        }
      }
      else if (is(tok_identifier) and key_name.is_null() and has_key) {
        key_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          //            has_key = true;
          return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expected ':' in for loop");
        }
      }

      else if (is(tok_colon, tok_in)) {

        if (colon_ptr) {
          return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "multiple ':' in for loop");
        }
        colon_ptr = _lexer->_stream.ptr();
      }

      lex();
    }
  }

  std::string_view vname = zb::strip_leading_and_trailing_spaces(val_name.get_string_unchecked());
  std::string_view kname;

  if (has_key) {
    kname = zb::strip_leading_and_trailing_spaces(key_name.get_string_unchecked());
  }

  std::string_view array_code = zb::strip_leading_and_trailing_spaces(
      std::string_view(colon_ptr, std::distance(colon_ptr, _lexer->_stream.ptr() - 1)));

  std::string_view type_list_code;
  if (type_restriction_end_token_index) {
    type_list_code = zb::strip_leading_and_trailing_spaces(
        std::string_view(type_list_begin, std::distance(type_list_begin, type_list_end)));
  }

  //  zb::print(ZBASE_VNAME(type_list_code));

  std::string_view input_code(input_code_begin, std::distance(input_code_begin, _lexer->_stream.ptr()));
  //  zb::print("--------------------", input_code);
  //  zb::print("--------------------");

  scope previous_scope = _scope;
  _scope.n_captures = _ccs->_n_capture;
  _scope.stack_size = _ccs->get_stack_size();

  //
  //
  zs::lexer* last_lexer = _lexer;

  zs::string var_type("", zs::string_allocator(_engine));

  if (type_restriction_end_token_index) {
    var_type.push_back('<');
    var_type.append(type_list_code);
    var_type.append(", null>");
  }

  zs::string output_code((zs::string_allocator(_engine)));

  if (has_key) {

    output_code = zs::strprint(_engine, //
        "var __private_array = ", array_code, //
        ", __private_array_end = __private_array.end()", //
        ", __private_iterator = __private_array.begin();", //
        "var", var_type, " ", kname, " = __private_iterator.get_if_not(__private_array_end), ", vname,
        " = __private_iterator.get_key_if_not(__private_array_end);", //
        " __private_iterator != __private_array_end;", //
        " ++__private_iterator, ", //
        kname, " = __private_iterator.get_if_not(__private_array_end), ", vname,
        " = __private_iterator.get_key_if_not(__private_array_end)");
  }
  else {

    output_code = zs::strprint(_engine, //
        "var __private_array = ", array_code, //
        ", __private_array_end = __private_array.end()", //
        ", __private_iterator = __private_array.begin();", //
        "var", var_type, " ", vname, " = __private_iterator.get_if_not(__private_array_end);", //
        " __private_iterator != __private_array_end;", //
        " ++__private_iterator, ", //
        vname, " = __private_iterator.get_if_not(__private_array_end)");
  }

  //  zs::string output_code = type_restriction_end_token_index
  //      ?
  //
  //      // With var<...>.
  //      zs::strprint(_engine, "var __private_array = ", array_code,
  //          ", __private_array_end = __private_array.end(), __private_iterator = __private_array.begin();
  //          var<", type_list_code, ", null> ", vname, " =
  //          __private_iterator.get_if_not(__private_array_end)",
  //          "; __private_iterator != __private_array_end;\n  ++__private_iterator, ", vname,
  //          " = __private_iterator.get_if_not(__private_array_end)")
  //
  //  // Normal var.
  //      : zs::strprint(_engine, "var __private_array = ", array_code,
  //            ", __private_array_end = __private_array.end(), __private_iterator = __private_array.begin();
  //            " "var ", vname, " = __private_iterator.get_if_not(__private_array_end)",
  //            "; __private_iterator != __private_array_end; ++__private_iterator, ", vname,
  //            " = __private_iterator.get_if_not(__private_array_end)");

  //  zb::print(output_code);
  zs::lexer lexer(_engine, output_code);
  _lexer = &lexer;

  lex();
  //  zb::print(_token);

  if (is_var_decl_tok()) {
    ZS_COMPILER_PARSE(p_decl_var);
  }
  //  zb::print(_token, "dksjdkslajdksa");
  lex();
  // for(var i = 0
  //     ^
  if (is_var_decl_tok()) {
    ZS_COMPILER_PARSE(p_decl_var);
  }
  else if (is_not(tok_semi_colon)) {
    ZS_COMPILER_PARSE(p_comma);
    _ccs->pop_target();
  }

  // for(var i = 0;
  //              ^
  ZS_COMPILER_EXPECT(tok_semi_colon);

  // The next instruction is the beginning of the condition.
  const int_t cond_inst_start_idx = _ccs->get_next_instruction_index();

  // for(var i = 0; i < 10; i++)
  //                ^
  int_t cond_inst_jz_idx = -1;
  if (is_not(tok_semi_colon)) {
    ZS_COMPILER_PARSE(p_comma);
    add_instruction<op_jz>(0, _ccs->pop_target());
    cond_inst_jz_idx = _ccs->get_instruction_index();
    //    jz_next_index =_ccs->get_next_instruction_index();
  }

  const int_t cond_inst_last_idx = _ccs->get_instruction_index();
  //  const int_t cond_inst_end_idx = _ccs->get_next_instruction_index();
  const bool has_cond = cond_inst_jz_idx > 0;

  // for(var i = 0; i < 10;
  //                      ^
  ZS_COMPILER_EXPECT(tok_semi_colon);

  const int_t incr_expr_inst_start_idx = _ccs->get_next_instruction_index();

  if (is_not(tok_rbracket)) {
    // for(var i = 0; i < 10; i++)
    //                        ^
    ZS_COMPILER_PARSE(p_comma);
    _ccs->pop_target();
  }

  if (_token == tok_lex_error) {
    last_lexer->_current_token = _lexer->_current_token;
    last_lexer->_last_token = _lexer->_last_token;
    _lexer = last_lexer;
    return zs::error_code::invalid;
  }

  last_lexer->_current_token = _lexer->_current_token;
  last_lexer->_last_token = _lexer->_last_token;
  _lexer = last_lexer;
  _token = tok_rbracket;

  //    lex();
  // for(var i = 0; i < 10; i++)
  //                           ^
  ZS_COMPILER_EXPECT(tok_rbracket);

  // This is the index of the instruction following the last instruction of the incr expr.
  const int_t incr_expr_inst_end_idx = _ccs->get_next_instruction_index();
  const int_t incr_expr_size = (incr_expr_inst_end_idx - incr_expr_inst_start_idx);

  // Here we want to copy the incr expr instructions in a buffer.
  // And remove them from the instruction vector.
  zs::vector<uint8_t> inst_buffer((zs::allocator<uint8_t>(_engine)));
  if (incr_expr_size > 0) {
    inst_buffer.resize(incr_expr_size);
    ::memcpy(inst_buffer.data(), _ccs->_instructions._data.data(incr_expr_inst_start_idx), incr_expr_size);
    _ccs->_instructions._data.resize(incr_expr_inst_start_idx);
    _ccs->_last_instruction_index = cond_inst_last_idx;
  }

  ZS_COMPILER_PARSE(p_statement, true);

  //        SQInteger expend = _fs->GetCurrentPos();
  //        SQInteger expsize = (expend - expstart) + 1;
  //        SQInstructionVec exp;
  //        if (expsize > 0) {
  //          for (SQInteger i = 0; i < expsize; i++)
  //            exp.push_back(_fs->GetInstruction(expstart + i));
  //          _fs->PopInstructions(expsize);
  //        }
  //        BEGIN_BREAKBLE_BLOCK()
  //        Statement();
  //  int_t continue_target = _ccs->get_instruction_index();

  //  int_t end_block_index = -1;
  if (incr_expr_size > 0) {
    size_t sz = _ccs->_instructions._data.size();
    _ccs->_instructions._data.resize(sz + incr_expr_size);
    ::memcpy(_ccs->_instructions._data.data(sz), inst_buffer.data(), incr_expr_size);

    // TODO: Fix this.
    _ccs->_last_instruction_index = _ccs->_instructions._data.size();
  }

  const int_t end_block_index = _ccs->get_next_instruction_index();

  add_instruction<op_jmp>((int32_t)(cond_inst_start_idx - end_block_index));

  if (has_cond > 0) {
    instruction_t<op_jz>* inst = (instruction_t<op_jz>*)(_ccs->_instructions._data.data(cond_inst_jz_idx));
    inst->offset = (int32_t)(_ccs->get_next_instruction_index() - cond_inst_jz_idx);
  }

  {
    int_t previous_n_capture = _ccs->_n_capture;

    if (_ccs->get_stack_size() != _scope.stack_size) {
      _ccs->set_stack_size(_scope.stack_size);
      if (previous_n_capture != (int_t)_ccs->_n_capture) {
        add_instruction<op_close>((uint32_t)_scope.stack_size);
      }
    }
    _scope = previous_scope;
  }

  //        _fs->AddInstruction(_OP_JMP, 0, jmppos - _fs->GetCurrentPos() - 1, 0);
  //        if (jzpos > 0)
  //          _fs->SetInstructionParam(jzpos, 1, _fs->GetCurrentPos() - jzpos);
  //
  //        END_BREAKBLE_BLOCK(continuetrg);
  //
  //        END_SCOPE();
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_for>() {
  using enum token_type;

  // for(var i = 0; i < 10; i++)
  // ^

  //  zb_loop() {
  //    switch (l.lex()) {
  //    case tok_none:
  //      break;
  //
  //    case tok_gt:
  //      return l.lex() == tok_lbracket;
  //
  //    default:
  //      return false;
  //    }
  //  }

  lex();

  //  // for(var i = 0; i < 10; i++)
  //  //    ^
  //  ZS_COMPILER_EXPECT(tok_lbracket);

  if (!is(tok_lbracket)) {
    return zs::error_code::invalid_token;
  }

  lexer l(*_lexer);
  std::vector<zs::token_type> toks;

  toks.resize(50);
  std::span<zs::token_type> sp(toks);

  if (zs::status_result status = l.lex_for_auto(sp)) {
    ZS_ASSERT(sp.back() == tok_rbracket);
    return parse<p_for_auto>(sp);
  }

  // for(var i = 0; i < 10; i++)
  //    ^
  ZS_COMPILER_EXPECT(tok_lbracket);

  scope previous_scope = _scope;
  _scope.n_captures = _ccs->_n_capture;
  _scope.stack_size = _ccs->get_stack_size();

  // for(var i = 0
  //     ^
  if (is_var_decl_tok()) {
    ZS_COMPILER_PARSE(p_decl_var);
  }
  else if (is_not(tok_semi_colon)) {
    ZS_COMPILER_PARSE(p_comma);
    _ccs->pop_target();
  }

  // for(var i = 0;
  //              ^
  ZS_COMPILER_EXPECT(tok_semi_colon);

  // The next instruction is the beginning of the condition.
  const int_t cond_inst_start_idx = _ccs->get_next_instruction_index();

  // for(var i = 0; i < 10; i++)
  //                ^
  int_t cond_inst_jz_idx = -1;
  if (is_not(tok_semi_colon)) {
    ZS_COMPILER_PARSE(p_comma);
    add_instruction<op_jz>(0, _ccs->pop_target());
    cond_inst_jz_idx = _ccs->get_instruction_index();
    //    jz_next_index =_ccs->get_next_instruction_index();
  }

  const int_t cond_inst_last_idx = _ccs->get_instruction_index();
  //  const int_t cond_inst_end_idx = _ccs->get_next_instruction_index();
  const bool has_cond = cond_inst_jz_idx > 0;

  // for(var i = 0; i < 10;
  //                      ^
  ZS_COMPILER_EXPECT(tok_semi_colon);

  const int_t incr_expr_inst_start_idx = _ccs->get_next_instruction_index();

  if (is_not(tok_rbracket)) {
    // for(var i = 0; i < 10; i++)
    //                        ^
    ZS_COMPILER_PARSE(p_comma);
    _ccs->pop_target();
  }

  // for(var i = 0; i < 10; i++)
  //                           ^
  ZS_COMPILER_EXPECT(tok_rbracket);

  // This is the index of the instruction following the last instruction of the incr expr.
  const int_t incr_expr_inst_end_idx = _ccs->get_next_instruction_index();
  const int_t incr_expr_size = (incr_expr_inst_end_idx - incr_expr_inst_start_idx);

  // Here we want to copy the incr expr instructions in a buffer.
  // And remove them from the instruction vector.
  zs::vector<uint8_t> inst_buffer((zs::allocator<uint8_t>(_engine)));
  if (incr_expr_size > 0) {
    inst_buffer.resize(incr_expr_size);
    ::memcpy(inst_buffer.data(), _ccs->_instructions._data.data(incr_expr_inst_start_idx), incr_expr_size);
    _ccs->_instructions._data.resize(incr_expr_inst_start_idx);
    _ccs->_last_instruction_index = cond_inst_last_idx;
  }

  ZS_COMPILER_PARSE(p_statement, true);

  //        SQInteger expend = _fs->GetCurrentPos();
  //        SQInteger expsize = (expend - expstart) + 1;
  //        SQInstructionVec exp;
  //        if (expsize > 0) {
  //          for (SQInteger i = 0; i < expsize; i++)
  //            exp.push_back(_fs->GetInstruction(expstart + i));
  //          _fs->PopInstructions(expsize);
  //        }
  //        BEGIN_BREAKBLE_BLOCK()
  //        Statement();
  //  int_t continue_target = _ccs->get_instruction_index();

  //  int_t end_block_index = -1;
  if (incr_expr_size > 0) {
    size_t sz = _ccs->_instructions._data.size();
    _ccs->_instructions._data.resize(sz + incr_expr_size);
    ::memcpy(_ccs->_instructions._data.data(sz), inst_buffer.data(), incr_expr_size);

    // TODO: Fix this.
    _ccs->_last_instruction_index = _ccs->_instructions._data.size();
  }

  const int_t end_block_index = _ccs->get_next_instruction_index();

  add_instruction<op_jmp>((int32_t)(cond_inst_start_idx - end_block_index));

  if (has_cond > 0) {
    instruction_t<op_jz>* inst = (instruction_t<op_jz>*)(_ccs->_instructions._data.data(cond_inst_jz_idx));
    inst->offset = (int32_t)(_ccs->get_next_instruction_index() - cond_inst_jz_idx);
  }

  {
    int_t previous_n_capture = _ccs->_n_capture;

    if (_ccs->get_stack_size() != _scope.stack_size) {
      _ccs->set_stack_size(_scope.stack_size);
      if (previous_n_capture != (int_t)_ccs->_n_capture) {
        add_instruction<op_close>((uint32_t)_scope.stack_size);
      }
    }
    _scope = previous_scope;
  }

  //        _fs->AddInstruction(_OP_JMP, 0, jmppos - _fs->GetCurrentPos() - 1, 0);
  //        if (jzpos > 0)
  //          _fs->SetInstructionParam(jzpos, 1, _fs->GetCurrentPos() - jzpos);
  //
  //        END_BREAKBLE_BLOCK(continuetrg);
  //
  //        END_SCOPE();
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_statement>(bool close_frame) {
  using enum token_type;
  using enum opcode;

  if (_add_line_info) {
    _ccs->add_line_infos(_lexer->get_line_info());
  }

  switch (_token) {
  case tok_semi_colon:
    lex();
    return {};

  case tok_const:
  case tok_var:
  case tok_array:
  case tok_table:
  case tok_string:
  case tok_char:
  case tok_int:
  case tok_bool:
  case tok_float:
    return parse<p_decl_var>();

  case tok_enum:
    return parse<p_decl_enum>();

  case tok_macro:
    return parse<p_macro>();

  case tok_hastag:
    return parse<p_preprocessor>();

  case tok_include:
    ZS_TODO("Implement include(...)");
    return zs::error_code::unimplemented;

  case tok_import:
    ZS_TODO("Implement import(...)");
    return zs::error_code::unimplemented;

  case tok_if:
    return parse<p_if>();

  case tok_for:
    return parse<p_for>();

  case tok_foreach:
    return parse<p_for_each>();

  case tok_class:
    return parse<p_class_statement>();

  case tok_struct:
    return parse<p_struct_statement>();

  case tok_export:
    return parse<p_export>();

  case tok_module:
    return parse<p_module>();

  case tok_global: {
    lex();

    if (is(tok_function)) {
      return parse<p_global_function_statement>();
    }

    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expected function i guess");
  }

  case tok_function:
    return parse<p_function_statement>();

  case tok_return: {

    if (_ccs->is_top_level() and _ccs->has_export()) {
      return ZS_COMPILER_ERROR(
          zs::error_code::invalid_token, "return statement is not allowed when using export.\n");
    }

    lex();

    if (!is_end_of_statement()) {
      //        SQInteger retexp = _fs->GetCurrentPos() + 1;
      //        CommaExpr();
      ZS_COMPILER_PARSE(p_comma);
      //        if (op == _OP_RETURN && _fs->_traps > 0)
      //          _fs->AddInstruction(_OP_POPTRAP, _fs->_traps, 0);
      //        _fs->_returnexp = retexp;
      //              _fs->AddInstruction(op, 1, _fs->PopTarget(),
      //              _fs->GetStackSize());
      // TODO: Fix this.
      add_instruction<op_return>(_ccs->pop_target(), true);
    }
    else {
      //        if (op == _OP_RETURN && _fs->_traps > 0)
      //          _fs->AddInstruction(_OP_POPTRAP, _fs->_traps, 0);
      //        _fs->_returnexp = -1;
      //        _fs->AddInstruction(op, 0xFF, 0, _fs->GetStackSize());

      add_instruction<op_return>(_ccs->get_stack_size(), true);
    }

    return {};
  }

  case tok_lcrlbracket: {

    scope previous_scope = _scope;
    _scope.n_captures = _ccs->_n_capture;
    _scope.stack_size = _ccs->get_stack_size();

    lex();

    while (is_not(tok_rcrlbracket, tok_default, tok_case)) {
      ZS_COMPILER_PARSE(p_statement, true);

      if (_lexer->_last_token != tok_rcrlbracket && _lexer->_last_token != tok_semi_colon) {
        //            OptionalSemicolon();
        ZS_COMPILER_PARSE(p_semi_colon);
      }
    }

    ZS_COMPILER_EXPECT(tok_rcrlbracket);

    if (close_frame) {

      int_t previous_n_capture = _ccs->_n_capture;

      if (_ccs->get_stack_size() != _scope.stack_size) {
        _ccs->set_stack_size(_scope.stack_size);
        if (previous_n_capture != (int_t)_ccs->_n_capture) {
          add_instruction<op_close>((uint32_t)_scope.stack_size);
        }
      }
    }
    else {
      if (_ccs->get_stack_size() != _scope.stack_size) {
        _ccs->set_stack_size(_scope.stack_size);
      }
    }
    _scope = previous_scope;

    return {};
  }

  default:
    //      zb::print("DFSJDKSJKDFS", _token, _lexer->get_value());
    ZS_COMPILER_PARSE(p_comma);
    //

    // @alex
    //-------------------------------------------------------------
    //    _ccs->pop_target();
    //
    //      _fs->DiscardTarget();
    return {};
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_variable_type_restriction>(
    zb::ref_wrapper<uint32_t> mask, zb::ref_wrapper<uint64_t> custom_mask) {
  using enum token_type;

  if (is_not(tok_lt)) {
    return zs::error_code::invalid;
  }

  mask.get() = 0;
  custom_mask.get() = 0;
  //  mask = 0;
  //  custom_mask = 0;
  int count = 0;
  int comma_count = 0;

  while (lex() != tok_gt) {

    switch (_token) {
    case tok_comma:
      if (count == 0) {
        zb::print("ERROR: Invalid comma");
        return zs::error_code::invalid_comma;
      }

      if (++comma_count != count) {
        zb::print("ERROR: Invalid comma");
        return zs::error_code::invalid_comma;
      }
      break;

    case tok_int:
    case tok_char:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_integer;
      break;

    case token_type::tok_float:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_float;
      break;

    case token_type::tok_bool:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_bool;
      break;

    case token_type::tok_string:
      count++;
      mask |= (uint32_t)zs::object_base::k_string_mask;
      break;

    case token_type::tok_array:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_array;
      break;

    case token_type::tok_table:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_table;
      break;

    case token_type::tok_null:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_null;
      break;

    case token_type::tok_exttype:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_extension;
      break;

    case token_type::tok_identifier: {
      count++;

      mask |= (uint32_t)zs::object_type_mask::k_table;
      mask |= (uint32_t)zs::object_type_mask::k_instance;
      mask |= (uint32_t)zs::object_type_mask::k_user_data;

      object name(_engine, _lexer->get_identifier_value());
      int_t type_index;

      ZS_RETURN_IF_ERROR(_ccs->get_restricted_type_index(name, type_index));

      custom_mask |= (uint64_t)(1 << type_index);
      break;
    }

    default:
      zb::print("ERROR: Invalid token");
      return zs::error_code::invalid_type;
    }
  }

  if (is_not(tok_gt)) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expected var<...>");
  }

  lex();

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_table_or_class>(token_type separator, token_type terminator) {
  using enum token_type;
  using enum opcode;

  //  int_t tpos = _fs->GetCurrentPos(), nkeys = 0;
  while (_token != terminator) {
    //    bool hasattrs = false;
    zs::object identifier;
    bool is_static = false;
    bool is_small_string_identifier = false;

    //    // check if is an attribute
    if (separator == tok_semi_colon) {
      //      if (_token == TK_ATTR_OPEN) {
      //        _fs->AddInstruction(_OP_NEWOBJ, _fs->PushTarget(), 0, 0,
      //        NOT_TABLE); Lex(); ParseTableOrClass(',', TK_ATTR_CLOSE); hasattrs
      //        = true;
      //      }
      if (is(tok_static)) {
        is_static = true;
        lex();
      }
    }

    switch (_token) {
    case tok_function:
      ZBASE_NO_BREAK;
    case tok_constructor: {

      int_t bound_target = 0xFF;
      bool is_constructor = is(tok_constructor);
      //      zb::print(_lexer->get_identifier());
      lex();

      zs::object var_name;

      if (is_constructor) {

        var_name = zs::_ss("constructor");
      }
      else {
        ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
      }
      //    if (_token == tok_lcrlbracket) {
      //            boundtarget = ParseBindEnv();
      //    }

      ZS_RETURN_IF_ERROR(add_string_instruction(var_name));

      //      add_instruction<op_load>(_ccs->new_target(), (uint32_t)_ccs->get_literal(ret_value));
      //      _fs->AddInstruction(_OP_LOAD, _ccs->new_target(), _fs->GetConstant(id));

      ZS_COMPILER_EXPECT(tok_lbracket);

      ZS_COMPILER_PARSE(p_create_function, CREF(var_name), bound_target, false);

      //      CreateFunction(varname, 0xFF, false);

      add_new_target_instruction<op_new_closure>(
          (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

      break;
    }

      //    case TK_FUNCTION:
      //    case TK_CONSTRUCTOR: {
      //      SQInteger tk = _token;
      //      Lex();
      //      SQObject id = tk == TK_FUNCTION ? Expect(TK_IDENTIFIER) :
      //      _fs->CreateString(_SC("constructor"));
      //      _fs->AddInstruction(_OP_LOAD, _fs->PushTarget(),
      //      _fs->GetConstant(id)); SQInteger boundtarget = 0xFF; if (_token ==
      //      _SC('[')) {
      //        boundtarget = ParseBindEnv();
      //      }
      //      Expect(_SC('('));
      //
      //      CreateFunction(id, boundtarget);
      //      _fs->AddInstruction(_OP_CLOSURE, _fs->PushTarget(),
      //      _fs->_functions.size() - 1, boundtarget);
      //    } break;

    case tok_lsqrbracket: {
      lex();
      ZS_COMPILER_PARSE(p_comma);
      ZS_COMPILER_EXPECT(tok_rsqrbracket);
      ZS_COMPILER_EXPECT(tok_eq);
      ZS_COMPILER_PARSE(p_expression);

      break;
    }

    case tok_string_value:
    case tok_escaped_string_value: {
      // Only works for tables
      if (separator == tok_comma) {
        zs::object value;
        ZS_COMPILER_EXPECT_GET(tok_string_value, value);
        ZS_RETURN_IF_ERROR(add_string_instruction(value));
        ZS_COMPILER_EXPECT(tok_colon);
        ZS_COMPILER_PARSE(p_expression);
        break;
      }
    }
    default:
      ZS_COMPILER_EXPECT_GET(tok_identifier, identifier);

      if ((identifier.is_small_string()
              || identifier.get_string_unchecked().size() <= constants::k_small_string_max_size)
          and separator == tok_comma) {
        is_small_string_identifier = true;
      }
      else {
        ZS_RETURN_IF_ERROR(add_string_instruction(identifier));
      }

      ZS_COMPILER_EXPECT(tok_eq);
      ZS_COMPILER_PARSE(p_expression);
    }

    if (_token == separator) {
      // optional comma/semicolon
      lex();
    }

    // hack recognizes a table from the separator
    if (separator == tok_comma) {
      if (is_small_string_identifier) {
        int_t val = _ccs->pop_target();
        int_t table = _ccs->top_target();
        add_instruction<op_new_slot_ss>(
            (uint8_t)table, zs::small_string_instruction_data::create(identifier), (uint8_t)val);
      }
      else {
        int_t val = _ccs->pop_target();
        int_t key = _ccs->pop_target();
        int_t table = _ccs->top_target();
        add_instruction<op_new_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val);
      }
    }
    else {
      int_t val = _ccs->pop_target();
      int_t key = _ccs->pop_target();
      int_t table = _ccs->top_target();
      // This for classes only as it invokes _newmember.
      add_instruction<op_new_class_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val, is_static);
    }
  }

  // hack recognizes a table from the separator
  if (separator == tok_comma) {
    //    _fs->SetInstructionParam(tpos, 1, nkeys);
  }
  lex();

  return {};
}

// Table.
ZS_JIT_COMPILER_PARSE_OP(p_table) {

  while (is_not(tok_rcrlbracket)) {
    zs::object key;
    bool is_small_string_key = false;

    int_t inst_idx = -1;
    int_t nextinst_idx = -1;

    switch (_token) {
    case tok_constructor:
      return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "A table cannot have a constructor.");

    case tok_function: {
      lex();
      ZS_COMPILER_EXPECT_GET(tok_identifier, key);

      if (!(is_small_string_key = is_small_string_identifier(key))) {
        ZS_RETURN_IF_ERROR(add_string_instruction(key));
      }

      int_t bound_target = 0xFF;
      if (is(tok_lsqrbracket)) {
        ZS_COMPILER_PARSE(p_bind_env, REF(bound_target));
      }

      ZS_COMPILER_EXPECT(tok_lbracket);
      ZS_COMPILER_PARSE(p_create_function, CREF(key), bound_target, false);

      add_new_target_instruction<op_new_closure>(
          (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);
      break;
    }

    case tok_lsqrbracket: {
      lex();
      ZS_COMPILER_PARSE(p_comma);
      ZS_COMPILER_EXPECT(tok_rsqrbracket);
      ZS_COMPILER_EXPECT(tok_eq);

      inst_idx = _ccs->get_instruction_index();
      nextinst_idx = _ccs->get_next_instruction_index();
      ZS_COMPILER_PARSE(p_expression);
      break;
    }

    case tok_string_value:
    case tok_escaped_string_value: {
      key = _lexer->get_value();
      lex();

      if (!(is_small_string_key = is_small_string_identifier(key))) {
        ZS_RETURN_IF_ERROR(add_string_instruction(key));
      }

      ZS_COMPILER_EXPECT(tok_colon);
      inst_idx = _ccs->get_instruction_index();
      nextinst_idx = _ccs->get_next_instruction_index();
      ZS_COMPILER_PARSE(p_expression);
      break;
    }

    default:
      ZS_COMPILER_EXPECT_GET(tok_identifier, key);

      if (!(is_small_string_key = is_small_string_identifier(key))) {
        ZS_RETURN_IF_ERROR(add_string_instruction(key));
      }

      ZS_COMPILER_EXPECT(tok_eq);

      inst_idx = _ccs->get_instruction_index();
      nextinst_idx = _ccs->get_next_instruction_index();
      ZS_COMPILER_PARSE(p_expression);
    }

    // Optional comma.
    lex_if(tok_comma);

    if (is_small_string_key) {
      zs::small_string_instruction_data key_ss_data = zs::small_string_instruction_data::create(key);
      uint8_t value_target = _ccs->pop_target();
      uint8_t top_target = _ccs->top_target();

      if (_ccs->get_instruction_index() == nextinst_idx) {

        zs::opcode last_op = _ccs->_instructions.get_opcode(nextinst_idx);

        switch (last_op) {
        case op_load_int: {
          // Get the value from the instruction before deleting it.
          int_t int_value = _ccs->_instructions.get_ref<op_load_int>(nextinst_idx).value;

          // Remove the last instruction.
          _ccs->_instructions._data.resize(nextinst_idx);

          add_instruction<op_new_slot_ss_integer>(top_target, key_ss_data, int_value);
          continue;
        }

        case op_load_float: {
          // Get the value from the instruction before deleting it.
          float_t float_value = _ccs->_instructions.get_ref<op_load_float>(nextinst_idx).value;

          // Remove the last instruction.
          _ccs->_instructions._data.resize(nextinst_idx);

          add_instruction<op_new_slot_ss_float>(top_target, key_ss_data, float_value);
          continue;
        }

        case op_load_bool: {
          // Get the value from the instruction before deleting it.
          bool_t bool_value = _ccs->_instructions.get_ref<op_load_bool>(nextinst_idx).value;

          // Remove the last instruction.
          _ccs->_instructions._data.resize(nextinst_idx);

          add_instruction<op_new_slot_ss_bool>(top_target, key_ss_data, bool_value);
          continue;
        }

        case op_load_small_string: {
          // Get the value from the instruction before deleting it.
          zs::small_string_instruction_data ss_value
              = _ccs->_instructions.get_ref<op_load_small_string>(nextinst_idx).value;

          // Remove the last instruction.
          _ccs->_instructions._data.resize(nextinst_idx);

          add_instruction<op_new_slot_ss_small_string>(top_target, key_ss_data, ss_value);
          continue;
        }
        }
      }

      add_instruction<op_new_slot_ss>(top_target, key_ss_data, value_target);
    }
    else {
      int_t val = _ccs->pop_target();
      int_t key = _ccs->pop_target();
      add_instruction<op_new_slot>((uint8_t)_ccs->top_target(), (uint8_t)key, (uint8_t)val);
    }
  }

  lex();

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_class>() {
  using enum token_type;

  //  int_t base = -1;
  //  int_t attrs = -1;
  if (is(tok_extend)) {
    lex();
    ZS_COMPILER_PARSE(p_expression);
    //    base = _fs->TopTarget();
  }

  if (is(tok_attribute_begin)) {
    lex();
    //    _fs->AddInstruction(_OP_NEWOBJ, _fs->PushTarget(), 0, 0, NOT_TABLE);
    ZS_COMPILER_PARSE(p_table_or_class, tok_comma, tok_attribute_end);
    //    attrs = _fs->TopTarget();
  }

  ZS_COMPILER_EXPECT(tok_lcrlbracket);

  //  if (attrs != -1)
  //    _fs->PopTarget();
  //  if (base != -1)
  //    _fs->PopTarget();
  //  _fs->AddInstruction(_OP_NEWOBJ, _fs->PushTarget(), base, attrs,
  //  NOT_CLASS);
  add_new_target_instruction<op_new_obj>(object_type::k_class);
  //  lex();
  ZS_COMPILER_PARSE(p_table_or_class, tok_semi_colon, tok_rcrlbracket);

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_class_statement>() {
  using enum token_type;
  using enum opcode;

  lex();
  expr_state es = _estate;
  _estate.no_get = true;

  // Check if the class is declared as `class var_name {` or `class something.var_name`.
  // The second condition is somehow a hack, it creates a new lexer from this one and lex the next token.
  // It would be nice a have a method in the lexer to peek at the next token without incrementing.
  const bool is_local = is(tok_identifier) and (((zs::lexer)*_lexer).lex() == tok_lcrlbracket);

  if (is_local) {
    zs::object var_name;
    ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
    ZS_COMPILER_PARSE(p_class);
    _ccs->pop_target();
    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
  }
  else {
    ZS_COMPILER_PARSE(p_prefixed, );

    switch (_estate.type) {
    case expr_type::e_expr:
      return ZS_COMPILER_ERROR(zs::error_code::invalid_operation, "Invalid class name");

    case expr_type::e_base:
      ZBASE_NO_BREAK;
    case expr_type::e_object: {
      ZS_COMPILER_PARSE(p_class, );

      int_t val = _ccs->pop_target();
      int_t key = _ccs->pop_target();
      //      int_t table = _ccs->top_target();
      int_t table = _ccs->pop_target();

      add_instruction<op_new_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val);

      //        _ccs->pop_target();
      //        ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));

      //      _ccs->pop_target();
      break;
    }

    case expr_type::e_local:
      ZBASE_NO_BREAK;
    case expr_type::e_capture:

      return ZS_COMPILER_ERROR(zs::error_code::invalid_operation,
          "Cannot create a class in a local with the syntax(class <local>)");
    }
  }

  //

  _estate = es;
  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_include_or_import_statement, token_type tok) {
  using enum token_type;
  const bool is_import = tok == tok_import;

  ZS_COMPILER_EXPECT(tok);

  const zs::line_info linfo = _lexer->get_line_info();

  object file_name = _lexer->get_value();

  if (_token == tok_lt) {
    lex();
    file_name = _lexer->get_value();
    lex();
    if (_token != tok_gt) {
      _error_message += zs::sstrprint(_engine, "parse include statement", linfo);
      return zs::error_code::invalid_include_syntax;
    }
  }

  if (!file_name.is_string()) {
    _error_message += zs::sstrprint(_engine, "parse include statement", linfo);
    return zs::error_code::invalid_include_syntax;
  }

  object res_file_name;
  if (auto err = _engine->resolve_file_path(file_name.get_string_unchecked(), res_file_name)) {
    _error_message += zs::sstrprint(_engine, "parse include statement", linfo);
    return zs::error_code::invalid_include_file;
  }

  if (is_import) {
    // Check for multiple inclusion.
    if (auto it = _ccs->_sdata._imported_files_set.find(res_file_name);
        it != _ccs->_sdata._imported_files_set.end()) {
      // Already imported, all good.
      lex();
      return {};
    }

    _ccs->_sdata._imported_files_set.insert(res_file_name);
  }

  zs::file_loader loader(_engine);

  zbase_assert(!res_file_name.is_string_view(), "cannot be a string_view");

  if (auto err = loader.open(res_file_name.get_string_unchecked().data())) {
    _error_message += zs::sstrprint(_engine, "parse include statement", linfo);
    return zs::error_code::open_file_error;
  }

  zs::lexer* last_lexer = _lexer;
  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(loader.content());

  lex();
  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    ZS_COMPILER_PARSE(p_statement, true);

    if (!zb::is_one_of(_lexer->last_token(), tok_rcrlbracket, tok_semi_colon)) {
      ZS_COMPILER_PARSE(p_semi_colon, );
    }
  }

  if (_token == tok_lex_error) {

    last_lexer->_current_token = _lexer->_current_token;
    last_lexer->_last_token = _lexer->_last_token;
    _lexer = last_lexer;

    return zs::error_code::invalid;
  }

  last_lexer->_current_token = _lexer->_current_token;
  last_lexer->_last_token = _lexer->_last_token;

  _lexer = last_lexer;

  lex();

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_bind_env>(zb::ref_wrapper<int_t> target) {

  lex();

  // TODO: Fix this.
  ZS_COMPILER_PARSE(p_expression, );
  int_t boundtarget = _ccs->top_target();

  target.get() = boundtarget;

  ZS_COMPILER_EXPECT(tok_rsqrbracket);
  //    Expect(_SC(']'));
  //  return boundtarget;
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_factor_identifier>() {
  using enum token_type;

  object var_name = _lexer->get_identifier();

  lex();

  // Check if `var_name` is a local variable.
  if (int_t pos = _ccs->find_local_variable(var_name); pos != -1) {
    _estate.type = expr_type::e_local;
    _estate.pos = pos;
    _ccs->push_target(pos);
    return {};
  }

  if (_ccs->is_exported_name(var_name)) {

    if (will_modify()) {
      return ZS_COMPILER_ERROR(
          zs::error_code::cant_modify_export_table, "export table cannot be modified.\n");
    }

    _ccs->push_export_target();
    ZS_RETURN_IF_ERROR(add_string_instruction(var_name));

    //    _estate.no_assign = true;

    if (needs_get()) {
      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->pop_target();
      add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, true);
      _estate.type = expr_type::e_object;
      _estate.pos = table_idx;

      // TODO: ???
      //    target.get() = table_idx;
    }
    else {

      //    if (is_not(tok_lbracket)) {
      //      _error_message += zs::strprint(_engine, "Trying to assign a global variable",
      //      _lexer->get_line_info()); return zs::error_code::inaccessible;
      //    }

      // We are calling a function or an operator.
      // The key is on top on the stack and the table under.
      // For a normal function call, this should bring us to the `case
      // tok_lbracket:` right under.
      _estate.type = expr_type::e_object;
      _estate.pos = _ccs->top_target() - 1;

      // TODO: ???
      //    target.get() = _ccs->top_target() - 1;
    }

    return {};
  }

  //
  if (_ccs->is_captured_exported_name(var_name)) {
    const int_t export_pos = _ccs->get_capture(zs::_ss("__exports__"));
    if (export_pos == -1) {
      return ZS_COMPILER_ERROR(zs::error_code::inaccessible, "module table is inaccessible.\n");
    }

    add_new_target_instruction<op_get_capture>((uint32_t)export_pos);

    ZS_RETURN_IF_ERROR(add_string_instruction(var_name));

    //    needs_get_no_assign();

    if (needs_get_no_assign()) {
      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->pop_target();
      add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, true);
      _estate.type = expr_type::e_object;
      _estate.pos = table_idx;

      // TODO: ???
      //    target.get() = table_idx;
    }
    else {

      //    if (is_not(tok_lbracket)) {
      //      _error_message += zs::strprint(_engine, "Trying to assign a global variable",
      //      _lexer->get_line_info()); return zs::error_code::inaccessible;
      //    }

      // We are calling a function or an operator.
      // The key is on top on the stack and the table under.
      // For a normal function call, this should bring us to the `case
      // tok_lbracket:` right under.
      _estate.type = expr_type::e_object;
      _estate.pos = _ccs->top_target() - 1;

      // TODO: ???
      //    target.get() = _ccs->top_target() - 1;
    }

    return {};
  }

  if (int_t pos = _ccs->get_capture(var_name); pos != -1) {
    // Handle a captured var.
    if (needs_get()) {
      _estate.pos = _ccs->new_target();
      _estate.type = expr_type::e_expr;
      add_instruction<op_get_capture>((uint8_t)_estate.pos, (uint32_t)pos);
      return {};
    }

    // We are calling a function or an operator.
    // The capture pos is on top on the stack.
    // For a normal function call, this should bring us to the `case
    // tok_lbracket:` right under.
    _estate.type = expr_type::e_capture;
    _estate.pos = pos;

    // TODO: Not sure about this.
    //    target.get() = pos;
    return {};
  }

  // TODO: Check for const value.

  // The variable (`var_name`) is not local. Handle a non-local variable, aka a
  // field. Push the 'this' pointer on the virtual stack (always found in offset
  // 0, so no instruction needs to be generated), and push the key next.
  //
  // Generate an _OP_LOAD instruction for the latter.
  // If we are not using the variable as a dref expr, generate the _OP_GET
  // instruction.

  _ccs->push_target(0);
  ZS_RETURN_IF_ERROR(add_string_instruction(var_name));

  if (needs_get_no_assign()) {
    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->pop_target();
    add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, true);
    _estate.type = expr_type::e_object;
    _estate.pos = table_idx;

    // TODO: ???
    //    target.get() = table_idx;
  }
  else {

    //    if (is_not(tok_lbracket)) {
    //      _error_message += zs::strprint(_engine, "Trying to assign a global variable",
    //      _lexer->get_line_info()); return zs::error_code::inaccessible;
    //    }

    // We are calling a function or an operator.
    // The key is on top on the stack and the table under.
    // For a normal function call, this should bring us to the `case
    // tok_lbracket:` right under.
    _estate.type = expr_type::e_object;
    _estate.pos = _ccs->top_target() - 1;

    // TODO: ???
    //    target.get() = _ccs->top_target() - 1;
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_factor_at>() {
  using enum token_type;

  token_type last_token = _lexer->_last_token;
  lex();

  if (is_not(tok_lbracket)) {
    return parse<p_macro_call>(last_token);
  }

  _ccs->push_target(0);
  ZS_RETURN_IF_ERROR(add_string_instruction(std::string_view("tostring")));

  if (needs_get()) {
    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->pop_target();
    add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, true);
    _estate.type = expr_type::e_object;
    _estate.pos = table_idx;

    // TODO: ???
    //    target.get() = table_idx;
  }
  else {

    if (is_not(tok_lbracket)) {
      _error_message += zs::sstrprint(_engine, "Trying to assign a global variable", _lexer->get_line_info());
      return zs::error_code::inaccessible;
    }
    // We are calling a function or an operator.
    // The key is on top on the stack and the table under.
    // For a normal function call, this should bring us to the `case
    // tok_lbracket:` right under.
    _estate.type = expr_type::e_object;
    _estate.pos = _ccs->top_target() - 1;

    // TODO: ???
    //    target.get() = _ccs->top_target() - 1;
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_prefixed>() {
  using enum token_type;

  //  int_t pos = -1;
  ZS_COMPILER_PARSE(p_factor, );

  zb_loop() {

    if (is(tok_lt) and is_template_function_call()) {
      ZS_COMPILER_PARSE(p_prefixed_lbracket_template, );
    }

    switch (_token) {
    case tok_dot: {
      lex();

      object var_name = object(_engine, _lexer->get_identifier_value());

      ZS_COMPILER_EXPECT(tok_identifier);

      ZS_RETURN_IF_ERROR(add_string_instruction(var_name));

      if (is(tok_eq)) {
        if (_estate.no_assign) {
          return ZS_COMPILER_ERROR(zs::error_code::invalid_operation,
              "cannot assign a global variable without the global keyword");
        }

        int_t table_idx = _ccs->get_up_target(1);
        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
        _estate.no_assign = false;
      }
      else if (needs_get()) {
        int_t key_idx = _ccs->pop_target();
        int_t table_idx = _ccs->pop_target();
        add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, false);
        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
        _estate.no_assign = false;
      }
      else {
        // We are calling a function or an operator.
        // The key is on top on the stack and the table under.
        // For a normal function call, this should bring us to the `case
        // tok_lbracket:` right under.
        _estate.type = expr_type::e_object;
        _estate.pos = _ccs->top_target() - 1;
      }
      break;
    }

    case tok_lbracket: {
      // `(`: We're calling a function.

      //      if (pos != _estate.pos and (pos == -1 or _estate.pos == -1)) {
      //        zb::print("klklk---", pos, _estate.pos, _token);
      //      }
      ZS_COMPILER_PARSE(p_prefixed_lbracket, );
      break;
    }

    case tok_lsqrbracket: {
      if (_lexer->last_token() == tok_endl) {
        return ZS_COMPILER_ERROR(zs::error_code::invalid_token,
            "cannot break deref/or comma needed after [exp]=exp slot declaration");
      }

      lex();
      ZS_COMPILER_PARSE(p_expression, );
      ZS_COMPILER_EXPECT(tok_rsqrbracket);

      if (is(tok_eq)) {
        int_t table_idx = _ccs->get_up_target(1);
        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
      }
      else if (needs_get()) {
        int_t key_idx = _ccs->pop_target();
        int_t table_idx = _ccs->pop_target();
        add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, false);
        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
      }
      else {
        // We are calling a function or an operator.
        // The key is on top on the stack and the table under.
        // For a normal function call, this should bring us to the `case
        // tok_lbracket:` right under.
        _estate.type = expr_type::e_object;
        _estate.pos = _ccs->top_target() - 1;
      }

      break;
    }

    case tok_decr:
    case tok_incr: {
      bool is_incr = is(tok_incr);

      if (is_end_of_statement()) {
        return {};
      }

      lex();

      switch (_estate.type) {
      case expr_type::e_expr:
        return ZS_COMPILER_ERROR(zs::error_code::invalid_operation, "Can't '++' or '--' an expression");

      case expr_type::e_base:
        zb::print("ERROR");
        //                      Error(_SC("'base' cannot be modified"));
        break;
      case expr_type::e_object:
        if (_estate.no_get == true) {
          zb::print("ERROR");
          //                        Error(_SC("can't '++' or '--' an expression"));
          break;
        } // mmh dor this make sense?
        //                      Emit2ArgsOP(_OP_PINC, diff);
        break;
      case expr_type::e_local: {
        int_t src = _ccs->pop_target();
        add_new_target_instruction<op_incr>(src, is_incr);
      } break;
      case expr_type::e_capture: {
        zb::print("ERROR");
        //                      SQInteger tmp1 = _fs->PushTarget();
        //                      SQInteger tmp2 = _fs->PushTarget();
        //                      _fs->AddInstruction(_OP_GETOUTER, tmp2, _es.epos);
        //                      _fs->AddInstruction(_OP_PINCL, tmp1, tmp2, 0, diff);
        //                      _fs->AddInstruction(_OP_SETOUTER, tmp2, _es.epos, tmp2);
        //                      _fs->PopTarget();
      }
      }

      break;
      ;
    }
    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_factor>() {
  using enum token_type;
  using enum opcode;

  switch (_token) {
  case tok_double_at: {
    lex();

    object var_name;
    ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);

    object ret_value;
    ZS_RETURN_IF_ERROR(_compile_time_consts._table->get(var_name, ret_value));

    switch (ret_value.get_type()) {
    case object_type::k_null:
      break;

    case object_type::k_bool:
      add_new_target_instruction<op_load_bool>(ret_value._bool);
      break;

    case object_type::k_integer:
      add_new_target_instruction<op_load_int>(ret_value._int);
      break;

    case object_type::k_float:
      add_new_target_instruction<op_load_float>(ret_value._float);
      break;

    case object_type::k_small_string:
      ZS_RETURN_IF_ERROR(add_string_instruction(ret_value));
      break;

    case object_type::k_long_string:
      ZS_RETURN_IF_ERROR(add_string_instruction(ret_value));
      break;

    case object_type::k_string_view:
      ZS_RETURN_IF_ERROR(add_string_instruction(ret_value));
      break;

    default:
      return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "define todo table ...");
    }

    break;
  }

  case tok_string_value: {
    std::string_view svalue = _lexer->get_string_value();
    ZS_RETURN_IF_ERROR(add_string_instruction(svalue));
    lex();
    break;
  }

  case tok_escaped_string_value: {
    std::string_view svalue = _lexer->get_escaped_string_value();
    ZS_RETURN_IF_ERROR(add_string_instruction(svalue));
    lex();
    break;
  }

  case tok_typeid: {
    lex();

    if (!lex_if(tok_lbracket)) {
      return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expected '(' after typeid");
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse<p_expression>()) {
        zb::print("ERRRO");
        return err;
      }
    }

    int_t tg = _ccs->pop_target();
    add_new_target_instruction<op_typeid>(tg);

    lex();
    break;
  }

  case tok_typeof: {
    lex();

    if (!lex_if(tok_lbracket)) {
      return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expected '(' after typeof");
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse<p_expression>()) {
        zb::print("ERRRO");
        return err;
      }
    }

    int_t tg = _ccs->pop_target();
    add_new_target_instruction<op_typeof>(tg);

    lex();
    break;
  }

  case tok_double_colon: {

    // Should we load the root or the 'this' table?

    if (_ccs->get_parent()) {
      //      _ccs->push_target(0);
      add_new_target_instruction<op_load_root>();
    }
    else {
      add_new_target_instruction<op_load_root>();
    }

    //

    //
    //      add_instruction<op_move>(_ccs->new_target(), (uint8_t)0);
    _estate.type = expr_type::e_object;
    _token = tok_dot; // hack: drop into PrefixExpr, case '.'
    _estate.pos = _ccs->top_target();
    //    target.get() = _estate.pos;

    _estate.no_assign = true;
    //    _estate.no_get = true;
    return {};
  }

  case tok_import: {
    lex();

    _ccs->push_target(0);

    ZS_RETURN_IF_ERROR(add_small_string_instruction("import"));

    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->pop_target();
    add_new_target_instruction<op_get>((uint8_t)table_idx, (uint8_t)key_idx, true);

    _estate.type = expr_type::e_object;

    break;
  }

  case tok_base: {
    lex();
    add_new_target_instruction<op_get_base>();
    _estate.type = expr_type::e_base;
    _estate.pos = _ccs->top_target();
    //    target.get() = _ccs->top_target();
    return {};
  }

  case tok_this: {
    return parse<p_factor_identifier>();
  }
  case tok_global: {
    add_new_target_instruction<op_load_global>();

    lex();
    _estate.type = expr_type::e_object;
    _estate.pos = _ccs->top_target();
    _estate.no_assign = false;
    //    target.get() = -1;
    return {};
  }

  case tok_constructor:
    return zs::error_code::invalid_token;

  case tok_identifier:
    return parse<p_factor_identifier>();

  case tok_at:
    return parse<p_factor_at>();

  case tok_null:
    add_new_target_instruction<op_load_null>();
    lex();
    break;

  case tok_none:
    add_new_target_instruction<op_load_none>();
    lex();
    break;

  case tok_integer_value:
    add_new_target_instruction<op_load_int>(_lexer->get_int_value());
    lex();
    break;

  case tok_float_value:
    add_new_target_instruction<op_load_float>(_lexer->get_float_value());
    lex();
    break;

  case tok_true:
    add_new_target_instruction<op_load_bool>(true);
    lex();
    break;

  case tok_false:
    add_new_target_instruction<op_load_bool>(false);
    lex();
    break;

  case tok_minus: {
    lex();

    switch (_token) {
    case tok_integer_value:
      add_new_target_instruction<op_load_int>(-_lexer->get_int_value());
      lex();
      break;

    case tok_float_value:
      add_new_target_instruction<op_load_float>(-_lexer->get_float_value());
      lex();
      break;

    default:
      // UnaryOP(_OP_NEG);
      return ZS_COMPILER_ERROR(zs::error_code::unimplemented, "unimplemented unary minus");

      break;
    }
    break;
  }

  case tok_lsqrbracket: {
    // Array.
    add_new_target_instruction<op_new_obj>(object_type::k_array);
    lex();

    while (is_not(tok_rsqrbracket)) {
      if (auto err = parse<p_expression>()) {
        //          return err;
        zb::print("ERRRO");
        return err;
      }

      if (_token == tok_comma) {
        lex();
      }

      int_t val = _ccs->pop_target();
      int_t array = _ccs->top_target();

      add_instruction<op_array_append>((uint8_t)array, (uint8_t)val);

      //              _fs->AddInstruction(_OP_APPENDARRAY, array, val,
      //              AAT_STACK);

      //              key++;
    }

    lex();
    //      _fs->AddInstruction(_OP_NEWOBJ, _fs->PushTarget(), 0, 0, NOT_ARRAY);
    //      SQInteger apos = _fs->GetCurrentPos(), key = 0;
    //      Lex();
    //      while (_token != _SC(']')) {
    //        Expression();
    //        if (_token == _SC(','))
    //          Lex();
    //        SQInteger val = _fs->PopTarget();
    //        SQInteger array = _fs->TopTarget();
    //        _fs->AddInstruction(_OP_APPENDARRAY, array, val, AAT_STACK);
    //        key++;
    //      }
    //      _fs->SetInstructionParam(apos, 1, key);
    //      Lex();
    break;
  }

  case tok_struct: {
    lex();
    ZS_COMPILER_PARSE(p_struct, );
    break;
  }

  case tok_lcrlbracket: {
    add_new_target_instruction<op_new_obj>(object_type::k_table);
    lex();
    ZS_COMPILER_PARSE(p_table, );
    break;
  }

  case tok_dollar: {
    ZS_COMPILER_PARSE(p_function, true);
    break;
  }
  case tok_function: {
    ZS_COMPILER_PARSE(p_function, false);
    break;
  }

  case tok_class: {
    lex();
    ZS_COMPILER_PARSE(p_class, );
    break;
  }

    //  case tok_not:
    //    lex();
    //
    //    switch (_token) {
    //    case tok_false:
    //      add_instruction<op_load_bool>(_ccs->new_target(), true);
    //      lex();
    //      break;
    //    case tok_true:
    //      add_instruction<op_load_bool>(_ccs->new_target(), false);
    //      lex();
    //      break;
    //    case tok_integer_value:
    //      add_instruction<op_load_bool>(_ccs->new_target(), !(bool)_lexer->get_int_value());
    //      lex();
    //      break;
    //
    //    case tok_float_value:
    //      add_instruction<op_load_bool>(_ccs->new_target(), !(bool)_lexer->get_float_value());
    //      lex();
    //      break;
    //
    //    case tok_identifier:
    //      add_instruction<op_not>(
    //          _ccs->new_target(), _ccs->find_local_variable(_lexer->get_identifier()));
    //      lex();
    //      break;
    //
    //    default:
    //      // UnaryOP(_OP_NEG);
    //      return handle_error(
    //          this, zs::error_code::unimplemented, "unimplemented unary minus",
    //          ZB_CURRENT_SOURCE_LOCATION());
    //
    //      break;
    //    }
    //    break;

  case tok_inv:
    break;

  case tok_not: {

    lex();
    expr_state es = _estate;
    _estate.no_get = true;

    ZS_COMPILER_PARSE(p_prefixed, );

    switch (_estate.type) {
    case expr_type::e_expr: {
      int_t src = _ccs->pop_target();
      add_new_target_instruction<op_not>((uint8_t)src);
      break;
      ;
    }

    case expr_type::e_base:
      return ZS_COMPILER_ERROR(zs::error_code::invalid_operation, "Can't '++' or '--' a base");

    case expr_type::e_object: {
      //        int_t key_idx = _ccs->pop_target();
      //        int_t table_idx = _ccs->pop_target();

      //        add_instruction<op_pobjincr>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx,
      //        is_incr);
      return zs::error_code::unimplemented;
      break;
      ;
    }

    case expr_type::e_local: {
      int_t src = _ccs->pop_target();
      add_new_target_instruction<op_not>((uint8_t)src);
      //      SQInteger src = _fs->TopTarget();
      //      _fs->AddInstruction(_OP_INCL, src, src, 0, diff);
      break;
    }

    case expr_type::e_capture:
      //      SQInteger tmp = _fs->PushTarget();
      //      _fs->AddInstruction(_OP_GETOUTER, tmp, _es.epos);
      //      _fs->AddInstruction(_OP_INCL, tmp, tmp, 0, diff);
      //      _fs->AddInstruction(_OP_SETOUTER, tmp, _es.epos, tmp);
      return zs::error_code::unimplemented;
    }

    _estate = es;
    return {};
  } break;

  case tok_decr:
    ZS_COMPILER_PARSE(p_prefixed_incr, false);
    break;

  case tok_incr:
    ZS_COMPILER_PARSE(p_prefixed_incr, true);
    break;

  case tok_lbracket: {
    lex();

    ZS_COMPILER_PARSE(p_comma);

    if (!lex_if(tok_rbracket)) {
      return ZS_COMPILER_ERROR(zs::error_code::unimplemented, "expression expected");
    }
    break;
  }

  case tok_file:
    ZS_RETURN_IF_ERROR(add_string_instruction(_ccs->_sdata._source_name));
    lex();
    break;

  case tok_line: {
    add_new_target_instruction<op_load_int>(_lexer->_current_line);
    lex();
    break;
  }

  case tok_line_str: {
    const auto& stream = _lexer->_stream;
    const char* begin = &(*stream._data.begin());
    const char* end = &(*stream._data.end());

    const char* it_line_begin = stream.ptr() - 1;
    while (it_line_begin > begin) {
      if (*it_line_begin == '\n') {
        ++it_line_begin;
        break;
      }

      --it_line_begin;
    }

    const char* it_line_end = stream.ptr();
    while (it_line_end < end) {
      if (*it_line_end == '\n') {
        break;
      }

      ++it_line_end;
    }

    std::string_view line_content(it_line_begin, std::distance(it_line_begin, it_line_end));

    ZS_RETURN_IF_ERROR(add_string_instruction(line_content));

    //        add_instruction<op_load_int>(_ccs->new_target(), _lexer->_current_line);
    lex();
    break;
  }
  case tok_hastag: {
    lex();

    object id;
    ZS_COMPILER_EXPECT_GET(tok_identifier, id);

    if (id == "as_string") {
      // Get the content of a file as string.
      ZS_COMPILER_PARSE(p_as_string);
    }
    else if (id == "as_table") {
      // Get the content of a file as table.
      ZS_COMPILER_PARSE(p_as_table);
    }
    else if (id == "as_value") {
      // Get the content of a file as value.
      ZS_COMPILER_PARSE(p_as_value);
    }
    else if (id == "load_json_file") {
      // Get the content of a file as table.
      ZS_COMPILER_PARSE(p_load_json_file);
    }
    else {
      return ZS_COMPILER_ERROR(
          zs::error_code::invalid_include_syntax, "expected `as_string`, `as_table` or ??");
    }
    break;
  }

  default: {
    return ZS_COMPILER_ERROR(zs::error_code::unimplemented, "expression expected");
    break;
  }
  }

  //  target.get() = -1;
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_load_json_file>() {
  using enum token_type;

  ZS_COMPILER_EXPECT(tok_lbracket);

  object filepath_value;
  ZS_COMPILER_EXPECT_GET(tok_string_value, filepath_value);
  ZS_COMPILER_EXPECT(tok_rbracket);

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_ERROR(zs::error_code::open_file_error, "cannot open file `as_string`");
  }

  zs::json_parser parser(_engine);

  object ret_value;
  if (auto err = parser.parse(_vm, loader.content(), nullptr, ret_value)) {
    return ZS_COMPILER_ERROR(err, "parse failed", parser.get_error());
  }

  add_new_target_instruction<op_load>((uint32_t)_ccs->get_literal(ret_value));

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_as_string>() {
  using enum token_type;

  ZS_COMPILER_EXPECT(tok_lbracket);

  object filepath_value;
  ZS_COMPILER_EXPECT_GET(tok_string_value, filepath_value);
  ZS_COMPILER_EXPECT(tok_rbracket);

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_ERROR(zs::error_code::open_file_error, "cannot open file `as_string`");
  }

  std::string_view svalue = loader.content();
  ZS_RETURN_IF_ERROR(add_string_instruction(svalue));

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_as_value>() {
  using enum token_type;

  ZS_COMPILER_EXPECT(tok_lbracket);

  object filepath_value;
  ZS_COMPILER_EXPECT_GET(tok_string_value, filepath_value);
  ZS_COMPILER_EXPECT(tok_rbracket);

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_ERROR(zs::error_code::open_file_error, "cannot open file `as_value`");
  }

  std::string_view svalue = loader.content();

  zs::vm vm(_engine);
  object ret_value;
  if (auto err = vm->load_buffer_as_value(svalue, filepath_value.get_string_unchecked(), ret_value)) {
    return ZS_COMPILER_ERROR(
        zs::error_code::invalid_include_file, "load `as_value` compile failed\n", vm->get_error());
  }

  add_new_target_instruction<op_load>((uint32_t)_ccs->get_literal(ret_value));
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_as_table>() {
  using enum token_type;

  ZS_COMPILER_EXPECT(tok_lbracket);

  object filepath_value;
  ZS_COMPILER_EXPECT_GET(tok_string_value, filepath_value);
  ZS_COMPILER_EXPECT(tok_rbracket);

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_ERROR(zs::error_code::open_file_error, "cannot open file `as_table`");
  }

  std::string_view svalue = loader.content();

  zs::vm vm(_engine);
  object ret_value;
  if (auto err = vm->load_buffer_as_value(svalue, filepath_value.get_string_unchecked(), ret_value)) {
    return ZS_COMPILER_ERROR(
        zs::error_code::invalid_include_file, "load `as_table` compile failed\n", vm->get_error());
  }

  if (!ret_value.is_table()) {
    return ZS_COMPILER_ERROR(
        zs::error_code::invalid_include_file, "result value is not a table in `as_table`");
  }

  add_new_target_instruction<op_load>((uint32_t)_ccs->get_literal(ret_value));
  return {};
}

// zs::error_code jit_compiler::expect(token_type tok) noexcept {
//
//   if (is_not(tok)) {
//
//     return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token ",
//         zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
//         zb::quoted<"'">(zs::token_to_string(tok)));
//   }
//
//   lex();
//   return zs::error_code::success;
// }
//
// zs::error_code jit_compiler::expect(token_type tok, const zb::source_location& loc) noexcept {
//   return expect(tok, zs::error_code::invalid_token, loc);
// }
//
// zs::error_code jit_compiler::expect(
//     token_type tok, zs::error_code ec, const zb::source_location& loc) noexcept {
//   if (!lex_if(tok)) {
//     return handle_error(ec,
//         zs::strprint(_engine, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)), ", expected
//         ",
//             zb::quoted<"'">(zs::token_to_string(tok))),
//         loc);
//   }
//
//   return zs::error_code::success;
// }

// zs::error_code jit_compiler::expect_get(
//     token_type tok, object& ret, const zb::source_location& loc) noexcept {
//   if (is_not(tok)) {
//     return handle_error(zs::error_code::invalid_token,
//         zs::strprint(_engine, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)), ", expected
//         ",
//             zb::quoted<"'">(zs::token_to_string(tok))),
//         loc);
//   }
//
//   ret = _lexer->get_value();
//   lex();
//   return zs::error_code::success;
// }
//
// zs::error_code jit_compiler::expect_get(token_type tok, object& ret) {
//
//   if (is_not(tok)) {
//     return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token ",
//         zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
//         zb::quoted<"'">(zs::token_to_string(tok)));
//   }
//
//   ret = _lexer->get_value();
//   lex();
//   return {};
// }

ZS_JIT_COMPILER_PARSE_OP(p_module) {

  if (_ccs->is_module() or !_ccs->is_top_level()) {
    return ZS_COMPILER_ERROR(zs::error_code::duplicated_module_tag,
        "The @module statement can only happen once in the top level scope.\n");
  }

  lex();

  _ccs->_sdata._is_module = true;

  //    ZS_RETURN_IF_ERROR(_ccs->create_export_table());

  if (is(tok_identifier)) {
    _ccs->_sdata._module_name = _lexer->get_identifier();
    lex();

    //    zb::print(_ccs->_sdata._module_name);
  }

  return {};
}

} // namespace zs.

#include "lang/jit/zjit_struct.h"
#include "lang/jit/zjit_variable_declaration.h"
#include "lang/jit/zjit_functions.h"

ZBASE_PRAGMA_POP()
