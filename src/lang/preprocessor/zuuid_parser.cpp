#include "lang/preprocessor/zuuid_parser.h"
#include "lang/zpreprocessor.h"
#include <random>

#define ZS_MACRO_PARSER_HANDLE_ERROR_STREAM(err, ...) \
  handle_error(err, zs::strprint(_engine, __VA_ARGS__), ZB_CURRENT_SOURCE_LOCATION())

#define ZS_MACRO_PARSER_HANDLE_ERROR_STRING(err, msg) handle_error(err, msg, ZB_CURRENT_SOURCE_LOCATION())

#define ZS_MACRO_PARSER_RETURN_IF_ERROR_STRING(X, err, msg) \
  if (zs::error_result err = X) {                           \
    return ZS_MACRO_PARSER_HANDLE_ERROR_STRING(err, msg);   \
  }

#define ZS_MACRO_PARSER_RETURN_IF_ERROR_STREAM(X, ...)            \
  if (zs::error_result err = (X)) {                               \
    return ZS_MACRO_PARSER_HANDLE_ERROR_STREAM(err, __VA_ARGS__); \
  }

namespace zs {

zs::error_result uuid_parser::handle_error(
    zs::error_code ec, std::string_view msg, const zb::source_location& loc) {
  zs::line_info linfo = _lexer->get_last_line_info();

  const auto& stream = _lexer->stream();
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
    _error_message += zs::strprint(_engine, "\nerror: ", linfo, new_line_padding, line_content,
        new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '", fname.substr(0, 80),
        "\n               ", fname.substr(80), "'", new_line_padding, "     in '", loc.file_name(), "'",
        new_line_padding, "      at line ", loc.line(), "\n", new_line_padding, "*** ", msg);
  }
  else {
    _error_message += zs::strprint(_engine, "\nerror: ", linfo, new_line_padding, line_content,
        new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '", loc.function_name(), "'",
        new_line_padding, "      in '", loc.file_name(), "'", new_line_padding, "      at line ", loc.line(),
        "\n", new_line_padding, "*** ", msg);
  }

  return ec;
}

zs::error_code uuid_parser::expect(token_type tok) noexcept {
  if (is_not(tok)) {
    return ZS_MACRO_PARSER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
        zb::quoted<"'">(zs::token_to_string(tok)));
  }

  lex();
  return zs::error_code::success;
}

zs::error_code uuid_parser::expect_get(token_type tok, object& ret) {
  using enum token_type;

  if (is_not(tok)) {
    return ZS_MACRO_PARSER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
        zb::quoted<"'">(zs::token_to_string(tok)));
  }

  ret = _lexer->get_value();
  lex();
  return {};
}

uuid_parser::uuid_parser(zs::preprocessor* pp)
    : engine_holder(pp->get_engine())
    , _preprocessor(pp)
    , _error_message(pp->get_engine()) {}

