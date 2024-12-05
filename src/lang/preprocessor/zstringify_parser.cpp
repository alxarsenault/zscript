#include "lang/preprocessor/zstringify_parser.h"
#include "lang/preprocessor/zpreprocessor_parser.h"

namespace zs {

stringify_parser::stringify_parser(zs::preprocessor* pp)
    : engine_holder(pp->get_engine())
    , _preprocessor(pp) {}

zs::error_result stringify_parser::parse(
    std::string_view input_code, zs::string& output_code, bool& did_replace, zs::virtual_machine* vm) {

  if (output_code != input_code) {
    output_code = input_code;
  }

  using enum token_type;

  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(input_code);

  lex();

  size_t start_index = 0;

  while (!zb::is_one_of(_token, tok_eof)) {

    if (is(tok_stringify)) {

      const char* begin_ptr = _lexer->stream().ptr() - 4;

      lex();
      if (is_not(tok_lbracket)) {
        return ZS_PREPROCESSOR_ERROR(
            zs::errc::invalid_parameter_type, "macro dasdsada have default parameters");
      }
      const char* content_begin = _lexer->stream().ptr();
      lex();
      int_t brack_count = 0;
      while (!zb::is_one_of(_token, tok_rbracket, tok_eof)) {

        lex();

        brack_count += _token == tok_lbracket;

        if (_token == tok_rbracket and brack_count) {
          brack_count--;
          _token = tok_none;
        }
      }

      if (is_not(tok_rbracket)) {
        return ZS_PREPROCESSOR_ERROR(
            zs::errc::invalid_parameter_type, "macro cannot have default parameters");
      }

      const char* content_end_ptr = _lexer->stream().ptr() - 1;
      std::string_view total(begin_ptr, std::distance(begin_ptr, content_end_ptr + 1));
      std::string_view content(content_begin, std::distance(content_begin, content_end_ptr));
      zb::print("OFD", content, "/////");
      zb::print("total", total, "/////");

      //      td::string_view key(begin_ptr, std::distance(begin_ptr, end_ptr));
      size_t kidx = output_code.find(total, start_index);
      //      zs::string numstr(zs::strprint(_engine, _preprocessor->_counter++));
      output_code.replace(output_code.begin() + kidx, output_code.begin() + kidx + total.size(), "\"");
      output_code.insert(kidx + 1, content);
      output_code.insert(kidx + 1 + content.size(), "\"");
      start_index = kidx + content.size() + 2;
      did_replace = true;
    }

    lex();
  }

  if (_token == tok_lex_error) {
    return zs::error_code::invalid;
  }

  //  output = zs::_s(_engine, output_code);
  return {};

  //  zs::lexer lexer(_engine);
  //  _lexer = &lexer;
  //  _lexer->init(input_code);
  //
  //  const char* begin_ptr = _lexer->stream().ptr();
  //  lex();
  //
  //  size_t start_index = 0;
  //
  //  constexpr std::string_view k_keyword_token = "@keyword";
  //
  //  while (!zb::is_one_of(_token, tok_eof)) {
  //
  //    if (is(tok_keyword)) {
  //
  //      begin_ptr = _lexer->stream().ptr() - k_keyword_token.size();
  //
  //      lex();
  //
  //      ZS_PREPROCESSOR_EXPECT(tok_lbracket, zs::error_code::invalid_include_syntax);
  //
  //      zs::small_vector<object, 4> strs((zs::allocator<object>(_engine)));
  //
  //      while (is_not(tok_rbracket)) {
  //
  //        if (is_not(tok_string_value, tok_escaped_string_value)) {
  //          return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token ");
  //        }
  //
  //        strs.push_back(_lexer->get_value());
  //
  //        //        zs::object str_content = _lexer->get_value();
  //
  //        lex();
  //
  //        lex_if(tok_comma);
  //      }
  //
  //      if (is_not(tok_rbracket)) {
  //        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token");
  //      }
  //
  //      //      if (is_not(tok_string_value, tok_escaped_string_value)) {
  //      //        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token ");
  //      //      }
  //      //
  //      //
  //      //
  //      //      strs.push_back(_lexer->get_value());
  //      //
  //      //      zs::object str_content = _lexer->get_value();
  //      //
  //      //      lex();
  //      //      if (is_not(tok_rbracket)) {
  //      //        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token");
  //      //      }
  //
  //      const char* end_ptr = _lexer->stream().ptr();
  //
  //      std::string_view key(begin_ptr, std::distance(begin_ptr, end_ptr));
  //
  //      size_t kidx = output_code.find(key, start_index);
  //
  //      zs::string con_str(_engine);
  //      for (const auto& s : strs) {
  //        con_str += s.get_string_unchecked();
  //      }
  //
  //      output_code.replace(output_code.begin() + kidx, output_code.begin() + kidx + key.size(), con_str);
  //      start_index = kidx + con_str.size();
  //
  //      did_replace = true;
  //    }
  //
  //    begin_ptr = _lexer->stream().ptr();
  //    lex(true);
  //  }
  //
  //  if (_token == tok_lex_error) {
  //    return zs::error_code::invalid;
  //  }

  //  return {};
}
} // namespace zs.

