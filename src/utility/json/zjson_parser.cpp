#include "zjson_parser.h"
#include "zvirtual_machine.h"

ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

#define ZS_JSON_PARSER_HANDLE_ERROR_STREAM(err, ...) \
  helper::handle_error(this, err, zs::sstrprint(_engine, __VA_ARGS__), ZB_CURRENT_SOURCE_LOCATION())

#define ZS_JSON_PARSER_HANDLE_ERROR_STRING(err, msg) \
  helper::handle_error(this, err, msg, ZB_CURRENT_SOURCE_LOCATION())

namespace zs {

struct json_parser::helper {

  static inline zs::error_result handle_error(
      json_parser* p, zs::error_code ec, std::string_view msg, const zb::source_location& loc) {
    zs::line_info linfo = p->_lexer->get_last_line_info();

    const auto& stream = p->_lexer->_stream;
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
      p->_error_message += zs::strprint(p->_engine, "\nerror: ", linfo, new_line_padding, line_content,
          new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '", fname.substr(0, 80),
          "\n               ", fname.substr(80), "'", new_line_padding, "     in '", loc.file_name(), "'",
          new_line_padding, "     at line ", loc.line(), "\n", new_line_padding, "*** ", msg);
    }
    else {
      p->_error_message += zs::strprint(p->_engine, "\nerror: ", linfo, new_line_padding, line_content,
          new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '", loc.function_name(),
          "'", new_line_padding, "      in '", loc.file_name(), "'", new_line_padding, "     at line ",
          loc.line(), "\n", new_line_padding, "*** ", msg);
    }

    return ec;
  }
};

//
// MARK: json_parser
//

json_parser::json_parser(zs::engine* eng)
    : engine_holder(eng)
    , _stack(zs::allocator<object>(eng))
    , _error_message(zs::allocator<char>(eng)) {}

json_parser::~json_parser() {}

json_token_type json_parser::lex() {
  _token = _lexer->lex();
  return _token;
}

zs::error_result json_parser::parse(zs::virtual_machine* vm, std::string_view content, const object& table,
    object& output, zs::json_token_type* prepended_token) {
  using enum json_token_type;

  _vm = vm;
  _table = table.is_table() ? &table : nullptr;
  zs::json_lexer lexer(_engine);

  _lexer = &lexer;

  _lexer->init(content);

  if (prepended_token) {
    _token = *prepended_token;
  }
  else {
    lex();
  }

  object ret_value;
  if (auto err = parse_value(ret_value)) {
    return helper::handle_error(this, err, "invalid token", ZB_CURRENT_SOURCE_LOCATION());
  }

  if (_token == tok_lex_error) {
    return zs::error_code::invalid;
  }

  output = ret_value;
  return {};
}

zs::error_result json_parser::parse_table(zs::object& value) {
  using enum json_token_type;

  ZS_RETURN_IF_ERROR(expect(tok_lcrlbracket));

  value = zs::object::create_table(_engine);

  while (!zb::is_one_of(_token, tok_rcrlbracket, tok_eof, tok_lex_error)) {

    object key, tvalue;
    if (auto err = parse_table_value(key, tvalue)) {
      return err;
    }

    value.get_table_internal_map()->emplace(key, tvalue);

    if (!is(tok_comma)) {
      break;
    }

    lex();
  }

  return {};
}

zs::error_result json_parser::parse_array(zs::object& value) {
  using enum json_token_type;

  ZS_RETURN_IF_ERROR(expect(tok_lsqrbracket));

  value = zs::object::create_array(_engine, 0);

  while (!zb::is_one_of(_token, tok_rsqrbracket, tok_eof, tok_lex_error)) {

    object avalue;
    if (auto err = parse_value(avalue)) {
      return err;
    }

    value.get_array_internal_vector()->push_back(std::move(avalue));

    if (!is(tok_comma)) {
      break;
    }

    lex();
  }

  return {};
}

