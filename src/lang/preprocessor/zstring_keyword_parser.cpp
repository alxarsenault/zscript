#include "lang/preprocessor/zstring_keyword_parser.h"
#include "lang/preprocessor/zpreprocessor_parser.h"

namespace zs {

string_keyword_parser::string_keyword_parser(zs::preprocessor* pp)
    : engine_holder(pp->get_engine())
    , _preprocessor(pp) {}

zs::error_result string_keyword_parser::parse(
    std::string_view input_code, zs::string& output_code, bool& did_replace, zs::virtual_machine* vm) {

  if (output_code != input_code) {
    output_code = input_code;
  }

  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(input_code);

  const char* begin_ptr = _lexer->stream().ptr();
  lex();

  size_t start_index = 0;

  constexpr std::string_view k_keyword_token = "@keyword";

  while (!zb::is_one_of(_token, tok_eof)) {

    if (is(tok_keyword)) {

      begin_ptr = _lexer->stream().ptr() - k_keyword_token.size();

      lex();

      ZS_PREPROCESSOR_EXPECT(tok_lbracket, zs::error_code::invalid_include_syntax);

      zs::small_vector<object, 4> strs((zs::allocator<object>(_engine)));

      while (is_not(tok_rbracket)) {

        if (is_not(tok_string_value, tok_escaped_string_value)) {
          return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token ");
        }

        strs.push_back(_lexer->get_value());

        //        zs::object str_content = _lexer->get_value();

        lex();

        lex_if(tok_comma);
      }

      if (is_not(tok_rbracket)) {
        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token");
      }

      //      if (is_not(tok_string_value, tok_escaped_string_value)) {
      //        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token ");
      //      }
      //
      //
      //
      //      strs.push_back(_lexer->get_value());
      //
      //      zs::object str_content = _lexer->get_value();
      //
      //      lex();
      //      if (is_not(tok_rbracket)) {
      //        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token");
      //      }

      const char* end_ptr = _lexer->stream().ptr();

      std::string_view key(begin_ptr, std::distance(begin_ptr, end_ptr));

      size_t kidx = output_code.find(key, start_index);

      zs::string con_str(_engine);
      for (const auto& s : strs) {
        con_str += s.get_string_unchecked();
      }

      output_code.replace(output_code.begin() + kidx, output_code.begin() + kidx + key.size(), con_str);
      start_index = kidx + con_str.size();

      did_replace = true;
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
