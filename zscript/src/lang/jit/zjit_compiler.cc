// #include "zjit_compiler.h"
// #include "zvirtual_machine.h"
//
//
// #include "lang/zopcode.h"
// #include "json/zjson_parser.h"
//
// #include <zbase/utility/print.h>
// #include <fmt/format.h>
// #include <zbase/function.h>
// #include <zbase/scoped.h>
//  #include <range/v3/all.hpp>

ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

#define ZS_COMPILER_HANDLE_ERROR_STREAM(err, ...) \
  helper::handle_error(this, err, zs::strprint(_engine, __VA_ARGS__), std::source_location::current())

#define ZS_COMPILER_HANDLE_ERROR_STRING(err, msg) \
  helper::handle_error(this, err, msg, std::source_location::current())

#define ZS_COMPILER_RETURN_IF_ERROR_STRING(X, err, msg) \
  if (zs::error_result err = X) {                       \
    return ZS_COMPILER_HANDLE_ERROR_STRING(err, msg);   \
  }

#define ZS_COMPILER_RETURN_IF_ERROR_STREAM(X, ...)            \
  if (zs::error_result err = (X)) {                           \
    return ZS_COMPILER_HANDLE_ERROR_STREAM(err, __VA_ARGS__); \
  }

namespace zs {

enum class jit_compiler::parse_op : uint8_t {

  //
  p_preprocessor,
  p_statement,
  p_expression,
  p_function_statement,
  p_global_function_statement,
  p_function,
  p_function_call_args,
  p_function_call_args_template,
  p_create_function,
  p_comma,
  p_semi_colon,
  p_decl_var,
  p_decl_enum,
  p_enum_table,
  p_variable_type_restriction,
  p_table_or_class,

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

#define ZS_JIT_COMPILER_PARSE_OP(name, ...) \
  template <>                               \
  zs::error_result jit_compiler::parse<jit_compiler::parse_op::name>(__VA_ARGS__)

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_table_or_class>(
    token_type separator, token_type terminator);

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_prefixed>();

ZS_JIT_COMPILER_PARSE_OP(p_struct);
ZS_JIT_COMPILER_PARSE_OP(p_struct_member_type, uint32_t* obj_type_mask, bool* is_static, bool* is_const);
ZS_JIT_COMPILER_PARSE_OP(p_struct_statement);
ZS_JIT_COMPILER_PARSE_OP(p_struct_content, struct_parser* sparser);

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_class_statement>();

template <>
zs::error_result jit_compiler::action<jit_compiler::action_type::act_move_if_current_target_is_local>() {
  int_t trg = _ccs->top_target();
  if (_ccs->is_local(trg)) {
    trg = _ccs->pop_target(); // pops the target and moves it

    add_instruction<opcode::op_move>((uint8_t)_ccs->new_target(), (uint8_t)trg);
  }

  return {};
}

template <>
zs::error_result jit_compiler::action<jit_compiler::action_type::act_invoke_expr>(
    zb::member_function_pointer<jit_compiler, zs::error_result> fct) {

  const expr_state es = _estate;
  _estate.type = expr_type::e_expr;
  _estate.pos = -1;
  _estate.no_get = false;
  _estate.no_assign = false;
  ZS_RETURN_IF_ERROR((this->*fct)());
  _estate = es;

  return {};
}

struct jit_compiler::helper {

  static zs::error_result add_small_string_instruction(
      jit_compiler* c, std::string_view s, int_t target_idx) {
    if (s.size() > zs::constants::k_small_string_max_size) {
      return zs::error_code::invalid;
    }

    struct uint64_t_pair {
      uint64_t value_1;
      uint64_t value_2;
    };

    uint64_t_pair spair = { 0, 0 };

    ::memcpy(&spair, s.data(), s.size());

    c->add_instruction<opcode::op_load_small_string>(target_idx, spair.value_1, spair.value_2);

    return {};
  }

  static zs::error_result add_small_string_instruction(jit_compiler* c, std::string_view s) {
    return add_small_string_instruction(c, s, c->_ccs->new_target());
  }

  static zs::error_result add_string_instruction(jit_compiler* c, std::string_view s, int_t target_idx) {
    if (s.size() > zs::constants::k_small_string_max_size) {
      c->add_instruction<opcode::op_load_string>(
          target_idx, (uint32_t)c->_ccs->get_literal(object::create_string(c->_engine, s)));
      return {};
    }

    struct uint64_t_pair {
      uint64_t value_1;
      uint64_t value_2;
    };

    uint64_t_pair spair = { 0, 0 };

    ::memcpy(&spair, s.data(), s.size());

    c->add_instruction<opcode::op_load_small_string>(target_idx, spair.value_1, spair.value_2);
    return {};
  }

  static zs::error_result add_string_instruction(jit_compiler* c, std::string_view s) {
    return add_string_instruction(c, s, c->_ccs->new_target());
  }
  
  static zs::error_result add_string_instruction(jit_compiler* c, const object& sobj) {
    return add_string_instruction(c, sobj, c->_ccs->new_target());
  }

  static zs::error_result add_string_instruction(jit_compiler* c, const object& sobj, int_t target_idx) {
    if (sobj.is_small_string()) {
      return add_small_string_instruction(c, sobj.get_small_string_unchecked(), target_idx);
    }

    c->add_instruction<opcode::op_load_string>(target_idx, (uint32_t)c->_ccs->get_literal(sobj));
    return {};
  }
 

  static inline zs::error_result handle_error(
      jit_compiler* comp, zs::error_code ec, std::string_view msg, const std::source_location& loc) {
    zs::line_info linfo = comp->_lexer->get_last_line_info();

    const auto& stream = comp->_lexer->_stream;
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

    const int column = linfo.column ? (int)linfo.column - 1 : 0;

    constexpr const char* new_line_padding = "\n       ";

    std::string_view fname = loc.function_name();

    //    if(fname.size() > 80) {
    //      comp->_error_message
    //          += zs::strprint<"">(comp->_engine, "\nerror: ",  linfo,
    //          new_line_padding, line_content,
    //              new_line_padding, zb::indent_t(column, 1), "^",
    //              new_line_padding, "from '", fname.substr(0, 80), "\n ",
    //              fname.substr(80),
    //              "'", new_line_padding, "in '", loc.file_name(), "'",
    //              new_line_padding, "at line ", loc.line(), new_line_padding,
    //              "*** ",msg);
    //    }
    //    else {
    //      comp->_error_message
    //          += zs::strprint<"">(comp->_engine, "\nerror: ",   linfo,
    //          new_line_padding, line_content,
    //              new_line_padding, zb::indent_t(column, 1), "^",
    //              new_line_padding, "from '", loc.function_name(),
    //              "'", new_line_padding, "in '", loc.file_name(), "'",
    //              new_line_padding, "at line ", loc.line(),
    //              new_line_padding,"*** ",msg);
    //    }

    if (fname.size() > 80) {
      comp->_error_message
          += zs::strprint<"">(comp->_engine, "\nerror: ", linfo, new_line_padding, line_content,
              new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '", fname.substr(0, 80),
              "\n               ", fname.substr(80), "'", new_line_padding, "     in '", loc.file_name(), "'",
              new_line_padding, "      at line ", loc.line(), "\n", new_line_padding, "*** ", msg);
    }
    else {
      comp->_error_message += zs::strprint<"">(comp->_engine, "\nerror: ", linfo, new_line_padding,
          line_content, new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '",
          loc.function_name(), "'", new_line_padding, "      in '", loc.file_name(), "'", new_line_padding,
          "      at line ", loc.line(), "\n", new_line_padding, "*** ", msg);
    }

    //    if(fname.size() > 80) {
    //      comp->_error_message
    //          += zs::strprint<"">(comp->_engine, "\nerror: ", msg, " ", linfo,
    //          new_line_padding, line_content,
    //              new_line_padding, zb::indent_t(column, 1), "^",
    //              new_line_padding, "from '", fname.substr(0, 80), "\n  ",
    //              fname.substr(80),
    //              "'", new_line_padding, "in '", loc.file_name(), "'",
    //              new_line_padding, "at line ", loc.line());
    //    }
    //    else {
    //      comp->_error_message
    //          += zs::strprint<"">(comp->_engine, "\nerror: ", msg, " ", linfo,
    //          new_line_padding, line_content,
    //              new_line_padding, zb::indent_t(column, 1), "^",
    //              new_line_padding, "from '", loc.function_name(),
    //              "'", new_line_padding, "in '", loc.file_name(), "'",
    //              new_line_padding, "at line ", loc.line());
    //    }
    //

    return ec;
  }

  static bool is_template_function_call(zs::jit_compiler* comp) {
    return comp->_lexer->is_template_function_call();
  }

  static bool needs_get(zs::jit_compiler* comp) {
    using enum token_type;

    switch (comp->_token) {
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
      if (!comp->is_end_of_statement()) {
        return false;
      }
      break;

    case tok_lt: {
      if (is_template_function_call(comp)) {
        return false;
      }
      break;
    }
    }

    return (!comp->_estate.no_get
        || (comp->_estate.no_get && (comp->_token == tok_dot || comp->_token == tok_lsqrbracket)));
  }

  static bool needs_get_no_assign(zs::jit_compiler* comp) {
    using enum token_type;

    switch (comp->_token) {
    case tok_lbracket:
      return false;
    case tok_incr:
    case tok_decr:
      if (!comp->is_end_of_statement()) {
        return false;
      }
      break;

    case tok_lt: {
      if (is_template_function_call(comp)) {
        return false;
      }
      break;
    }
    }

    return (!comp->_estate.no_get
        || (comp->_estate.no_get && (comp->_token == tok_dot || comp->_token == tok_lsqrbracket)));
  }

  template <opcode Op>
  ZB_CHECK static zs::error_result do_arithmetic_expr(zs::jit_compiler* comp,
      zb::member_function_pointer<jit_compiler, zs::error_result> fct, std::string_view symbol) {
    comp->lex();
    ZS_RETURN_IF_ERROR(comp->action<action_type::act_invoke_expr>(fct));

    int_t op2 = comp->_ccs->pop_target();
    int_t op1 = comp->_ccs->pop_target();

    comp->add_instruction<Op>(comp->_ccs->new_target(), (uint8_t)op1, (uint8_t)op2);
    comp->_estate.type = expr_type::e_expr;

    return {};
  }

