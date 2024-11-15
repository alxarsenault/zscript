#include "lang/zparser.h"
#include <zbase/strings/stack_string.h>
#include <zbase/strings/parse_utils.h>

ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

#define ZS_PARSER_HANDLE_ERROR_STREAM(err, ...) \
  helper::handle_error(this, err, zs::strprint(_engine, __VA_ARGS__), ZB_CURRENT_SOURCE_LOCATION())

#define ZS_PARSER_HANDLE_ERROR_STRING(err, msg) \
  helper::handle_error(this, err, msg, ZB_CURRENT_SOURCE_LOCATION())

#define ZS_PARSER_RETURN_IF_ERROR_STRING(X, err, msg) \
  if (zs::error_result err = X) {                     \
    return ZS_PARSER_HANDLE_ERROR_STRING(err, msg);   \
  }

#define ZS_PARSER_RETURN_IF_ERROR_STREAM(X, err, ...)       \
  if (zs::error_result err = X) {                           \
    return ZS_PARSER_HANDLE_ERROR_STREAM(err, __VA_ARGS__); \
  }

namespace zs {

enum class parser::parse_op : uint8_t {
  p_preprocessor,
  p_statement,
  p_expression,
  p_function_statement,
  p_function,
  p_function_call_args,
  p_create_function,
  p_comma,
  p_semi_colon,
  p_decl_var,
  p_variable_type_restriction,
  p_table_or_class,
  p_class,

  p_if,
  p_if_block,
  p_factor,
  p_bind_env,
  p_define,
  p_as_table,
  p_load_json_file,
  p_as_string,
  p_as_value,

  // '.', '[', '++', '--', '('.
  p_prefixed,

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

  // '|'.
  p_bitwise_or,

  // '&&'.
  p_and,

  // '||'
  p_or,

  count
};

using enum ast_node_type;
using enum parser::parse_op;
struct parser::helper {

  static inline zs::error_result handle_error(
      parser* comp, zs::error_code ec, std::string_view msg, const zb::source_location& loc) {
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
    return ec;
  }

  //  inline static void parse_line(parser* p) {
  //    if(!p->_output_lines) {
  //      return;
  //    }
  //
  //    zb::utf8_span_stream ss(p->_lexer->_stream);
  //    ss.goto_next_line_end();
  //
  //    const char* start_ptr = p->_last_ptr ? p->_last_ptr : p->_lexer->_stream.ptr();
  //
  //    int_t sz = std::distance(start_ptr, ss.ptr() - 1);
  //
  //    if (sz <= 0) {
  //      return;
  //    }
  //
  //    std::string_view line_str(start_ptr, sz);
  //
  //    line_str = zb::strip_leading_and_trailing_spaces(zb::strip_leading_and_trailing_endlines(line_str));
  //
  //    if (auto endl = line_str.find("\n"); endl != std::string_view::npos) {
  //      line_str = line_str.substr(0, endl);
  //    }
  //
  //    line_str = zb::strip_leading_and_trailing_spaces(line_str);
  //
  //    //      zb::print(sz,  "KKKKk", line_str);
  //
  //    if (!p->_stack.empty() and p->top().type() != (int)ast_line) {
  //
  //      p->top().add_child(ast_node(p->_engine, ast_line, zs::_s(p->_engine, line_str))
  //                             .with_attributes({ { zs::_ss("line"), p->_lexer->_current_line },
  //                                 { zs::_ss("column"), p->_lexer->_current_column } }));
  //      //      node.as_node().add_attribute(zs::_ss("line"), p->_lexer->_current_line);
  //      //      node.as_node().add_attribute(zs::_ss("column"), p->_lexer->_current_column);
  //      //  _lexer->
  //      //      p->top().add_child(node);
  //    }
  //  }

