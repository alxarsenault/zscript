#include "jit/zjit_compiler.h"
#include <zbase/strings/parse_utils.h>
#include <zbase/memory/ref_wrapper.h>

#include "zvirtual_machine.h"
#include "object/zfunction_prototype.h"
#include "jit/zclosure_compile_state.h"
#include "utility/json/zjson_lexer.h"
#include "utility/json/zjson_parser.h"

#define ZS_COMPILER_PARSE_CPP 1
#include "jit/zjit_compiler_defs.h"

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

  _token = prepended_token ? *prepended_token : lex();

  while (is_not(tok_eof, tok_lex_error)) {
    ZS_RETURN_IF_ERROR(parse_statement(true));

    if (last_is_not(tok_rcrlbracket, tok_semi_colon, tok_doc_block)) {
      ZS_RETURN_IF_ERROR(parse_semi_colon());
    }

    _is_header--;
  }

  if (is(tok_lex_error)) {
    return invalid;
  }

  //  if (_ccs->is_top_level() and _ccs->has_export()) {
  //    _ccs->push_export_target();
  //    add_instruction<op_return_export>(pop_target());
  //  }

  _ccs->set_stack_size(stack_size);

  if (_add_line_info) {
    _ccs->add_line_infos(_lexer->get_line_info());
  }

  _ccs->set_stack_size(0);

  output = _ccs->build_function_prototype();
  return {};
}

