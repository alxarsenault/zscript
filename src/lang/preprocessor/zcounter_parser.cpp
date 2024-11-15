#include "lang/preprocessor/zcounter_parser.h"
#include "lang/preprocessor/zpreprocessor_parser.h"

namespace zs {

counter_parser::counter_parser(zs::preprocessor* pp)
    : engine_holder(pp->get_engine())
    , _preprocessor(pp) {}

zs::error_result counter_parser::parse(
    std::string_view input_code, zs::string& output_code, bool& did_include, zs::virtual_machine* vm) {

  if (output_code != input_code) {
    output_code = input_code;
  }

  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(input_code);

  const char* begin_ptr = _lexer->stream().ptr();
  lex();

  size_t start_index = 0;

  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    if (is(tok_counter)) {

      const char* end_ptr = _lexer->stream().ptr();

      std::string_view key(begin_ptr, std::distance(begin_ptr, end_ptr));
      size_t kidx = output_code.find(key, start_index);
      zs::string numstr(zs::strprint(_engine, _preprocessor->_counter++));
      output_code.replace(output_code.begin() + kidx, output_code.begin() + kidx + key.size(), numstr);
      start_index = kidx + numstr.size();

      did_include = true;
    }

    begin_ptr = _lexer->stream().ptr();
    lex();
  }

  if (_token == tok_lex_error) {
    return zs::error_code::invalid;
  }

  return {};
}

} // namespace zs.