  inline static statement_info parse_line(parser* p) {
    if (!p->_output_lines) {
      return statement_info(p->_lexer->get_line_info());
    }

    zb::utf8_span_stream ss(p->_lexer->_stream);
    ss.goto_next_line_end();

    const char* start_ptr = p->_last_ptr ? p->_last_ptr : p->_lexer->_stream.ptr();

    int_t sz = std::distance(start_ptr, ss.ptr() - 1);

    if (sz <= 0) {
      return statement_info(p->_lexer->get_line_info());
    }

    std::string_view line_str(start_ptr, sz);

    line_str = zb::strip_leading_spaces_and_tabs(line_str);
    line_str = zb::strip_leading_and_trailing_endlines(line_str);
    line_str = zb::strip_leading_and_trailing_spaces(line_str);
    line_str = zb::strip_leading_and_trailing_endlines(line_str);

    //    if (auto endl = line_str.find("\n"); endl != std::string_view::npos) {
    //      line_str = line_str.substr(0, endl);
    //    }

    //    line_str = zb::strip_leading_and_trailing_spaces(line_str);

    //      zb::print(sz,  "KKKKk", line_str);

    if (p->_stack.empty() or zs::ast_node_name_to_type(p->top().name()) == ast_line) {
      return statement_info(p->_lexer->get_line_info());

      //      node.as_node().add_attribute(zs::_ss("line"), p->_lexer->_current_line);
      //      node.as_node().add_attribute(zs::_ss("column"), p->_lexer->_current_column);
      //  _lexer->
      //      p->top().add_child(node);
    }

    return statement_info(line_str, p->_lexer->get_line_info());
    //    p->top().add_child(ast_node(p->_engine, ast_line, zs::_s(p->_engine, line_str))
    //                           .with_attributes({ { zs::_ss("line"), p->_lexer->_current_line },
    //                               { zs::_ss("column"), p->_lexer->_current_column } }));
  }
};

//
// MARK: Parse forward declare.
//

template <>
zs::error_result parser::parse<p_statement>(bool close_frame);

template <>
zs::error_result parser::parse<p_function_call_args>(bool rawcall);

template <>
zs::error_result parser::parse<p_semi_colon>();

template <>
zs::error_result parser::parse<p_function_statement>();

template <>
zs::error_result parser::parse<p_variable_type_restriction>(
    std::reference_wrapper<uint32_t> mask, std::reference_wrapper<uint64_t> custom_mask);

template <>
zs::error_result parser::parse<p_expression>();

template <>
zs::error_result parser::parse<p_create_function>(
    std::reference_wrapper<const object> name, int_t boundtarget, bool lambda);

template <>
zs::error_result parser::parse<p_factor>();

template <>
zs::error_result parser::parse<p_bind_env>(std::reference_wrapper<int_t> target);

template <>
zs::error_result parser::parse<p_as_table>();

template <>
zs::error_result parser::parse<p_as_string>();

template <>
zs::error_result parser::parse<p_decl_var>();
template <>
zs::error_result parser::parse<p_as_value>();

template <>
zs::error_result parser::parse<p_load_json_file>();

template <>
zs::error_result parser::parse<p_prefixed>();

template <>
zs::error_result parser::parse<p_comma>();

template <>
zs::error_result parser::parse<p_preprocessor>();

template <>
zs::error_result parser::parse<p_if>();

//
// MARK: Compiler.
//

parser::parser(zs::engine* eng)
    : engine_holder(eng)
    , _error_message(zs::allocator<char>(eng))
    , _root(ast_node(eng, ast_root))
    , _stack(zs::allocator<zs::object>(eng))
    , _filename(zs::allocator<char>(eng)) {

  _stack.push_back(_root);
}

parser::~parser() {}

inline token_type parser::lex() {
  _last_ptr = _lexer->_stream.ptr();
  _token = _lexer->lex();
  return _token;
}

zs::error_result parser::parse(
    std::string_view content, std::string_view filename, object& output, zs::token_type* prepended_token) {

  _filename = filename;

  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(content);
  _lexer->_export_block_comments = true;

  if (prepended_token) {
    _token = *prepended_token;
  }
  else {
    lex();
  }

  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    if (auto err = parse<p_statement>(true)) {
      return err;
    }

    if (!zb::is_one_of(_lexer->last_token(), tok_rcrlbracket, tok_semi_colon)) {
      ZS_RETURN_IF_ERROR(parse<p_semi_colon>());
    }
  }

  if (_token == tok_lex_error) {
    return zs::error_code::invalid;
  }

  _root = _stack[0];

  if (_stack.size() == 1) {
    _stack.pop_back();
  }

  return {};
}

template <>
zs::error_result parser::parse<p_statement>(bool close_frame) {

  using enum parse_op;

  zs::statement_info sinfo = helper::parse_line(this);

  switch (_token) {
  case tok_semi_colon:
    lex();
    return {};

  case tok_doc_block:

    top().add_child(ast_node(_engine, ast_doc_block, zs::_s(_engine, _lexer->get_string_value())));
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
  case tok_float: {
    ZS_RETURN_IF_ERROR(parse<p_decl_var>());

    top().add_attribute("line", sinfo.loc.line);
    top().add_attribute("col", sinfo.loc.column);
    if (!sinfo.content.empty()) {
      top().value() = zs::_s(_engine, sinfo.content);
    }

    add_top_to_previous();

    return {};
  }

  case tok_hastag:
    return parse<p_preprocessor>();

  case tok_include:
    return zs::error_code::unimplemented;

  case tok_import:
    return zs::error_code::unimplemented;

  case tok_if: {

    ZS_RETURN_IF_ERROR(parse<p_if>());
    top().add_attribute("line", sinfo.loc.line);
    top().add_attribute("col", sinfo.loc.column);
    if (!sinfo.content.empty()) {
      top().value() = zs::_s(_engine, sinfo.content);
    }

    add_top_to_previous();
    return {};
  }

  case tok_function:
    return parse<p_function_statement>();

  case tok_return: {
    lex();

    if (!is_end_of_statement()) {
      ZS_RETURN_IF_ERROR(parse<p_comma>());

      ast_node expr(_engine, ast_return_statement);
      expr.add_child(get_pop_back());
      add_child_to_top(std::move(expr));
    }
    else {
      ast_node expr(_engine, ast_return_statement);
      expr.add_child(get_pop_back());
      add_child_to_top(std::move(expr));
    }

    return {};
  }

  case tok_lcrlbracket: {

    lex();

    while (is_not(tok_rcrlbracket, tok_default, tok_case)) {
      ZS_RETURN_IF_ERROR(parse<p_statement>(true));

      if (_lexer->_last_token != tok_rcrlbracket && _lexer->_last_token != tok_semi_colon) {

        ZS_RETURN_IF_ERROR(parse<p_semi_colon>());
      }
    }

    ZS_RETURN_IF_ERROR(expect(tok_rcrlbracket));

    return {};
  }

  default:
    ZS_RETURN_IF_ERROR(parse<p_comma>());

    top().add_attribute("line", sinfo.loc.line);
    top().add_attribute("col", sinfo.loc.column);
    if (!sinfo.content.empty()) {
      top().value() = zs::_s(_engine, sinfo.content);
    }

    add_top_to_previous();
    return {};
  }

  return {};
}

template <>
zs::error_result parser::parse<p_exponential>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_prefixed;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {
    case tok_exp: {
      //      ZS_RETURN_IF_ERROR(
      //          helper::do_arithmetic_expr<opcode::op_exp>(this,
      //          &compiler::parse<p_exponential>,
      //          "^"));
      break;
    }

    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result parser::parse<p_mult>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_exponential;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {
    case tok_mul: {
      lex();
      ZS_RETURN_IF_ERROR(parse<next_op>());
      push_arith_expr_node(ast_op_mul);
    } break;

    case tok_div: {
      lex();
      ZS_RETURN_IF_ERROR(parse<next_op>());
      push_arith_expr_node(ast_op_div);
    } break;

    case tok_mod: {
      lex();
      ZS_RETURN_IF_ERROR(parse<next_op>());
      push_arith_expr_node(ast_op_mod);
    } break;

    default:
      return {};
    }
  }
  return {};
}