zs::error_result json_parser::parse_table_value(zs::object& key, zs::object& value) {
  using enum json_token_type;

  if (is(tok_string_value)) {
    key = zs::_s(_engine, _lexer->get_string_value());
  }
  else if (is(tok_escaped_string_value)) {
    key = zs::_s(_engine, _lexer->get_escaped_string_value());
  }
  else {
    return zs::error_code::invalid_token;
  }

  lex();

  ZS_RETURN_IF_ERROR(expect(tok_colon));

  return parse_value(value);
}

zs::error_result json_parser::parse_value(zs::object& value) {
  using enum json_token_type;

  switch (_token) {
  case tok_lcrlbracket: {

    if (auto err = parse_table(value)) {
      return err;
    }

    ZS_RETURN_IF_ERROR(expect(tok_rcrlbracket));
    return {};
  }

  case tok_lsqrbracket: {
    if (auto err = parse_array(value)) {
      return err;
    }
    ZS_RETURN_IF_ERROR(expect(tok_rsqrbracket));

    return {};
  }

  case tok_none:
    value = zs::object::create_none();
    lex();
    return {};

  case tok_string_value:
    value = zs::_s(_engine, _lexer->get_string_value());
    lex();
    return {};

  case tok_escaped_string_value:
    value = zs::_s(_engine, _lexer->get_escaped_string_value());
    lex();
    return {};

  case tok_integer_value:
    value = _lexer->get_int_value();
    lex();
    return {};

  case tok_float_value:
    value = _lexer->get_float_value();
    lex();
    return {};

  case tok_false:
    value = false;
    lex();
    return {};

  case tok_true:
    value = true;
    lex();
    return {};

  case tok_null:
    value.reset();
    lex();
    return {};

  case tok_identifier: {
    if (_vm && _table) {
      ZS_RETURN_IF_ERROR(_vm->get(*_table, _lexer->get_identifier(), value));
    }
    else if (_table) {
      ZS_RETURN_IF_ERROR(_table->_table->get(_lexer->get_identifier(), value));
    }
    else {
      return zs::error_code::inaccessible;
    }
    lex();

    if (is(tok_dot)) {
      zs::object key;

      lex();
      ZS_RETURN_IF_ERROR(expect_get(tok_identifier, key));

      if (_vm) {
        ZS_RETURN_IF_ERROR(_vm->get(value, key, value));
      }
      else {
        ZS_RETURN_IF_ERROR(value._table->get(key, value));
      }
    }

    if (is(tok_dot)) {
      zs::object key;

      lex();
      ZS_RETURN_IF_ERROR(expect_get(tok_identifier, key));

      if (_vm) {
        ZS_RETURN_IF_ERROR(_vm->get(value, key, value));
      }
      else {
        ZS_RETURN_IF_ERROR(value._table->get(key, value));
      }
    }
    return {};
  }

  default:
    return zs::error_code::invalid_token;
  }

  return {};
}

bool json_parser::is_end_of_statement() const noexcept {
  using enum json_token_type;
  return (_lexer->last_token() == tok_endl) || is(tok_eof, tok_rcrlbracket);
}

zs::error_code json_parser::expect(json_token_type tok) noexcept {

  if (is_not(tok)) {
    _error_message
        += zs::strprint(_engine, "invalid token ", zb::quoted<"'">(zs::json_token_to_string(_token)),
            ", expected ", zb::quoted<"'">(zs::json_token_to_string(tok)), _lexer->get_line_info());
    return zs::error_code::invalid_token;
  }

  lex();
  return zs::error_code::success;
}

zs::error_code json_parser::expect_get(json_token_type tok, object& ret) {
  using enum json_token_type;

  if (is_not(tok)) {
    _error_message
        += zs::strprint(_engine, "invalid token ", zb::quoted<"'">(zs::json_token_to_string(_token)),
            ", expected ", zb::quoted<"'">(zs::json_token_to_string(tok)), _lexer->get_line_info(),
            ZB_CURRENT_SOURCE_LOCATION());

    return zs::error_code::invalid_token;
  }

  ret = _lexer->get_value();
  lex();
  return {};
}

} // namespace zs.

ZBASE_PRAGMA_POP()