zs::error_result jit_compiler::add_stack_variable(
    const object& name, int_t* ret_pos, uint32_t mask, uint64_t custom_mask, bool is_const) {

  if (auto lvi = _ccs->find_local_variable_ptr(name); lvi and lvi->scope_id == scope_id()) {
    return duplicated_local_variable_name;
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
    return ZS_COMPILER_ERROR(invalid_token, "invalid string size in add_small_string_instruction.\n");
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

// zs::error_result jit_compiler::add_export_string_instruction(const object& var_name) {
//   if (auto err = _ccs->add_exported_name(var_name)) {
//     return ZS_COMPILER_ERROR(err, "duplicated value keys in export statement.\n");
//   }
//
//   add_string_instruction(var_name);
//   return {};
// }

// zs::error_result jit_compiler::add_to_export_table(const object& var_name) {
//   int_t table_idx;
//   ZS_RETURN_IF_ERROR(_ccs->find_local_variable(zs::_ss("__exports__"), table_idx));
//
//   if (is_small_string_identifier(var_name)) {
//     int_t value_idx = pop_target();
//
//     ZS_COMPILER_RETURN_IF_ERROR(
//         _ccs->add_exported_name(var_name), "duplicated value keys in export statement.\n");
//
//     add_instruction<op_rawset_ss>(
//         (uint8_t)-1, (uint8_t)table_idx, zs::ss_inst_data::create(var_name), (uint8_t)value_idx, true);
//   }
//   else {
//     ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
//     int_t key_idx = pop_target();
//     int_t value_idx = pop_target();
//     add_instruction<op_rawset>((uint8_t)-1, (uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx,
//     true);
//   }
//
//   return {};
// }

//
// MARK: Parse.
//

struct parse_uniary_arith_prefixed {};

template <>
auto jit_compiler::create_local_lambda<parse_uniary_arith_prefixed>() {
  return [this](arithmetic_uop uop) -> error_result {
    lex();
    expr_state es = _estate;
    _estate.no_get = true;

    ZS_RETURN_IF_ERROR(parse_prefixed());

    switch (_estate.type) {
    case expr_type::e_expr:
      return ZS_COMPILER_ERROR(invalid_operation, "Can't '++' or '--' an expression");

    case expr_type::e_base:
      return ZS_COMPILER_ERROR(invalid_operation, "Can't '++' or '--' a base");

    case expr_type::e_object: {
      target_t key_idx = pop_target();
      target_t table_idx = pop_target();
      add_new_target_instruction<op_obj_uarith>(table_idx, key_idx, uop);
      break;
    }

    case expr_type::e_local: {
      add_top_target_instruction<op_uarith>(top_target(), uop);
      //      add_new_target_instruction<op_uarith>(top_target(), uop);
      break;
    }

    case expr_type::e_capture:
      return unimplemented;
    }

    _estate = es;
    return {};
  };
}

struct parse_prefixed_lbracket {};

template <>
auto jit_compiler::create_local_lambda<parse_prefixed_lbracket>() {
  return [this](bool table_call) -> error_result {
    bool is_member_call = false;

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

    if (!table_call) {
      lex();
    }

    ZS_COMPILER_PARSE(p_function_call_args, false, table_call);

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
  };
}

struct parse_prefixed_lbracket_template {};

template <>
auto jit_compiler::create_local_lambda<parse_prefixed_lbracket_template>() {
  return [this]() -> error_result {
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
      if (_estate.pos == -1) {
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
  };
}

template <>
zs::error_result jit_compiler::parse<p_exponential>() {

  ZS_RETURN_IF_ERROR(parse_prefixed());

  zb_loop() {
    switch (_token) {
    case tok_exp:
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<p_exponential>(aop_exp));
      break;

    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_mult>() {
  static constexpr parse_op next_op = p_exponential;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    switch (_token) {

    case tok_mul:
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_mul));
      break;

    case tok_div:
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_div));
      break;

    case tok_mod:
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_mod));
      break;

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
    case tok_add:
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_add));
      break;

    case tok_minus:
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_sub));
      break;

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
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_lshift));
      break;
    case tok_rshift:
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_rshift));
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
      return unimplemented;
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
  static constexpr parse_op next_op = p_eq_compare;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    if (is(tok_bitwise_and)) {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_bitwise_and));
    }
    else {
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_bitwise_xor>() {
  static constexpr parse_op next_op = p_bitwise_and;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    if (is(tok_xor)) {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_bitwise_xor));
    }
    else {
      return {};
    }
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_bitwise_or>() {
  static constexpr parse_op next_op = p_bitwise_xor;

  ZS_COMPILER_PARSE(next_op);

  zb_loop() {
    if (is(tok_bitwise_or)) {
      ZS_RETURN_IF_ERROR(do_arithmetic_expr<next_op>(aop_bitwise_or));
    }

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

  if (is(tok_or)) {

    target_t first_exp = pop_target();
    target_t target_idx = new_target();

    add_instruction<op_or>(target_idx, first_exp, 0);
    int_t or_pos = get_instruction_index();

    if (target_idx != first_exp) {
      add_instruction<op_move>(target_idx, first_exp);
    }

    lex();
    ZS_COMPILER_PARSE(p_or);

    add_instruction<op_to_bool>(target_idx, pop_target());

    get_instruction_ref<op_or>(or_pos).offset = (i32)(get_next_instruction_index() - or_pos);

    _estate.type = expr_type::e_expr;
  }

  return {};
}
template <>
zs::error_result jit_compiler::parse<p_triple_or>() {

  ZS_COMPILER_PARSE(p_or);

  zb_loop() {
    if (is(tok_triple_or)) {

      target_t first_exp = pop_target();
      target_t trg = new_target();

      add_instruction<op_triple_or>(trg, first_exp, 0);
      int_t or_pos = get_instruction_index();

      if (trg != first_exp) {
        add_instruction<op_move>(trg, first_exp);
      }
      lex();
      ZS_COMPILER_PARSE(p_triple_or);
      target_t second_exp = pop_target();
      if (trg != second_exp) {
        add_instruction<op_move>(trg, second_exp);
      }

      get_instruction_ref<op_triple_or>(or_pos).offset = (i32)(get_next_instruction_index() - or_pos);

      _estate.type = expr_type::e_expr;
    }
    break;
  }
  return {};
}
static int64_t opcode_to_type_mask(opcode op) {
  using enum opcode;

  switch (op) {
  case op_load_char:
    return (uint32_t)object_type_mask::k_integer;

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

zs::error_result jit_compiler::check_compile_time_mask(
    zs::opcode last_op, const variable_type_info& vinfo, bool& procesed) {
  if (!vinfo.has_mask()) {
    procesed = true;
    return {};
  }

  if (vinfo.has_custom_mask()) {
    procesed = false;
    return {};
  }

  int64_t op_mask = opcode_to_type_mask(last_op);

  if (op_mask == -1) {
    procesed = false;
    return {};
  }

  if (vinfo.mask & (uint32_t)op_mask) {
    procesed = true;
    return {};
  }

  return invalid_type_assignment;
}

template <opcode Op>
static zs::error_result reassign(uint8_t* instptr, target_t dst, bool& did_replace) {
  if (instruction_t<Op>& inst = *((instruction_t<Op>*)(instptr)); inst.lhs_idx == dst) {
    ZS_ASSERT(inst.op == Op);
    inst.target_idx = dst;
    did_replace = true;
  }

  return {};
}

namespace {
  struct parse_assign {};
} // namespace.

template <>
auto jit_compiler::create_local_lambda<parse_assign>() {
  return [this](expr_state estate) -> error_result {
    expr_type es_type = estate.type;
    int_t es_pos = estate.pos;

    switch (es_type) {
    case expr_type::e_local: {
      // Pop the value to assign.
      target_t src = pop_target();

      // Top target is the local variable.
      target_t dst = top_target();

      target_type_info_t type_info = _ccs->top_target_type_info();

      if (type_info.is_const()) {
        return ZS_COMPILER_ERROR(invalid_type_assignment, "trying to assign to a const value");
      }

      bool did_assign = false;
      bool did_type_mask = false;

      int_t inst_idx = get_instruction_index();
      opcode inst_op = get_instruction_opcode(inst_idx);
      uint8_t* inst_data = get_instructions_internal_vector().data(inst_idx);

      switch (inst_op) {
      case op_arith:
        reassign<op_arith>(inst_data, dst, did_assign);
        break;
      }

      if (dst == src) {
        zb::print("BINGO", ZS_DEVELOPER_SOURCE_LOCATION(), dst, src);
      }

      if (did_assign) {
        if (type_info.has_custom_mask()) {
          add_instruction<op_check_custom_type_mask>(dst, type_info.mask, type_info.custom_mask);
        }
        else if (type_info.has_mask()) {
          add_instruction<op_check_type_mask>(dst, type_info.mask);
        }
        return {};
      }

      ZS_COMPILER_RETURN_IF_ERROR(check_compile_time_mask(inst_op, type_info, did_type_mask),
          "wrong type mask '", zs::get_exposed_object_type_name(opcode_to_object_type(inst_op)),
          "' expected ", zs::object_type_mask_printer{ type_info.mask, "'", "'" }, ".");

      if (did_type_mask) {
        add_instruction<op_assign>(dst, src);
        return {};
      }

      if (type_info.has_custom_mask()) {
        add_instruction<op_assign_custom>(dst, src, type_info.mask, type_info.custom_mask);
      }
      else if (type_info.has_mask()) {
        add_instruction<op_assign_w_mask>(dst, src, type_info.mask);
      }
      else {
        add_instruction<op_assign>(dst, src);
      }

      return {};
    }

    case expr_type::e_capture: {
      u32 cidx = (u32)es_pos;
      add_new_target_instruction<op_set_capture>(cidx, pop_target());

      return {};
    }

    case expr_type::e_object: {
      if (_ccs->_target_stack.size() < 3) {
        return ZS_COMPILER_ERROR(compile_stack_error, "invalid target stack size for object in eq");
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

      return ZS_COMPILER_ERROR(invalid_operation, "invalid invalid_operation");
    }

    return {};
  };
}

zs::error_result jit_compiler::parse_expression() {

  zb::scoped auto_expr_state_cache = [&, es = _estate]() { _estate = es; };

  _estate.type = expr_type::e_expr;
  _estate.pos = -1;
  _estate.no_get = false;

  ZS_COMPILER_PARSE(p_triple_or);

  if (is(tok_question_mark)) {

    lex();

    // If true, we keep going to the op_move and op_jmp.
    add_instruction<op_jz>(0, pop_target());
    const int_t jz_inst_idx = get_instruction_index();

    target_t target_idx = new_target();
    ZS_RETURN_IF_ERROR(parse_expression());

    if (target_t first_exp = pop_target(); target_idx != first_exp) {
      add_instruction<op_move>(target_idx, first_exp);
    }

    const int_t jz_end_idx = get_next_instruction_index();

    add_instruction<op_jmp>(0);
    const int_t jmp_inst_idx = get_instruction_index();

    ZS_COMPILER_EXPECT(tok_colon);

    const int_t jmp_pos = get_next_instruction_index();

    ZS_RETURN_IF_ERROR(parse_expression());

    if (target_t second_exp = pop_target(); target_idx != second_exp) {
      add_instruction<op_move>(target_idx, second_exp);
    }

    get_instruction_ref<op_jmp>(jmp_inst_idx).offset = (int32_t)(get_next_instruction_index() - jmp_inst_idx);
    get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(jmp_pos - jz_inst_idx);

    return {};
  }

  if (is(tok_double_question_mark)) {

    lex();
    target_t value_idx = pop_target();
    target_t target_idx = new_target();

    add_instruction<op_if_null>(target_idx, value_idx, 0, false);
    const int_t jz_inst_idx = get_instruction_index();

    ZS_RETURN_IF_ERROR(parse_expression());

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
    return ZS_COMPILER_ERROR(invalid_type_assignment, "Can't assign an expression");

  case expr_type::e_base:
    return ZS_COMPILER_ERROR(invalid_type_assignment, "'base' cannot be modified");
  }

  lex();

  ZS_RETURN_IF_ERROR(parse_expression());

  switch (op) {
    // Assign.
  case tok_eq:
    return call_local_lambda<parse_assign>(pre_expr_state);

  case tok_add_eq:
  case tok_minus_eq:
  case tok_mul_eq:
  case tok_div_eq:
  case tok_exp_eq:
  case tok_mod_eq:
  case tok_lshift_eq:
  case tok_rshift_eq:
  case tok_bitwise_or_eq:
  case tok_bitwise_and_eq:
  case tok_double_arrow_eq:
    return parse_arith_eq(op);

  default:
    break;
  }

  return {};
}

zs::error_result jit_compiler::parse_comma() {

  ZS_RETURN_IF_ERROR(parse_expression());

  while (lex_if(tok_comma)) {
    pop_target();
    ZS_RETURN_IF_ERROR(parse_comma());
  }
  return {};
}

zs::error_result jit_compiler::parse_semi_colon() {

  if (lex_if(tok_semi_colon)) {
    return {};
  }

  if (!is_end_of_statement()) {
    return ZS_COMPILER_ERROR(
        invalid_token, "Invalid '", zs::token_to_string(_token), "' token, a line-end or ';' were expected.");
  }

  return {};
}
//
// template <>
// zs::error_result jit_compiler::parse<p_export>() {
//
//  ZS_ASSERT(is(tok_export), "Invalid token");
//
//  if (next_is(tok_dot)) {
//    return parse_comma();
//  }
//  //
//
//  if (!_ccs->is_top_level()) {
//    return ZS_COMPILER_ERROR(invalid_token, "export can only be called on top level.\n");
//  }
//
//  ZS_RETURN_IF_ERROR(_ccs->create_export_table());
//
//  if (is_var_decl_tok(_lexer->peek())) {
//    return parse_variable_declaration();
//  }
//
//  lex();
//
//  if (is(tok_function)) {
//    lex();
//    return parse<p_export_function_statement>();
//  }
//
//  if (is(tok_struct)) {
//    lex();
//    zs::object var_name;
//    ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
//
//    ZS_RETURN_IF_ERROR(parse_struct(&var_name));
//
//    add_to_export_table(var_name);
//    return {};
//  }
//
//  if (is(tok_lcrlbracket)) {
//    lex();
//    const int_t export_target_idx = _ccs->push_export_target();
//    ZS_COMPILER_PARSE(p_export_table);
//
//    // Pop the export table.
//    if (_ccs->pop_target() != export_target_idx) {
//      return ZS_COMPILER_ERROR(compile_stack_error, "export target error");
//    }
//
//    return {};
//  }
//
//  if (is(tok_identifier)) {
//    const token_type next_token = _lexer->peek();
//
//    if (!zb::is_one_of(next_token, tok_endl, tok_semi_colon)) {
//      return ZS_COMPILER_ERROR(invalid_token, "invalid token after export");
//    }
//
//    object var_name = _lexer->get_identifier();
//    ZS_RETURN_IF_ERROR(parse_expression());
//    add_to_export_table(var_name);
//    return {};
//  }
//  //
//  return ZS_COMPILER_ERROR(invalid_token, "invalid token after export");
//}

// ZS_JIT_COMPILER_PARSE_OP(p_export_table) {
//
//   while (is_not(tok_rcrlbracket)) {
//     switch (_token) {
//     case tok_function: {
//       lex();
//       zs::object var_name;
//       ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);
//       ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
//       ZS_RETURN_IF_ERROR(parse_function(var_name, false));
//       break;
//     }
//
//     case tok_lsqrbracket: {
//       lex();
//       ZS_RETURN_IF_ERROR(parse_comma());
//       ZS_COMPILER_EXPECT(tok_rsqrbracket);
//       ZS_COMPILER_EXPECT(tok_eq);
//       ZS_RETURN_IF_ERROR(parse_expression());
//       break;
//     }
//
//     case tok_string_value:
//     case tok_escaped_string_value: {
//       zs::object value;
//       ZS_COMPILER_EXPECT_GET(tok_string_value, value);
//       ZS_RETURN_IF_ERROR(add_export_string_instruction(value));
//       ZS_COMPILER_EXPECT(tok_colon);
//       ZS_RETURN_IF_ERROR(parse_expression());
//       break;
//     }
//
//     case tok_identifier: {
//       const token_type next_token = _lexer->peek();
//
//       if (!zb::is_one_of(next_token, tok_eq, tok_endl, tok_comma, tok_rcrlbracket)) {
//         return ZS_COMPILER_ERROR(invalid_token, "invalid token after export");
//       }
//
//       zs::object var_name = _lexer->get_identifier();
//       ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
//
//       if (next_token == tok_eq) {
//         lex();
//         lex();
//       }
//
//       ZS_RETURN_IF_ERROR(parse_expression());
//       break;
//     }
//
//     default:
//       return ZS_COMPILER_ERROR(invalid_token, "invalid token after export");
//     }
//
//     if (_token == tok_comma) {
//       // optional comma.
//       lex();
//     }
//
//     int_t val = _ccs->pop_target();
//     int_t key = _ccs->pop_target();
//
//     //<<BECAUSE OF THIS NO COMMON EMIT FUNC IS POSSIBLE.
//     int_t table = _ccs->top_target();
//
//     add_instruction<op_new_slot>((uint8_t)table, (uint8_t)key, (uint8_t)val);
//   }
//
//   lex();
//
//   return {};
// }

template <>
zs::error_result jit_compiler::parse<p_enum_table>() {

  while (is_not(tok_rcrlbracket)) {
    switch (_token) {
    case tok_lsqrbracket:
      return ZS_COMPILER_ERROR(invalid_operation, "Enum keys can only be regular identifier");

    case tok_string_value:
    case tok_escaped_string_value:
      return ZS_COMPILER_ERROR(
          invalid_operation, "Enum keys can only be regular identifier i.e. no json style identifier)");

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
        ZS_RETURN_IF_ERROR(parse_expression());
      }
      break;
    }

    default:
      return ZS_COMPILER_ERROR(
          invalid_operation, "Enum can only contain integers, floats, bools and strings.");
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
    return ZS_COMPILER_ERROR(identifier_expected, "expected identifier");
  }

  object var_name(_engine, _lexer->get_identifier_value());

  lex();

  // Optional `=`.
  lex_if(tok_eq);

  if (is_not(tok_lcrlbracket)) {
    return ZS_COMPILER_ERROR(invalid_token, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)),
        ", expected ", zb::quoted<"'">(zs::token_to_string(tok_lcrlbracket)));
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

zs::error_result jit_compiler::parse_if() {

  auto parse_if_block = [&]() -> error_result {
    if (is_not(tok_lcrlbracket)) {
      ZS_RETURN_IF_ERROR(parse_statement(true));

      if (last_is_not(tok_rcrlbracket, tok_semi_colon)) {
        ZS_RETURN_IF_ERROR(parse_semi_colon());
      }

      return {};
    }

    zb::scoped auto_scope = start_new_auto_scope();
    lex();

    while (is_not(tok_rcrlbracket, tok_default, tok_case)) {
      ZS_RETURN_IF_ERROR(parse_statement(true));

      if (last_is_not(tok_rcrlbracket, tok_semi_colon)) {
        ZS_RETURN_IF_ERROR(parse_semi_colon());
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
  };

  zs::instruction_vector& ivec = _ccs->_instructions;

  ZS_COMPILER_EXPECT(tok_if);
  ZS_COMPILER_EXPECT(tok_lbracket);

  scope previous_scope = _scope;
  zb::scoped auto_scope([&]() { _scope = previous_scope; });

  bool has_var_decl = false;

  if (is_var_decl_tok()) {
    has_var_decl = true;
    previous_scope = start_new_scope();
    ZS_RETURN_IF_ERROR(parse_variable_declaration());

    if (lex_if(tok_semi_colon)) {
      ZS_RETURN_IF_ERROR(parse_comma());
    }
    else {
      _ccs->push_var_target(_ccs->_vlocals.back().pos);
    }
  }
  else {
    ZS_RETURN_IF_ERROR(parse_comma());
  }

  ZS_COMPILER_EXPECT(tok_rbracket);

  add_instruction<op_jz>(0, pop_target());
  const int_t jz_inst_idx = get_instruction_index();

  ZS_RETURN_IF_ERROR(parse_if_block());

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
  ZS_RETURN_IF_ERROR(parse_if_block());

  get_instruction_ref<op_jmp>(jmp_inst_idx).offset = (int32_t)(get_next_instruction_index() - jmp_inst_idx);
  get_instruction_ref<op_jz>(jz_inst_idx).offset = (int32_t)(jmp_end_idx - jz_inst_idx);

  if (has_var_decl) {
    close_capture_scope();
  }

  return {};
}

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

  ZS_RETURN_IF_ERROR(parse_expression());

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

    return ZS_COMPILER_ERROR(invalid_token, "expected var or type");
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

          //          return handle_error( invalid_token, "expected ':' in for
          //          loop",
          //              ZB_CURRENT_SOURCE_LOCATION());
        }
      }
      else if (is(tok_identifier) and i > type_restriction_end_token_index and key_name.is_null()
          and has_key) {
        key_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          return ZS_COMPILER_ERROR(invalid_token, "expected ':' in for loop");
        }
      }

      else if (is(tok_colon, tok_in)) {
        if (colon_ptr) {
          return ZS_COMPILER_ERROR(invalid_token, "multiple ':' in for loop");
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
          //          return handle_error( invalid_token, "expected ':' in for
          //          loop",
          //              ZB_CURRENT_SOURCE_LOCATION());
        }
      }
      else if (is(tok_identifier) and key_name.is_null() and has_key) {
        key_name = _lexer->get_identifier();

        if (!zb::is_one_of(sp[i + 1], tok_colon, tok_in)) {
          //            has_key = true;
          return ZS_COMPILER_ERROR(invalid_token, "expected ':' in for loop");
        }
      }

      else if (is(tok_colon, tok_in)) {

        if (colon_ptr) {
          return ZS_COMPILER_ERROR(invalid_token, "multiple ':' in for loop");
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
        " !__private_iterator.is_same(__private_array_end);", //
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
        " !__private_iterator.is_same(__private_array_end);", //
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
    ZS_RETURN_IF_ERROR(parse_variable_declaration());
  }

  lex();

  // for(var i = 0
  //     ^
  if (is_var_decl_tok()) {
    ZS_RETURN_IF_ERROR(parse_variable_declaration());
  }
  else if (is_not(tok_semi_colon)) {
    ZS_RETURN_IF_ERROR(parse_comma());
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
    ZS_RETURN_IF_ERROR(parse_comma());
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
    ZS_RETURN_IF_ERROR(parse_comma());
    _ccs->pop_target();
  }

  if (_token == tok_lex_error) {
    last_lexer->_current_token = _lexer->_current_token;
    last_lexer->_last_token = _lexer->_last_token;
    _lexer = last_lexer;
    return invalid;
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

  ZS_RETURN_IF_ERROR(parse_statement(true));

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
        invalid_token, "Expected '(' after 'for', got '", zs::token_to_string(_token), "'.");
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
    ZS_RETURN_IF_ERROR(parse_variable_declaration());
  }
  else if (is_not(tok_semi_colon)) {
    ZS_RETURN_IF_ERROR(parse_comma());
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
    ZS_RETURN_IF_ERROR(parse_comma());
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
    ZS_RETURN_IF_ERROR(parse_comma());
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
    ZS_RETURN_IF_ERROR(parse_statement(true));

    // Append the increment code to the instructions vector.
    instsvec_data.insert(instsvec_data.end(), inst_buffer.begin(), inst_buffer.end());
    _ccs->_last_instruction_index = instsvec_data.size() - last_instruction_size;
  }
  else {
    ZS_RETURN_IF_ERROR(parse_statement(true));
  }

  int_t new_incr_inst_pos = new_incr_inst_pos = _ccs->_last_instruction_index;
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
      if (new_incr_inst_pos != -1) {
        get_instruction_ref<op_jmp>(idx).offset = (i32)(new_incr_inst_pos - idx);
      }
    }

    _ccs->_unresolved_continues.clear();
  }

  return {};
}

