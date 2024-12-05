#include "lang/jit/zjit_compiler.h"
#include <zbase/strings/parse_utils.h>
#include <zbase/memory/ref_wrapper.h>

#include "zvirtual_machine.h"
#include "objects/zfunction_prototype.h"
#include "lang/jit/zclosure_compile_state.h"
#include "json/zjson_lexer.h"
#include "json/zjson_parser.h"
#include "xml/zxml_lexer.h"
#include "xml/zxml_parser.h"

#define ZS_COMPILER_PARSE_CPP 1
#include "lang/jit/zjit_compiler_defs.h"

ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

namespace zs {
jit_compiler::jit_compiler(zs::engine* eng)
    : engine_holder(eng)
    , _error_message(zs::allocator<char>(eng))
    , _errors(eng)
    , _compile_time_consts(zs::object::create_table(eng))
    , _doc_blocks(zs::allocator<object>(eng))
    , _macros(zs::allocator<macro>(eng)) {
  _scope_id_counter = 0;
  _scope.n_captures = 0;
  _scope.stack_size = 0;
  _scope.scope_id = 0;
}

// zs::error_result jit_compiler::handle_error(
//     zs::error_code ec, std::string_view msg, const zs::compiler_source_location& loc) {
//
//   std::string_view line_content = get_line_content(_lexer->stream());
//   zs::line_info linfo = _lexer->get_line_info();
//
//   _error_message += zs::strprint(_engine, "\nError: ", linfo, "\n'''", line_content, "\n",
//       zb::indent_t(linfo.column - 1), "^\n'''\n\nfrom function : '", loc.function_name(),
//       "'\n     file     : '", loc.file_name(), "'\n     line     : ",loc.line()," ", loc.line_content(),
//       "\n\nMessage:\n", msg);
//
//   return ec;
// }
// inline error_message(zs::engine* eng, error_source esrc, zs::error_code ec, Message&& message,Filename&&
// filename,  Code&& code, zs::line_info line, const zs::developer_source_location& loc)

ZS_CHECK zs::string jit_compiler::get_error() const noexcept {

  zs::ostringstream stream(zs::create_string_stream(_engine));

  _errors.print(stream);

  return stream.str();
}

zs::error_result jit_compiler::handle_error(zs::error_code ec, const zs::line_info& linfo,
    std::string_view msg, const zs::developer_source_location& loc) {
  std::string_view line_content = get_line_content(_lexer->stream(), linfo);

  _errors.emplace_back(_engine, error_source::compiler, ec, msg,
      _ccs->state_data()._source_name.is_string() ? _ccs->state_data()._source_name.get_string_unchecked()
                                                  : "",
      line_content, linfo, loc);

  //  zs::ostringstream stream(zs::create_string_stream(_engine));
  //  stream << "error: " << msg <<(msg.ends_with('\n') ? "" : "\n") <<"line: "<< linfo.line << ":" <<
  //  linfo.column << "\n\n";
  //
  //  stream <<"'''"<< line_content << "\n\n'''\n";
  //
  //  stream << "\nDev:\n  file: '" <<loc.file_name() << "'\n  function: '" <<loc.function_name() << "'\n
  //  line: "<<loc.line() <<"\n";
  //
  //  if(!loc.line_content().empty()) {
  //    stream  << "  content: '"<< loc.line_content()<<"'\n";
  //  }
  //
  //
  //  _error_message.push_back('\n');
  //  _error_message.append(stream.str());
  //  zb::stream_print(stream, );

  //  _error_message += zs::strprint(_engine, "\nError: ", linfo, "\n'''", line_content, "\n",
  //      zb::indent_t(linfo.column - 1), "^\n'''\n\nfrom function : '", loc.function_name(),
  //      "'\n     file     : '", loc.file_name(), "'\n     line     : ", loc.line()," ", loc.line_content(),
  //      "\n\nMessage:\n", msg);

  return ec;
}

zs::error_result jit_compiler::compile(std::string_view content, object filename, object& output,
    zs::virtual_machine* vm, zs::token_type* prepended_token, bool with_vargs, bool add_line_info) {
  _add_line_info = add_line_info;
  _vm = vm;

  _estate = expr_state{};
  _is_header = 1;

  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(content);
  _lexer->_export_block_comments = true;

  jit::shared_state_data sdata(_engine);
  zs::closure_compile_state c_compile_state(_engine, sdata);
  _ccs = &c_compile_state;

  _ccs->name = zs::_ss("main");
  _ccs->_sdata._source_name = std::move(filename);

  ZS_RETURN_IF_ERROR(add_parameter(zs::_ss("__this__")));

  if (with_vargs) {
    ZS_RETURN_IF_ERROR(add_parameter(zs::_ss("vargs")));
    add_new_target_instruction<op_load_null>();
    _ccs->add_default_param(top_target());
    pop_target();
  }

  int_t stack_size = _ccs->get_stack_size();
  //
  //  {
  //    add_instruction<op_load_lib_ss>(new_target(), zs::ss_inst_data::create("zs"));
  //    pop_target();
  //    ZS_RETURN_IF_ERROR(add_stack_variable(zs::_ss("zs")));
  //  }

  //  {
  //    add_instruction<op_load_lib_ss>(new_target(), zs::ss_inst_data::create("sys"));
  //    pop_target();
  //    ZS_RETURN_IF_ERROR(add_stack_variable(zs::_ss("sys")));
  //  }
  //  {
  //    add_instruction<op_load_lib_ss>(new_target(), zs::ss_inst_data::create("fs"));
  //    pop_target();
  //    ZS_RETURN_IF_ERROR(add_stack_variable(zs::_ss("fs")));
  //  }

  _token = prepended_token ? *prepended_token : lex();

  while (is_not(tok_eof, tok_lex_error)) {
    ZS_COMPILER_PARSE(p_statement, true);

    if (last_is_not(tok_rcrlbracket, tok_semi_colon, tok_doc_block)) {
      ZS_COMPILER_PARSE(p_semi_colon);
    }

    _is_header--;
  }

  if (is(tok_lex_error)) {
    return zs::errc::invalid;
  }

  if (_ccs->is_top_level() and _ccs->has_export()) {
    _ccs->push_export_target();
    add_instruction<op_return_export>(pop_target());
  }

  _ccs->set_stack_size(stack_size);

  if (_add_line_info) {
    _ccs->add_line_infos(_lexer->get_line_info());
  }

  _ccs->set_stack_size(0);

  zs::function_prototype_object* fpo = _ccs->build_function_prototype();
  output = zs::object(fpo, false);

  return {};
}

zs::error_result jit_compiler::add_stack_variable(
    const object& name, int_t* ret_pos, uint32_t mask, uint64_t custom_mask, bool is_const) {

  if (auto lvi = _ccs->find_local_variable_ptr(name); lvi and lvi->scope_id == scope_id()) {
    return zs::errc::duplicated_local_variable_name;
  }

  return _ccs->push_local_variable(name, scope_id(), ret_pos, mask, custom_mask, is_const);
}

void jit_compiler::move_if_current_target_is_local() {
  target_t top_idx = top_target();

  // and top_idx != _ccs->_target_stack.size() -1
  if (_ccs->is_local(top_idx)) {
    // Pops the target and moves it.
    //    target_t tgt = pop_target();
    //    target_t dst = new_target();

    target_t src = pop_target();
    target_t dst = new_target();
    if (dst != src) {
      add_instruction<op_move>(dst, src);
    }
    else {
      zb::print("....................................................");
    }
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

zs::error_result jit_compiler::add_small_string_instruction(std::string_view s, int_t target_idx) {
  if (s.size() > zs::constants::k_small_string_max_size) {
    return ZS_COMPILER_ERROR(errc::invalid_token, "invalid string size in add_small_string_instruction.\n");
  }

  add_instruction<op_load_small_string>(target_idx, zs::ss_inst_data::create(s));
  return {};
}

void jit_compiler::add_string_instruction(const object& sobj, int_t target_idx) {
  if (std::string_view s = sobj.get_string_unchecked(); s.size() <= zs::constants::k_small_string_max_size) {
    add_small_string_instruction(s, target_idx);
  }
  else {
    add_instruction<op_load_string>(target_idx, (uint32_t)_ccs->get_literal(sobj));
  }
}

void jit_compiler::add_string_instruction(std::string_view s, int_t target_idx) {
  if (s.size() > zs::constants::k_small_string_max_size) {
    add_instruction<op_load_string>(target_idx, (uint32_t)_ccs->get_literal(zs::_s(_engine, s)));
  }
  else {

    add_small_string_instruction(s, target_idx);
  }
}

zs::error_result jit_compiler::add_export_string_instruction(const object& var_name) {
  if (auto err = _ccs->add_exported_name(var_name)) {
    return ZS_COMPILER_ERROR(err, "duplicated value keys in export statement.\n");
  }

  add_string_instruction(var_name);
  return {};
}

zs::error_result jit_compiler::add_to_export_table(const object& var_name) {
  int_t table_idx;
  ZS_RETURN_IF_ERROR(_ccs->find_local_variable(zs::_ss("__exports__"), table_idx));

  if (is_small_string_identifier(var_name)) {
    int_t value_idx = pop_target();

    ZS_COMPILER_RETURN_IF_ERROR(
        _ccs->add_exported_name(var_name), "duplicated value keys in export statement.\n");

    add_instruction<op_rawset_ss>(
        (uint8_t)-1, (uint8_t)table_idx, zs::ss_inst_data::create(var_name), (uint8_t)value_idx, true);
  }
  else {
    ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
    int_t key_idx = pop_target();
    int_t value_idx = pop_target();
    add_instruction<op_rawset>((uint8_t)-1, (uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx, true);
  }

  return {};
}

//
// MARK: Parse.
//

template <>
zs::error_result jit_compiler::parse<p_prefixed_incr>(bool is_incr) {

  lex();
  expr_state es = _estate;
  _estate.no_get = true;

  ZS_COMPILER_PARSE(p_prefixed);

  switch (_estate.type) {
  case expr_type::e_expr:
    return ZS_COMPILER_ERROR(zs::errc::invalid_operation, "Can't '++' or '--' an expression");

  case expr_type::e_base:
    return ZS_COMPILER_ERROR(zs::errc::invalid_operation, "Can't '++' or '--' a base");

  case expr_type::e_object: {
    target_t key_idx = pop_target();
    target_t table_idx = pop_target();
    add_new_target_instruction<op_pobjincr>(table_idx, key_idx, is_incr);
    break;
  }

  case expr_type::e_local: {
    add_top_target_instruction<op_pincr>(top_target(), is_incr);
    break;
  }

  case expr_type::e_capture:
    //      SQInteger tmp = _fs->PushTarget();
    //      _fs->AddInstruction(_OP_GETOUTER, tmp, _es.epos);
    //      _fs->AddInstruction(_OP_INCL, tmp, tmp, 0, diff);
    //      _fs->AddInstruction(_OP_SETOUTER, tmp, _es.epos, tmp);
    return zs::errc::unimplemented;
  }

  _estate = es;
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_prefixed_lbracket>() {

  bool is_member_call = false;
  //  if (pos != _estate.pos and (pos == -1 or _estate.pos == -1)) {
  //    zb::print("klklk---", pos, _estate.pos, _token);
  //  }

  switch (_estate.type) {
  case expr_type::e_object: {

    // The table is the root table?
    if (_estate.pos == k_invalid_target or _estate.pos == -1) {
      // Nothing to do here other than pushing it on the stack.
      add_new_target_instruction<op_move>(0);
    }
    else {
      is_member_call = true;

      // We need to call a function from a table e.g. `table.fct();`.
      // The get wasn't done in the `case tok_dot:` above especially for this.

      target_t key_idx = pop_target();
      target_t table_idx = top_target();

      // Get the item at the given `key_idx`, from the table at `table_idx`.
      add_new_target_instruction<op_get>(table_idx, key_idx, make_get_op_flags(_estate.pos == 0, true));

      // To prepare for a function call, we now have the closure on top of
      // the stack. Since we want this table as first arg, we need to push
      // it on the stack after the closure.
      _estate.type = expr_type::e_object;
      _estate.pos = new_target();
      add_instruction<op_move>(_estate.pos, table_idx);
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
      zb::print("DSLJDSKJDLKS capture");
      // Nothing to do here other than pushing it on the stack.
      //            add_instruction<op_move>(_ccs->new_target(),
      //            (uint8_t)0);
    }
    else {
      // TODO: Fix this.
      //      zb::print("DSLJDSKJDLKS CAPTURE");

      // Push the captured closure.
      add_new_target_instruction<op_get_capture>((uint32_t)_estate.pos);
      _estate.pos = top_target();

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
  // result that we need to remove. To solve this, we pop them both, and
  // push the result back on top.
  if (is_member_call) {
    // Pop result.
    target_t result_idx = pop_target();

    // Pop table.
    pop_target();

    // Move the result back on top.
    add_new_target_instruction<op_move>(result_idx);
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_prefixed_lbracket_template>() {

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
      target_t key_idx = pop_target();
      target_t table_idx = top_target();

      // Get the item at the given `key_idx`, from the table at `table_idx`.
      add_new_target_instruction<op_get>(table_idx, key_idx, get_op_flags_t::gf_none);

      // To prepare for a function call, we now have the closure on top of
      // the stack. Since we want this table as first arg, we need to push
      // it on the stack after the closure.
      _estate.type = expr_type::e_object;
      _estate.pos = new_target();
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
      _estate.pos = top_target();

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
    target_t result_idx = pop_target();

    // Pop table.
    pop_target();

    // Move the result back on top.
    add_new_target_instruction<op_move>(result_idx);
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_exponential>() {

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

#define ZS_COMPILER_CMP_OP(OP, ...)                                  \
  lex();                                                             \
                                                                     \
  ZS_RETURN_IF_ERROR(expr_call([&]() { return parse<next_op>(); })); \
                                                                     \
  target_t op2 = pop_target();                                       \
  target_t op1 = pop_target();                                       \
                                                                     \
  add_new_target_instruction<OP>(__VA_ARGS__);                       \
  _estate.type = expr_type::e_expr;

template <>
zs::error_result jit_compiler::parse<p_compare>() {
  static constexpr parse_op next_op = p_shift;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    switch (_token) {
    case tok_gt: {
      ZS_COMPILER_CMP_OP(op_cmp, compare_op::gt, op1, op2);
      break;
    }

    case tok_lt: {
      ZS_COMPILER_CMP_OP(op_cmp, compare_op::lt, op1, op2);
      break;
    }
    case tok_gt_eq: {
      ZS_COMPILER_CMP_OP(op_cmp, compare_op::ge, op1, op2);
      break;
    }

    case tok_lt_eq: {
      ZS_COMPILER_CMP_OP(op_cmp, compare_op::le, op1, op2);
      break;
    }
    case tok_in: {
      return zs::errc::unimplemented;
      //      helper::binary_exp(this, op_cmp,
      //      &compiler::parse<p_shift>);
      // binary_exp(_OP_EXISTS, &SQCompiler::ShiftExp);
      break;
    }

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
  static constexpr parse_op next_op = p_compare;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    switch (_token) {

    case tok_eq_eq: {
      ZS_COMPILER_CMP_OP(op_eq, op1, op2, false);
    } break;

    case tok_not_eq: {
      ZS_COMPILER_CMP_OP(op_ne, op1, op2, false);
    } break;

    case tok_three_way_compare: {
      ZS_COMPILER_CMP_OP(op_cmp, compare_op::tw, op1, op2);
    } break;

    case tok_double_arrow: {
      ZS_COMPILER_CMP_OP(op_cmp, compare_op::double_arrow, op1, op2);
    } break;

    case tok_double_arrow_eq: {
      ZS_COMPILER_CMP_OP(op_cmp, compare_op::double_arrow_eq, op1, op2);
    } break;

    default:
      return {};
    }
  }
  return {};
}

#undef ZS_COMPILER_CMP_OP

template <>
zs::error_result jit_compiler::parse<p_bitwise_and>() {

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

  ZS_COMPILER_PARSE(p_bitwise_or);
  zb_loop() {
    switch (_token) {
    case tok_and: {
      target_t first_exp = pop_target();
      target_t trg = new_target();

      add_instruction<op_and>(trg, first_exp, 0);
      int_t and_pos = get_instruction_index();

      if (trg != first_exp) {
        add_instruction<op_move>(trg, first_exp);
      }
      lex();
      ZS_COMPILER_PARSE(p_and);
      target_t second_exp = pop_target();
      if (trg != second_exp) {
        add_instruction<op_move>(trg, second_exp);
      }

      get_instruction_ref<op_and>(and_pos).offset = (i32)(get_next_instruction_index() - and_pos);

      _estate.type = expr_type::e_expr;
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

  ZS_COMPILER_PARSE(p_and);

  zb_loop() {
    if (is(tok_or)) {

      target_t first_exp = pop_target();
      target_t trg = new_target();

      add_instruction<op_or>(trg, first_exp, 0);
      int_t or_pos = get_instruction_index();

      if (trg != first_exp) {
        add_instruction<op_move>(trg, first_exp);
      }
      lex();
      ZS_COMPILER_PARSE(p_or);
      target_t second_exp = pop_target();
      if (trg != second_exp) {
        add_instruction<op_move>(trg, second_exp);
      }

      get_instruction_ref<op_or>(or_pos).offset = (i32)(get_next_instruction_index() - or_pos);

      _estate.type = expr_type::e_expr;
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
static int64_t opcode_to_type_mask(opcode op) {
  using enum opcode;

  switch (op) {
  case op_load_int:
    return (uint32_t)object_type_mask::k_integer;

  case op_load_float:
    return (uint32_t)object_type_mask::k_float;
  case op_load_bool:
    return (uint32_t)object_type_mask::k_bool;
  case op_load_small_string:
    return (uint32_t)object_type_mask::k_small_string;
  case op_load_string:
    return zs::object_base::k_string_mask;
  }
  return -1;
}
static object_type opcode_to_object_type(opcode op) {
  using enum opcode;

  switch (op) {
  case op_load_int:
    return object_type::k_integer;

  case op_load_float:
    return object_type::k_float;
  case op_load_bool:
    return object_type::k_bool;
  case op_load_small_string:
    return object_type::k_small_string;
  case op_load_string:
    return zs::object_type::k_long_string;
  }
  return object_type::k_null;
}

template <>
zs::error_result jit_compiler::parse<p_compile_time_mask>(
    zs::opcode last_op, uint32_t mask, uint64_t custom_mask, bool* procesed) {
  if (!mask) {
    *procesed = true;
    return {};
  }

  int64_t op_mask = opcode_to_type_mask(last_op);

  if (op_mask == -1) {
    *procesed = false;
    return {};
  }
  if (mask & (uint32_t)op_mask) {
    *procesed = true;
    return {};
  }
  return zs::errc::invalid_type_assignment;
}

template <opcode Op>
static zs::error_result chaneffdfd(uint8_t* instptr, target_t dst, bool* did_replace) {
  if (instruction_t<Op>& inst = *((instruction_t<Op>*)(instptr)); inst.lhs_idx == dst) {
    ZS_ASSERT(inst.op == Op);
    zb::print(Op, "-->changed target");
    inst.target_idx = dst;
    //      replace_last_instruction<op_mul_eq>( inst.lhs_idx,  inst.rhs_idx);
    *did_replace = true;
  }
  return {};
}
template <>
zs::error_result jit_compiler::parse<p_replace_assign>(target_t dst, bool* did_replace) {
  using enum opcode;

  int_t inst_idx = get_instruction_index();
  opcode inst_op = get_instruction_opcode(inst_idx);
  *did_replace = false;

  switch (inst_op) {
  case op_add:
    return chaneffdfd<op_add>(get_instructions_internal_vector().data(inst_idx), dst, did_replace);

  case op_sub:
    return chaneffdfd<op_sub>(get_instructions_internal_vector().data(inst_idx), dst, did_replace);

  case op_mul:
    return chaneffdfd<op_mul>(get_instructions_internal_vector().data(inst_idx), dst, did_replace);

  case op_div:
    return chaneffdfd<op_div>(get_instructions_internal_vector().data(inst_idx), dst, did_replace);
  }
  //      if (instruction_t<op_mul>& inst = get_instructions().get_ref<op_mul>(inst_idx); inst.lhs_idx == dst)
  //      {
  //      zb::print("op_mul->op_mul_eq");
  //        inst.target_idx = dst;
  ////      replace_last_instruction<op_mul_eq>( inst.lhs_idx,  inst.rhs_idx);
  //      *did_replace = true;
  //    }
  //    return {};
  //  case op_add:
  //      if (instruction_t<op_mul>& inst = get_instructions().get_ref<op_mul>(inst_idx); inst.lhs_idx == dst)
  //      { zb::print("op_add->op_add_eq"); replace_last_instruction<op_add_eq>(inst.lhs_idx, inst.rhs_idx);
  //      *did_replace = true;
  //    }
  //    return {};
  //
  //  case op_div:
  //      if (instruction_t<op_div>& inst = get_instructions().get_ref<op_div>(inst_idx); inst.lhs_idx == dst)
  //      { zb::print("op_div->op_div_eq"); replace_last_instruction<op_div_eq>(inst.lhs_idx, inst.rhs_idx);
  //      *did_replace = true;
  //    }
  //    return {};
  //
  //  case op_sub:
  //      if (instruction_t<op_sub>& inst = get_instructions().get_ref<op_sub>(inst_idx); inst.lhs_idx == dst)
  //      { zb::print("op_sub->op_sub_eq"); replace_last_instruction<op_sub_eq>(inst.lhs_idx, inst.rhs_idx);
  //      *did_replace = true;
  //    }
  //    return {};
  //  }

  //  *did_replace = false;
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_assign>(expr_state estate) {

  expr_type es_type = estate.type;
  int_t es_pos = estate.pos;

  switch (es_type) {
  case expr_type::e_local: {
    // Pop the value to assign.
    target_t src = pop_target();

    // Top target is the local variable.
    target_t dst = top_target();

    variable_type_info_t type_info = _ccs->top_target_type_info();

    if (type_info.is_const) {
      return ZS_COMPILER_ERROR(zs::errc::invalid_type_assignment, "trying to assign to a const value");
    }

    int_t inst_idx = get_instruction_index();
    opcode inst_op = get_instruction_opcode(inst_idx);

    bool did_assign = false;
    bool did_type_mask = false;

    ZS_COMPILER_RETURN_IF_ERROR(parse<p_replace_assign>(dst, &did_assign), "v = v");

    if (!did_assign) {
      ZS_COMPILER_RETURN_IF_ERROR(
          parse<p_compile_time_mask>(inst_op, type_info.mask, type_info.custom_mask, &did_type_mask),
          "wrong type mask '", zs::get_exposed_object_type_name(opcode_to_object_type(inst_op)),
          "' expected ", zs::object_type_mask_printer{ type_info.mask, "'", "'" }, ".");
    }

    if (dst == src) {
      zb::print("BINGO", ZS_DEVELOPER_SOURCE_LOCATION(), dst, src);
    }

    if (did_assign) {
      add_check_type_instruction(type_info, dst); 
      return {};
    }
  
    if (did_type_mask) {
      type_info.mask = 0;
      type_info.custom_mask=0;
    }
    
    add_assign_instruction(   dst,   src,  type_info.mask  ,  type_info.custom_mask   ) ;

    
//    if (did_type_mask or !type_info.has_mask()) {
//      add_instruction<op_assign>(dst, src);
//      return {};
//    }
//
//    if (type_info.has_custom_mask()) {
//      add_instruction<op_assign_custom>(dst, src, type_info.mask, type_info.custom_mask);
//      return {};
//    }
//
//    ZS_ASSERT(type_info.has_mask());
//    add_instruction<op_assign_w_mask>(dst, src, type_info.mask);

    return {};
    
    //    if (auto err = parse<p_replace_assign>(get_instructions()[inst_idx], dst, &did_replace_instruction))
    //    {
    //      return ZS_COMPILER_ERROR(zs::errc::invalid_operation, "v = v");
    //    }
    //    else if (did_replace_instruction) {
    //      return {};
    //    }

//    if (!did_move_inst) {
//      if (auto err = parse<p_compile_time_mask>(
//              inst_op, type_info.mask, type_info.custom_mask, &is_compile_time_mask)) {
//        return ZS_COMPILER_ERROR(zs::errc::invalid_type_assignment, "wrong type mask '",
//            zs::get_exposed_object_type_name(opcode_to_object_type(inst_op)), "' expected ",
//            zs::object_type_mask_printer{ type_info.mask, "'", "'" }, ".");
//      }
//    }
//    if (dst == src) {
//      zb::print("BINGO", ZS_DEVELOPER_SOURCE_LOCATION(), dst, src);
//    }
//
//    if (did_replace_instruction) {
//      if (type_info.has_mask()) {
//        if (type_info.has_custom_mask()) {
//          add_instruction<op_check_custom_type_mask>(dst, type_info.mask, type_info.custom_mask);
//        }
//        else {
//          add_instruction<op_check_type_mask>(dst, type_info.mask);
//        }
//      }
//      return {};
//    }
//
//    if (is_compile_time_mask) {
//      add_instruction<op_move>(dst, src);
//      return {};
//    }
//
//    if (type_info.has_custom_mask()) {
//      add_instruction<op_assign_custom>(dst, src, type_info.mask, type_info.custom_mask);
//      return {};
//    }
//
//    ZS_ASSERT(type_info.has_mask());
//    add_instruction<op_assign>(dst, src, type_info.mask);
//
//    return {};
  }

  case expr_type::e_capture: {
    u32 cidx = (u32)es_pos;
    add_new_target_instruction<op_set_capture>(cidx, pop_target());

    return {};
  }

  case expr_type::e_object: {
    if (_ccs->_target_stack.size() < 3) {
      return ZS_COMPILER_ERROR(zs::errc::compile_stack_error, "invalid target stack size for object in eq");
    }

    target_t value_idx = pop_target();
    target_t key_idx = pop_target();
    target_t table_idx = pop_target();

    if (table_idx == 0 and _ccs->is_top_level()) {
      add_new_target_instruction<op_rawset>(table_idx, key_idx, value_idx, !_estate.no_new_set);
    }
    else {
      add_new_target_instruction<op_set>(table_idx, key_idx, value_idx, !_estate.no_new_set);
    }

    return {};
  }

  default:

    return ZS_COMPILER_ERROR(zs::errc::invalid_operation, "invalid invalid_operation");
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_expression>() {

  zb::scoped auto_expr_state_cache = [&, es = _estate]() { _estate = es; };

  _estate.type = expr_type::e_expr;
  _estate.pos = -1;
  _estate.no_get = false;

  ZS_COMPILER_PARSE(p_or);

  if (is(tok_question_mark)) {
    ZS_TODO("Implement");
    return zs::errc::unimplemented;
  }

  //  Lex();
  //        _fs->AddInstruction(_OP_JZ, _fs->PopTarget());
  //        SQInteger jzpos = _fs->GetCurrentPos();
  //        SQInteger trg = _fs->PushTarget();
  //        Expression();
  //        SQInteger first_exp = _fs->PopTarget();
  //        if (trg != first_exp) {
  //          _fs->AddInstruction(_OP_MOVE, trg, first_exp);
  //        }
  //        SQInteger endfirstexp = _fs->GetCurrentPos();
  //        _fs->AddInstruction(_OP_JMP, 0, 0);
  //        Expect(_SC(':'));
  //        SQInteger jmppos = _fs->GetCurrentPos();
  //        Expression();
  //        SQInteger second_exp = _fs->PopTarget();
  //        if (trg != second_exp) {
  //          _fs->AddInstruction(_OP_MOVE, trg, second_exp);
  //        }
  //        _fs->SetInstructionParam(jmppos, 1, _fs->GetCurrentPos() - jmppos);
  //        _fs->SetInstructionParam(jzpos, 1, endfirstexp - jzpos + 1);
  //        _fs->SnoozeOpt();

  if (is(tok_double_question_mark)) {

    lex();
    target_t value_idx = pop_target();
    target_t target_idx = new_target();

    add_instruction<op_if_null>(target_idx, value_idx, 0, false);
    const int_t jz_inst_idx = get_instruction_index();

    ZS_COMPILER_PARSE(p_expression);

    if (value_idx = pop_target(); target_idx != value_idx) {
      add_instruction<op_move>(target_idx, value_idx);
    }

    const int_t jmp_end_idx = get_next_instruction_index();
    //
    get_instruction_ref<op_if_null>(jz_inst_idx).offset = (int32_t)(jmp_end_idx - jz_inst_idx);

    return {};
  }
  if (is(tok_triple_question_mark)) {

    lex();
    target_t value_idx = pop_target();
    target_t target_idx = new_target();

    add_instruction<op_if_null>(target_idx, value_idx, 0, true);
    const int_t jz_inst_idx = get_instruction_index();

    ZS_COMPILER_PARSE(p_expression);

    if (value_idx = pop_target(); target_idx != value_idx) {
      add_instruction<op_move>(target_idx, value_idx);
    }

    const int_t jmp_end_idx = get_next_instruction_index();
    //
    get_instruction_ref<op_if_null>(jz_inst_idx).offset = (int32_t)(jmp_end_idx - jz_inst_idx);

    return {};
  }

  // TODO: Add question_mark?
  if (is_not(tok_eq, tok_minus_eq, tok_add_eq, tok_mul_eq, tok_div_eq, tok_exp_eq, tok_mod_eq, tok_lshift_eq,
          tok_rshift_eq, tok_bitwise_or_eq, tok_bitwise_and_eq, tok_double_arrow_eq)) {
    return {};
  }

  zs::token_type op = _token;
  expr_state pre_expr_state = _estate;

  switch (pre_expr_state.type) {
  case expr_type::e_expr:
    return ZS_COMPILER_ERROR(zs::errc::invalid_type_assignment, "Can't assign an expression");

  case expr_type::e_base:
    return ZS_COMPILER_ERROR(zs::errc::invalid_type_assignment, "'base' cannot be modified");
  }

  lex();

  ZS_COMPILER_PARSE(p_expression);

  switch (op) {
    // Assign.
  case tok_eq:
    return parse<p_assign>(pre_expr_state);

  case tok_add_eq: {
    expr_type es_type = _estate.type;

    switch (es_type) {
    case expr_type::e_local: {
      int_t src = _ccs->pop_target();
      int_t target = _ccs->top_target();
      add_instruction<op_add_eq>((uint8_t)target, (uint8_t)src);
      break;
    }

    case expr_type::e_object: {

      if (_ccs->_target_stack.size() < 3) {
        return ZS_COMPILER_ERROR(
            zs::errc::compile_stack_error, "invalid target stack size for object in add_eq");
      }

      target_t value_idx = pop_target();
      target_t key_idx = pop_target();
      target_t table_idx = pop_target();
      add_new_target_instruction<op_object_add_eq>(table_idx, key_idx, value_idx);

      break;
    }

    default: {
      return zs::errc::unimplemented;
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
    case expr_type::e_object: {

      if (_ccs->_target_stack.size() < 3) {
        return ZS_COMPILER_ERROR(
            zs::errc::compile_stack_error, "invalid target stack size for object in add_eq");
      }

      target_t value_idx = pop_target();
      target_t key_idx = pop_target();
      target_t table_idx = pop_target();
      add_new_target_instruction<op_object_mul_eq>(table_idx, key_idx, value_idx);

      break;
    }
    default:
      return zs::errc::unimplemented;
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
    return zs::errc::unimplemented;

  default:
    break;
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_comma>() {

  ZS_COMPILER_PARSE(p_expression);

  while (lex_if(tok_comma)) {
    pop_target();
    ZS_COMPILER_PARSE(p_comma);
  }
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_semi_colon>() {

  if (lex_if(tok_semi_colon)) {
    return {};
  }

  if (!is_end_of_statement()) {
    //    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "invalid token '", zs::token_to_string(_token),
    //    "', an endl or semi-colon was expected.");
    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "Invalid '", zs::token_to_string(_token),
        "' token, a line-end or ';' were expected.");
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_export>() {

  ZS_ASSERT(is(tok_export), "Invalid token");

  if (next_is(tok_dot)) {
    return parse<p_comma>();
  }
  //

  if (!_ccs->is_top_level()) {
    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "export can only be called on top level.\n");
  }

  ZS_RETURN_IF_ERROR(_ccs->create_export_table());

  if (is_var_decl_tok(_lexer->peek())) {
    return parse<p_decl_var>();
  }

  lex();

  if (is(tok_function)) {
    lex();
    return parse<p_export_function_statement>();
  }

  //  if (is_var_decl_tok()) {
  //    return parse<p_decl_var_internal>(true);
  //  }

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
      return ZS_COMPILER_ERROR(zs::errc::compile_stack_error, "export target error");
    }

    return {};
  }

  if (is(tok_identifier)) {
    const token_type next_token = _lexer->peek();

    if (!zb::is_one_of(next_token, tok_endl, tok_semi_colon)) {
      return ZS_COMPILER_ERROR(zs::errc::invalid_token, "invalid token after export");
    }

    object var_name = _lexer->get_identifier();
    ZS_COMPILER_PARSE(p_expression);
    add_to_export_table(var_name);
    return {};
  }
  //
  return ZS_COMPILER_ERROR(zs::errc::invalid_token, "invalid token after export");
}

ZS_JIT_COMPILER_PARSE_OP(p_export_table) {

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
        return ZS_COMPILER_ERROR(zs::errc::invalid_token, "invalid token after export");
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
      return ZS_COMPILER_ERROR(zs::errc::invalid_token, "invalid token after export");
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

template <>
zs::error_result jit_compiler::parse<p_enum_table>() {

  while (is_not(tok_rcrlbracket)) {
    switch (_token) {
    case tok_lsqrbracket:
      return ZS_COMPILER_ERROR(zs::errc::invalid_operation, "Enum keys can only be regular identifier");

    case tok_string_value:
    case tok_escaped_string_value:
      return ZS_COMPILER_ERROR(zs::errc::invalid_operation,
          "Enum keys can only be regular identifier i.e. no json style identifier)");

    case tok_identifier: {
      zs::object identifier;
      ZS_COMPILER_EXPECT_GET(tok_identifier, identifier);
      add_string_instruction(identifier);

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
          zs::errc::invalid_operation, "Enum can only contain integers, floats, bools and strings.");
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

  ZS_ASSERT(_token == tok_enum);

  lex();

  //
  //
  //

  if (is_not(tok_identifier)) {
    return ZS_COMPILER_ERROR(zs::errc::identifier_expected, "expected identifier");
  }

  object var_name(_engine, _lexer->get_identifier_value());

  lex();

  // Optional `=`.
  lex_if(tok_eq);

  if (is_not(tok_lcrlbracket)) {
    return ZS_COMPILER_ERROR(errc::invalid_token, "invalid token ",
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
        zb::quoted<"'">(zs::token_to_string(tok_lcrlbracket)));
  }

  _enum_counter = 0;
  add_new_target_instruction<op_new_obj>(object_type::k_table);
  lex();
  ZS_COMPILER_PARSE(p_enum_table);

  int_t src = _ccs->pop_target();

  if (int_t dest = _ccs->new_target(); dest != src) {
    add_instruction<op_move>((uint8_t)dest, (uint8_t)src);
  }

  add_top_target_instruction<op_close_enum>();

  pop_target();

  ZS_COMPILER_RETURN_IF_ERROR(
      add_stack_variable(var_name), "Duplicated local variable name ", var_name, ".\n");

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

  if (is_not(tok_identifier)) {
    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "macro cannot have default parameters");
  }
  zs::object identifier = _lexer->get_identifier();

  size_t macro_index = _macros.ifind_if([&](const macro& m) { return m.name == identifier; });

  if (macro_index == zs::vector<macro>::npos) {
    zb::print("macro_index", macro_index);
    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "macro doesn't exists");
  }

  lex();

  if (is_not(tok_lbracket)) {
    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "macro cannot have default parameters");
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
    return ZS_COMPILER_ERROR(zs::errc::invalid_parameter_type, "macro cannot have default parameters");
  }

  const macro& m = _macros[macro_index];

  const zs::array_object& in_params = params.as_array();
  const zs::array_object& macro_params = m.params.as_array();

  if (macro_params.size() != in_params.size()) {
    return ZS_COMPILER_ERROR(zs::errc::invalid_parameter_count, "macro invalid number of parameters");
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

    return zs::errc::invalid;
  }

  last_lexer->_current_token = _lexer->_current_token;
  last_lexer->_last_token = _lexer->_last_token;

  _lexer = last_lexer;
  //  lex();

  //  zb::print("DSLKDJSKLDJSLD", _token);
  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_macro) {

  ZS_COMPILER_EXPECT(tok_macro);

  if (is_not(tok_identifier)) {
    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "macro cannot have default parameters");
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
        return ZS_COMPILER_ERROR(zs::errc::invalid_argument,
            "function with default parameters cannot have variable number of parameters");
      }

      //        ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("vargv")));
      //        fct_state->_vargs_params = true;
      lex();

      if (is_not(tok_rbracket)) {
        return ZS_COMPILER_ERROR(zs::errc::invalid_token, "expected ')' after a variadic (...) parameter");
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
        return ZS_COMPILER_ERROR(zs::errc::invalid_parameter_type, "macro cannot have default parameters");
      }

      if (is(tok_comma)) {
        lex();
      }
      else if (is_not(tok_rbracket)) {
        return ZS_COMPILER_ERROR(
            zs::errc::invalid_token, "expected ')' or ',' at the end of function declaration");
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
    return ZS_COMPILER_ERROR(zs::errc::invalid_parameter_type, "macro cannot have default parameters");
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
    return zs::errc::invalid_token;
  }

  return _compile_time_consts._table->set(identifier, std::move(value));
}

// template <>
// zs::error_result jit_compiler::parse<p_preprocessor>() {
//
//   ZS_COMPILER_EXPECT(tok_hastag);
//
//   switch (_token) {
//   case tok_include:
//     return parse<p_include_or_import_statement>(tok_include);
//
//   case tok_import:
//     return parse<p_include_or_import_statement>(tok_import);
//
//   case tok_define:
//     return parse<p_define>();
//
//   default:
//     _error_message += zs::strprint(_engine, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)),
//         ", expected {'include', 'import', 'macro', 'define', 'if', 'elif', "
//         "'else'}",
//         _lexer->get_line_info());
//     return zs::errc::invalid_token;
//   }
//
//   return {};
// }

template <>
zs::error_result jit_compiler::parse<p_if_block>() {
  if (is_not(tok_lcrlbracket)) {
    ZS_COMPILER_PARSE(p_statement, true);

    if (last_is_not(tok_rcrlbracket, tok_semi_colon)) {
      ZS_COMPILER_PARSE(p_semi_colon);
    }

    return {};
  }

  zb::scoped auto_scope = start_new_auto_scope();
  lex();

  while (is_not(tok_rcrlbracket, tok_default, tok_case)) {
    ZS_COMPILER_PARSE(p_statement, true);

    if (last_is_not(tok_rcrlbracket, tok_semi_colon)) {
      ZS_COMPILER_PARSE(p_semi_colon);
    }
  }

  ZS_COMPILER_EXPECT(tok_rcrlbracket);

  if (_ccs->get_stack_size() != _scope.stack_size) {
    int_t previous_n_capture = _ccs->_n_capture;
    _ccs->set_stack_size(_scope.stack_size);
    if (previous_n_capture != (int_t)_ccs->_n_capture) {
      add_instruction<op_close>((uint32_t)_scope.stack_size);
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_if>() {
  zs::instruction_vector& ivec = _ccs->_instructions;

  ZS_COMPILER_EXPECT(tok_if);
  ZS_COMPILER_EXPECT(tok_lbracket);

  scope previous_scope = _scope;
  zb::scoped auto_scope([&]() { _scope = previous_scope; });

  bool has_var_decl = false;

  if (is_var_decl_tok()) {
    has_var_decl = true;
    previous_scope = start_new_scope();
    ZS_COMPILER_PARSE(p_decl_var);

    if (lex_if(tok_semi_colon)) {
      ZS_COMPILER_PARSE(p_comma);
    }
    else {
      _ccs->push_var_target(_ccs->_vlocals.back().pos);
    }
  }
  else {
    ZS_COMPILER_PARSE(p_comma);
  }

  ZS_COMPILER_EXPECT(tok_rbracket);

  add_instruction<op_jz>(0, pop_target());
  const int_t jz_inst_idx = get_instruction_index();

  ZS_COMPILER_PARSE(p_if_block);

  if (is_not(tok_else)) {
    get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(get_next_instruction_index() - jz_inst_idx);

    if (has_var_decl) {
      close_capture_scope();
    }

    return {};
  }

  add_instruction<op_jmp>(0);
  const int_t jmp_inst_idx = get_instruction_index();
  const int_t jmp_end_idx = get_next_instruction_index();

  lex();
  ZS_COMPILER_PARSE(p_if_block);

  get_instruction_ref<op_jmp>(jmp_inst_idx).offset = (int32_t)(get_next_instruction_index() - jmp_inst_idx);
  get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(jmp_end_idx - jz_inst_idx);

  if (has_var_decl) {
    close_capture_scope();
  }

  return {};
}

//
// template <>
// zs::error_result jit_compiler::parse<p_if>() {
//  zs::instruction_vector& ivec = _ccs->_instructions;
//
//  ZS_COMPILER_EXPECT(tok_if);
//  ZS_COMPILER_EXPECT(tok_lbracket);
//
////  inline scope start_new_scope() noexcept {
////    return std::exchange(
////        _scope, { (int_t)_ccs->_n_capture, (int_t)_ccs->get_stack_size(), ++_scope_id_counter });
////  }
//
//  scope previous_scope = _scope;
//
//  bool has_var_decl = false;
//
//  if (is_var_decl_tok()) {
//    previous_scope = start_new_scope();
//
//    has_var_decl = true;
//
////    zb::scoped auto_scope = start_new_auto_scope();
//    ZS_COMPILER_PARSE(p_decl_var);
//
//    if(lex_if(tok_semi_colon)) {
//      ZS_COMPILER_PARSE(p_comma);
//    }
//    else {
//      _ccs->push_var_target( _ccs->_vlocals.back().pos);
//    }
//
//    ZS_COMPILER_EXPECT(tok_rbracket);
//
//    add_instruction<op_jz>(0, pop_target());
//    const int_t jz_inst_idx = get_instruction_index();
//
//    ZS_COMPILER_PARSE(p_if_block);
//
//    if (is_not(tok_else)) {
//      get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(get_next_instruction_index() -
//      jz_inst_idx); return {};
//    }
//
//    add_instruction<op_jmp>(0);
//    const int_t jmp_inst_idx = get_instruction_index();
//    const int_t jmp_end_idx = get_next_instruction_index();
//
//    lex();
//    ZS_COMPILER_PARSE(p_if_block);
//
//    get_instruction_ref<op_jmp>(jmp_inst_idx).offset = (int32_t)(get_next_instruction_index() -
//    jmp_inst_idx); get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(jmp_end_idx - jz_inst_idx);
//
//    if (_ccs->get_stack_size() != _scope.stack_size) {
//      int_t previous_n_capture = _ccs->_n_capture;
//      _ccs->set_stack_size(_scope.stack_size);
//      if (previous_n_capture != (int_t)_ccs->_n_capture) {
//        add_instruction<op_close>((uint32_t)_scope.stack_size);
//      }
//    }
//
//  }
//  else {
//    ZS_COMPILER_PARSE(p_comma);
//
//    ZS_COMPILER_EXPECT(tok_rbracket);
//
//    add_instruction<op_jz>(0, pop_target());
//    const int_t jz_inst_idx = get_instruction_index();
//
//    ZS_COMPILER_PARSE(p_if_block);
//
//    if (is_not(tok_else)) {
//      get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(get_next_instruction_index() -
//      jz_inst_idx); return {};
//    }
//
//    add_instruction<op_jmp>(0);
//    const int_t jmp_inst_idx = get_instruction_index();
//    const int_t jmp_end_idx = get_next_instruction_index();
//
//    lex();
//    ZS_COMPILER_PARSE(p_if_block);
//
//    get_instruction_ref<op_jmp>(jmp_inst_idx).offset = (int32_t)(get_next_instruction_index() -
//    jmp_inst_idx); get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(jmp_end_idx - jz_inst_idx);
//
//  }
//
//  return {};
//}

//
// template <>
// zs::error_result jit_compiler::parse<p_if>() {
//  zs::instruction_vector& ivec = _ccs->_instructions;
//
//  ZS_COMPILER_EXPECT(tok_if);
//  ZS_COMPILER_EXPECT(tok_lbracket);
//
//  zb::scoped auto_scope = start_new_auto_scope();
//
//  if (is_var_decl_tok()) {
//    ZS_COMPILER_PARSE(p_decl_var);
//    _ccs->push_var_target( _ccs->_vlocals.back().pos);
//  }
//  else {
//    ZS_COMPILER_PARSE(p_comma);
//  }
//
//  ZS_COMPILER_EXPECT(tok_rbracket);
//
//  add_instruction<op_jz>(0, pop_target());
//  const int_t jz_inst_idx = get_instruction_index();
//
////  ZS_COMPILER_PARSE(p_if_block);
//  ZS_COMPILER_PARSE(p_if_block_var_decl);
//
//  if (is_not(tok_else)) {
//    get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(get_next_instruction_index() - jz_inst_idx);
//    return {};
//  }
//
//  add_instruction<op_jmp>(0);
//  const int_t jmp_inst_idx = get_instruction_index();
//  const int_t jmp_end_idx = get_next_instruction_index();
//
//  lex();
////  ZS_COMPILER_PARSE(p_if_block);
//  ZS_COMPILER_PARSE(p_if_block_var_decl);
//
//  get_instruction_ref<op_jmp>(jmp_inst_idx).offset = (int32_t)(get_next_instruction_index() - jmp_inst_idx);
//  get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(jmp_end_idx - jz_inst_idx);
//
//  return {};
//}

template <>
zs::error_result jit_compiler::parse<p_for_each>() {

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

  //  ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
  ZS_COMPILER_RETURN_IF_ERROR(
      add_stack_variable(var_name), "Duplicated local variable name ", idx_name, ".\n");
  ZS_COMPILER_EXPECT(tok_colon);

  zb::print(var_name, "KLKKL");

  zb::scoped auto_scope = start_new_auto_scope();

  ZS_COMPILER_PARSE(p_expression);

  ZS_COMPILER_EXPECT(tok_rbracket);
  //      // put the table in the stack(evaluate the table expression)
  //      Expression();
  //      Expect(_SC(')'));
  int_t container = _ccs->top_target();
  //      // push the index local var

  int_t indexpos;
  int_t valuepos;
  int_t itrpos;
  ZS_RETURN_IF_ERROR(_ccs->find_local_variable(idx_name, indexpos));
  ZS_RETURN_IF_ERROR(_ccs->find_local_variable(var_name, valuepos));
  ZS_RETURN_IF_ERROR(_ccs->find_local_variable(iter_name, itrpos));

  add_instruction<op_load_null>(indexpos);
  add_instruction<op_load_null>(valuepos);
  add_instruction<op_load_null>(itrpos);

  //  int_t indexpos = _ccs->find_local_variable(idx_name);
  //  add_instruction<op_load_null>(indexpos);

  //      // push the value local var
  //  int_t valuepos = _ccs->find_local_variable(var_name);
  //  add_instruction<op_load_null>(valuepos);
  //      // push reference index
  // int_t itrpos = _ccs->find_local_variable(iter_name); // use invalid id to make it inaccessible
  //  add_instruction<op_load_null>(itrpos);

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

  if (zb::is_one_of(sp[1], tok_int, tok_char, tok_bool, tok_float, tok_number, tok_string, tok_table)) {
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

    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "expected var or type");
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

          //          return handle_error( zs::errc::invalid_token, "expected ':' in for
          //          loop",
          //              ZB_CURRENT_SOURCE_LOCATION());
        }
      }
      else if (is(tok_identifier) and i > type_restriction_end_token_index and key_name.is_null()
          and has_key) {
        key_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          return ZS_COMPILER_ERROR(zs::errc::invalid_token, "expected ':' in for loop");
        }
      }

      else if (is(tok_colon, tok_in)) {
        if (colon_ptr) {
          return ZS_COMPILER_ERROR(zs::errc::invalid_token, "multiple ':' in for loop");
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
          //          return handle_error( zs::errc::invalid_token, "expected ':' in for
          //          loop",
          //              ZB_CURRENT_SOURCE_LOCATION());
        }
      }
      else if (is(tok_identifier) and key_name.is_null() and has_key) {
        key_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          //            has_key = true;
          return ZS_COMPILER_ERROR(zs::errc::invalid_token, "expected ':' in for loop");
        }
      }

      else if (is(tok_colon, tok_in)) {

        if (colon_ptr) {
          return ZS_COMPILER_ERROR(zs::errc::invalid_token, "multiple ':' in for loop");
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

  zb::scoped auto_scope = start_new_auto_scope();

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
    return zs::errc::invalid;
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
  ZS_ASSERT(is(tok_for));

  // for(var i = 0; i < 10; i++)
  // ^
  lex();

  // for(var i = 0; i < 10; i++)
  //    ^
  if (!is(tok_lbracket)) {
    return ZS_COMPILER_ERROR(
        zs::errc::invalid_token, "Expected '(' after 'for', got '", zs::token_to_string(_token), "'.");
  }

  // Check if we have a for auto syntax i.e. for(var a : ...) {}.
  {
    lexer l(*_lexer);
    std::vector<zs::token_type> toks;
    toks.resize(200, tok_none);
    std::span<zs::token_type> sp(toks);

    if (zs::status_result status = l.lex_for_auto(sp)) {
      ZS_ASSERT(sp.back() == tok_rbracket);
      return parse<p_for_auto>(sp);
    }
  }

  // for(var i = 0; i < 10; i++)
  //    ^
  ZS_COMPILER_EXPECT(tok_lbracket);

  zb::scoped auto_scope = start_new_auto_scope_with_close_capture();

  // for(var i = 0
  //     ^
  if (is_var_decl_tok()) {
    ZS_COMPILER_PARSE(p_decl_var);
  }
  else if (is_not(tok_semi_colon)) {
    ZS_COMPILER_PARSE(p_comma);
    pop_target();
  }

  // for(var i = 0;
  //              ^
  ZS_COMPILER_EXPECT(tok_semi_colon);

  // The next instruction is the beginning of the condition instructions.
  const int_t cond_inst_start_idx = get_next_instruction_index();

  // for(var i = 0; i < 10; i++)
  //                ^
  int_t cond_inst_jz_idx = -1;
  if (is_not(tok_semi_colon)) {
    ZS_COMPILER_PARSE(p_comma);
    add_instruction<op_jz>(0, pop_target());
    cond_inst_jz_idx = get_instruction_index();
  }

  const int_t cond_inst_last_idx = get_instruction_index();

  // for(var i = 0; i < 10;
  //                      ^
  ZS_COMPILER_EXPECT(tok_semi_colon);

  // The next instruction is the beginning of the increment instructions.
  const int_t incr_expr_inst_start_idx = get_next_instruction_index();

  if (is_not(tok_rbracket)) {
    // for(var i = 0; i < 10; i++)
    //                        ^
    ZS_COMPILER_PARSE(p_comma);
    pop_target();
  }

  // for(var i = 0; i < 10; i++)
  //                           ^
  ZS_COMPILER_EXPECT(tok_rbracket);

  _ccs->_breaks.push_back(0);
  _ccs->_continues.push_back(0);

  
  if (const int_t incr_expr_size = (get_next_instruction_index() - incr_expr_inst_start_idx);
      incr_expr_size > 0) {

    const int_t last_instruction_size = zs::get_instruction_size(get_instruction_opcode());

    zs::vector<uint8_t>& instsvec_data = get_instructions_internal_vector();

    // Here we want to copy the incr expr instructions in a buffer and remove
    // them from the instruction vector.
    zs::vector<uint8_t> inst_buffer((zs::allocator<uint8_t>(_engine)));
    inst_buffer.resize(incr_expr_size);

    zb::memcpy(inst_buffer.data(), instsvec_data.data(incr_expr_inst_start_idx), incr_expr_size);
    instsvec_data.resize(incr_expr_inst_start_idx);
    _ccs->_last_instruction_index = cond_inst_last_idx;

    // Parse the content of the for loop.
    ZS_COMPILER_PARSE(p_statement, true);

    // Append the increment code to the instructions vector.
    instsvec_data.insert(instsvec_data.end(), inst_buffer.begin(), inst_buffer.end());
    _ccs->_last_instruction_index = instsvec_data.size() - last_instruction_size;
  
  }
  else {
    ZS_COMPILER_PARSE(p_statement, true);
  }
  int_t new_incr_inst_pos =new_incr_inst_pos = _ccs->_last_instruction_index;
  // Jump back up to the beginning of the condition instructions.
  add_instruction<op_jmp>((i32)(cond_inst_start_idx - get_next_instruction_index()));

  if (cond_inst_jz_idx > 0) {
    // If there was a condition, we set it's offset to the end of the loop.
    get_instruction_ref<op_jz>(cond_inst_jz_idx).offset
        = (i32)(get_next_instruction_index() - cond_inst_jz_idx);
  }

  if (!_ccs->_unresolved_breaks.empty()) {
    for (size_t idx : _ccs->_unresolved_breaks) {
      get_instruction_ref<op_jmp>(idx).offset = (i32)(get_next_instruction_index() - idx);
    }

    _ccs->_unresolved_breaks.clear();
  }
  
  if (!_ccs->_unresolved_continues.empty()) {
    for (size_t idx : _ccs->_unresolved_continues) {
      if(new_incr_inst_pos!=-1) {
        get_instruction_ref<op_jmp>(idx).offset = (i32)(new_incr_inst_pos- idx);
      }
    }
 
    _ccs->_unresolved_continues.clear();
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_statement>(bool close_frame) {

  if (_add_line_info) {
    _ccs->add_line_infos(_lexer->get_line_info());
  }

  switch (_token) {
  case tok_doc_block: {
    std::string_view val = zb::strip_leading_and_trailing_endlines(_lexer->get_escaped_string_value());
    //      zb::print("DOOCOCOCOC", val);
    _has_doc_block = true;
    _doc_blocks.push_back(zs::_s(_engine, val));
    lex();
    return {};
  }

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
  case tok_number:
    if (_has_doc_block) {
      zb::print("DOC-BLOCK");
      _has_doc_block = false;
    }
    return parse<p_decl_var>();

  case tok_enum:
    if (_has_doc_block) {
      zb::print("DOC-BLOCK");
      _has_doc_block = false;
    }
    return parse<p_decl_enum>();

    //  case tok_macro:
    //    return parse<p_macro>();
    //
    //  case tok_hastag:
    //    return parse<p_preprocessor>();

    //  case tok_include:
    //    ZS_TODO("Implement include(...)");
    //    return zs::errc::unimplemented;
    //
    //  case tok_import:
    //    ZS_TODO("Implement import(...)");
    //    return zs::errc::unimplemented;

  case tok_if:
    if (_has_doc_block) {
      return zs::errc::missing_doc_statement;
    }
    return parse<p_if>();

  case tok_for:
    if (_has_doc_block) {
      return zs::errc::missing_doc_statement;
    }
    return parse<p_for>();

  case tok_foreach:
    if (_has_doc_block) {
      return zs::errc::missing_doc_statement;
    }
    return parse<p_for_each>();

  case tok_class:
    if (_has_doc_block) {
      zb::print("DOC-BLOCK");
      _has_doc_block = false;
    }
    return parse<p_class_statement>();

  case tok_struct:
    return parse<p_struct_statement>();

  case tok_export: {
    if (_has_doc_block) {
      zb::print("DOC-BLOCK");
      _has_doc_block = false;
    }
    return parse<p_export>();
  }

  case tok_break: {
    if (_has_doc_block) {
      return zs::errc::missing_doc_statement;
    }
    //      if(_fs->_breaktargets.size() <= 0)Error(_SC("'break' has to be in a loop block"));
    //      if(_fs->_breaktargets.top() > 0){
    //          _fs->AddInstruction(_OP_POPTRAP, _fs->_breaktargets.top(), 0);
    //      }
    //      RESOLVE_OUTERS();
    //      _fs->AddInstruction(_OP_JMP, 0, -1234);
    //      _fs->_unresolvedbreaks.push_back(_fs->GetCurrentPos());
    //      Lex();
    close_capture_scope();

    add_instruction<op_jmp>(0);
    _ccs->_unresolved_breaks.push_back(get_instruction_index());
    lex();
    return {};
    //      return ZS_COMPILER_ERROR( zs::errc::unimplemented, "BREAK");
  }
      
    case tok_continue: {
      if (_has_doc_block) {
        return zs::errc::missing_doc_statement;
      }
  
//      close_capture_scope();

      add_instruction<op_jmp>(0);
      _ccs->_unresolved_continues.push_back(get_instruction_index());
      lex();
      return {};
     }

  case tok_use:
    if (_has_doc_block) {
      return zs::errc::missing_doc_statement;
    }
    return parse<p_use>();

  case tok_module:
    _is_header++;
    return parse<p_module_info>(module_info_type::module);

  case tok_author:
    _is_header++;
    return parse<p_module_info>(module_info_type::author);

  case tok_brief:
    _is_header++;
    return parse<p_module_info>(module_info_type::brief);

  case tok_version:
    _is_header++;
    return parse<p_module_info>(module_info_type::version);

  case tok_date:
    _is_header++;
    return parse<p_module_info>(module_info_type::date);

  case tok_copyright:
    _is_header++;
    return parse<p_module_info>(module_info_type::copyright);

  case tok_global: {
    if (_has_doc_block) {
      zb::print("DOC-BLOCK");
      _has_doc_block = false;
    }
    lex();

    if (is(tok_function)) {
      return parse<p_global_function_statement>();
    }

    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "expected function i guess");
  }

  case tok_function:
    if (_has_doc_block) {
      zb::print("DOC-BLOCK");
      _has_doc_block = false;
    }
    return parse<p_function_statement>();

  case tok_dollar:
    return parse<p_function_statement>();

  case tok_return: {
    if (_has_doc_block) {
      return zs::errc::missing_doc_statement;
    }

    if (_ccs->is_top_level() and _ccs->has_export()) {
      return ZS_COMPILER_ERROR(
          zs::errc::invalid_token, "return statement is not allowed when using export.\n");
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
    if (_has_doc_block) {
      return zs::errc::missing_doc_statement;
    }
    zb::scoped auto_scope = start_new_auto_scope();
    lex();

    while (is_not(tok_rcrlbracket, tok_default, tok_case)) {
      ZS_COMPILER_PARSE(p_statement, true);

      if (_lexer->_last_token != tok_rcrlbracket
          && !(_lexer->_last_token == tok_semi_colon or _lexer->_last_token == tok_doc_block)) {
        ZS_COMPILER_PARSE(p_semi_colon);
      }
    }

    ZS_COMPILER_EXPECT(tok_rcrlbracket);

    if (close_frame) {
      int_t previous_n_capture = _ccs->_n_capture;

      if (_ccs->get_stack_size() != _scope.stack_size) {
        _ccs->set_stack_size(_scope.stack_size);

        if (previous_n_capture != (int_t)_ccs->_n_capture) {
          //          zb::print("close",_lexer->get_line_info(), previous_n_capture ,
          //          (int_t)_ccs->_n_capture);
          add_instruction<op_close>((uint32_t)_scope.stack_size);
        }
      }
    }
    else {
      if (_ccs->get_stack_size() != _scope.stack_size) {
        _ccs->set_stack_size(_scope.stack_size);
      }
    }

    return {};
  }

  default:
    if (_has_doc_block) {
      return zs::errc::missing_doc_statement;
    }

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
zs::error_result jit_compiler::parse<p_table_or_class>(token_type separator, token_type terminator) {

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

      add_string_instruction(var_name);

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
        add_string_instruction(value);
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
        add_string_instruction(identifier);
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

template <>
zs::error_result jit_compiler::parse<p_class>() {

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
    ZS_COMPILER_RETURN_IF_ERROR(
        add_stack_variable(var_name), "Duplicated local variable name ", var_name, ".\n");
  }
  else {
    ZS_COMPILER_PARSE(p_prefixed);

    switch (_estate.type) {
    case expr_type::e_expr:
      return ZS_COMPILER_ERROR(zs::errc::invalid_operation, "Invalid class name");

    case expr_type::e_base:
      ZBASE_NO_BREAK;
    case expr_type::e_object: {
      ZS_COMPILER_PARSE(p_class);

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

      return ZS_COMPILER_ERROR(
          zs::errc::invalid_operation, "Cannot create a class in a local with the syntax(class <local>)");
    }
  }

  //

  _estate = es;
  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_include_or_import_statement, token_type tok) {

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
      return zs::errc::invalid_include_syntax;
    }
  }

  if (!file_name.is_string()) {
    _error_message += zs::sstrprint(_engine, "parse include statement", linfo);
    return zs::errc::invalid_include_syntax;
  }

  object res_file_name;
  if (auto err = _engine->resolve_file_path(file_name.get_string_unchecked(), res_file_name)) {
    _error_message += zs::sstrprint(_engine, "parse include statement", linfo);
    return zs::errc::invalid_include_file;
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

  ZS_ASSERT(!res_file_name.is_string_view(), "cannot be a string_view");

  if (auto err = loader.open(res_file_name.get_string_unchecked().data())) {
    _error_message += zs::sstrprint(_engine, "parse include statement", linfo);
    return zs::errc::open_file_error;
  }

  zs::lexer* last_lexer = _lexer;
  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(loader.content());

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

    return zs::errc::invalid;
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
  ZS_COMPILER_PARSE(p_expression);
  int_t boundtarget = top_target();

  target.get() = boundtarget;

  ZS_COMPILER_EXPECT(tok_rsqrbracket);
  //    Expect(_SC(']'));
  //  return boundtarget;
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_factor_identifier>(identifier_type itype) {

  object var_name;

  if (itype == identifier_type::exports) {
    var_name = zs::_ss("__exports__");
  }
  else {
    var_name = get_identifier();
  }

  ZS_TRACE("p_factor_identifier var_name:", var_name);

  lex();

  //  and zb::is_one_of(var_name,"zs", "fs", "sys")
  if (is(tok_double_colon)) {
    add_new_target_instruction<op_load_lib_ss>(zs::ss_inst_data::create(var_name));
    _estate.type = expr_type::e_object;
    _estate.pos = _ccs->top_target();
    return {};
  }

  // Check if `var_name` is a local variable.
  if (zs::optional_result<int_t> pos = _ccs->find_local_variable(var_name)) {
    _estate.type = expr_type::e_local;
    _estate.pos = pos;
    push_var_target(pos);
    return {};
  }

  if (const bool iexported = _ccs->has_exported_name(var_name)) {

    if (_ccs->is_top_level()) {
      _ccs->push_export_target();
    }
    else {
      if (zs::optional_result<int_t> pos = _ccs->get_capture(zs::_ss("__exports__"))) {
        add_new_target_instruction<op_get_capture>((uint32_t)pos);
      }
      else {
        return ZS_COMPILER_ERROR(zs::errc::inaccessible, "__exports__ table is inaccessible.\n");
      }
    }

    add_string_instruction(var_name);

    if (needs_get()) {
      target_t key_idx = pop_target();
      target_t table_idx = pop_target();
      add_new_target_instruction<op_get>(table_idx, key_idx, get_op_flags_t::gf_look_in_root);
      _estate.type = expr_type::e_object;
      _estate.pos = table_idx;
    }
    else {
      _estate.type = expr_type::e_object;
      _estate.pos = up_target(-2);
    }

    return {};
  }

  if (zs::optional_result<int_t> pos = _ccs->get_capture(var_name)) {
    // Handle a captured var.
    if (needs_get()) {
      _estate.pos = new_target();
      _estate.type = expr_type::e_expr;
      add_instruction<op_get_capture>((uint8_t)_estate.pos, (u32)pos);
      return {};
    }

    _estate.type = expr_type::e_capture;
    _estate.pos = pos;
    //    _estate.no_get = false;
    //    _estate.no_assign = false;
    //    _estate.no_new_set = false;
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

  push_this_target();
  ZS_TRACE("THIS TARGET", var_name, top_target());
  add_string_instruction(var_name);

  if (needs_get()) {
    target_t key_idx = pop_target();
    target_t table_idx = pop_target();
    add_new_target_instruction<op_get>(table_idx, key_idx, get_op_flags_t::gf_look_in_root);
    _estate.type = expr_type::e_object;
    _estate.pos = table_idx;
    _estate.no_new_set = false;
  }
  else {

    if (_ccs->is_top_level() and is(tok_eq)) {
      return ZS_COMPILER_ERROR(zs::errc::invalid_operation, "Can't see the unknown global variable '",
          var_name.get_string_unchecked(), "'.");
    }

    //    if(_ccs->is_top_level()) {
    //      if( will_modify()) {
    //        return ZS_COMPILER_ERROR(
    //                                 zs::errc::invalid_operation, "cannot assign a global variable without
    //                                 the global keyword");
    //      }
    //    }

    // We are calling a function or an operator.
    // The key is on top on the stack and the table under.
    // For a normal function call, this should bring us to the
    // `parse<p_prefixed>() -> case tok_lbracket:`.
    _estate.type = expr_type::e_object;
    _estate.pos = up_target(-2);
    _estate.no_new_set = true;
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_factor_at>() {

  token_type last_token = _lexer->_last_token;
  lex();

  if (is_not(tok_lbracket)) {
    return parse<p_macro_call>(last_token);
  }

  push_this_target();
  add_string_instruction(std::string_view("__tostring"));

  if (needs_get()) {
    target_t key_idx = pop_target();
    target_t table_idx = pop_target();
    add_new_target_instruction<op_get>(table_idx, key_idx, get_op_flags_t::gf_look_in_root);
    _estate.type = expr_type::e_object;
    _estate.pos = table_idx;
  }
  else {

    if (is_not(tok_lbracket)) {
      _error_message += zs::sstrprint(_engine, "Trying to assign a global variable", _lexer->get_line_info());
      return zs::errc::inaccessible;
    }
    // We are calling a function or an operator.
    // The key is on top on the stack and the table under.
    // For a normal function call, this should bring us to the `case
    // tok_lbracket:` right under.
    _estate.type = expr_type::e_object;
    _estate.pos = up_target(-2);
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_prefixed>() {
  object pname;
  ZS_COMPILER_PARSE(p_factor, &pname);

  zb_loop() {
    object name = pname;
    pname = nullptr;

    if (is(tok_lt) and is_template_function_call()) {
      ZS_COMPILER_PARSE(p_prefixed_lbracket_template);
    }

    switch (_token) {
    case token_type::tok_double_colon: {
      //      if (!zb::is_one_of(name, "zs","sys","fs")) {
      //        return ZS_COMPILER_ERROR(zs::errc::invalid_name, "Only zs::, fs:: and sys:: are allowed for
      //        now.");
      //      }

      lex();
      object var_name;
      ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);

      add_string_instruction(var_name);

      if (is(tok_eq)) {
        if (_estate.no_assign) {
          return ZS_COMPILER_ERROR(
              zs::errc::invalid_operation, "cannot assign a global variable without the global keyword");
        }

        target_t table_idx = up_target(-2);

        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
        _estate.no_assign = false;
      }
      else if (needs_get()) {
        target_t key_idx = pop_target();
        target_t table_idx = pop_target();
        add_new_target_instruction<op_get>(table_idx, key_idx, get_op_flags_t::gf_none);
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
        _estate.pos = up_target(-2);
      }
      break;
    }
    case tok_dot: {
      lex();
      object var_name;
      ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);

      if (name == "__exports__" and _ccs->is_top_level() and is(tok_eq)) {
        ZS_COMPILER_RETURN_IF_ERROR(add_export_string_instruction(var_name),
            "Duplicated local variable name '", var_name.get_string_unchecked(), "'.\n");
      }
      else {
        add_string_instruction(var_name);
      }

      if (is(tok_eq)) {
        if (_estate.no_assign) {
          return ZS_COMPILER_ERROR(
              zs::errc::invalid_operation, "cannot assign a global variable without the global keyword");
        }

        target_t table_idx = up_target(-2);
        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
        _estate.no_assign = false;
      }
      else if (needs_get()) {
        target_t key_idx = pop_target();
        target_t table_idx = pop_target();
        add_new_target_instruction<op_get>(table_idx, key_idx, get_op_flags_t::gf_none);
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
        _estate.pos = up_target(-2);
      }
      break;
    }

    case tok_lbracket: {
      // `(`: We're calling a function.

      //      if (pos != _estate.pos and (pos == -1 or _estate.pos == -1)) {
      //        zb::print("klklk---", pos, _estate.pos, _token);
      //      }
      ZS_COMPILER_PARSE(p_prefixed_lbracket);
      break;
    }

    case tok_lsqrbracket: {
      if (last_is(tok_endl)) {
        return ZS_COMPILER_ERROR(
            zs::errc::invalid_token, "cannot break deref/or comma needed after [exp]=exp slot declaration");
      }

      lex();
      ZS_COMPILER_PARSE(p_expression);
      ZS_COMPILER_EXPECT(tok_rsqrbracket);

      if (is(tok_eq)) {
        int_t table_idx = _ccs->get_up_target(1);
        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
      }
      else if (needs_get()) {
        target_t key_idx = pop_target();
        target_t table_idx = pop_target();
        add_new_target_instruction<op_get>(table_idx, key_idx, get_op_flags_t::gf_none);
        _estate.type = expr_type::e_object;
        _estate.pos = table_idx;
      }
      else {
        // We are calling a function or an operator.
        // The key is on top on the stack and the table under.
        // For a normal function call, this should bring us to the `case
        // tok_lbracket:` right under.
        _estate.type = expr_type::e_object;
        _estate.pos = up_target(-2);
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
        return ZS_COMPILER_ERROR(zs::errc::invalid_operation, "Can't '++' or '--' an expression");

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

        add_new_target_instruction<op_incr>(pop_target(), is_incr);
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
    }
    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_factor>(object* name) {

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
      add_string_instruction(ret_value);
      break;

    case object_type::k_long_string:
      add_string_instruction(ret_value);
      break;

    case object_type::k_string_view:
      add_string_instruction(ret_value);
      break;

    default:
      return ZS_COMPILER_ERROR(zs::errc::invalid_token, "define todo table ...");
    }

    break;
  }

  case tok_string_value: {
    std::string_view svalue = _lexer->get_string_value();
    add_string_instruction(svalue);
    lex();
    break;
  }

  case tok_escaped_string_value: {
    std::string_view svalue = _lexer->get_escaped_string_value();
    add_string_instruction(svalue);
    lex();
    break;
  }

  case tok_typeid: {
    lex();

    if (!lex_if(tok_lbracket)) {
      return ZS_COMPILER_ERROR(zs::errc::invalid_token, "expected '(' after typeid");
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse<p_expression>()) {
        zb::print("ERRRO");
        return err;
      }
    }

    target_t tg = pop_target();
    add_new_target_instruction<op_typeid>(tg);

    lex();
    break;
  }

  case tok_typeof: {
    lex();

    if (!lex_if(tok_lbracket)) {
      return ZS_COMPILER_ERROR(zs::errc::invalid_token, "expected '(' after typeof");
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse<p_expression>()) {
        zb::print("ERRRO");
        return err;
      }
    }

    target_t tg = pop_target();
    add_new_target_instruction<op_typeof>(tg);

    lex();
    break;
  }

  case tok_double_colon: {

    // Should we load the root or the 'this' table?

    if (_ccs->is_top_level()) {
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
    _estate.pos = top_target();
    //    target.get() = _estate.pos;

    _estate.no_assign = true;
    //    _estate.no_get = true;
    return {};
  }

  case tok_import: {
    lex();

    ZS_TRACE("p_factor: tok_import");
    push_this_target();
    ZS_RETURN_IF_ERROR(add_small_string_instruction("import"));

    target_t key_idx = pop_target();
    target_t table_idx = pop_target();
    add_new_target_instruction<op_get>(table_idx, key_idx, get_op_flags_t::gf_look_in_root);

    _estate.type = expr_type::e_object;

    break;
  }

  case tok_base: {
    lex();
    add_new_target_instruction<op_get_base>();
    _estate.type = expr_type::e_base;
    _estate.pos = top_target();
    //    target.get() = _ccs->top_target();
    return {};
  }

  case tok_this: {
    return parse<p_factor_identifier>(identifier_type::normal);
  }

  case tok_export: {
    if (name) {
      *name = zs::_ss("__exports__");
    }
    return parse<p_factor_identifier>(identifier_type::exports);
  }

  case tok_global: {
    add_new_target_instruction<op_load_global>();

    lex();
    _estate.type = expr_type::e_object;
    _estate.pos = top_target();
    _estate.no_assign = false;

    return {};
  }

  case tok_constructor:
    return zs::errc::invalid_token;

  case tok_identifier:
    if (name) {
      *name = get_identifier();
    }

    return parse<p_factor_identifier>(identifier_type::normal);

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
      return ZS_COMPILER_ERROR(zs::errc::unimplemented, "unimplemented unary minus");

      break;
    }
    break;
  }

  case tok_lsqrbracket: {
    // Array.
    add_new_target_instruction<op_new_obj>(object_type::k_array);
    lex();

    while (is_not(tok_rsqrbracket)) {

      ZS_COMPILER_PARSE(p_expression);

      if (_token == tok_comma) {
        lex();
      }

      add_top_target_instruction<op_array_append>(pop_target());

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
    ZS_COMPILER_PARSE(p_struct);
    break;
  }

  case tok_lcrlbracket: {
    add_new_target_instruction<op_new_obj>(object_type::k_table);
    lex();
    ZS_COMPILER_PARSE(p_table);
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
    ZS_COMPILER_PARSE(p_class);
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
    //          this,  zs::errc::unimplemented, "unimplemented unary minus",
    //          ZB_CURRENT_SOURCE_LOCATION());
    //
    //      break;
    //    }
    //    break;

  case tok_inv:
    break;

  case tok_not: {

    lex();

    //    zs::token_type op = _token;
    //    expr_type es_type = _estate.type;
    int_t es_pos = _estate.pos;

    auto auto_state = new_auto_state();
    _estate.no_get = true;

    ZS_COMPILER_PARSE(p_prefixed);

    switch (_estate.type) {
    case expr_type::e_expr: {
      add_new_target_instruction<op_not>(pop_target());
      break;
    }

    case expr_type::e_base:
      return ZS_COMPILER_ERROR(zs::errc::invalid_operation, "Can't '!' (not) a base");

    case expr_type::e_object: {
      if (_ccs->_target_stack.size() < 2) {
        return ZS_COMPILER_ERROR(
            zs::errc::compile_stack_error, "Invalid target stack size for object in tok_not.");
      }
      target_t key_idx = pop_target();
      target_t table_idx = pop_target();
      //      add_new_target_instruction<op_get>(table_idx, key_idx, true);
      //      add_new_target_instruction<op_not>(pop_target());
      add_new_target_instruction<op_obj_not>(table_idx, key_idx);
      return {};
      //        add_instruction<op_pobjincr>(_ccs->new_target(), (uint8_t)table_idx, (uint8_t)key_idx,
      //        is_incr);
      //      return ZS_COMPILER_ERROR(zs::errc::unimplemented, "Not ");
      //      return zs::errc::;
      //
    }

    case expr_type::e_local: {

      add_new_target_instruction<op_not>(pop_target());
      //      SQInteger src = _fs->TopTarget();
      //      _fs->AddInstruction(_OP_INCL, src, src, 0, diff);
      break;
    }

    case expr_type::e_capture:
      //      SQInteger tmp = _fs->PushTarget();
      //      _fs->AddInstruction(_OP_GETOUTER, tmp, _es.epos);
      //      _fs->AddInstruction(_OP_INCL, tmp, tmp, 0, diff);
      //      _fs->AddInstruction(_OP_SETOUTER, tmp, _es.epos, tmp);
      return zs::errc::unimplemented;
    }

    return {};
  }

  case tok_decr:
    ZS_COMPILER_PARSE(p_prefixed_incr, false);
    break;

  case tok_incr:
    ZS_COMPILER_PARSE(p_prefixed_incr, true);
    break;

  case tok_lbracket: {

    if (_lexer->is_right_arrow_function_call()) {

      ZS_COMPILER_PARSE(p_arrow_lamda);
      break;
    }
    else {

      //    _lexer->current_token()
      lex();

      ZS_COMPILER_PARSE(p_comma);

      if (!lex_if(tok_rbracket)) {
        return ZS_COMPILER_ERROR(
            zs::errc::invalid_token, "Invalid '", zs::token_to_string(_token), "' token, ')' was expected.");
      }
    }
    break;
  }

  case tok_file:
    add_string_instruction(_ccs->_sdata._source_name);
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

    add_string_instruction(line_content);

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
      return ZS_COMPILER_ERROR(zs::errc::invalid_include_syntax, "expected `as_string`, `as_table` or ??");
    }
    break;
  }

  default:
    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "Invalid '", zs::token_to_string(_token), "' token.");
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_load_json_file>() {
  ZS_COMPILER_EXPECT(tok_lbracket);

  object filepath_value;
  ZS_COMPILER_EXPECT_GET(tok_string_value, filepath_value);
  ZS_COMPILER_EXPECT(tok_rbracket);

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_ERROR(zs::errc::open_file_error, "cannot open file `as_string`");
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

  ZS_COMPILER_EXPECT(tok_lbracket);

  object filepath_value;
  ZS_COMPILER_EXPECT_GET(tok_string_value, filepath_value);
  ZS_COMPILER_EXPECT(tok_rbracket);

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_ERROR(zs::errc::open_file_error, "cannot open file `as_string`");
  }

  std::string_view svalue = loader.content();
  add_string_instruction(svalue);

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_as_value>() {
  ZS_COMPILER_EXPECT(tok_lbracket);

  object filepath_value;
  ZS_COMPILER_EXPECT_GET(tok_string_value, filepath_value);
  ZS_COMPILER_EXPECT(tok_rbracket);

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_ERROR(zs::errc::open_file_error, "cannot open file `as_value`");
  }

  std::string_view svalue = loader.content();

  zs::vm vm(_engine);
  object ret_value;
  if (auto err = vm->load_buffer_as_value(svalue, filepath_value.get_string_unchecked(), ret_value)) {
    return ZS_COMPILER_ERROR(
        zs::errc::invalid_include_file, "load `as_value` compile failed\n", vm->get_error());
  }

  add_new_target_instruction<op_load>((uint32_t)_ccs->get_literal(ret_value));
  return {};
}

template <>
zs::error_result jit_compiler::parse<p_as_table>() {
  ZS_COMPILER_EXPECT(tok_lbracket);

  object filepath_value;
  ZS_COMPILER_EXPECT_GET(tok_string_value, filepath_value);
  ZS_COMPILER_EXPECT(tok_rbracket);

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath_value.get_string_unchecked())) {
    return ZS_COMPILER_ERROR(zs::errc::open_file_error, "cannot open file `as_table`");
  }

  std::string_view svalue = loader.content();

  zs::vm vm(_engine);
  object ret_value;
  if (auto err = vm->load_buffer_as_value(svalue, filepath_value.get_string_unchecked(), ret_value)) {
    return ZS_COMPILER_ERROR(
        zs::errc::invalid_include_file, "load `as_table` compile failed\n", vm->get_error());
  }

  if (!ret_value.is_table()) {
    return ZS_COMPILER_ERROR(zs::errc::invalid_include_file, "result value is not a table in `as_table`");
  }

  add_new_target_instruction<op_load>((uint32_t)_ccs->get_literal(ret_value));
  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_use) {
  ZS_ASSERT(is(tok_use));
  lex();

  ZS_COMPILER_PARSE(p_expression);
  add_instruction<op_use>(0, pop_target());
  return {};
}

} // namespace zs.

#include "lang/jit/parse_op/zjit_table.h"
#include "lang/jit/parse_op/zjit_struct.h"
#include "lang/jit/parse_op/zjit_variable.h"
#include "lang/jit/parse_op/zjit_functions.h"
#include "lang/jit/parse_op/zjit_module.h"
#undef ZS_COMPILER_PARSE_CPP

ZBASE_PRAGMA_POP()