template <>
zs::error_result parser::parse<p_plus>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_mult;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {
    case tok_add: {

      lex();
      ZS_RETURN_IF_ERROR(parse<next_op>());
      push_arith_expr_node(ast_op_add);
      break;
    }

    case tok_minus: {
      lex();
      ZS_RETURN_IF_ERROR(parse<next_op>());
      push_arith_expr_node(ast_op_sub);
      break;
    }

    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result parser::parse<p_shift>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_plus;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {
    case tok_lshift:
      //      helper::binary_exp(this, opcode::op_bitw,
      //      &compiler::parse<p_plus>);

      // binary_exp(_OP_BITW, &SQCompiler::PlusExp, BW_SHIFTL);
      break;
    case tok_rshift:
      //      helper::binary_exp(this, opcode::op_bitw,
      //      &compiler::parse<p_plus>);

      // binary_exp(_OP_BITW, &SQCompiler::PlusExp, BW_SHIFTR);
      break;
    default:
      return {};
    }
  }
  return {};
}

template <>
zs::error_result parser::parse<p_compare>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_shift;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {
    case tok_gt:
      //      helper::binary_exp(this, opcode::op_cmp,
      //      &compiler::parse<p_shift>);
      // binary_exp(_OP_CMP, &SQCompiler::ShiftExp, CMP_G);
      break;
    case tok_lt:
      //      helper::binary_exp(this, opcode::op_cmp,
      //      &compiler::parse<p_shift>);
      // binary_exp(_OP_CMP, &SQCompiler::ShiftExp, CMP_L);
      break;
    case tok_gt_eq:
      //      helper::binary_exp(this, opcode::op_cmp,
      //      &compiler::parse<p_shift>);
      // binary_exp(_OP_CMP, &SQCompiler::ShiftExp, CMP_GE);
      break;
    case tok_lt_eq:
      //      helper::binary_exp(this, opcode::op_cmp,
      //      &compiler::parse<p_shift>);
      // binary_exp(_OP_CMP, &SQCompiler::ShiftExp, CMP_LE);
      break;
    case tok_in:
      //      helper::binary_exp(this, opcode::op_cmp,
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
zs::error_result parser::parse<p_eq_compare>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_compare;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {

    case tok_eq_eq: {
      lex();
      ZS_RETURN_IF_ERROR(parse<next_op>());
      push_arith_expr_node(ast_eq_eq);
    } break;

    case tok_not_eq: {
      lex();
      ZS_RETURN_IF_ERROR(parse<next_op>());
      push_arith_expr_node(ast_not_eq);
    } break;

    case tok_three_way_compare:
      ZS_TODO("Implement");
      //      helper::binary_exp(this, opcode::op_cmp,
      //      &compiler::parse<p_compare>);
      break;
    case tok_double_arrow:
      ZS_TODO("Implement");
      //      helper::binary_exp(this, opcode::op_cmp,
      //      &compiler::parse<p_compare>);
      break;

    case tok_double_arrow_eq:
      //      helper::binary_exp(this, opcode::op_cmp,
      //      &compiler::parse<p_compare>);
      break;
    default:
      return {};
    }
  }
  return {};
}

template <>
zs::error_result parser::parse<p_bitwise_and>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_eq_compare;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    if (is(tok_bitwise_and)) {
      //      helper::binary_exp(this, opcode::op_bitw,
      //      &compiler::parse<p_eq_compare>);
      //        binary_exp(_OP_BITW, &SQCompiler::EqExp, BW_AND);
    }
    else {
      return {};
    }
  }
  return {};
}

template <>
zs::error_result parser::parse<p_bitwise_or>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_bitwise_and;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    if (is(tok_bitwise_or)) {
      //      helper::binary_exp(this, opcode::op_bitw,
      //      &compiler::parse<p_bitwise_and>);
    }
    else {
      return {};
    }
  }

  return {};
}

template <>
zs::error_result parser::parse<p_and>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_bitwise_or;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    switch (_token) {
    case tok_and: {
      lex();
      ZS_RETURN_IF_ERROR(parse<p_and>());
      break;
    }

    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result parser::parse<p_or>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_and;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  zb_loop() {
    if (is(tok_or)) {

      lex();
      ZS_RETURN_IF_ERROR(parse<p_or>());
    }
    break;
  }
  return {};
}