// #include "lang/zpreprocessor.h"
// #include <random>
//
// #d efine ZS_MACRO_PARSER_HANDLE_ERROR_STREAM(err, ...) \
//  handle_error(err, zs::sstrprint(_engine, __VA_ARGS__), ZB_CURRENT_SOURCE_LOCATION())
//
// #define ZS_MACRO_PARSER_HANDLE_ERROR_STRING(err, msg) handle_error(err, msg, ZB_CURRENT_SOURCE_LOCATION())
//
// #d ef ine ZS_MACRO_PARSER_RETURN_IF_ERROR_STRING(X, err, msg) \
//  if (zs::error_result err = X) {                           \
//    return ZS_MACRO_PARSER_HANDLE_ERROR_STRING(err, msg);   \
//  }
//
// #d ef ine ZS_MACRO_PARSER_RETURN_IF_ERROR_STREAM(X, ...)            \
//  if (zs::error_result err = (X)) {                               \
//    return ZS_MACRO_PARSER_HANDLE_ERROR_STREAM(err, __VA_ARGS__); \
//  }
//
// namespace zs {
//
// zs::error_result stringify_parser::handle_error(
//     zs::error_code ec, std::string_view msg, const zb::source_location& loc) {
//   zs::line_info linfo = _lexer->get_last_line_info();
//
//   const auto& stream = _lexer->stream();
//   const char* begin = &(*stream._data.begin());
//   const char* end = &(*stream._data.end());
//
//   const char* it_line_begin = stream.ptr() - 1;
//   while (it_line_begin > begin) {
//     if (*it_line_begin == '\n') {
//       ++it_line_begin;
//       break;
//     }
//
//     --it_line_begin;
//   }
//
//   const char* it_line_end = stream.ptr();
//   while (it_line_end < end) {
//     if (*it_line_end == '\n') {
//       break;
//     }
//
//     ++it_line_end;
//   }
//
//   std::string_view line_content(it_line_begin, std::distance(it_line_begin, it_line_end));
//
//   const int column = linfo.column ? (int)linfo.column - 1 : 0;
//
//   constexpr const char* new_line_padding = "\n       ";
//
//   std::string_view fname = loc.function_name();
//
//   if (fname.size() > 80) {
//     _error_message += zs::strprint(_engine, "\nerror: ", linfo, new_line_padding, line_content,
//         new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '", fname.substr(0, 80),
//         "\n               ", fname.substr(80), "'", new_line_padding, "     in '", loc.file_name(), "'",
//         new_line_padding, "      at line ", loc.line(), "\n", new_line_padding, "*** ", msg);
//   }
//   else {
//     _error_message += zs::strprint(_engine, "\nerror: ", linfo, new_line_padding, line_content,
//         new_line_padding, zb::indent_t(column, 1), "^", new_line_padding, "from '", loc.function_name(),
//         "'", new_line_padding, "      in '", loc.file_name(), "'", new_line_padding, "      at line ",
//         loc.line(),
//         "\n", new_line_padding, "*** ", msg);
//   }
//
//   return ec;
// }
//
// zs::error_code stringify_parser::expect(token_type tok) noexcept {
//   if (is_not(tok)) {
//     return ZS_MACRO_PARSER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
//         zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
//         zb::quoted<"'">(zs::token_to_string(tok)));
//   }
//
//   lex();
//   return zs::error_code::success;
// }
//
// zs::error_code stringify_parser::expect_get(token_type tok, object& ret) {
//   using enum token_type;
//
//   if (is_not(tok)) {
//     return ZS_MACRO_PARSER_HANDLE_ERROR_STREAM(zs::error_code::invalid_token, "invalid token ",
//         zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",
//         zb::quoted<"'">(zs::token_to_string(tok)));
//   }
//
//   ret = _lexer->get_value();
//   lex();
//   return {};
// }
//
// stringify_parser::stringify_parser(zs::preprocessor* pp)
//     : engine_holder(pp->get_engine())
//     , _preprocessor(pp)
//     , _error_message(pp->get_engine()) {}