  //  void emit_compound_arith(token_type tok, SQInteger etype, SQInteger pos) {
  //    /* Generate code depending on the expression type */
  //    switch (etype) {
  //    case LOCAL: {
  //      SQInteger p2 = _fs->PopTarget(); // src in OP_GET
  //      SQInteger p1 = _fs->PopTarget(); // key in OP_GET
  //      _fs->PushTarget(p1);
  //      // EmitCompArithLocal(tok, p1, p1, p2);
  //      _fs->AddInstruction(ChooseArithOpByToken(tok), p1, p2, p1, 0);
  //      _fs->SnoozeOpt();
  //    } break;
  //    case OBJECT:
  //    case BASE: {
  //      SQInteger val = _fs->PopTarget();
  //      SQInteger key = _fs->PopTarget();
  //      SQInteger src = _fs->PopTarget();
  //      /* _OP_COMPARITH mixes dest obj and source val in the arg1 */
  //      _fs->AddInstruction(
  //          _OP_COMPARITH, _fs->PushTarget(), (src << 16) | val, key,
  //          ChooseCompArithCharByToken(tok));
  //    } break;
  //    case OUTER: {
  //      SQInteger val = _fs->TopTarget();
  //      SQInteger tmp = _fs->PushTarget();
  //      _fs->AddInstruction(_OP_GETOUTER, tmp, pos);
  //      _fs->AddInstruction(ChooseArithOpByToken(tok), tmp, val, tmp, 0);
  //      _fs->PopTarget();
  //      _fs->PopTarget();
  //      _fs->AddInstruction(_OP_SETOUTER, _fs->PushTarget(), pos, tmp);
  //    } break;
  //    }
  //  }

  template <class Fct>
  inline static zs::error_result expr_call(zs::jit_compiler* comp, Fct&& fct) {
    expr_state es = std::exchange(comp->_estate, expr_state{ expr_type::e_expr, -1, false });
    zs::error_result res = fct();
    comp->_estate = es;
    return res;
  }

  template <class Fct>
  inline static zs::error_result expr_call(zs::jit_compiler* comp, Fct&& fct, expr_state e) {
    expr_state es = std::exchange(comp->_estate, e);
    zs::error_result res = fct();
    comp->_estate = es;
    return res;
  }
};

//
// MARK: Parse forward declare.
//

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_statement>(bool close_frame);

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_function_call_args>(bool rawcall);

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_function_call_args_template>(
    std::string_view meta_code);

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_function_statement>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_global_function_statement>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_variable_type_restriction>(
    std::reference_wrapper<uint32_t> mask, std::reference_wrapper<uint64_t> custom_mask);

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_expression>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_create_function>(
    std::reference_wrapper<const object> name, int_t boundtarget, bool lambda);

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_factor>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_factor_identifier>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_factor_at>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_bind_env>(
    std::reference_wrapper<int_t> target);

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_as_table>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_as_string>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_as_value>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_load_json_file>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_if_block>();

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_if>();

//
// MARK: Parse.
//

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_prefixed_incr>(bool is_incr) {
  using enum token_type;

  lex();
  expr_state es = _estate;
  _estate.no_get = true;

  ZS_RETURN_IF_ERROR(parse<parse_op::p_prefixed>());

  switch (_estate.type) {
  case expr_type::e_expr:
    return helper::handle_error(this, zs::error_code::invalid_operation, "Can't '++' or '--' an expression",
        std::source_location::current());

  case expr_type::e_base:
    return helper::handle_error(this, zs::error_code::invalid_operation, "Can't '++' or '--' a base",
        std::source_location::current());

  case expr_type::e_object: {
    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->pop_target();

    add_instruction<opcode::op_pobjincr>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, is_incr);

    break;
    ;
  }

  case expr_type::e_local: {
    int_t src = _ccs->top_target();
    add_instruction<opcode::op_pincr>((uint8_t)src, (uint8_t)src, is_incr);
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_prefixed_lbracket>() {
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
      add_instruction<op_move>(_ccs->new_target(), (uint8_t)0);
    }
    else {
      is_member_call = true;

      // We need to call a function from a table e.g. `table.fct();`.
      // The get wasn't done in the `case tok_dot:` above especially for
      // this.

      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->top_target();

      // Get the item at the given `key_idx`, from the table at `table_idx`.
      add_instruction<op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);

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

      // Nothing to do here other than pushing it on the stack.
      //            add_instruction<opcode::op_move>(_ccs->new_target(),
      //            (uint8_t)0);
    }
    else {
      // TODO: Fix this.

      // Push the captured closure.
      add_instruction<op_get_capture>(_ccs->new_target(), (uint32_t)_estate.pos);
      _estate.pos = _ccs->top_target();

      // TODO: Was should we put here???
      // Push the root table.
      add_instruction<op_move>(_ccs->new_target(), (uint8_t)0);
    }

    break;
  }

  default:
    add_instruction<op_move>(_ccs->new_target(), (uint8_t)0);
  }

  _estate.type = expr_type::e_expr;
  lex();

  ZS_RETURN_IF_ERROR(parse<parse_op::p_function_call_args>(false));

  // When `is_member_call` is true, we are stuck with a table below the
  // result that we need to remove. TRo solve this, we pop them both, and
  // push the result back on top.
  if (is_member_call) {
    // Pop result.
    int_t result_idx = _ccs->pop_target();

    // Pop table.
    _ccs->pop_target();

    // Move the result back on top.
    add_instruction<op_move>(_ccs->new_target(), (uint8_t)result_idx);
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_prefixed_lbracket_template>() {
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
      add_instruction<op_move>(_ccs->new_target(), (uint8_t)0);
    }
    else {
      is_member_call = true;

      // We need to call a function from a table e.g. `table.fct();`.
      // The get wasn't done in the `case tok_dot:` above especially for
      // this.
      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->top_target();

      // Get the item at the given `key_idx`, from the table at `table_idx`.
      add_instruction<op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, false);

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
      //            add_instruction<opcode::op_move>(_ccs->new_target(),
      //            (uint8_t)0);
    }
    else {
      // TODO: Fix this.

      // Push the captured closure.
      add_instruction<op_get_capture>(_ccs->new_target(), (uint32_t)_estate.pos);
      _estate.pos = _ccs->top_target();

      // TODO: Was should we put here???
      // Push the root table.
      add_instruction<op_move>(_ccs->new_target(), (uint8_t)0);
    }