template <>
zs::error_result parser::parse<p_expression>() {

  using enum parse_op;

  //  push_node(ast_expr);

  static constexpr parse_op next_op = p_or;
  ZS_RETURN_IF_ERROR(parse<next_op>());
  //
  //  expr_state es = _estate;
  //  _estate.type = expr_type::e_expr;
  //  _estate.pos = -1;
  //  _estate.no_get = false;
  //
  //  zb::scoped expr_state_cache = [&]() { _estate = es; };

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
  //  expr_type es_type = _estate.type;
  //
  //  if (es_type == expr_type::e_expr) {
  //    _error_message += zs::strprint(
  //        _engine, "Can't assign an expression", _lexer->get_line_info(),
  //        ZB_CURRENT_SOURCE_LOCATION());
  //
  //    return zs::error_code::invalid_value_type_assignment;
  //  }
  //  else if (es_type == expr_type::e_base) {
  //    _error_message += zs::strprint(
  //        _engine, "'base' cannot be modified", _lexer->get_line_info(),
  //        ZB_CURRENT_SOURCE_LOCATION());
  //    return zs::error_code::invalid_value_type_assignment;
  //  }

  lex();
  if (auto err = parse<p_expression>()) {
    return err;
  }

  switch (op) {
  // Assign.
  case tok_eq: {
    if (zs::ast_node_name_to_type(_stack(-2).as_node().name()) == ast_dot) {
      //        push_arith_expr_node(ast_set);

      ast_node value_node = get_pop_back();
      ast_node dot_node = get_pop_back();

      //        _stack.pop_back();
      //        _stack.pop_back();

      ast_node set_expr(_engine, ast_set);
      set_expr.as_node().children() = std::move(dot_node.as_node().children());

      value_node.add_attribute("role", zs::_ss("value"));
      set_expr.add_child(std::move(value_node));

      //        _stack(-2).as_node().add_attribute("role", zs::_ss("lhs"));
      //        _stack(-1).as_node().add_attribute("role", zs::_ss("rhs"));
      //        expr.add_children(std::move(_stack(-2)), std::move(_stack(-1)));

      _stack.push_back(std::move(set_expr));
    }
    else {
      push_arith_expr_node(ast_assignment);
    }
    break;
  }

  case tok_add_eq:
    push_arith_expr_node(ast_op_add_eq);
    break;

  case tok_mul_eq:
    push_arith_expr_node(ast_op_mul_eq);
    break;

  case tok_minus_eq:
    push_arith_expr_node(ast_op_sub_eq);
    break;

  case tok_div_eq:
    push_arith_expr_node(ast_op_div_eq);
    break;

  case tok_exp_eq:
    push_arith_expr_node(ast_op_exp_eq);
    break;

  case tok_mod_eq:
    push_arith_expr_node(ast_op_mod_eq);
    break;

  case tok_lshift_eq:
  case tok_rshift_eq:
  case tok_bitwise_or_eq:
  case tok_bitwise_and_eq:
  case tok_double_arrow_eq: {
    return zs::error_code::unimplemented;
  } break;

  default:
    break;
  }
  return {};
}

template <>
zs::error_result parser::parse<p_comma>() {

  using enum parse_op;

  static constexpr parse_op next_op = p_expression;
  ZS_RETURN_IF_ERROR(parse<next_op>());

  while (is(tok_comma)) {
    lex();
    ZS_RETURN_IF_ERROR(parse<p_comma>());
  }

  return {};
}

template <>
zs::error_result parser::parse<p_semi_colon>() {
  if (is(tok_semi_colon)) {
    lex();
    return {};
  }

  if (!is_end_of_statement()) {
    return helper::handle_error(
        this, zs::error_code::invalid_token, "invalid token", ZB_CURRENT_SOURCE_LOCATION());
  }

  return {};
}

template <>
zs::error_result parser::parse<p_decl_var>() {

  using enum parse_op;
  using enum object_type;

  token_type variable_type = _token;

  lex();

  ast_node var_type(
      _engine, ast_var_type, zs::object((zs::int_t)k_none).with_flags(object_flags_t::f_type_info));

  //
  //  ast_node var_type(_engine, ast_var_type);
  //  var_type._value = (zs::int_t)k_none;
  //  var_type._value._flags = object_flags_t::type_info;

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
  zb::stack_string<32> type_str;
  zb::stack_string<32> cpp_type_str;

  switch (variable_type) {
  case tok_char:
  case tok_int:
    mask = zs::get_object_type_mask(k_integer);
    var_type.as_node().value() = zs::object((zs::int_t)k_integer).with_flags(object_flags_t::f_type_info);
    type_str = "int";
    cpp_type_str = "zs::int_t";
    //    var_type._value = (zs::int_t)k_integer;
    //    var_type._value._flags = object_flags_t::type_info;
    break;
  case tok_float:
    mask = zs::get_object_type_mask(k_float);
    var_type.as_node().value() = zs::object((zs::int_t)k_float).with_flags(object_flags_t::f_type_info);
    type_str = "float";
    cpp_type_str = "zs::float_t";
    //    var_type._value = (zs::int_t)k_float;
    //    var_type._value._flags = object_flags_t::type_info;
    break;
  case tok_string:
    mask = zs::object_base::k_string_mask;
    var_type.as_node().value() = zs::object((zs::int_t)k_long_string).with_flags(object_flags_t::f_type_info);
    type_str = "string";
    cpp_type_str = "zs::var";
    //    var_type._value = (zs::int_t)k_long_string;
    //    var_type._value._flags = object_flags_t::type_info;
    break;
  case tok_array:
    mask = zs::get_object_type_mask(k_array);
    var_type.as_node().value() = zs::object((zs::int_t)k_array).with_flags(object_flags_t::f_type_info);
    type_str = "array";
    cpp_type_str = "zs::var";
    //    var_type._value = (zs::int_t)k_array;
    //    var_type._value._flags = object_flags_t::type_info;
    break;
  case tok_table:
    mask = zs::get_object_type_mask(k_table);
    var_type.as_node().value() = zs::object((zs::int_t)k_table).with_flags(object_flags_t::f_type_info);
    type_str = "table";
    cpp_type_str = "zs::var";
    //    var_type._value = (zs::int_t)k_table;
    //    var_type._value._flags = object_flags_t::type_info;
    break;
  case tok_bool:
    mask = zs::get_object_type_mask(k_bool);
    var_type.as_node().value() = zs::object((zs::int_t)k_bool).with_flags(object_flags_t::f_type_info);
    type_str = "bool";
    cpp_type_str = "zs::bool_t";
    //    var_type._value = (zs::int_t)k_bool;
    //    var_type._value._flags = object_flags_t::type_info;
    break;
  case tok_var:
    type_str = "var";
    cpp_type_str = "zs::var";
    // Parsing a typed var (var<type1, type2, ...>).
    if (is(tok_lt)) {
      if (auto err = parse<p_variable_type_restriction>(std::ref(mask), std::ref(custom_mask))) {
        return helper::handle_error(
            this, err, "parsing variable type restriction `var<....>`", ZB_CURRENT_SOURCE_LOCATION());
      }
    }
    break;
  }

  if (mask) {
  }

  zb_loop() {
    if (is_not(tok_identifier)) {
      return helper::handle_error(
          this, zs::error_code::identifier_expected, "expected identifier", ZB_CURRENT_SOURCE_LOCATION());
    }

    ast_node vdecl(_engine, ast_variable_declaration);
    vdecl.as_node().attributes().emplace_back(zs::_ss("name"), _lexer->get_identifier());
    vdecl.as_node().attributes().emplace_back(zs::_ss("type"), zs::_s(_engine, type_str));
    vdecl.as_node().attributes().emplace_back(zs::_ss("cpp-type"), zs::_s(_engine, cpp_type_str));
    //    vdecl.as_node().add_child(var_type);

    lex();

    // @code `var name = ...;`
    if (is(tok_eq)) {
      lex();
      ZS_RETURN_IF_ERROR(parse<p_expression>());

      zbase_assert(!_stack.empty(), "should not be empty");

      vdecl.as_node().add_child(get_pop_back());
      push_node(std::move(vdecl));
    }

    // @code `var name;`
    else {
    }

    if (is_not(tok_comma)) {
      break;
    }

    lex();
  }

  return {};
}