zs::error_result jit_compiler::parse_statement(bool close_frame) {
  if (_add_line_info) {
    _ccs->add_line_infos(_lexer->get_line_info());
  }

  switch (_token) {
  case tok_doc_block: {
    std::string_view val = zb::strip_leading_and_trailing_endlines(_lexer->get_escaped_string_value());
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
    return parse_variable_declaration();

  case tok_enum:
    if (_has_doc_block) {
      zb::print("DOC-BLOCK");
      _has_doc_block = false;
    }
    return parse<p_decl_enum>();

  case tok_if:
    if (_has_doc_block) {
      return missing_doc_statement;
    }
    return parse_if();

  case tok_for:
    if (_has_doc_block) {
      return missing_doc_statement;
    }
    return parse<p_for>();

  case tok_foreach:
    if (_has_doc_block) {
      return missing_doc_statement;
    }
    return parse<p_for_each>();

  case tok_class:
    if (_has_doc_block) {
      zb::print("DOC-BLOCK");
      _has_doc_block = false;
    }
    return ZS_COMPILER_ERROR(unimplemented, "Class is not implemented yet.");

  case tok_struct:
    return parse_struct_statement();

  case tok_break: {
    if (_has_doc_block) {
      return missing_doc_statement;
    }

    close_capture_scope();

    add_instruction<op_jmp>(0);
    _ccs->_unresolved_breaks.push_back(get_instruction_index());
    lex();
    return {};
  }

  case tok_continue: {
    if (_has_doc_block) {
      return missing_doc_statement;
    }

    add_instruction<op_jmp>(0);
    _ccs->_unresolved_continues.push_back(get_instruction_index());
    lex();
    return {};
  }

  case tok_use:
    return ZS_COMPILER_ERROR(unimplemented, "The 'use' token is reserved.");

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

    //  case tok_global: {
    //    if (_has_doc_block) {
    //      zb::print("DOC-BLOCK");
    //      _has_doc_block = false;
    //    }
    //    lex();
    //
    //    if (is(tok_function)) {
    //      return parse<p_global_function_statement>();
    //    }
    //
    //    return ZS_COMPILER_ERROR(invalid_token, "expected function i guess");
    //  }

  case tok_function:
    if (_has_doc_block) {
      zb::print("DOC-BLOCK");
      _has_doc_block = false;
    }

    return parse_function_statement();

  case tok_dollar:
    return ZS_COMPILER_ERROR(invalid_token, "'$' functions cannot be a statement.");

  case tok_return: {
    if (_has_doc_block) {
      return missing_doc_statement;
    }

    //    if (_ccs->is_top_level() and _ccs->has_export()) {
    //      return ZS_COMPILER_ERROR(invalid_token, "return statement is not allowed when using export.\n");
    //    }

    lex();

    if (!is_end_of_statement()) {

      ZS_RETURN_IF_ERROR(parse_comma());

      // TODO: Fix this.
      add_instruction<op_return>(_ccs->pop_target(), true);
    }
    else {
      add_instruction<op_return>(_ccs->get_stack_size(), true);
    }

    return {};
  }

  case tok_lcrlbracket: {
    if (_has_doc_block) {
      return missing_doc_statement;
    }

    zb::scoped auto_scope = start_new_auto_scope();
    lex();

    while (is_not(tok_rcrlbracket, tok_default, tok_case)) {
      ZS_RETURN_IF_ERROR(parse_statement(true));

      if (last_is_not(tok_rcrlbracket, tok_semi_colon, tok_doc_block)) {
        ZS_RETURN_IF_ERROR(parse_semi_colon());
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

    return {};
  }

  default:
    if (_has_doc_block) {
      return missing_doc_statement;
    }

    ZS_RETURN_IF_ERROR(parse_comma());
    // @alex
    //-------------------------------------------------------------
    // pop_target();
    return {};
  }

  return {};
}

zs::error_result jit_compiler::parse_prefixed() {
  object pname;
  ZS_RETURN_IF_ERROR(parse_factor(&pname));

  zb_loop() {
    object name = pname;
    pname = nullptr;

    if (is(tok_lt) and is_template_function_call()) {
      ZS_RETURN_IF_ERROR(call_local_lambda<parse_prefixed_lbracket_template>());
    }

    switch (_token) {
    case token_type::tok_double_colon: {
      lex();
      object var_name;
      ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);

      add_string_instruction(var_name);

      if (is(tok_eq)) {
        if (_estate.no_assign) {
          return ZS_COMPILER_ERROR(
              invalid_operation, "cannot assign a global variable without the global keyword");
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

      //      if (name == "__exports__" and _ccs->is_top_level() and is(tok_eq)) {
      //        ZS_COMPILER_RETURN_IF_ERROR(add_export_string_instruction(var_name),
      //            "Duplicated local variable name '", var_name.get_string_unchecked(), "'.\n");
      //      }
      //      else {
      add_string_instruction(var_name);
      //      }

      if (is(tok_eq)) {
        if (_estate.no_assign) {
          return ZS_COMPILER_ERROR(
              invalid_operation, "cannot assign a global variable without the global keyword");
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
      ZS_RETURN_IF_ERROR(call_local_lambda<parse_prefixed_lbracket>(false));
      break;
    }

    case tok_lcrlbracket: {
      // `{`: We're calling a function.
      ZS_RETURN_IF_ERROR(call_local_lambda<parse_prefixed_lbracket>(true));
      break;
    }

    case tok_lsqrbracket: {
      if (last_is(tok_endl)) {
        return ZS_COMPILER_ERROR(
            invalid_token, "cannot break deref/or comma needed after [exp]=exp slot declaration");
      }

      expr_state es = _estate;
      lex();
      ZS_RETURN_IF_ERROR(parse_expression());
      ZS_COMPILER_EXPECT(tok_rsqrbracket);

      {
        switch (es.type) {
        case expr_type::e_expr: {
          if (is_not(tok_eq) and needs_get()) {
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
        case expr_type::e_local: {

          if (is_not(tok_eq) and needs_get()) {
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
        case expr_type::e_base: {
          break;
        }
        case expr_type::e_object: {
          if (is_not(tok_eq) and needs_get()) {
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
        case expr_type::e_capture: {
          break;
        }
        }
      }
      break;
    }

    case tok_decr:
    case tok_incr: {
      arithmetic_uop uop = is(tok_incr) ? uop_incr : uop_decr;

      if (is_end_of_statement()) {
        return {};
      }

      lex();

      switch (_estate.type) {
      case expr_type::e_expr:
        return ZS_COMPILER_ERROR(invalid_operation, "Can't '++' or '--' an expression");

      case expr_type::e_base:
        zb::print("ERROR");
        //                      Error(_SC("'base' cannot be modified"));
        break;

      case expr_type::e_object: {
        if (_estate.no_get == true) {
          zb::print("ERROR");
          //                        Error(_SC("can't '++' or '--' an expression"));
          break;
        } // mmh dor this make sense?
        //                      Emit2ArgsOP(_OP_PINC, diff);

        target_t key_idx = pop_target();
        target_t table_idx = pop_target();
        add_new_target_instruction<op_obj_uarith>(table_idx, key_idx, uop);
        //          add_new_target_instruction<op_uarith>(pop_target(), is_incr ? uop_incr : uop_decr);
        break;
      }

      case expr_type::e_local: {
        add_new_target_instruction<op_uarith>(pop_target(), uop);
      } break;

      case expr_type::e_capture: {
        zb::print("ERROR");
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

struct factor_identifier {};

template <>
auto jit_compiler::create_local_lambda<factor_identifier>() {
  return [this]() -> error_result {
    object var_name = get_identifier();

    //    if (itype == identifier_type::exports) {
    //      var_name = zs::_ss("__exports__");
    //    }
    //    else {
    //      var_name = get_identifier();
    //    }

    ZS_TRACE("factor_identifier var_name:", var_name);

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

    //    if (const bool iexported = _ccs->has_exported_name(var_name)) {
    //
    //      if (_ccs->is_top_level()) {
    //        _ccs->push_export_target();
    //      }
    //      else {
    //        if (zs::optional_result<int_t> pos = _ccs->get_capture(zs::_ss("__exports__"))) {
    //          add_new_target_instruction<op_get_capture>((uint32_t)pos);
    //        }
    //        else {
    //          return ZS_COMPILER_ERROR(inaccessible, "__exports__ table is inaccessible.\n");
    //        }
    //      }
    //
    //      add_string_instruction(var_name);
    //
    //      if (needs_get()) {
    //        target_t key_idx = pop_target();
    //        target_t table_idx = pop_target();
    //        add_new_target_instruction<op_get>(table_idx, key_idx, get_op_flags_t::gf_look_in_root);
    //        _estate.type = expr_type::e_object;
    //        _estate.pos = table_idx;
    //      }
    //      else {
    //        _estate.type = expr_type::e_object;
    //        _estate.pos = up_target(-2);
    //      }
    //
    //      return {};
    //    }

    if (zs::optional_result<int_t> pos = _ccs->get_capture(var_name)) {
      // Handle a captured var.
      if (needs_get()) {
        _estate.pos = new_target();
        _estate.type = expr_type::e_object;
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
        return ZS_COMPILER_ERROR(invalid_operation, "Can't see the unknown global variable '",
            var_name.get_string_unchecked(), "'.");
      }

      //    if(_ccs->is_top_level()) {
      //      if( will_modify()) {
      //        return ZS_COMPILER_ERROR(
      //                                 invalid_operation, "cannot assign a global variable without
      //                                 the global keyword");
      //      }
      //    }

      // We are calling a function or an operator.
      // The key is on top on the stack and the table under.
      // For a normal function call, this should bring us to the
      // `parse_prefixed() -> case tok_lbracket:`.
      _estate.type = expr_type::e_object;
      _estate.pos = up_target(-2);
      _estate.no_new_set = true;
    }

    return {};
  };
}

zs::error_result jit_compiler::parse_factor(object* name) {

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
      add_new_target_instruction<op_load_bool>((bool)ret_value._int);
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
      return ZS_COMPILER_ERROR(invalid_token, "define todo table ...");
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
      return ZS_COMPILER_ERROR(invalid_token, "expected '(' after typeid");
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse_expression()) {
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
      return ZS_COMPILER_ERROR(invalid_token, "expected '(' after typeof");
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse_expression()) {
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

    _estate.type = expr_type::e_object;
    // hack: drop into PrefixExpr, case '.'
    _token = tok_dot;
    _estate.pos = top_target();

    _estate.no_assign = true;
    return {};
  }

  case tok_import: {
    lex();

    ZS_TRACE("parse_factor: tok_import");
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
    return {};
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
    return invalid_token;

  case tok_this:
    return call_local_lambda<factor_identifier>();

    //  case tok_export:
    //    if (name) {
    //      *name = zs::_ss("__exports__");
    //    }
    //    return call_local_lambda<factor_identifier>(identifier_type::exports);

  case tok_identifier:
    if (name) {
      *name = get_identifier();
    }

    return call_local_lambda<factor_identifier>();

  case tok_at:
    return ZS_COMPILER_ERROR(invalid_token, "Invalid token '@'.");

  case tok_null:
    add_new_target_instruction<op_load_null>();
    lex();
    break;

  case tok_none:
    add_new_target_instruction<op_load_none>();
    lex();
    break;

  case tok_char_value:
    add_new_target_instruction<op_load_char>(_lexer->get_int_value());
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

    default: {
      auto auto_state = new_auto_state();

      // Given this expression: '-obj.key'.
      // We want to prevent a 'get' in case the object has a __unm meta method.
      _estate.no_get = true;

      ZS_RETURN_IF_ERROR(parse_prefixed());

      switch (_estate.type) {
      case expr_type::e_capture:
        return ZS_COMPILER_ERROR(unimplemented, "unimplemented unary minus for capture");

      case expr_type::e_base:
        return ZS_COMPILER_ERROR(invalid_operation, "Can't '-' (unary minus) a base");

      case expr_type::e_expr:
        add_new_target_instruction<op_umin>(pop_target());
        return {};

      case expr_type::e_object: {
        if (_ccs->_target_stack.size() < 2) {
          return ZS_COMPILER_ERROR(compile_stack_error, "Invalid target stack size for object in umin.");
        }

        target_t key_idx = pop_target();
        target_t table_idx = pop_target();
        add_new_target_instruction<op_obj_umin>(table_idx, key_idx);
        return {};
      }
      case expr_type::e_local:
        add_new_target_instruction<op_umin>(pop_target());
        return {};
      }

      return ZS_COMPILER_ERROR(unimplemented, "unimplemented unary minus");
    }
    }

    break;
  }

  case tok_lsqrbracket: {
    // Array.
    add_new_target_instruction<op_new_obj>(object_type::k_array);
    lex();

    while (is_not(tok_rsqrbracket)) {

      ZS_RETURN_IF_ERROR(parse_expression());

      if (_token == tok_comma) {
        lex();
      }

      add_top_target_instruction<op_array_append>(pop_target());
    }

    lex();
    break;
  }

  case tok_struct: {
    lex();
    return parse_struct(nullptr);
  }

  case tok_lcrlbracket: {
    add_new_target_instruction<op_new_obj>(object_type::k_table);
    lex();
    ZS_COMPILER_PARSE(p_table);
    break;
  }

  case tok_dollar: {
    lex();
    object dummy;
    return parse_function(dummy, true);
  }

  case tok_function: {
    lex();
    object dummy;
    return parse_function(dummy, false);
  }

  case tok_class: {
    lex();
    return ZS_COMPILER_ERROR(unimplemented, "Class is not implemented yet.");
  }

  case tok_inv:
    break;

  case tok_not: {

    lex();
    auto auto_state = new_auto_state();
    _estate.no_get = true;

    ZS_RETURN_IF_ERROR(parse_prefixed());

    switch (_estate.type) {
    case expr_type::e_expr: {
      add_new_target_instruction<op_not>(pop_target());
      break;
    }

    case expr_type::e_base:
      return ZS_COMPILER_ERROR(invalid_operation, "Can't '!' (not) a base");

    case expr_type::e_object: {
      if (_ccs->_target_stack.size() < 2) {
        return ZS_COMPILER_ERROR(compile_stack_error, "Invalid target stack size for object in tok_not.");
      }
      target_t key_idx = pop_target();
      target_t table_idx = pop_target();
      add_new_target_instruction<op_obj_not>(table_idx, key_idx);
      return {};
      //
    }

    case expr_type::e_local: {

      add_new_target_instruction<op_not>(pop_target());
      break;
    }

    case expr_type::e_capture:
      return unimplemented;
    }

    return {};
  }

  case tok_incr:
    return call_local_lambda<parse_uniary_arith_prefixed>(uop_pre_incr);

  case tok_decr:
    return call_local_lambda<parse_uniary_arith_prefixed>(uop_pre_decr);

  case tok_lbracket: {
    if (_lexer->is_right_arrow_function_call()) {
      ZS_COMPILER_PARSE(p_arrow_lamda);
      break;
    }
    else {
      lex();
      ZS_RETURN_IF_ERROR(parse_comma());

      if (!lex_if(tok_rbracket)) {
        return ZS_COMPILER_ERROR(
            invalid_token, "Invalid '", zs::token_to_string(_token), "' token, ')' was expected.");
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

  default:
    return ZS_COMPILER_ERROR(invalid_token, "Invalid '", zs::token_to_string(_token), "' token.");
  }

  return {};
}

} // namespace zs.

#include "jit/parse_op/zjit_table.h"
#include "jit/parse_op/zjit_struct.h"
#include "jit/parse_op/zjit_variable.h"
#include "jit/parse_op/zjit_functions.h"
#include "jit/parse_op/zjit_module.h"
#include "jit/parse_op/zjit_arith.h"
#undef ZS_COMPILER_PARSE_CPP

ZBASE_PRAGMA_POP()