    break;
  }

  default:
    add_instruction<op_move>(_ccs->new_target(), (uint8_t)0);
  }

  _estate.type = expr_type::e_expr;
  lex();

  ZS_RETURN_IF_ERROR(parse<parse_op::p_function_call_args_template>(meta_code));

  // When `is_member_call` is true, we are stuck with a table below the
  // result that we need to remove. TRo solve this, we pop them both, and
  // push the result back on top.
  if (is_member_call) {
    // Pop result.
    int_t result_idx = _ccs->pop_target();

    // Pop table.
    _ccs->pop_target();

    // Move the result back on top.
    add_instruction<op_move>(_ccs->new_target(), (uint8_t)result_idx);
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_exponential>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(parse<parse_op::p_prefixed>());

  zb_loop() {
    switch (_token) {
    case tok_exp: {
      ZS_RETURN_IF_ERROR(helper::do_arithmetic_expr<opcode::op_exp>(
          this, &jit_compiler::parse<parse_op::p_exponential>, "^"));
      break;
    }

    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_mult>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(parse<parse_op::p_exponential>());

  zb_loop() {
    switch (_token) {

    case tok_mul: {
      ZS_RETURN_IF_ERROR(helper::do_arithmetic_expr<opcode::op_mul>(
          this, &jit_compiler::parse<parse_op::p_exponential>, "*"));
    } break;

    case tok_div: {
      ZS_RETURN_IF_ERROR(helper::do_arithmetic_expr<opcode::op_div>(
          this, &jit_compiler::parse<parse_op::p_exponential>, "/"));
    } break;

    case tok_mod: {
      ZS_RETURN_IF_ERROR(helper::do_arithmetic_expr<opcode::op_mod>(
          this, &jit_compiler::parse<parse_op::p_exponential>, "%"));
    } break;

    default:
      return {};
    }
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_plus>() {
  static constexpr parse_op next_op = parse_op::p_mult;

  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {
    case tok_add: {
      ZS_RETURN_IF_ERROR(
          helper::do_arithmetic_expr<opcode::op_add>(this, &jit_compiler::parse<next_op>, "+"));
      break;
    }

    case tok_minus: {
      ZS_RETURN_IF_ERROR(
          helper::do_arithmetic_expr<opcode::op_sub>(this, &jit_compiler::parse<next_op>, "-"));
      break;
    }

    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_shift>() {
  using enum token_type;
  static constexpr parse_op next_op = parse_op::p_plus;

  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {
    case tok_lshift:
      ZS_RETURN_IF_ERROR(
          helper::do_arithmetic_expr<opcode::op_lshift>(this, &jit_compiler::parse<next_op>, "<<"));
      break;
    case tok_rshift:
      ZS_RETURN_IF_ERROR(
          helper::do_arithmetic_expr<opcode::op_rshift>(this, &jit_compiler::parse<next_op>, ">>"));
      break;
      break;
    default:
      return {};
    }
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_compare>() {
  static constexpr parse_op next_op = parse_op::p_shift;

  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {
    case tok_gt: {
      lex();

      ZS_RETURN_IF_ERROR(helper::expr_call(this, [&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_instruction<opcode::op_cmp>(_ccs->new_target(), compare_op::gt, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_lt: {
      lex();

      ZS_RETURN_IF_ERROR(helper::expr_call(this, [&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_instruction<opcode::op_cmp>(_ccs->new_target(), compare_op::lt, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_gt_eq: {
      lex();

      ZS_RETURN_IF_ERROR(helper::expr_call(this, [&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_instruction<opcode::op_cmp>(_ccs->new_target(), compare_op::ge, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_lt_eq: {
      lex();

      ZS_RETURN_IF_ERROR(helper::expr_call(this, [&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_instruction<opcode::op_cmp>(_ccs->new_target(), compare_op::le, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_in:
      //      helper::binary_exp(this, opcode::op_cmp,
      //      &compiler::parse<parse_op::p_shift>);
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_eq_compare>() {
  using enum token_type;
  static constexpr parse_op next_op = parse_op::p_compare;

  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {

    case tok_eq_eq: {
      lex();

      ZS_RETURN_IF_ERROR(helper::expr_call(this, [&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_instruction<opcode::op_eq>(_ccs->new_target(), (uint8_t)op1, (uint8_t)op2, false);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_not_eq: {
      lex();

      ZS_RETURN_IF_ERROR(helper::expr_call(this, [&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_instruction<opcode::op_ne>(_ccs->new_target(), (uint8_t)op1, (uint8_t)op2, false);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_three_way_compare: {
      lex();

      ZS_RETURN_IF_ERROR(helper::expr_call(this, [&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_instruction<opcode::op_cmp>(_ccs->new_target(), compare_op::tw, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_double_arrow: {
      lex();

      ZS_RETURN_IF_ERROR(helper::expr_call(this, [&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_instruction<opcode::op_cmp>(
          _ccs->new_target(), compare_op::double_arrow, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    case tok_double_arrow_eq: {
      lex();

      ZS_RETURN_IF_ERROR(helper::expr_call(this, [&]() { return parse<next_op>(); }));

      int_t op2 = _ccs->pop_target();
      int_t op1 = _ccs->pop_target();

      add_instruction<opcode::op_cmp>(
          _ccs->new_target(), compare_op::double_arrow_eq, (uint8_t)op1, (uint8_t)op2);
      _estate.type = expr_type::e_expr;
    } break;

    default:
      return {};
    }
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_bitwise_and>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(parse<parse_op::p_eq_compare>());

  zb_loop() {
    if (is(tok_bitwise_and)) {

      ZS_RETURN_IF_ERROR(helper::do_arithmetic_expr<opcode::op_bitwise_and>(
          this, &jit_compiler::parse<parse_op::p_eq_compare>, "&"));
    }

    //      helper::binary_exp(this, opcode::op_bitw,
    //      &compiler::parse<parse_op::p_bitwise_and>);

    //        binary_exp(_OP_BITW, &SQCompiler::BitwiseAndExp, BW_OR);

    else {
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_bitwise_xor>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(parse<parse_op::p_bitwise_and>());

  zb_loop() {
    if (is(tok_xor)) {
      ZS_RETURN_IF_ERROR(helper::do_arithmetic_expr<opcode::op_bitwise_xor>(
          this, &jit_compiler::parse<parse_op::p_bitwise_and>, "xor"));
    }
    else {
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_bitwise_or>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(parse<parse_op::p_bitwise_xor>());

  zb_loop() {
    if (is(tok_bitwise_or)) {

      ZS_RETURN_IF_ERROR(helper::do_arithmetic_expr<opcode::op_bitwise_or>(
          this, &jit_compiler::parse<parse_op::p_bitwise_xor>, "|"));
    }

    //      helper::binary_exp(this, opcode::op_bitw,
    //      &compiler::parse<parse_op::p_bitwise_and>);

    //        binary_exp(_OP_BITW, &SQCompiler::BitwiseAndExp, BW_OR);

    else {
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_and>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(parse<parse_op::p_bitwise_or>());

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
      ZS_RETURN_IF_ERROR(parse<jit_compiler::parse_op::p_and>());
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_or>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(parse<jit_compiler::parse_op::p_and>());

  zb_loop() {
    if (is(tok_or)) {

      lex();
      ZS_RETURN_IF_ERROR(parse<parse_op::p_or>());
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_expression>() {
  using enum token_type;
  using enum opcode;
  using enum object_type;

  expr_state es = _estate;
  _estate.type = expr_type::e_expr;
  _estate.pos = -1;
  _estate.no_get = false;

  zb::scoped expr_state_cache = [&]() { _estate = es; };

  ZS_RETURN_IF_ERROR(parse<parse_op::p_or>());

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
  //
  if (es_type == expr_type::e_expr) {
    _error_message += zs::strprint(
        _engine, "Can't assign an expression", _lexer->get_line_info(), std::source_location::current());

    return zs::error_code::invalid_value_type_assignment;
  }
  else if (es_type == expr_type::e_base) {
    _error_message += zs::strprint(
        _engine, "'base' cannot be modified", _lexer->get_line_info(), std::source_location::current());
    return zs::error_code::invalid_value_type_assignment;
  }

  lex();
  if (auto err = parse<parse_op::p_expression>()) {
    return err;
  }

  switch (op) {
    // Assign.
  case tok_eq: {
    //    zb::print(tok_eq, es_type, std::source_location::current());

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
        return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
            zs::strprint(_engine, "trying to assign to a const value"), std::source_location::current());
      }

      bool skip_mask = false;

      if (mask) {
        zs::opcode last_op
            = zs::instruction_iterator(&_ccs->_instructions._data[_ccs->get_instruction_index()])
                  .get_opcode();

        switch (last_op) {
        case op_load_int:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_integer)))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(
                    _engine, "wrong type mask", k_integer, "expected", zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;

        case op_load_float:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_float)))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(
                    _engine, "wrong type mask", k_float, "expected", zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;

        case op_load_bool:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_bool)))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(
                    _engine, "wrong type mask", k_bool, "expected", zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;

        case op_load_small_string:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_small_string)))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(_engine, "wrong type mask", k_small_string, "expected",
                    zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;

        case op_load_string:
          if (!(skip_mask = (mask & zs::object_base::k_string_mask))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(_engine, "wrong type mask", k_long_string, "expected",
                    zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;
        }
      }

      //
      //             if (!skip_mask) {
      //               if (custom_mask) {
      //                 add_instruction<opcode::op_check_custom_type_mask>((uint8_t)_ccs->top_target(),
      //                 mask, custom_mask);
      //               }
      //               else if (mask) {
      //                 add_instruction<opcode::op_check_type_mask>((uint8_t)_ccs->top_target(),
      //                 mask);
      //               }
      //             }

      add_instruction<opcode::op_move>((uint8_t)dst, (uint8_t)src);

      if (!skip_mask) {
        if (custom_mask) {
          add_instruction<opcode::op_check_custom_type_mask>((uint8_t)dst, mask, custom_mask);
        }
        else if (mask) {
          add_instruction<opcode::op_check_type_mask>((uint8_t)dst, mask);
        }
      }
      break;
    }

    case expr_type::e_object: {

      if (_ccs->_target_stack.size() < 3) {
        return helper::handle_error(this, zs::error_code::invalid_operation,
            zs::strprint(_engine, "wrong type mask", k_long_string, "expected"),
            std::source_location::current());
      }

      int_t value_idx = _ccs->pop_target();
      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->top_target();

      add_instruction<opcode::op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);

      _estate.type = expr_type::e_object;
      _estate.pos = table_idx;
      break;
    }

    default:
      return zs::error_code::unimplemented;
    }
  } break;

  case tok_add_eq: {
    expr_type es_type = _estate.type;

    switch (es_type) {
    case expr_type::e_local: {
      int_t src = _ccs->pop_target();
      int_t target = _ccs->top_target();
      add_instruction<opcode::op_add_eq>((uint8_t)target, (uint8_t)src);
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
      add_instruction<opcode::op_mul_eq>((uint8_t)target, (uint8_t)src);
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_comma>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());

  while (_token == tok_comma) {
    lex();
    ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_semi_colon>() {
  using enum token_type;

  if (is(tok_semi_colon)) {
    lex();
    return {};
  }

  if (!is_end_of_statement()) {
    return helper::handle_error(
        this, zs::error_code::invalid_token, "invalid token", std::source_location::current());
  }

  return {};
}

// TODO: Prevent from declaring empty const variable.
template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_decl_var>() {
  using enum token_type;
  using enum opcode;
  using enum object_type;

  token_type variable_type = _token;

  lex();

  // var function name().
  // TODO: Let's forget about this one for now.
  if (is(tok_function)) {
    ZS_TODO("Implement function declaration.");
    int_t bound_target = 0xFF;
    lex();

    zs::object var_name;
    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));

    //    if (_token == tok_lcrlbracket) {
    //            boundtarget = ParseBindEnv();
    //    }

    ZS_RETURN_IF_ERROR(expect(tok_lbracket));

    ZS_RETURN_IF_ERROR(parse<parse_op::p_create_function>(std::cref(var_name), bound_target, false));

    //      CreateFunction(varname, 0xFF, false);

    add_instruction<op_new_closure>(
        (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);
    _ccs->pop_target();
    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
    return {};
  }

  //
  //
  //

  const bool is_const = variable_type == tok_const;
  if (is_const) {
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
      lex();
      break;

    case tok_identifier:
      break;
    default:
      return zs::error_code::invalid_token;
    }
  }

  uint32_t mask = 0;
  uint64_t custom_mask = 0;

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
      if (auto err = parse<parse_op::p_variable_type_restriction>(std::ref(mask), std::ref(custom_mask))) {
        return helper::handle_error(
            this, err, "parsing variable type restriction `var<....>`", std::source_location::current());
      }
    }
    break;
  }

  zb_loop() {
    if (is_not(tok_identifier)) {
      return helper::handle_error(
          this, zs::error_code::identifier_expected, "expected identifier", std::source_location::current());
    }

    object var_name(_engine, _lexer->get_identifier_value());

    lex();

    // @code `var name = ...;`
    if (is(tok_eq)) {
      lex();
      ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());

      int_t src = _ccs->pop_target();
      int_t dest = _ccs->new_target(mask, custom_mask, is_const);

      zs::opcode last_op
          = zs::instruction_iterator(&_ccs->_instructions._data[_ccs->get_instruction_index()]).get_opcode();

      bool skip_mask = false;
      if (mask) {
        switch (last_op) {
        case op_load_int:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_integer)))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(
                    _engine, "wrong type mask", k_integer, "expected", zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;

        case op_load_float:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_float)))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(
                    _engine, "wrong type mask", k_float, "expected", zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;

        case op_load_bool:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_bool)))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(
                    _engine, "wrong type mask", k_bool, "expected", zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;

        case op_load_small_string:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_small_string)))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(_engine, "wrong type mask", k_small_string, "expected",
                    zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;

        case op_load_string:
          if (!(skip_mask = (mask & zs::object_base::k_string_mask))) {
            return helper::handle_error(this, zs::error_code::invalid_value_type_assignment,
                zs::strprint(_engine, "wrong type mask", k_long_string, "expected",
                    zs::object_type_mask_printer{ mask }),
                std::source_location::current());
          }
          break;
        }
      }

      if (dest != src) {
        //                          if (_fs->IsLocal(src)) {
        //                              _fs->SnoozeOpt();
        //                          }
        add_instruction<opcode::op_move>((uint8_t)dest, (uint8_t)src);
      }

      if (!skip_mask) {
        if (custom_mask) {
          add_instruction<opcode::op_check_custom_type_mask>((uint8_t)_ccs->top_target(), mask, custom_mask);
        }
        else if (mask) {
          add_instruction<opcode::op_check_type_mask>((uint8_t)_ccs->top_target(), mask);
        }
      }
    }

    // @code `var name;`
    else {
      add_instruction<opcode::op_load_null>(_ccs->new_target());
    }

    _ccs->pop_target();
    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name, nullptr, mask, custom_mask, is_const));

    if (is_not(tok_comma)) {
      break;
    }

    lex();
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_enum_table>() {
  using enum token_type;

  while (is_not(tok_rcrlbracket)) {
    switch (_token) {
    case tok_lsqrbracket:
      return helper::handle_error(this, zs::error_code::invalid_operation,
          "Enum keys can only be regular identifier", std::source_location::current());

    case tok_string_value:
    case tok_escaped_string_value:
      return helper::handle_error(this, zs::error_code::invalid_operation,
          "Enum keys can only be regular identifier i.e. no json style "
          "identifier)",
          std::source_location::current());

    case tok_identifier: {
      zs::object identifier;
      ZS_RETURN_IF_ERROR(expect_get(tok_identifier, identifier));
      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, identifier));

      // No value enum field.
      if (is(tok_comma, tok_rcrlbracket)) {
        add_instruction<opcode::op_load_none>(_ccs->new_target());
      }
      else {
        ZS_RETURN_IF_ERROR(expect(tok_eq));
        ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());
      }
      break;
    }

    default:
      return helper::handle_error(this, zs::error_code::invalid_operation,
          "Enum can only contain integers, floats, bools and strings.", std::source_location::current());
    }

    lex_if(tok_comma);

    int_t val = _ccs->pop_target();
    int_t key = _ccs->pop_target();
    int_t table = _ccs->top_target();
    add_instruction<opcode::op_new_enum_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val);
  }

  lex();

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_decl_enum>() {
  using enum token_type;
  using enum opcode;
  using enum object_type;

  zbase_assert(_token == tok_enum);

  lex();

  //
  //
  //

  if (is_not(tok_identifier)) {
    return helper::handle_error(
        this, zs::error_code::identifier_expected, "expected identifier", std::source_location::current());
  }

  object var_name(_engine, _lexer->get_identifier_value());

  lex();

  // Optional `=`.
  lex_if(tok_eq);

  if (is_not(tok_lcrlbracket)) {
    return ZS_COMPILER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
        zb::quoted<"'">(zs::token_to_string(tok_lcrlbracket)));
    return zs::error_code::invalid_token;
  }

  _enum_counter = 0;
  add_instruction<opcode::op_new_obj>(_ccs->new_target(), object_type::k_table);
  lex();
  ZS_RETURN_IF_ERROR(parse<parse_op::p_enum_table>());

  int_t src = _ccs->pop_target();

  if (int_t dest = _ccs->new_target(); dest != src) {
    add_instruction<opcode::op_move>((uint8_t)dest, (uint8_t)src);
  }

  add_instruction<opcode::op_close_enum>((uint8_t)_ccs->top_target());

  _ccs->pop_target();

  ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_define>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(expect(tok_define));

  zs::object identifier;
  ZS_RETURN_IF_ERROR(expect_get(tok_identifier, identifier));

  ZS_RETURN_IF_ERROR(expect(tok_eq));

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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_preprocessor>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(expect(tok_hastag));

  switch (_token) {
  case tok_include:
    return parse_include_or_import_statement(token_type::tok_include);

  case tok_import:
    return parse_include_or_import_statement(token_type::tok_import);

  case tok_define:
    //      return parse_include_or_import_statement( );
    return parse<jit_compiler::parse_op::p_define>();

  default:
    _error_message
        += zs::strprint<"">(_engine, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)),
            ", expected {'include', 'import', 'macro', 'define', 'if', 'elif', "
            "'else'}",
            _lexer->get_line_info());
    return zs::error_code::invalid_token;
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_if_block>() {
  using enum token_type;
  if (is(tok_lcrlbracket)) {
    //       BEGIN_SCOPE();

    scope previous_scope = _scope;
    _scope.n_captures = _ccs->_n_capture;
    _scope.stack_size = _ccs->get_stack_size();

    lex();

    while (is_not(tok_rcrlbracket, tok_default, tok_case)) {
      ZS_RETURN_IF_ERROR(parse<parse_op::p_statement>(true));

      if (_lexer->_last_token != tok_rcrlbracket && _lexer->_last_token != tok_semi_colon) {
        //            OptionalSemicolon();
        ZS_RETURN_IF_ERROR(parse<parse_op::p_semi_colon>());
      }
    }

    ZS_RETURN_IF_ERROR(expect(tok_rcrlbracket));

    {
      int_t previous_n_capture = _ccs->_n_capture;

      if (_ccs->get_stack_size() != _scope.stack_size) {
        _ccs->set_stack_size(_scope.stack_size);
        if (previous_n_capture != (int_t)_ccs->_n_capture) {
          add_instruction<opcode::op_close>((uint32_t)_scope.stack_size);
        }
      }
      _scope = previous_scope;
    }
  }
  else {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_statement>(true));

    if (_lexer->_last_token != tok_rcrlbracket && _lexer->_last_token != tok_semi_colon) {
      //            OptionalSemicolon();
      ZS_RETURN_IF_ERROR(parse<parse_op::p_semi_colon>());
    }

    //       Statement();
    //       if (_lex._prevtoken != _SC('}') && _lex._prevtoken != _SC(';'))
    //         OptionalSemicolon();
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_if>() {
  using enum token_type;
  using enum opcode;

  zs::instruction_vector& ivec = _ccs->_instructions;

  ZS_RETURN_IF_ERROR(expect(tok_if));
  ZS_RETURN_IF_ERROR(expect(tok_lbracket));
  ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());
  ZS_RETURN_IF_ERROR(expect(tok_rbracket));

  add_instruction<op_jz>(0, _ccs->pop_target());
  const int_t jz_inst_idx = _ccs->get_instruction_index();

  ZS_RETURN_IF_ERROR(parse<parse_op::p_if_block>());

  if (is_not(tok_else)) {
    ivec.get<op_jz>(jz_inst_idx)->offset = (int32_t)(_ccs->get_next_instruction_index() - jz_inst_idx);
    return {};
  }

  add_instruction<op_jmp>(0);
  const int_t jmp_inst_idx = _ccs->get_instruction_index();
  const int_t jmp_end_idx = _ccs->get_next_instruction_index();

  lex();
  ZS_RETURN_IF_ERROR(parse<parse_op::p_if_block>());

  ivec.get<op_jmp>(jmp_inst_idx)->offset = (int32_t)(_ccs->get_next_instruction_index() - jmp_inst_idx);
  ivec.get<op_jz>(jz_inst_idx)->offset = (int32_t)(jmp_end_idx - jz_inst_idx);

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_for_each>() {
  using enum token_type;

  // foreach(var i : arr)
  // ^
  lex();

  // foreach(var i : arr)
  //        ^
  ZS_RETURN_IF_ERROR(expect(tok_lbracket));
  ZS_RETURN_IF_ERROR(expect(tok_var));

  zs::object var_name;
  zs::object idx_name = zs::_ss("index");
  zs::object iter_name = zs::_ss("iter");

  ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
  ZS_RETURN_IF_ERROR(_ccs->push_local_variable(idx_name));
  ZS_RETURN_IF_ERROR(expect(tok_colon));

  zb::print(var_name);

  scope previous_scope = _scope;
  _scope.n_captures = _ccs->_n_capture;
  _scope.stack_size = _ccs->get_stack_size();
  ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());

  ZS_RETURN_IF_ERROR(expect(tok_rbracket));
  //      // put the table in the stack(evaluate the table expression)
  //      Expression();
  //      Expect(_SC(')'));
  int_t container = _ccs->top_target();
  //      // push the index local var
  int_t indexpos = _ccs->find_local_variable(idx_name);
  add_instruction<opcode::op_load_null>(indexpos);

  //      // push the value local var
  int_t valuepos = _ccs->find_local_variable(var_name);
  add_instruction<opcode::op_load_null>(valuepos);
  //      // push reference index
  int_t itrpos = _ccs->find_local_variable(iter_name); // use invalid id to make it inaccessible
  add_instruction<opcode::op_load_null>(itrpos);

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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_for_auto>(std::span<zs::token_type> sp) {
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

    return helper::handle_error(
        this, zs::error_code::invalid_token, "expected var or type", std::source_location::current());
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

          //          return helper::handle_error(this, zs::error_code::invalid_token, "expected ':' in for
          //          loop",
          //              std::source_location::current());
        }
      }
      else if (is(tok_identifier) and i > type_restriction_end_token_index and key_name.is_null()
          and has_key) {
        key_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          return helper::handle_error(this, zs::error_code::invalid_token, "expected ':' in for loop",
              std::source_location::current());
        }
      }

      else if (is(tok_colon, tok_in)) {
        if (colon_ptr) {
          return helper::handle_error(this, zs::error_code::invalid_token, "multiple ':' in for loop",
              std::source_location::current());
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
          //          return helper::handle_error(this, zs::error_code::invalid_token, "expected ':' in for
          //          loop",
          //              std::source_location::current());
        }
      }
      else if (is(tok_identifier) and key_name.is_null() and has_key) {
        key_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          //            has_key = true;
          return helper::handle_error(this, zs::error_code::invalid_token, "expected ':' in for loop",
              std::source_location::current());
        }
      }

      else if (is(tok_colon, tok_in)) {

        if (colon_ptr) {
          return helper::handle_error(this, zs::error_code::invalid_token, "multiple ':' in for loop",
              std::source_location::current());
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

    output_code = zs::strprint<"">(_engine, //
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

    output_code = zs::strprint<"">(_engine, //
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
    ZS_RETURN_IF_ERROR(parse<parse_op::p_decl_var>());
  }
  //  zb::print(_token, "dksjdkslajdksa");
  lex();
  // for(var i = 0
  //     ^
  if (is_var_decl_tok()) {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_decl_var>());
  }
  else if (is_not(tok_semi_colon)) {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());
    _ccs->pop_target();
  }

  // for(var i = 0;
  //              ^
  ZS_RETURN_IF_ERROR(expect(tok_semi_colon));

  // The next instruction is the beginning of the condition.
  const int_t cond_inst_start_idx = _ccs->get_next_instruction_index();

  // for(var i = 0; i < 10; i++)
  //                ^
  int_t cond_inst_jz_idx = -1;
  if (is_not(tok_semi_colon)) {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());
    add_instruction<opcode::op_jz>(0, _ccs->pop_target());
    cond_inst_jz_idx = _ccs->get_instruction_index();
    //    jz_next_index =_ccs->get_next_instruction_index();
  }

  const int_t cond_inst_last_idx = _ccs->get_instruction_index();
  //  const int_t cond_inst_end_idx = _ccs->get_next_instruction_index();
  const bool has_cond = cond_inst_jz_idx > 0;

  // for(var i = 0; i < 10;
  //                      ^
  ZS_RETURN_IF_ERROR(expect(tok_semi_colon));

  const int_t incr_expr_inst_start_idx = _ccs->get_next_instruction_index();

  if (is_not(tok_rbracket)) {
    // for(var i = 0; i < 10; i++)
    //                        ^
    ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());
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
  ZS_RETURN_IF_ERROR(expect(tok_rbracket));

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

  ZS_RETURN_IF_ERROR(parse<parse_op::p_statement>(true));

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

  add_instruction<opcode::op_jmp>((int32_t)(cond_inst_start_idx - end_block_index));

  if (has_cond > 0) {
    instruction_t<opcode::op_jz>* inst
        = (instruction_t<opcode::op_jz>*)(_ccs->_instructions._data.data(cond_inst_jz_idx));
    inst->offset = (int32_t)(_ccs->get_next_instruction_index() - cond_inst_jz_idx);
  }

  {
    int_t previous_n_capture = _ccs->_n_capture;

    if (_ccs->get_stack_size() != _scope.stack_size) {
      _ccs->set_stack_size(_scope.stack_size);
      if (previous_n_capture != (int_t)_ccs->_n_capture) {
        add_instruction<opcode::op_close>((uint32_t)_scope.stack_size);
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_for>() {
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
  //  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  if (!is(tok_lbracket)) {
    return zs::error_code::invalid_token;
  }

  lexer l(*_lexer);
  std::vector<zs::token_type> toks;

  toks.resize(50);
  std::span<zs::token_type> sp(toks);

  if (zs::status_result status = l.lex_for_auto(sp)) {
    ZS_ASSERT(sp.back() == tok_rbracket);
    return parse<parse_op::p_for_auto>(sp);
  }

  // for(var i = 0; i < 10; i++)
  //    ^
  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  scope previous_scope = _scope;
  _scope.n_captures = _ccs->_n_capture;
  _scope.stack_size = _ccs->get_stack_size();

  // for(var i = 0
  //     ^
  if (is_var_decl_tok()) {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_decl_var>());
  }
  else if (is_not(tok_semi_colon)) {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());
    _ccs->pop_target();
  }

  // for(var i = 0;
  //              ^
  ZS_RETURN_IF_ERROR(expect(tok_semi_colon));

  // The next instruction is the beginning of the condition.
  const int_t cond_inst_start_idx = _ccs->get_next_instruction_index();

  // for(var i = 0; i < 10; i++)
  //                ^
  int_t cond_inst_jz_idx = -1;
  if (is_not(tok_semi_colon)) {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());
    add_instruction<opcode::op_jz>(0, _ccs->pop_target());
    cond_inst_jz_idx = _ccs->get_instruction_index();
    //    jz_next_index =_ccs->get_next_instruction_index();
  }

  const int_t cond_inst_last_idx = _ccs->get_instruction_index();
  //  const int_t cond_inst_end_idx = _ccs->get_next_instruction_index();
  const bool has_cond = cond_inst_jz_idx > 0;

  // for(var i = 0; i < 10;
  //                      ^
  ZS_RETURN_IF_ERROR(expect(tok_semi_colon));

  const int_t incr_expr_inst_start_idx = _ccs->get_next_instruction_index();

  if (is_not(tok_rbracket)) {
    // for(var i = 0; i < 10; i++)
    //                        ^
    ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());
    _ccs->pop_target();
  }

  // for(var i = 0; i < 10; i++)
  //                           ^
  ZS_RETURN_IF_ERROR(expect(tok_rbracket));

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

  ZS_RETURN_IF_ERROR(parse<parse_op::p_statement>(true));

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

  add_instruction<opcode::op_jmp>((int32_t)(cond_inst_start_idx - end_block_index));

  if (has_cond > 0) {
    instruction_t<opcode::op_jz>* inst
        = (instruction_t<opcode::op_jz>*)(_ccs->_instructions._data.data(cond_inst_jz_idx));
    inst->offset = (int32_t)(_ccs->get_next_instruction_index() - cond_inst_jz_idx);
  }

  {
    int_t previous_n_capture = _ccs->_n_capture;

    if (_ccs->get_stack_size() != _scope.stack_size) {
      _ccs->set_stack_size(_scope.stack_size);
      if (previous_n_capture != (int_t)_ccs->_n_capture) {
        add_instruction<opcode::op_close>((uint32_t)_scope.stack_size);
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_statement>(bool close_frame) {
  using enum token_type;
  using enum opcode;
  using ps = parse_op;

  _ccs->add_line_infos(_lexer->get_line_info());

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
    return parse<ps::p_decl_var>();

  case tok_enum:
    return parse<ps::p_decl_enum>();

  case tok_hastag:
    return parse<ps::p_preprocessor>();

  case tok_include:
    ZS_TODO("Implement include(...)");
    return zs::error_code::unimplemented;

  case tok_import:
    ZS_TODO("Implement import(...)");
    return zs::error_code::unimplemented;

  case tok_if:
    return parse<ps::p_if>();

  case tok_for:
    return parse<ps::p_for>();

  case tok_foreach:
    return parse<ps::p_for_each>();

  case tok_class:
    return parse<ps::p_class_statement>();

  case tok_struct:
    return parse<ps::p_struct_statement>();

  case tok_global: {
    lex();

    if (is(tok_function)) {
      return parse<ps::p_global_function_statement>();
    }

    return helper::handle_error(
        this, zs::error_code::invalid_token, "expected function i guess", std::source_location::current());
  }

  case tok_function:
    return parse<ps::p_function_statement>();

  case tok_return: {
    lex();

    if (!is_end_of_statement()) {
      //        SQInteger retexp = _fs->GetCurrentPos() + 1;
      //        CommaExpr();
      ZS_RETURN_IF_ERROR(parse<ps::p_comma>());
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
      ZS_RETURN_IF_ERROR(parse<ps::p_statement>(true));

      if (_lexer->_last_token != tok_rcrlbracket && _lexer->_last_token != tok_semi_colon) {
        //            OptionalSemicolon();
        ZS_RETURN_IF_ERROR(parse<ps::p_semi_colon>());
      }
    }

    ZS_RETURN_IF_ERROR(expect(tok_rcrlbracket));

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
    ZS_RETURN_IF_ERROR(parse<ps::p_comma>());
    //    _ccs->pop_target();
    //      _fs->DiscardTarget();
    return {};
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_variable_type_restriction>(
    std::reference_wrapper<uint32_t> mask, std::reference_wrapper<uint64_t> custom_mask) {
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
    return helper::handle_error(
        this, zs::error_code::invalid_token, "expected var<...>", std::source_location::current());
  }

  lex();

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_global_function_statement>() {
  using enum token_type;
  using enum opcode;
  using ps = parse_op;

  lex();

  add_instruction<opcode::op_load_global>(_ccs->new_target());

  zs::object var_name;
  ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
  ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));

  int_t bound_target = 0xFF;

  if (is(tok_lsqrbracket)) {
    ZS_RETURN_IF_ERROR(parse<ps::p_bind_env>(std::ref(bound_target)));
  }

  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  ZS_RETURN_IF_ERROR(parse<ps::p_create_function>(std::cref(var_name), bound_target, false));

  add_instruction<op_new_closure>(
      (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

  int_t value_idx = _ccs->pop_target();
  int_t key_idx = _ccs->pop_target();
  int_t table_idx = _ccs->top_target();

  add_instruction<opcode::op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);

  _estate.type = expr_type::e_object;
  _estate.pos = table_idx;

  //    if (is_global) {
  //      lex();
  //      ZS_RETURN_IF_ERROR(expect(tok_dot));
  //      ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
  //
  //      add_instruction<opcode::op_load_global>(_ccs->new_target());
  //      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));
  //
  //      if(is(tok_dot)) {
  //        int_t key_idx = _ccs->pop_target();
  //        int_t table_idx = _ccs->pop_target();
  //        add_instruction<opcode::op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);
  //      }
  //
  //      while (is(tok_dot)) {
  //        lex();
  //        ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
  //        ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));
  //
  //        if(is(tok_dot)) {
  //          int_t key_idx = _ccs->pop_target();
  //          int_t table_idx = _ccs->pop_target();
  //          add_instruction<opcode::op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);
  //        }
  //      }
  //
  //    }
  //    else {
  //      ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
  //    }
  //
  //    int_t bound_target = 0xFF;
  //
  //    if (is(tok_lsqrbracket)) {
  //      ZS_RETURN_IF_ERROR(parse<ps::p_bind_env>(std::ref(bound_target)));
  //    }
  //
  //    ZS_RETURN_IF_ERROR(expect(tok_lbracket));
  //
  //    ZS_RETURN_IF_ERROR(parse<ps::p_create_function>(std::cref(var_name), bound_target, false));
  //
  //    add_instruction<op_new_closure>(
  //        (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);
  //
  //    if (is_global) {
  //      int_t value_idx = _ccs->pop_target();
  //      int_t key_idx = _ccs->pop_target();
  //      int_t table_idx = _ccs->top_target();
  //
  //      add_instruction<opcode::op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
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

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_function_statement>() {
  using enum token_type;
  using enum opcode;
  using ps = parse_op;

  lex();

  zs::object var_name;

  //    bool is_global = is(tok_global, tok_double_colon);
  bool is_global = is(tok_global);

  if (is_global) {
    lex();
    ZS_RETURN_IF_ERROR(expect(tok_dot));
    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));

    add_instruction<opcode::op_load_global>(_ccs->new_target());
    ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));

    if (is(tok_dot)) {
      int_t key_idx = _ccs->pop_target();
      int_t table_idx = _ccs->pop_target();
      add_instruction<opcode::op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);
    }

    while (is(tok_dot)) {
      lex();
      ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));

      if (is(tok_dot)) {
        int_t key_idx = _ccs->pop_target();
        int_t table_idx = _ccs->pop_target();
        add_instruction<opcode::op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);
      }
    }
  }
  else {
    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
  }

  int_t bound_target = 0xFF;

  if (is(tok_lsqrbracket)) {
    ZS_RETURN_IF_ERROR(parse<ps::p_bind_env>(std::ref(bound_target)));
  }

  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  ZS_RETURN_IF_ERROR(parse<ps::p_create_function>(std::cref(var_name), bound_target, false));

  add_instruction<op_new_closure>(
      (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

  if (is_global) {
    int_t value_idx = _ccs->pop_target();
    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->top_target();

    add_instruction<opcode::op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);

    _estate.type = expr_type::e_object;
    _estate.pos = table_idx;
  }
  else {
    _ccs->pop_target();
    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_function>(bool lambda) {
  using enum token_type;
  using enum opcode;

  lex();

  int_t bound_target = 0xFF;

  if (is(tok_lsqrbracket)) {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_bind_env>(std::ref(bound_target)));
  }

  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  object dummy;
  ZS_RETURN_IF_ERROR(parse<parse_op::p_create_function>(std::cref(dummy), bound_target, lambda));

  add_instruction<op_new_closure>(
      (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_function_call_args>(bool rawcall) {

  using enum token_type;

  int_t nargs = 1; // this.

  while (is_not(tok_rbracket)) {
    if (auto err = parse<parse_op::p_expression>()) {
      return err;
    }

    //        MoveIfCurrentTargetIsLocal();

    ZS_RETURN_IF_ERROR(action<action_type::act_move_if_current_target_is_local>());
    //    helper::move_if_current_target_is_local(this);
    nargs++;

    if (is(tok_comma)) {
      lex();

      if (is(tok_rbracket)) {

        return helper::handle_error(this, zs::error_code::invalid_token, "expression expected, found ')'",
            std::source_location::current());
      }
    }
  }
  lex();

  ZS_TODO("Implement");
  // Rawcall.
  if (rawcall) {

    if (nargs < 3) {

      return helper::handle_error(this, zs::error_code::invalid_argument,
          "rawcall requires at least 2 parameters (callee and this)", std::source_location::current());
    }

    nargs -= 2; // removes callee and this from count
  }

  //  zb::print("NARGS", nargs, _ccs->top_target());

  for (int_t i = 0; i < (nargs - 1); i++) {

    _ccs->pop_target();
  }

  int_t stack_base = _ccs->pop_target();

  int_t closure = _ccs->pop_target();

  add_instruction<opcode::op_call>(_ccs->new_target(), // target_idx.
      (uint8_t)closure, // closure_idx.
      (uint8_t)stack_base, // this_idx.
      (uint8_t)nargs, // n_params.
      (uint64_t)stack_base // stack_base.
  );

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_function_call_args_template>(
    std::string_view meta_code) {

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
        if (auto err = parse<parse_op::p_expression>()) {
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

    ZS_RETURN_IF_ERROR(action<action_type::act_move_if_current_target_is_local>());
  }

  while (is_not(tok_rbracket)) {
    if (auto err = parse<parse_op::p_expression>()) {
      return err;
    }

    ZS_RETURN_IF_ERROR(action<action_type::act_move_if_current_target_is_local>());
    nargs++;

    if (is(tok_comma)) {
      lex();

      if (is(tok_rbracket)) {
        return helper::handle_error(this, zs::error_code::invalid_token, "expression expected, found ')'",
            std::source_location::current());
      }
    }
  }

  lex();

  for (int_t i = 0; i < (nargs - 1); i++) {
    _ccs->pop_target();
  }

  int_t stack_base = _ccs->pop_target();

  int_t closure = _ccs->pop_target();

  add_instruction<op_call>(_ccs->new_target(), // target_idx.
      (uint8_t)closure, // closure_idx.
      (uint8_t)stack_base, // this_idx.
      (uint8_t)nargs, // n_params.
      (uint64_t)stack_base // stack_base.
  );

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_table_or_class>(
    token_type separator, token_type terminator) {
  using enum token_type;
  using enum opcode;

  //  int_t tpos = _fs->GetCurrentPos(), nkeys = 0;
  while (_token != terminator) {
    //    bool hasattrs = false;
    bool is_static = false;

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
        ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
      }
      //    if (_token == tok_lcrlbracket) {
      //            boundtarget = ParseBindEnv();
      //    }

      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));

      //      add_instruction<opcode::op_load>(_ccs->new_target(), (uint32_t)_ccs->get_literal(ret_value));
      //      _fs->AddInstruction(_OP_LOAD, _ccs->new_target(), _fs->GetConstant(id));

      ZS_RETURN_IF_ERROR(expect(tok_lbracket));

      ZS_RETURN_IF_ERROR(parse<parse_op::p_create_function>(std::cref(var_name), bound_target, false));

      //      CreateFunction(varname, 0xFF, false);

      add_instruction<op_new_closure>(
          (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);

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
      ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());
      ZS_RETURN_IF_ERROR(expect(tok_rsqrbracket));
      ZS_RETURN_IF_ERROR(expect(tok_eq));
      ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());

      break;
    }

    case tok_string_value:
    case tok_escaped_string_value: {
      // Only works for tables
      if (separator == tok_comma) {
        zs::object value;
        ZS_RETURN_IF_ERROR(expect_get(tok_string_value, value));
        ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, value));
        ZS_RETURN_IF_ERROR(expect(tok_colon));
        ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());
        break;
      }
    }
    default:
      zs::object identifier;
      ZS_RETURN_IF_ERROR(expect_get(tok_identifier, identifier));
      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, identifier));

      ZS_RETURN_IF_ERROR(expect(tok_eq));
      ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());
    }

    if (_token == separator) {
      // optional comma/semicolon
      lex();
    }

    //    nkeys++;
    int_t val = _ccs->pop_target();
    int_t key = _ccs->pop_target();
    //    SQInteger attrs = hasattrs ? _fs->PopTarget() : -1;
    //    ((void)attrs);

    //    assert((hasattrs && (attrs == key - 1)) || !hasattrs);
    //
    //      unsigned char flags = (hasattrs ? NEW_SLOT_ATTRIBUTES_FLAG : 0) |
    //      (isstatic ? NEW_SLOT_STATIC_FLAG
    //      :
    //    0);

    //<<BECAUSE OF THIS NO COMMON EMIT FUNC IS POSSIBLE.
    int_t table = _ccs->top_target();

    // hack recognizes a table from the separator
    if (separator == tok_comma) {
      add_instruction<opcode::op_new_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val);
      //        _fs->AddInstruction(_OP_NEWSLOT, 0xFF, table, key, val);
    }
    else {
      // This for classes only as it invokes _newmember.
      add_instruction<opcode::op_new_class_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val, is_static);
    }
  }

  // hack recognizes a table from the separator
  if (separator == tok_comma) {
    //    _fs->SetInstructionParam(tpos, 1, nkeys);
  }
  lex();

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_create_function>(
    std::reference_wrapper<const object> name, int_t boundtarget, bool lambda) {
  using enum token_type;
  using enum object_type;

  zs::closure_compile_state* fct_state = _ccs->push_child_state();
  fct_state->name = name;
  fct_state->source_name = _ccs->source_name;

  ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("this")));

  int_t def_params = 0;

  // Parsing function parameters: `function (parameters)`.
  while (is_not(tok_rbracket)) {
    if (is(tok_triple_dots)) {
      // TODO: Named triple dots?

      if (def_params > 0) {
        return helper::handle_error(this, zs::error_code::invalid_argument,
            "function with default parameters cannot have variable number of "
            "parameters",
            std::source_location::current());
      }

      ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("vargv")));
      fct_state->_vargs_params = true;
      lex();

      if (is_not(tok_rbracket)) {
        return helper::handle_error(this, zs::error_code::invalid_token,
            "expected ')' after a variadic (...) parameter", std::source_location::current());
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
            if (auto err
                = parse<parse_op::p_variable_type_restriction>(std::ref(mask), std::ref(custom_mask))) {
              return helper::handle_error(this, err, "parsing variable type restriction `var<....>`",
                  std::source_location::current());
            }
          }
          break;
        }
      }

      zs::object param_name;
      ZS_RETURN_IF_ERROR(expect_get(tok_identifier, param_name));
      ZS_RETURN_IF_ERROR(fct_state->add_parameter(param_name, mask, custom_mask, is_const));

      if (is(tok_eq)) {
        lex();
        ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());
        fct_state->add_default_param(_ccs->top_target());
        def_params++;
      }

      // If a default parameter was defined, all of them (from that point) needs
      // to have one too.
      else if (def_params > 0) {
        return helper::handle_error(this, zs::error_code::invalid_token,
            "expected '=' after a default paramter definition", std::source_location::current());
      }

      if (is(tok_comma)) {
        lex();
      }
      else if (is_not(tok_rbracket)) {
        return helper::handle_error(this, zs::error_code::invalid_token,
            "expected ')' or ',' at the end of function declaration", std::source_location::current());
      }
    }
  }
  //  //

  ZS_RETURN_IF_ERROR(expect(tok_rbracket));

  //  if (boundtarget != 0xFF) {
  //    //      _fs->pop_target();
  //  }
  //
  for (int_t n = 0; n < def_params; n++) {
    _ccs->pop_target();
  }
  //
  zs::closure_compile_state* curr_chunk = std::exchange(_ccs, fct_state);
  //  _ccs = fct_state;

  if (is_not(tok_lcrlbracket) and lambda) {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());
  }
  else {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_statement>(false));
  }

  //  if (lambda) {
  //    ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());
  //    //      _fs->AddInstruction(_OP_RETURN, 1, _fs->PopTarget());
  //  }
  //  else {
  //    ZS_RETURN_IF_ERROR(parse<parse_op::p_statement>(false));
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
  //  add_instruction<opcode::op_return>(0, false);
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

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_class>() {
  using enum token_type;

  //  int_t base = -1;
  //  int_t attrs = -1;
  if (is(tok_extend)) {
    lex();
    ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());
    //    base = _fs->TopTarget();
  }

  if (is(tok_attribute_begin)) {
    lex();
    //    _fs->AddInstruction(_OP_NEWOBJ, _fs->PushTarget(), 0, 0, NOT_TABLE);
    ZS_RETURN_IF_ERROR(parse<parse_op::p_table_or_class>(tok_comma, tok_attribute_end));
    //    attrs = _fs->TopTarget();
  }

  ZS_RETURN_IF_ERROR(expect(tok_lcrlbracket));

  //  if (attrs != -1)
  //    _fs->PopTarget();
  //  if (base != -1)
  //    _fs->PopTarget();
  //  _fs->AddInstruction(_OP_NEWOBJ, _fs->PushTarget(), base, attrs,
  //  NOT_CLASS);
  add_instruction<opcode::op_new_obj>(_ccs->new_target(), object_type::k_class);
  //  lex();
  ZS_RETURN_IF_ERROR(parse<parse_op::p_table_or_class>(tok_semi_colon, tok_rcrlbracket));

  return {};
}

//  template <>
//  zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_function_statement>() {
//    using enum token_type;
//    using enum opcode;
//
//    lex();
//
//    zs::object_ptr var_name;
//
//    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
//
//
//    int_t bound_target = 0xFF;
//
//    if (is(tok_lsqrbracket)) {
//      ZS_RETURN_IF_ERROR(parse<parse_op::p_bind_env>(std::ref(bound_target)));
//    }
//
//    ZS_RETURN_IF_ERROR(expect(tok_lbracket));
//
//    ZS_RETURN_IF_ERROR(parse<parse_op::p_create_function>(std::cref(var_name), bound_target, false));
//
//    add_instruction<op_new_closure>(
//        (uint8_t)_ccs->new_target(), (uint32_t)(_ccs->_functions.size() - 1), (uint8_t)bound_target);
//
//    _ccs->pop_target();
//    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
//
//    return {};
//  }
//

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_class_statement>() {
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
    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));
    ZS_RETURN_IF_ERROR(parse<parse_op::p_class>());
    _ccs->pop_target();
    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));
  }
  else {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_prefixed>());

    switch (_estate.type) {
    case expr_type::e_expr:
      return helper::handle_error(
          this, zs::error_code::invalid_operation, "Invalid class name", std::source_location::current());

    case expr_type::e_base:
      ZBASE_NO_BREAK;
    case expr_type::e_object: {
      ZS_RETURN_IF_ERROR(parse<parse_op::p_class>());

      int_t val = _ccs->pop_target();
      int_t key = _ccs->pop_target();
      //      int_t table = _ccs->top_target();
      int_t table = _ccs->pop_target();

      add_instruction<opcode::op_new_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val);

      //        _ccs->pop_target();
      //        ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));

      //      _ccs->pop_target();
      break;
    }

    case expr_type::e_local:
      ZBASE_NO_BREAK;
    case expr_type::e_capture:

      return helper::handle_error(this, zs::error_code::invalid_operation,
          "Cannot create a class in a local with the syntax(class <local>)", std::source_location::current());
    }
  }

  //

  _estate = es;
  return {};
}

//
// MARK: Compiler.
//

jit_compiler::jit_compiler(zs::engine* eng)
    : engine_holder(eng)
    , _error_message(zs::allocator<char>(eng))
    , _compile_time_consts(zs::object::create_table(eng)) {
  _scope.n_captures = 0;
  _scope.stack_size = 0;
}

jit_compiler::~jit_compiler() {}

inline token_type jit_compiler::lex() {

  _token = _lexer->lex();

  if (_in_template and _token == tok_gt) {
    _token = tok_rsqrbracket;
  }
  // #ifdef ZS_COMPILER_DEV
  //   _dev.push_token_info(_token, _lexer->get_line_info(),
  //   _lexer->get_debug_value());
  // #endif // ZS_COMPILER_DEV.

  return _token;
}

zs::error_result jit_compiler::compile(std::string_view content, std::string_view filename, object& output,
    zs::virtual_machine* vm, zs::token_type* prepended_token, bool with_vargs) {
  using enum token_type;

  _vm = vm;
  zs::lexer lexer(_engine);

  _lexer = &lexer;

  //  _shared_state = &shared_state;
  _lexer->init(content);

  // Init expression state.
  //  _exp_state.etype = zs::expression_type::expr;
  //  _exp_state.epos = 0;
  //  _exp_state.do_not_get = true;

  zs::closure_compile_state c_compile_state(_engine, nullptr);
  c_compile_state.name = zs::object(_engine, "main");
  c_compile_state.source_name = zs::object(_engine, filename);

  _ccs = &c_compile_state;
  ZS_RETURN_IF_ERROR(_ccs->add_parameter(object::create_small_string("this")));
 
  if(with_vargs) {
    ZS_RETURN_IF_ERROR(_ccs->add_parameter(object::create_small_string("vargs")));
    add_instruction<opcode::op_load_null>(_ccs->new_target());
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

    if (auto err = parse<parse_op::p_statement>(true)) {
      return err;
    }

    if (!zb::is_one_of(_lexer->last_token(), tok_rcrlbracket, tok_semi_colon)) {
      ZS_RETURN_IF_ERROR(parse<parse_op::p_semi_colon>());
    }
  }

  if (_token == tok_lex_error) {
    return zs::error_code::invalid;
  }

  _ccs->set_stack_size(stack_size);
  _ccs->add_line_infos(_lexer->get_line_info());

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

zs::error_result jit_compiler::parse_include_or_import_statement(token_type tok) {
  using enum token_type;
  const bool is_import = tok == tok_import;

  ZS_RETURN_IF_ERROR(expect(tok));

  const zs::line_info linfo = _lexer->get_line_info();

  object file_name = _lexer->get_value();

  if (_token == tok_lt) {
    lex();
    file_name = _lexer->get_value();
    lex();
    if (_token != tok_gt) {
      _error_message += zs::strprint(_engine, "parse include statement", linfo);
      return zs::error_code::invalid_include_syntax;
    }
  }

  if (!file_name.is_string()) {
    _error_message += zs::strprint(_engine, "parse include statement", linfo);
    return zs::error_code::invalid_include_syntax;
  }

  object res_file_name;
  if (auto err = _engine->resolve_file_path(file_name.get_string_unchecked(), res_file_name)) {
    _error_message += zs::strprint(_engine, "parse include statement", linfo);
    return zs::error_code::invalid_include_file;
  }

  if (is_import) {
    // Check for multiple inclusion.
    if (auto it = _ccs->_imported_files_set.find(res_file_name); it != _ccs->_imported_files_set.end()) {
      // Already imported, all good.
      lex();
      return {};
    }

    _ccs->_imported_files_set.insert(res_file_name);
  }

  zs::file_loader loader(_engine);

  zbase_assert(!res_file_name.is_string_view(), "cannot be a string_view");

  if (auto err = loader.open(res_file_name.get_string_unchecked().data())) {
    _error_message += zs::strprint(_engine, "parse include statement", linfo);
    return zs::error_code::open_file_error;
  }

  zs::lexer* last_lexer = _lexer;
  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(loader.content());

  lex();
  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    ZS_RETURN_IF_ERROR(parse<parse_op::p_statement>(true));

    if (!zb::is_one_of(_lexer->last_token(), tok_rcrlbracket, tok_semi_colon)) {
      ZS_RETURN_IF_ERROR(parse<parse_op::p_semi_colon>());
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_bind_env>(
    std::reference_wrapper<int_t> target) {

  lex();

  // TODO: Fix this.
  ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());
  int_t boundtarget = _ccs->top_target();

  target.get() = boundtarget;
  //    Expect(_SC(']'));
  //  return boundtarget;
  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_factor_identifier>() {
  using enum token_type;

  object var_name = _lexer->get_identifier();

  lex();

  // Check if `var_name` is a local variable.
  if (int_t pos = _ccs->find_local_variable(var_name); pos != -1) {
    _estate.type = expr_type::e_local;
    _estate.pos = pos;
    _ccs->push_target(pos);
    //    target.get() = pos;
    return {};
  }

  if (int_t pos = _ccs->get_capture(var_name); pos != -1) {
    // Handle a captured var.
    if (helper::needs_get(this)) {
      _estate.pos = _ccs->new_target();
      _estate.type = expr_type::e_expr;
      add_instruction<opcode::op_get_capture>((uint8_t)_estate.pos, (uint32_t)pos);
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
  ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));

  if (helper::needs_get_no_assign(this)) {
    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->pop_target();
    add_instruction<opcode::op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_factor_at>() {
  using enum token_type;

  lex();

  _ccs->push_target(0);
  ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, std::string_view("tostring"), _ccs->new_target()));

  if (helper::needs_get(this)) {
    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->pop_target();
    add_instruction<opcode::op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);
    _estate.type = expr_type::e_object;
    _estate.pos = table_idx;

    // TODO: ???
    //    target.get() = table_idx;
  }
  else {

    if (is_not(tok_lbracket)) {
      _error_message += zs::strprint(_engine, "Trying to assign a global variable", _lexer->get_line_info());
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_prefixed>() {
  using enum token_type;

  //  int_t pos = -1;
  ZS_RETURN_IF_ERROR(parse<parse_op::p_factor>());

  zb_loop() {

    if (is(tok_lt) and helper::is_template_function_call(this)) {
      ZS_RETURN_IF_ERROR(parse<parse_op::p_prefixed_lbracket_template>());
    }

    switch (_token) {
    case tok_dot: {
      //        pos = -1;
      lex();

      object var_name = object(_engine, _lexer->get_identifier_value());

      ZS_RETURN_IF_ERROR(expect(tok_identifier));

      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name));

      if (is(tok_eq)) {
        if (_estate.no_assign) {
          return helper::handle_error(this, zs::error_code::invalid_operation,
              "cannot assign a global variable without the global keyword", std::source_location::current());
        }

        int_t table_idx = _ccs->get_up_target(1);
        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
        _estate.no_assign = false;
      }
      else if (helper::needs_get(this)) {
        int_t key_idx = _ccs->pop_target();
        int_t table_idx = _ccs->pop_target();
        add_instruction<opcode::op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, false);
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
      ZS_RETURN_IF_ERROR(parse<parse_op::p_prefixed_lbracket>());
      break;
    }

    case tok_lsqrbracket: {
      if (_lexer->last_token() == tok_endl) {
        return helper::handle_error(this, zs::error_code::invalid_token,
            "cannot break deref/or comma needed after [exp]=exp slot "
            "declaration",
            std::source_location::current());
      }

      lex();
      ZS_RETURN_IF_ERROR(parse<parse_op::p_expression>());
      ZS_RETURN_IF_ERROR(expect(tok_rsqrbracket));

      if (is(tok_eq)) {
        int_t table_idx = _ccs->get_up_target(1);
        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
      }
      else if (helper::needs_get(this)) {
        int_t key_idx = _ccs->pop_target();
        int_t table_idx = _ccs->pop_target();
        add_instruction<opcode::op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, false);
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
        return helper::handle_error(this, zs::error_code::invalid_operation,
            "Can't '++' or '--' an expression", std::source_location::current());

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
        add_instruction<opcode::op_incr>(_ccs->new_target(), src, is_incr);
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
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_factor>() {
  using enum token_type;
  using enum opcode;

  switch (_token) {
  case tok_double_at: {
    lex();

    object var_name;
    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));

    object ret_value;
    ZS_RETURN_IF_ERROR(_compile_time_consts._table->get(var_name, ret_value));

    switch (ret_value.get_type()) {
    case object_type::k_null:
      break;

    case object_type::k_bool:
      add_instruction<opcode::op_load_bool>(_ccs->new_target(), ret_value._bool);
      break;

    case object_type::k_integer:
      add_instruction<opcode::op_load_int>(_ccs->new_target(), ret_value._int);
      break;

    case object_type::k_float:
      add_instruction<opcode::op_load_float>(_ccs->new_target(), ret_value._float);
      break;

    case object_type::k_small_string:
      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, ret_value.get_small_string_unchecked()));
      break;

    case object_type::k_long_string:
      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, ret_value.get_long_string_unchecked()));
      break;

    case object_type::k_string_view:
      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, ret_value.get_string_view_unchecked()));
      break;

    default:
      return ZS_COMPILER_HANDLE_ERROR_STRING(zs::error_code::invalid_token, "define todo table ...");
    }

    break;
  }

  case tok_string_value: {
    std::string_view svalue = _lexer->get_string_value();
    ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, svalue));
    lex();
    break;
  }

  case tok_escaped_string_value: {
    std::string_view svalue = _lexer->get_escaped_string_value();
    ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, svalue));
    lex();
    break;
  }

  case tok_typeid: {
    lex();

    if (zs::error_result err = expect(tok_lbracket)) {
      return helper::handle_error(
          this, zs::error_code::invalid_token, "expected '(' after typeid", std::source_location::current());
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse<parse_op::p_expression>()) {
        zb::print("ERRRO");
        return err;
      }
    }

    int_t tg = _ccs->pop_target();
    add_instruction<opcode::op_typeid>(_ccs->new_target(), tg);

    lex();
    break;
  }

  case tok_typeof: {
    lex();

    if (zs::error_result err = expect(tok_lbracket)) {
      return helper::handle_error(
          this, zs::error_code::invalid_token, "expected '(' after typeof", std::source_location::current());
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse<parse_op::p_expression>()) {
        zb::print("ERRRO");
        return err;
      }
    }

    int_t tg = _ccs->pop_target();
    add_instruction<opcode::op_typeof>(_ccs->new_target(), tg);

    lex();
    break;
  }

  case tok_double_colon: {

    add_instruction<opcode::op_load_root>(_ccs->new_target());
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

    ZS_RETURN_IF_ERROR(helper::add_small_string_instruction(this, "import"));

    int_t key_idx = _ccs->pop_target();
    int_t table_idx = _ccs->pop_target();
    add_instruction<opcode::op_get>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx, true);

    _estate.type = expr_type::e_object;

    break;
  }

  case tok_base: {
    lex();
    add_instruction<opcode::op_get_base>((uint8_t)_ccs->new_target());
    _estate.type = expr_type::e_base;
    _estate.pos = _ccs->top_target();
    //    target.get() = _ccs->top_target();
    return {};
  }

  case tok_this: {
    return parse<parse_op::p_factor_identifier>();
  }
  case tok_global: {
    add_instruction<opcode::op_load_global>(_ccs->new_target());

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
    return parse<parse_op::p_factor_identifier>();

  case tok_at:
    return parse<parse_op::p_factor_at>();

  case tok_null:
    add_instruction<opcode::op_load_null>(_ccs->new_target());
    lex();
    break;

  case tok_none:
    add_instruction<opcode::op_load_none>(_ccs->new_target());
    lex();
    break;

  case tok_integer_value:
    add_instruction<opcode::op_load_int>(_ccs->new_target(), _lexer->get_int_value());
    lex();
    break;

  case tok_float_value:
    add_instruction<opcode::op_load_float>(_ccs->new_target(), _lexer->get_float_value());
    lex();
    break;

  case tok_true:
    add_instruction<opcode::op_load_bool>(_ccs->new_target(), true);
    lex();
    break;

  case tok_false:
    add_instruction<opcode::op_load_bool>(_ccs->new_target(), false);
    lex();
    break;

  case tok_minus: {
    lex();

    switch (_token) {
    case tok_integer_value:
      add_instruction<opcode::op_load_int>(_ccs->new_target(), -_lexer->get_int_value());
      lex();
      break;

    case tok_float_value:
      add_instruction<opcode::op_load_float>(_ccs->new_target(), -_lexer->get_float_value());
      lex();
      break;

    default:
      // UnaryOP(_OP_NEG);
      return helper::handle_error(
          this, zs::error_code::unimplemented, "unimplemented unary minus", std::source_location::current());

      break;
    }
    break;
  }

  case tok_lsqrbracket: {
    // Array.
    add_instruction<opcode::op_new_obj>(_ccs->new_target(), object_type::k_array);
    lex();

    while (is_not(tok_rsqrbracket)) {
      if (auto err = parse<parse_op::p_expression>()) {
        //          return err;
        zb::print("ERRRO");
        return err;
      }

      if (_token == tok_comma) {
        lex();
      }

      int_t val = _ccs->pop_target();
      int_t array = _ccs->top_target();

      add_instruction<opcode::op_array_append>((uint8_t)array, (uint8_t)val);

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
    ZS_RETURN_IF_ERROR(parse<parse_op::p_struct>());
    break;
  }

  case tok_lcrlbracket: {
    add_instruction<opcode::op_new_obj>(_ccs->new_target(), object_type::k_table);
    lex();
    ZS_RETURN_IF_ERROR(parse<parse_op::p_table_or_class>(tok_comma, tok_rcrlbracket));
    break;
  }

  case tok_dollar: {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_function>(true));
    break;
  }
  case tok_function: {
    ZS_RETURN_IF_ERROR(parse<parse_op::p_function>(false));
    break;
  }

  case tok_class: {
    lex();
    ZS_RETURN_IF_ERROR(parse<parse_op::p_class>());
    break;
  }

    //  case tok_not:
    //    lex();
    //
    //    switch (_token) {
    //    case tok_false:
    //      add_instruction<opcode::op_load_bool>(_ccs->new_target(), true);
    //      lex();
    //      break;
    //    case tok_true:
    //      add_instruction<opcode::op_load_bool>(_ccs->new_target(), false);
    //      lex();
    //      break;
    //    case tok_integer_value:
    //      add_instruction<opcode::op_load_bool>(_ccs->new_target(), !(bool)_lexer->get_int_value());
    //      lex();
    //      break;
    //
    //    case tok_float_value:
    //      add_instruction<opcode::op_load_bool>(_ccs->new_target(), !(bool)_lexer->get_float_value());
    //      lex();
    //      break;
    //
    //    case tok_identifier:
    //      add_instruction<opcode::op_not>(
    //          _ccs->new_target(), _ccs->find_local_variable(_lexer->get_identifier()));
    //      lex();
    //      break;
    //
    //    default:
    //      // UnaryOP(_OP_NEG);
    //      return helper::handle_error(
    //          this, zs::error_code::unimplemented, "unimplemented unary minus",
    //          std::source_location::current());
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

    ZS_RETURN_IF_ERROR(parse<parse_op::p_prefixed>());

    switch (_estate.type) {
    case expr_type::e_expr: {
      int_t src = _ccs->pop_target();
      add_instruction<opcode::op_not>((uint8_t)_ccs->new_target(), (uint8_t)src);
      break;
      ;
    }

    case expr_type::e_base:
      return helper::handle_error(this, zs::error_code::invalid_operation, "Can't '++' or '--' a base",
          std::source_location::current());

    case expr_type::e_object: {
      //        int_t key_idx = _ccs->pop_target();
      //        int_t table_idx = _ccs->pop_target();

      //        add_instruction<opcode::op_pobjincr>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx,
      //        is_incr);
      return zs::error_code::unimplemented;
      break;
      ;
    }

    case expr_type::e_local: {
      int_t src = _ccs->pop_target();
      add_instruction<opcode::op_not>((uint8_t)_ccs->new_target(), (uint8_t)src);
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
    ZS_RETURN_IF_ERROR(parse<parse_op::p_prefixed_incr>(false));
    break;

  case tok_incr:
    ZS_RETURN_IF_ERROR(parse<parse_op::p_prefixed_incr>(true));
    break;

  case tok_lbracket: {
    lex();

    ZS_RETURN_IF_ERROR(parse<parse_op::p_comma>());

    if (zs::error_result err = expect(tok_rbracket)) {
      return helper::handle_error(
          this, zs::error_code::unimplemented, "expression expected", std::source_location::current());
    }
    break;
  }

  case tok_file:
      ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, _ccs->source_name ));
      lex();
    break;

  case tok_line: {
    add_instruction<opcode::op_load_int>(_ccs->new_target(), _lexer->_current_line);
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

    ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, line_content));

    //        add_instruction<opcode::op_load_int>(_ccs->new_target(), _lexer->_current_line);
    lex();
    break;
  }
  case tok_hastag: {
    lex();

    object id;
    ZS_RETURN_IF_ERROR(expect_get(tok_identifier, id));

    if (id == "as_string") {
      // Get the content of a file as string.
      ZS_RETURN_IF_ERROR(parse<jit_compiler::parse_op::p_as_string>());
    }
    else if (id == "as_table") {
      // Get the content of a file as table.
      ZS_RETURN_IF_ERROR(parse<jit_compiler::parse_op::p_as_table>());
    }
    else if (id == "as_value") {
      // Get the content of a file as value.
      ZS_RETURN_IF_ERROR(parse<jit_compiler::parse_op::p_as_value>());
    }
    else if (id == "load_json_file") {
      // Get the content of a file as table.
      ZS_RETURN_IF_ERROR(parse<jit_compiler::parse_op::p_load_json_file>());
    }
    else {
      return helper::handle_error(this, zs::error_code::invalid_include_syntax,
          "expected `as_string`, `as_table` or ??", std::source_location::current());
    }
    break;
  }

  default: {
    return helper::handle_error(
        this, zs::error_code::unimplemented, "expression expected", std::source_location::current());
    break;
  }
  }

  //  target.get() = -1;
  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_load_json_file>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  object filepath_value;
  ZS_RETURN_IF_ERROR(expect_get(tok_string_value, filepath_value));
  ZS_RETURN_IF_ERROR(expect(tok_rbracket));

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_HANDLE_ERROR_STRING(zs::error_code::open_file_error, "cannot open file `as_string`");
  }

  zs::json_parser parser(_engine);

  object ret_value;
  if (auto err = parser.parse(_vm, loader.content(), nullptr, ret_value)) {
    return ZS_COMPILER_HANDLE_ERROR_STREAM(err, "parse failed", parser.get_error());
  }

  add_instruction<opcode::op_load>(_ccs->new_target(), (uint32_t)_ccs->get_literal(ret_value));

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_as_string>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  object filepath_value;
  ZS_RETURN_IF_ERROR(expect_get(tok_string_value, filepath_value));
  ZS_RETURN_IF_ERROR(expect(tok_rbracket));

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_HANDLE_ERROR_STRING(zs::error_code::open_file_error, "cannot open file `as_string`");
  }

  std::string_view svalue = loader.content();
  ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, svalue));

  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_as_value>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  object filepath_value;
  ZS_RETURN_IF_ERROR(expect_get(tok_string_value, filepath_value));
  ZS_RETURN_IF_ERROR(expect(tok_rbracket));

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_HANDLE_ERROR_STRING(zs::error_code::open_file_error, "cannot open file `as_value`");
  }

  std::string_view svalue = loader.content();

  zs::vm vm(_engine);
  object ret_value;
  if (auto err = vm->load_buffer_as_value(svalue, filepath_value.get_string_unchecked(), ret_value)) {
    return ZS_COMPILER_HANDLE_ERROR_STREAM(
        zs::error_code::invalid_include_file, "load `as_value` compile failed\n", vm->get_error());
  }

  add_instruction<opcode::op_load>(_ccs->new_target(), (uint32_t)_ccs->get_literal(ret_value));
  return {};
}