template <>
zs::error_result parser::parse<p_create_function>(
    std::reference_wrapper<const object> name, int_t boundtarget, bool lambda) {

  ast_node function_expr(_engine, ast_function_declaration);
  function_expr.add_attribute("name", name.get());

  using enum token_type;
  using enum object_type;

  //    ast_node vdecl(_engine, ast_variable_declaration,
  //    _lexer->get_identifier());

  //    vdecl.add_child(var_type);
  //    lex();

  //  zs::closure_compile_state* fct_state = _ccs->push_child_state();
  //  fct_state->name = name;
  //  fct_state->source_name = _ccs->source_name;

  //  ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("this")));

  int_t def_params = 0;

  // Parsing function parameters: `function (parameters)`.
  while (is_not(tok_rbracket)) {

    if (is(tok_triple_dots)) {
      // TODO: Named triple dots?

      if (def_params > 0) {
        return helper::handle_error(this, zs::error_code::invalid_argument,
            "function with default parameters cannot have variable number of "
            "parameters",
            ZB_CURRENT_SOURCE_LOCATION());
      }

      //      ZS_RETURN_IF_ERROR(fct_state->add_parameter(zs::_ss("vargv")));
      //      fct_state->_vargs_params = true;
      lex();

      if (is_not(tok_rbracket)) {
        return helper::handle_error(this, zs::error_code::invalid_token,
            "expected ')' after a variadic (...) parameter", ZB_CURRENT_SOURCE_LOCATION());
      }

      break;
    }
    else {

      ast_node var_type(_engine, ast_var_type, object((int_t)k_none).with_flags(object_flags_t::f_type_info));
      //      var_type._value = (zs::int_t)k_none;
      //      var_type._value._flags = object_flags_t::type_info;

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
        case tok_var:
          // Parsing a typed var (var<type1, type2, ...>).
          if (is(tok_lt)) {
            if (auto err = parse<p_variable_type_restriction>(std::ref(mask), std::ref(custom_mask))) {
              return helper::handle_error(
                  this, err, "parsing variable type restriction `var<....>`", ZB_CURRENT_SOURCE_LOCATION());
            }
          }
          break;
        }
      }

      zs::object param_name;
      ZS_RETURN_IF_ERROR(expect_get(tok_identifier, param_name));
      //      ZS_RETURN_IF_ERROR(fct_state->add_parameter(param_name, mask,
      //      custom_mask, is_const));

      ast_node param_decl(_engine, ast_parameter_declaration, param_name);
      param_decl.as_node().add_child(var_type);

      if (is(tok_eq)) {
        lex();
        ZS_RETURN_IF_ERROR(parse<p_expression>());
        //        _ccs->add_default_param(_ccs->top_target());
        def_params++;
      }

      // If a default parameter was defined, all of them (from that point) needs
      // to have one too.
      else if (def_params > 0) {
        return helper::handle_error(this, zs::error_code::invalid_token,
            "expected '=' after a default paramter definition", ZB_CURRENT_SOURCE_LOCATION());
      }

      if (is(tok_comma)) {
        lex();
      }
      else if (is_not(tok_rbracket)) {
        return helper::handle_error(this, zs::error_code::invalid_token,
            "expected ')' or ',' at the end of function declaration", ZB_CURRENT_SOURCE_LOCATION());
      }

      function_expr.as_node().add_child(param_decl);
    }
  }
  //  //

  ZS_RETURN_IF_ERROR(expect(tok_rbracket));
  top().add_child(function_expr);
  //  if (boundtarget != 0xFF) {
  //    //      _fs->pop_target();
  //  }
  //
  //  for (int_t n = 0; n < def_params; n++) {
  //    _ccs->pop_target();
  //  }
  //
  //  zs::closure_compile_state* curr_chunk = std::exchange(_ccs, fct_state);
  //  _ccs = fct_state;

  if (lambda) {
    ZS_RETURN_IF_ERROR(parse<p_expression>());
    //      _fs->AddInstruction(_OP_RETURN, 1, _fs->PopTarget());
  }
  else {
    ZS_RETURN_IF_ERROR(parse<p_statement>(false));
  }

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
  //  fct_state->set_stack_size(0);

  //  zs::function_prototype_object* fpo =
  //  fct_state->build_function_prototype();

  //  object_ptr output;
  //  object_raw_init(output);
  //  output._type = object_type::k_function_prototype;
  //  object_proxy::as_function_prototype(output) = fpo;

  //  _ccs = curr_chunk;
  //  _ccs->_functions.push_back(output);
  //  _ccs->pop_child_state();

  //  return {};

  return {};
}

template <>
zs::error_result parser::parse<p_function_statement>() {
  using enum token_type;

  lex();

  zs::object var_name;

  ZS_RETURN_IF_ERROR(expect_get(tok_identifier, var_name));

  int_t bound_target = 0xFF;

  if (is(tok_lsqrbracket)) {
    ZS_RETURN_IF_ERROR(parse<p_bind_env>(std::ref(bound_target)));
  }

  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  ZS_RETURN_IF_ERROR(parse<p_create_function>(std::cref(var_name), bound_target, false));

  //  _ccs->pop_target();
  //  ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name));

  return {};
}

