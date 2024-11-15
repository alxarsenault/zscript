#include "lang/preprocessor/zinclude_parser.h"
#include "lang/preprocessor/zpreprocessor_parser.h"

namespace zs {

include_parser::include_parser(zs::preprocessor* pp)
    : engine_holder(pp->get_engine())
    , _preprocessor(pp) {}

zs::error_result include_parser::parse(
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

    if (is(tok_include, tok_import)) {

      bool is_import = is(tok_import);
      lex();

      ZS_PREPROCESSOR_EXPECT(tok_lbracket, zs::error_code::invalid_include_syntax);

      if (is_not(tok_string_value, tok_escaped_string_value)) {
        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token ");
      }
      zs::object file_name = _lexer->get_value();

      lex();
      if (is_not(tok_rbracket)) {
        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_syntax, "invalid token");
      }

      //      if (_lexer->peek() == tok_semi_colon) {
      //        lex();
      //      }

      const char* end_ptr = _lexer->stream().ptr();

      std::string_view key(begin_ptr, std::distance(begin_ptr, end_ptr));
      object res_file_name;

      if (auto err = _engine->resolve_file_path(file_name.get_string_unchecked(), res_file_name)) {
        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_include_file, "parse include statement");
      }

      if (is_import) {
        // Check for multiple inclusion.
        if (auto it = _preprocessor->_imported_files_set.find(res_file_name);
            it != _preprocessor->_imported_files_set.end()) {

          size_t kidx = output_code.find(key, start_index);

          output_code.erase(output_code.begin() + kidx, output_code.begin() + kidx + key.size());
          start_index = kidx;

          begin_ptr = _lexer->stream().ptr();
          lex(true);
          continue;
        }

        _preprocessor->_imported_files_set.insert(res_file_name);
      }

      zs::file_loader loader(_engine);
      zbase_assert(!res_file_name.is_string_view(), "cannot be a string_view");

      if (auto err = loader.open(res_file_name.get_string_unchecked().data())) {
        return ZS_PREPROCESSOR_ERROR(zs::error_code::open_file_error, "parse include statement");
      }

      size_t kidx = output_code.find(key, start_index);

      output_code.replace(
          output_code.begin() + kidx, output_code.begin() + kidx + key.size(), loader.content());
      start_index = kidx + loader.content().size();

      did_include = true;
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