std::string_view Guid(std::array<char, 40>& buffer) {

  buffer.fill(0);
  std::random_device random_device;

  std::uniform_int_distribution<uint64_t> distribution;
  std::array<uint64_t, 2> bytes = { distribution(random_device), distribution(random_device) };

  // Clear the version bits and set the version to 4:
  bytes[0] &= 0xffffffff'ffff0fffULL;
  bytes[0] |= 0x00000000'00004000ULL;

  // Set the two most significant bits (bits 6 and 7) of the
  // clock_seq_hi_and_reserved to zero and one, respectively:
  bytes[1] &= 0x3fffffff'ffffffffULL;
  bytes[1] |= 0x80000000'00000000ULL;

  const auto ret = std::snprintf( // NOLINT(runtime/printf)
      buffer.data(), buffer.size(), "ID_%08X_%04X_%04X_%04X_%012llX", (uint32_t)(bytes[0] >> 32),
      (uint32_t)((bytes[0] >> 16) & 0x0000ffff), (uint32_t)(bytes[0] & 0x0000ffff),
      (uint32_t)(bytes[1] >> 48), bytes[1] & 0x0000ffff'ffffffffULL);

  //  zb::print(std::string_view((const char*)buffer.data(), buffer.size()));
  return std::string_view(buffer.data(), buffer.size() - 1);
}

zs::error_result uuid_parser::parse(std::string_view code, std::string_view filename, object& output,
    bool& did_include, zs::virtual_machine* vm) {
  using enum token_type;

  zs::string output_code(code, _engine);

  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(code);

  lex();

  size_t start_index = 0;

  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    if (is(tok_uuid)) {
      zs::object identifier;
      const char* begin_ptr = _lexer->stream().ptr() - 5;

      if (_lexer->peek() == tok_lbracket) {
        lex();
        if (zs::error_result err = expect(tok_lbracket)) {
          return zs::error_code::invalid_include_syntax;
        }

        if (is_not(tok_identifier)) {
          return ZS_MACRO_PARSER_HANDLE_ERROR_STRING(
              zs::error_code::invalid_include_syntax, "invalid token ");
        }

        identifier = _lexer->get_identifier();

        lex();
        if (is_not(tok_rbracket)) {
          return ZS_MACRO_PARSER_HANDLE_ERROR_STRING(zs::error_code::invalid_include_syntax, "invalid token");
        }
        //        lex();
      }
      const char* end_ptr = _lexer->stream().ptr();
      //      std::string_view key(_lexer->stream().ptr() - 5, 5);

      std::string_view key(begin_ptr, std::distance(begin_ptr, end_ptr));
      //      zbase_assert(key == "@uuid", "invalid @uuid");
      zb::print("DLKJDS", key, "DDD");
      size_t key_pos = output_code.find(key, start_index);

      if (key_pos == std::string::npos) {
        return ZS_MACRO_PARSER_HANDLE_ERROR_STRING(zs::error_code::invalid_name, "invalid uuid");
      }

      std::array<char, 40> byte_uuid;
      std::string_view suuid = Guid(byte_uuid);

      if (identifier.is_string()) {
        _preprocessor->_uuid_map.as_table()[identifier] = zs::_s(_engine, suuid);
      }

      output_code.replace(output_code.begin() + key_pos, output_code.begin() + key_pos + key.size(), suuid);
      start_index = key_pos + suuid.size();

      did_include = true;
    }

    lex();
  }

  if (_token == tok_lex_error) {
    return zs::error_code::invalid;
  }

  //  look_and_replace_one_uuid_val(zs::_ss("asj"), zs::_ss("PPPP"), output_code, did_include);

  for (auto it : _preprocessor->_uuid_map.as_table()) {
    if (auto err = look_and_replace_one_uuid_val(it.first, it.second, output_code, did_include)) {
      return ZS_MACRO_PARSER_HANDLE_ERROR_STRING(err, "invalid uuid");
    }
  }
  output = zs::_s(_engine, output_code);
  return {};
}

zs::error_result uuid_parser::look_and_replace_one_uuid_val(
    zs::object oname, zs::object oval, zs::string& output_code, bool& found_one) {

  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(output_code);

  //  const zs::string name("$" + zs::string(oname.get_string_unchecked(), _engine));

  const char* begin_ptr = _lexer->stream().ptr();
  lex(true);

  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    if (is(tok_dollar)) {
      begin_ptr = _lexer->stream().ptr();
      lex();
    }

    if (is(tok_identifier)) {
      zs::object identifier_name = _lexer->get_identifier();
      if (identifier_name == oname.get_string_unchecked()) {

        const char* end_ptr = _lexer->stream().ptr();
        //      std::string_view key(_lexer->stream().ptr() - 5, 5);

        std::string_view key(begin_ptr, std::distance(begin_ptr, end_ptr));
        //        int_t dist = std::distance((const char*)output_code.data(), begin_ptr);

        //        std::string_view in_code = std::string_view(output_code).substr(dist);
        //
        //
        size_t lindex = std::distance((const char*)output_code.data(), begin_ptr - 1);
        size_t rindex = std::distance((const char*)output_code.data(), end_ptr);
        //        //        zb::print(ZBASE_VNAME(lindex), ZBASE_VNAME(rindex));
        //
        output_code.replace(
            output_code.begin() + lindex, output_code.begin() + rindex, oval.get_string_unchecked());
        //
        //        //        _lexer = &lexer;
        //
        found_one = true;

        return {};
      }
    }

    begin_ptr = _lexer->stream().ptr();
    lex(true);
  }

  if (_token == tok_lex_error) {
    return zs::error_code::invalid;
  }

  return {};
}
} // namespace zs.