template <>
zs::error_result parser::parse<p_define>() {
  return zs::error_code::unimplemented;
}

template <>
zs::error_result parser::parse<p_preprocessor>() {
  return zs::error_code::unimplemented;
}

template <>
zs::error_result parser::parse<p_if_block>() {
  using enum token_type;
  if (is(tok_lcrlbracket)) {
    //       BEGIN_SCOPE();
    lex();
    //       Statements();
    //       ZS_RETURN_IF_ERROR(expect(tok_rcrlbracket));

    while (is_not(tok_rcrlbracket, tok_default, tok_case)) {
      ZS_RETURN_IF_ERROR(parse<p_statement>(true));

      if (_lexer->_last_token != tok_rcrlbracket && _lexer->_last_token != tok_semi_colon) {
        //            OptionalSemicolon();
        ZS_RETURN_IF_ERROR(parse<p_semi_colon>());
      }
    }

    ZS_RETURN_IF_ERROR(expect(tok_rcrlbracket));

    //       if (true) {
    //         END_SCOPE();
    //       }
    //       else {
    //         END_SCOPE_NO_CLOSE();
    //       }
  }
  else {
    ZS_RETURN_IF_ERROR(parse<p_statement>(true));

    if (_lexer->_last_token != tok_rcrlbracket && _lexer->_last_token != tok_semi_colon) {
      //            OptionalSemicolon();
      ZS_RETURN_IF_ERROR(parse<p_semi_colon>());
    }

    //       Statement();
    //       if (_lex._prevtoken != _SC('}') && _lex._prevtoken != _SC(';'))
    //         OptionalSemicolon();
  }

  return {};
}

template <>
zs::error_result parser::parse<p_if>() {

  using enum parse_op;

  ZS_RETURN_IF_ERROR(expect(tok_if));
  ZS_RETURN_IF_ERROR(expect(tok_lbracket));

  ZS_RETURN_IF_ERROR(parse<p_comma>());

  ZS_RETURN_IF_ERROR(expect(tok_rbracket));

  push_node(ast_node(_engine, ast_if_statement)
                .with_children(get_pop_back().with_attribute("role", zs::_ss("condition"))));

  push_node(ast_if_block);

  ZS_RETURN_IF_ERROR(parse<p_if_block>());

  //  int_t end_if_block_index = _ccs->get_instruction_index();

  if (is_not(tok_else)) {

    // Pop if block and put it in if_expr.
    add_top_to_previous();

    //    instruction_t<opcode::op_jz>* inst =
    //    _ccs->_instructions[jz_index].get<opcode::op_jz>();

    //    instruction_t<opcode::op_jz>* inst
    //        = (instruction_t<opcode::op_jz>*)(_ccs->_instructions._data.data()
    //        + jz_index);
    //    inst->offset = (int32_t)(end_if_block_index - jz_index);

    return {};
  }

  //  {
  //    ast_node expr(_engine, k_jump);
  //    expr.add_child( get_pop_back() );
  //    _stack.push_back(std::move(expr));
  //  }
  //  add_instruction<opcode::op_jmp>(0);
  //  int_t jmp_index = _ccs->get_instruction_index();

  // Close if_expr.
  add_top_to_previous();

  {
    // Create else block.
    push_node(ast_else_block);

    lex();
    ZS_RETURN_IF_ERROR(parse<p_if_block>());

    add_top_to_previous();
  }
  //  int_t end_else_block_index = _ccs->get_instruction_index();
  {
    //    instruction_t<opcode::op_jmp>* inst
    //        =
    //        (instruction_t<opcode::op_jmp>*)(_ccs->_instructions._data.data()
    //        + jmp_index);
    //    inst->offset = (int32_t)(end_else_block_index - jmp_index);
  }

  {
    //    instruction_t<opcode::op_jz>* inst
    //        = (instruction_t<opcode::op_jz>*)(_ccs->_instructions._data.data()
    //        + jz_index);
    //    inst->offset = (int32_t)(jmp_index - jz_index);
  }
  return {};
}

template <>
zs::error_result parser::parse<p_variable_type_restriction>(
    std::reference_wrapper<uint32_t> mask, std::reference_wrapper<uint64_t> custom_mask) {

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
      //      int_t type_index = _ccs->get_restricted_type_index(name);
      //      custom_mask |= (uint64_t)(1 << type_index);
      break;
    }

    default:
      zb::print("ERROR: Invalid token");
      return zs::error_code::invalid_type;
    }
  }

  if (is_not(tok_gt)) {
    return helper::handle_error(
        this, zs::error_code::invalid_token, "expected var<...>", ZB_CURRENT_SOURCE_LOCATION());
  }

  lex();
}

template <>
zs::error_result parser::parse<p_function>(bool lambda) {
  return {};
}

template <>
zs::error_result parser::parse<p_function_call_args>(bool rawcall) {
  return {};
}

