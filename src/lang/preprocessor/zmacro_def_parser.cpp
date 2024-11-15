#include "lang/preprocessor/zmacro_def_parser.h"
#include "lang/preprocessor/zpreprocessor_parser.h"

namespace zs {
macro_def_parser::macro_def_parser(zs::preprocessor* pp)
    : engine_holder(pp->get_engine())
    , _preprocessor(pp) {}

zs::error_result macro_def_parser::parse(
    std::string_view input_code, zs::string& output_code, bool& found_macro, zs::virtual_machine* vm) {

  if (output_code != input_code) {
    output_code = input_code;
  }

  zbase_assert(output_code == input_code);

  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(input_code);
  lex();

  size_t start_index = 0;

  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    if (is(tok_macro)) {
      const char* begin_ptr = _lexer->stream().ptr() - 6;

      if (auto err = parse_macro()) {
        return err;
      }

      const char* end_ptr = _lexer->stream().ptr();
      std::string_view content(begin_ptr, std::distance(begin_ptr, end_ptr));

      //      zb::print("^^^^", content, "^^^^");

      size_t content_pos = output_code.find(content, start_index);
      output_code.erase(
          output_code.begin() + content_pos, output_code.begin() + content_pos + content.size());

      start_index = content_pos + 1;
      found_macro = true;
    }

    lex();
  }

  if (_token == tok_lex_error) {
    return zs::error_code::invalid;
  }

  return {};
}

zs::error_result macro_def_parser::parse_macro() {

  lex();

  if (is_not(tok_identifier)) {
    return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_token, "@macro 'name' was expected");
  }

  zs::object identifier = _lexer->get_identifier();
  const char* params_begin = _lexer->stream().ptr();
  lex();

  ZS_PREPROCESSOR_EXPECT(tok_lbracket);

  int_t def_params = 0;
  zs::object param_names = zs::_a(_engine, 0);
  zs::object default_values = zs::_a(_engine, 0);

  // Parsing macro parameters: `@macro (parameters)`.
  while (is_not(tok_rbracket)) {
    if (is(tok_triple_dots)) {
      // TODO: Named triple dots?

      if (def_params > 0) {
        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_argument,
            "function with default parameters cannot have variable number of parameters");
      }

      lex();

      if (is_not(tok_rbracket)) {
        return ZS_PREPROCESSOR_ERROR(
            zs::error_code::invalid_token, "expected ')' after a variadic (...) parameter");
      }

      break;
    }

    else {

      if (is_not(tok_identifier)) {
        return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_token, "expected identifier");
      }

      zs::object param_name = _lexer->get_identifier();
      lex();

      param_names.as_array().push(param_name);
      //      param_names.as_array().push(
      //          zs::object::create_concat_string(_engine, "$", param_name.get_string_unchecked()));

      if (is(tok_eq)) {
        def_params++;
        const char* param_begin = _lexer->stream().ptr();

        int_t brack_count = 0;
        int_t ctrlbrack_count = 0;
        int_t sqbrack_count = 0;

        const char* params_last_end = _lexer->stream().ptr();
        while (!zb::is_one_of(_token, tok_rbracket, tok_eof, tok_lex_error)) {
          const char* params_end = _lexer->stream().ptr();
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

              std::string_view param(param_begin, std::distance(param_begin, params_last_end));

              param = zb::strip_all(param);

              if (!param.empty()) {
                default_values.as_array().push(zs::_s(_engine, param));
              }
              else {
                default_values.as_array().push(zs::object::create_none());
              }

              break;
            }

            brack_count--;
            _token = tok_none;
          }

          if (_token == tok_comma and !brack_count and !ctrlbrack_count and !sqbrack_count) {
            std::string_view param(param_begin, std::distance(param_begin, params_end));
            param = zb::strip_all(param);
            default_values.as_array().push(zs::_s(_engine, param));
            break;
            //            params.as_array().push(zs::_s(_engine, param));
            //            //
            //            param_begin = _lexer->stream().ptr();
          }
        }

        //        return ZS_MACRO_PARSER_HANDLE_ERROR_STRING(
        //            zs::error_code::invalid_parameter_type, "macro cannot have default parameters");
      }
      // If a default parameter was defined, all of them (from that point) needs
      // to have one too.
      else if (def_params > 0) {
        return ZS_PREPROCESSOR_ERROR(
            zs::error_code::invalid_token, "expected '=' after a default paramter definition");
      }
      else {
        default_values.as_array().push(zs::object::create_none());
      }

      if (is(tok_comma)) {
        lex();
      }
      else if (is_not(tok_rbracket)) {
        return ZS_PREPROCESSOR_ERROR(
            zs::error_code::invalid_token, "expected ')' or ',' at the end of function declaration");
      }
    }
  }

  const char* params_end = _lexer->stream().ptr();
  const char* content_begin = _lexer->stream().ptr();

  ZS_PREPROCESSOR_EXPECT(tok_rbracket);

  std::string_view params(params_begin, std::distance(params_begin, params_end));

  if (is_not(tok_lcrlbracket)) {
    return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_token,
        "Missing '{' after @macro parameters definition.\n           '@macro(...) {}' must be enclosed with "
        "brackets '{}'.\n");
  }

  int_t brack_count = 0;
  while (!zb::is_one_of(_token, tok_rcrlbracket, tok_eof, tok_lex_error)) {

    lex();

    brack_count += _token == tok_lcrlbracket;

    if (_token == tok_rcrlbracket and brack_count) {
      brack_count--;
      _token = tok_none;
    }
  }

  if (is_not(tok_rcrlbracket)) {
    return ZS_PREPROCESSOR_ERROR(
        zs::error_code::invalid_parameter_type, "macro cannot have default parameters");
  }

  const char* content_end = _lexer->stream().ptr();

  std::string_view content(content_begin, std::distance(content_begin, content_end));

  content = zb::strip_all(content);
  content = content.substr(1);
  content = content.substr(0, content.size() - 1);
  content = zb::strip_all(content);
  zs::macro m = { identifier, zs::_s(_engine, content), param_names, default_values };

  _preprocessor->_macros.push_back(std::move(m));
  return {};
}

} // namespace zs.