template <>
zs::error_result jit_compiler::parse<jit_compiler::parse_op::p_as_table>() {
  using enum token_type;

  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  object filepath_value;
  ZS_RETURN_IF_ERROR(expect_get(tok_string_value, filepath_value));
  ZS_RETURN_IF_ERROR(expect(tok_rbracket));

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_HANDLE_ERROR_STRING(zs::error_code::open_file_error, "cannot open file `as_table`");
  }

  std::string_view svalue = loader.content();

  zs::vm vm(_engine);
  object ret_value;
  if (auto err = vm->load_buffer_as_value(svalue, filepath_value.get_string_unchecked(), ret_value)) {
    return ZS_COMPILER_HANDLE_ERROR_STREAM(
        zs::error_code::invalid_include_file, "load `as_table` compile failed\n", vm->get_error());
  }

  if (!ret_value.is_table()) {
    return ZS_COMPILER_HANDLE_ERROR_STRING(
        zs::error_code::invalid_include_file, "result value is not a table in `as_table`");
  }

  add_instruction<opcode::op_load>(_ccs->new_target(), (uint32_t)_ccs->get_literal(ret_value));
  return {};
}

zs::error_code jit_compiler::expect(token_type tok) noexcept {

  if (is_not(tok)) {

    return ZS_COMPILER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
        zb::quoted<"'">(zs::token_to_string(tok)));
    return zs::error_code::invalid_token;
  }

  lex();
  return zs::error_code::success;
}

zs::error_code jit_compiler::expect_get(token_type tok, object& ret) {
  using enum token_type;

  if (is_not(tok)) {
    return ZS_COMPILER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
        zb::quoted<"'">(zs::token_to_string(tok)));
  }

  ret = _lexer->get_value();
  lex();
  return {};
}
} // namespace zs.

#include "lang/jit/zjit_struct.h"

ZBASE_PRAGMA_POP()