template <>
zs::error_result parser::parse<p_table_or_class>(token_type separator, token_type terminator) {

  using enum parse_op;

  ast_node expr(_engine, ast_table_declaration);

  //  int_t tpos = _fs->GetCurrentPos(), nkeys = 0;
  while (_token != terminator) {

    switch (_token) {

    case tok_function: {
      lex();
      ZS_RETURN_IF_ERROR(expect(tok_identifier));
      break;
    }

    case tok_lsqrbracket: {
      lex();
      ZS_RETURN_IF_ERROR(parse<p_comma>());
      ZS_RETURN_IF_ERROR(expect(tok_rsqrbracket));
      ZS_RETURN_IF_ERROR(expect(tok_eq));
      ZS_RETURN_IF_ERROR(parse<p_expression>());
      break;
    }

    case tok_string_value:
    case tok_escaped_string_value: {

      // only works for tables
      if (separator == tok_comma) {
        zs::object value;
        ZS_RETURN_IF_ERROR(expect_get(tok_string_value, value));
        push_node(ast_string_value, std::move(value));

        ZS_RETURN_IF_ERROR(expect(tok_colon));
        ZS_RETURN_IF_ERROR(parse<p_expression>());

        break;
      }

      break;
    }
    default:
      zs::object identifier;
      ZS_RETURN_IF_ERROR(expect_get_identifier(identifier));
      push_node(ast_string_value, std::move(identifier));
      ZS_RETURN_IF_ERROR(expect(tok_eq));
      ZS_RETURN_IF_ERROR(parse<p_expression>());
    }

    if (_token == separator) {
      // optional comma/semicolon
      lex();
    }

    ast_node new_table_field_expr(_engine, ast_new_table_field);
    _stack(-2).as_node().add_attribute("role", zs::_ss("key"));
    _stack(-1).as_node().add_attribute("role", zs::_ss("value"));

    new_table_field_expr.add_child(std::move(_stack(-2)));
    new_table_field_expr.add_child(std::move(_stack(-1)));
    _stack.pop_back();
    _stack.pop_back();
    expr.add_child(std::move(new_table_field_expr));
  }

  // hack recognizes a table from the separator
  if (separator == tok_comma) {
    //    _fs->SetInstructionParam(tpos, 1, nkeys);
  }
  lex();

  push_node(std::move(expr));
  return {};
}

template <>
zs::error_result parser::parse<p_class>() {
  return {};
}

zs::error_result parser::parse_include_or_import_statement(token_type tok) { return {}; }

template <>
zs::error_result parser::parse<p_bind_env>(std::reference_wrapper<int_t> target) {
  return {};
}

template <>
zs::error_result parser::parse<p_prefixed>() {

  using enum parse_op;

  //  int_t pos = -1;
  ZS_RETURN_IF_ERROR(parse<p_factor>());

  zb_loop() {
    switch (_token) {
    case tok_dot: {
      lex();

      object var_name;
      ZS_RETURN_IF_ERROR(expect_get_identifier(var_name));

      ast_node dot_expr(_engine, ast_dot);
      dot_expr.add_child(get_pop_back().with_attribute("role", zs::_ss("object")));
      dot_expr.add_child(
          ast_node(_engine, ast_string_value, std::move(var_name)).with_attribute("role", zs::_ss("key")));
      push_node(std::move(dot_expr));

      //      push_node(ast_node(_engine, ast_dot)
      //                    .with_children(get_pop_back(), ast_node(_engine, ast_string_value,
      //                    std::move(var_name))));
      break;
    }
    case tok_lsqrbracket: {
      if (_lexer->last_token() == tok_endl) {
        return helper::handle_error(this, zs::error_code::invalid_token,
            "cannot break deref/or comma needed after [exp]=exp slot "
            "declaration",
            ZB_CURRENT_SOURCE_LOCATION());
      }

      lex();
      ZS_RETURN_IF_ERROR(parse<p_expression>());
      ast_node expr(_engine, ast_dot);
      _stack(-2).as_node().add_attribute("role", zs::_ss("object"));
      _stack(-1).as_node().add_attribute("role", zs::_ss("key"));
      expr.add_children(std::move(_stack(-2)), std::move(_stack(-1)));
      _stack.pop_back();
      _stack.pop_back();
      _stack.push_back(std::move(expr));
      ZS_RETURN_IF_ERROR(expect(tok_rsqrbracket));

      break;
    }

    case tok_decr:
    case tok_incr: {
      if (is_end_of_statement()) {
        return {};
      }

      lex();
      return {};
    }

    case tok_lbracket: {
      //      switch (_estate.type) {
      //      case expr_type::e_object: {
      //        if (_estate.pos == -1) {
      //          add_instruction<opcode::op_move>(_ccs->new_target(),
      //          (uint8_t)0);
      //        }
      //        else {
      //          add_instruction<opcode::op_move>(_ccs->new_target(),
      //          (uint8_t)_estate.pos);
      //        }
      //
      //        break;
      //      }
      //      case expr_type::e_base:
      //        break;
      //      case expr_type::e_capture:
      //        break;
      //      default:
      //        add_instruction<opcode::op_move>(_ccs->new_target(),
      //        (uint8_t)0);
      //       }
      //
      //      _estate.type = expr_type::e_expr;
      lex();

      ZS_RETURN_IF_ERROR(parse<p_function_call_args>(false));
      break;
    }
    default:
      return {};
    }
  }

  return {};
}