// zs::error_result stringify_parser::parse(std::string_view code, std::string_view filename, object& output,
//     bool& did_include, zs::virtual_machine* vm) {
//   using enum token_type;
//
//   zs::string output_code(code, _engine);
//
//   zs::lexer lexer(_engine);
//   _lexer = &lexer;
//   _lexer->init(code);
//
//   lex();
//
//   size_t start_index = 0;
//
//   while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {
//
//     if (is(tok_stringify)) {
//
//       const char* begin_ptr = _lexer->stream().ptr() - 4;
//
//       lex();
//       if (is_not(tok_lbracket)) {
//         return ZS_MACRO_PARSER_HANDLE_ERROR_STRING(
//             zs::error_code::invalid_parameter_type, "macro dasdsada have default parameters");
//       }
//       const char* content_begin = _lexer->stream().ptr();
//       lex();
//       int_t brack_count = 0;
//       while (!zb::is_one_of(_token, tok_rbracket, tok_eof, tok_lex_error)) {
//
//         lex();
//
//         brack_count += _token == tok_lbracket;
//
//         if (_token == tok_rbracket and brack_count) {
//           brack_count--;
//           _token = tok_none;
//         }
//       }
//
//       if (is_not(tok_rbracket)) {
//         return ZS_MACRO_PARSER_HANDLE_ERROR_STRING(
//             zs::error_code::invalid_parameter_type, "macro cannot have default parameters");
//       }
//
//       const char* content_end_ptr = _lexer->stream().ptr() - 1;
//       std::string_view total(begin_ptr, std::distance(begin_ptr, content_end_ptr + 1));
//       std::string_view content(content_begin, std::distance(content_begin, content_end_ptr));
//       zb::print("OFD", content, "/////");
//       zb::print("total", total, "/////");
//
//       //      td::string_view key(begin_ptr, std::distance(begin_ptr, end_ptr));
//       size_t kidx = output_code.find(total, start_index);
//       //      zs::string numstr(zs::strprint(_engine, _preprocessor->_counter++));
//       output_code.replace(output_code.begin() + kidx, output_code.begin() + kidx + total.size(), "\"");
//       output_code.insert(kidx + 1, content);
//       output_code.insert(kidx + 1 + content.size(), "\"");
//       start_index = kidx + content.size() + 2;
//       did_include = true;
//     }
//
//     lex();
//   }
//
//   if (_token == tok_lex_error) {
//     return zs::error_code::invalid;
//   }
//
//   output = zs::_s(_engine, output_code);
//   return {};
// }
//
// } // namespace zs.