template <>
zs::error_result parser::parse<p_factor>() {

  using enum parse_op;

  switch (_token) {
  case tok_at: {
    return zs::error_code::unimplemented;
  }

  case tok_string_value: {
    push_node(ast_string_value, zs::_s(_engine, _lexer->get_string_value()));
    lex();
    break;
  }

  case tok_escaped_string_value: {
    push_node(ast_string_value, zs::_s(_engine, _lexer->get_escaped_string_value()));
    lex();
    break;
  }

  case tok_this:
    break;

  case tok_base:
    break;

  case tok_typeid: {
    lex();

    if (zs::error_result err = expect(tok_lbracket)) {
      return helper::handle_error(
          this, zs::error_code::invalid_token, "expected '(' after typeid", ZB_CURRENT_SOURCE_LOCATION());
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse<p_expression>()) {
        zb::print("ERRRO");
        return err;
      }
    }

    lex();
    break;
  }

  case tok_typeof: {
    lex();

    if (zs::error_result err = expect(tok_lbracket)) {
      return helper::handle_error(
          this, zs::error_code::invalid_token, "expected '(' after typeof", ZB_CURRENT_SOURCE_LOCATION());
    }

    while (is_not(tok_rbracket)) {
      if (auto err = parse<p_expression>()) {
        zb::print("ERRRO");
        return err;
      }
    }

    //    int_t tg = _ccs->pop_target();
    //    add_instruction<opcode::op_typeof>(_ccs->new_target(), tg);

    lex();
    break;
  }

  case tok_double_colon: {

    //    add_instruction<opcode::op_load_root>(_ccs->new_target());
    //    _estate.type = expr_type::e_object;
    //    _token = tok_dot; // hack: drop into PrefixExpr, case '.'
    //    _estate.pos = -1;
    //    target.get() = -1;
    return {};
  }

  case tok_import: {
    lex();
    break;
  }

  case tok_identifier: {
    push_node(ast_identifier, _lexer->get_identifier());
    lex();
    break;
  }

  case tok_constructor:
    break;

  case tok_null:
    push_node(ast_null_value, nullptr);
    lex();
    break;

  case tok_none:
    push_node(ast_null_value, object::create_none());
    lex();
    break;

  case tok_integer_value:
    push_node(ast_integer_value, _lexer->get_int_value());
    lex();
    break;

  case tok_float_value:
    push_node(ast_float_value, _lexer->get_float_value());
    lex();
    break;

  case tok_true:
    push_node(ast_bool_value, true);
    lex();
    break;

  case tok_false:
    push_node(ast_bool_value, false);
    lex();
    break;

  case tok_minus: {
    lex();

    switch (_token) {
    case tok_integer_value:
      push_node(ast_integer_value, -_lexer->get_int_value());
      lex();
      break;

    case tok_float_value:
      push_node(ast_float_value, -_lexer->get_float_value());
      lex();
      break;

    default:
      // UnaryOP(_OP_NEG);
      return helper::handle_error(
          this, zs::error_code::unimplemented, "unimplemented unary minus", ZB_CURRENT_SOURCE_LOCATION());

      break;
    }
    break;
  }

  case tok_lsqrbracket: {
    // Array.
    ast_node arr_expr(_engine, ast_array_declaration);

    lex();

    while (is_not(tok_rsqrbracket)) {
      if (auto err = parse<p_expression>()) {

        zb::print("ERRRO");
        return err;
      }

      if (_token == tok_comma) {
        lex();
      }

      arr_expr.add_child(ast_node(_engine, ast_array_element).with_children(get_pop_back()));
    }

    push_node(std::move(arr_expr));

    lex();
    break;
  }

  case tok_lcrlbracket: {
    //    add_instruction<opcode::op_new_obj>(_ccs->new_target(),
    //    object_type::k_table);
    lex();
    ZS_RETURN_IF_ERROR(parse<p_table_or_class>(tok_comma, tok_rcrlbracket));
    break;
  }

  case tok_function: {
    ZS_RETURN_IF_ERROR(parse<p_function>(false));
    break;
  }

  case tok_class: {
    lex();
    ZS_RETURN_IF_ERROR(parse<p_class>());
    break;
  }

  case tok_not:
    break;

  case tok_inv:
    break;

  case tok_decr:
    break;

  case tok_incr:
    break;

  case tok_lbracket: {
    lex();

    ZS_RETURN_IF_ERROR(parse<p_comma>());

    if (zs::error_result err = expect(tok_rbracket)) {
      return helper::handle_error(
          this, zs::error_code::unimplemented, "expression expected", ZB_CURRENT_SOURCE_LOCATION());
    }
    break;
  }

  case tok_file:
    break;

  case tok_line:
    break;

  case tok_hastag: {
    lex();

    object id;
    ZS_RETURN_IF_ERROR(expect_get_identifier(id));

    if (id == "as_string") {
      // Get the content of a file as string.
      ZS_RETURN_IF_ERROR(parse<p_as_string>());
    }
    else if (id == "as_table") {
      // Get the content of a file as table.
      ZS_RETURN_IF_ERROR(parse<p_as_table>());
    }
    else if (id == "as_value") {
      // Get the content of a file as value.
      ZS_RETURN_IF_ERROR(parse<p_as_value>());
    }
    else if (id == "load_json_file") {
      // Get the content of a file as table.
      ZS_RETURN_IF_ERROR(parse<p_load_json_file>());
    }
    else {
      return helper::handle_error(this, zs::error_code::invalid_include_syntax,
          "expected `as_string`, `as_table` or ??", ZB_CURRENT_SOURCE_LOCATION());
    }
    break;
  }

  default: {
    return helper::handle_error(
        this, zs::error_code::unimplemented, "expression expected", ZB_CURRENT_SOURCE_LOCATION());
    break;
  }
  }

  //  target.get() = -1;
  return {};
}

template <>
zs::error_result parser::parse<p_load_json_file>() {
  return {};
}

template <>
zs::error_result parser::parse<p_as_string>() {
  return {};
}

template <>
zs::error_result parser::parse<p_as_value>() {
  return {};
}

template <>
zs::error_result parser::parse<p_as_table>() {
  return {};
}

zs::error_code parser::expect(token_type tok) noexcept {

  if (is_not(tok)) {
    return ZS_PARSER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
        zb::quoted<"'">(zs::token_to_string(tok)));
  }

  lex();
  return zs::error_code::success;
}

zs::error_code parser::expect_get(token_type tok, object& ret) {

  if (is_not(tok)) {
    return ZS_PARSER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
        zb::quoted<"'">(zs::token_to_string(tok)));
  }

  ret = _lexer->get_value();
  lex();
  return {};
}

zs::error_code parser::expect_get_identifier(object& ret) {

  if (is_not(tok_identifier)) {
    return ZS_PARSER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
        zb::quoted<"'">(zs::token_to_string(tok_identifier)));
  }

  ret = _lexer->get_identifier();
  lex();
  return {};
}

ast_node parser::get_pop_back() noexcept { return ast_node(_stack.get_pop_back()); }

void parser::add_top_to_previous() { add_child_to_top(get_pop_back()); }
} // namespace zs.

ZBASE_PRAGMA_POP()
