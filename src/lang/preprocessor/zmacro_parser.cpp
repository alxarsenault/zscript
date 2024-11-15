#include "lang/preprocessor/zmacro_parser.h"
#include "lang/preprocessor/zpreprocessor_parser.h"

namespace zs {

macro_parser::macro_parser(zs::preprocessor* pp)
    : engine_holder(pp->get_engine())
    , _preprocessor(pp) {}

zs::error_result macro_parser::parse(
    std::string_view input_code, zs::string& output_code, bool& found_macro, zs::virtual_machine* vm) {

  output_code = input_code;

  bool keep_going = true;
  while (keep_going) {
    if (auto err = replace_pass(output_code, keep_going)) {
      return err;
    }

    found_macro = found_macro or keep_going;
  }

  return {};
}

zs::error_result macro_parser::replace_pass(zs::string& output_code, bool& keep_going) {
  bool found_any = false;

  for (auto& m : _preprocessor->_macros) {
    if (auto err = look_and_replace_one_macro_call(m, output_code, found_any)) {
      return err;
    }
  }

  keep_going = found_any;

  return {};
}

zs::error_result macro_parser::look_and_replace_one_macro_call(
    zs::macro& m, zs::string& output_code, bool& found_one) {
  std::string_view mname = m.name.get_string_unchecked();

  zs::lexer lexer(_engine);
  _lexer = &lexer;
  _lexer->init(output_code);

  const char* begin_ptr = _lexer->stream().ptr();
  lex(true);

  while (!zb::is_one_of(_token, tok_eof, tok_lex_error)) {

    if (is(tok_dollar)) {
      lex();
    }

    if (is(tok_identifier)) {
      zs::object identifier_name = _lexer->get_identifier();
      if (identifier_name == mname) {

        //        int_t dist = std::distance((const char*)output_code.data(), begin_ptr);
        int_t dist
            = std::distance((const char*)output_code.data(), _lexer->stream().ptr() - mname.size() - 1);

        std::string_view in_code = std::string_view(output_code).substr(dist);

        zs::lexer macro_call_lexer(_engine);
        _lexer = &macro_call_lexer;
        _lexer->init(in_code);

        lex();

        zs::string output(_engine);
        const char* end_ptr = nullptr;
        if (auto err = parse_macro_call(output, end_ptr)) {
          return err;
        }

        size_t lindex = std::distance((const char*)output_code.data(), in_code.data());
        size_t rindex = std::distance((const char*)output_code.data(), end_ptr);
        //        zb::print(ZBASE_VNAME(lindex), ZBASE_VNAME(rindex));

        output_code.replace(output_code.begin() + lindex, output_code.begin() + rindex, output);

        //        _lexer = &lexer;

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

zs::error_result macro_parser::parse_macro_call(zs::string& output_code, const char*& end_ptr) {
  using enum token_type;
  using enum object_type;

  if (is_not(tok_dollar)) {
    return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_token, "macro tok_dolloar");
  }

  lex();

  if (is_not(tok_identifier)) {
    return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_token, "macro cannot have default parameters");
  }

  zs::object identifier = _lexer->get_identifier();

  size_t macro_index
      = _preprocessor->_macros.ifind_if([&](const zs::macro& m) { return m.name == identifier; });

  if (macro_index == zs::vector<macro>::npos) {
    return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_token, "macro doesn't exists");
  }

  lex();

  if (is_not(tok_lbracket)) {
    return ZS_PREPROCESSOR_ERROR(zs::error_code::invalid_token, "macro cannot have default parameters");
  }

  const char* param_begin = _lexer->stream().ptr();

  int_t brack_count = 0;
  int_t ctrlbrack_count = 0;
  int_t sqbrack_count = 0;
  zs::object params = zs::_a(_engine, 0);
  bool first = true;

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
      param_begin = _lexer->stream().ptr();
    }
  }

  std::string_view param(param_begin, std::distance(param_begin, params_last_end));

  param = zb::strip_all(param);

  if (!param.empty() and param[0] != ')') {
    params.as_array().push(zs::_s(_engine, param));
  }

  if (is_not(tok_rbracket)) {
    return ZS_PREPROCESSOR_ERROR(
        zs::error_code::invalid_parameter_type, "macro cannot have default parameters");
  }

  end_ptr = _lexer->stream().ptr();
  const zs::macro& m = _preprocessor->_macros[macro_index];

  zs::array_object& in_params = params.as_array();
  const zs::array_object& macro_params = m.params.as_array();

  if (macro_params.size() != in_params.size()) {

    if (in_params.size() > macro_params.size()) {
      return ZS_PREPROCESSOR_ERROR(
          zs::error_code::invalid_parameter_count, "too many parameters in macro call");
    }

    if (m.has_default_values() and in_params.size() >= m.first_default_value_index()) {
      for (size_t i = in_params.size(); i < macro_params.size(); i++) {
        in_params.push(m.default_values.as_array()[i]);
      }
    }
    else {
      return ZS_PREPROCESSOR_ERROR(
          zs::error_code::invalid_parameter_count, "not enough parameters in macro call");
    }
  }

  output_code = m.content.get_string_unchecked();

  if (auto err = replace_macro_content(output_code, m, in_params)) {
    return err;
  }

  return {};
}

zs::error_result macro_parser::replace_macro_content(
    zs::string& output_code, const macro& m, zs::array_object& in_params) {

  if (!m.params.is_array()) {
    output_code = m.content.get_string_unchecked();
  }

  zs::unordered_map<zs::object, size_t> mset((zs::unordered_map_allocator<zs::object, size_t>(_engine)));

  const zs::array_object& macro_params = m.params.as_array();
  for (size_t i = 0; i < macro_params.size(); i++) {
    mset.emplace(macro_params[i], i);
  }

  zs::lexer lexer(_engine);
  lexer.init(output_code);

  zs::lexer_ref lref;
  lref._lexer = &lexer;
  lref._token = tok_none;

  const char* identifier_begin = lexer.stream().ptr();
  token_type tok = lref.lex();

  while (!zb::is_one_of(tok, tok_eof, tok_lex_error)) {
    if (tok == tok_identifier) {
      const char* identifier_end = lexer.stream().ptr();

      std::string_view johnson(identifier_begin, std::distance(identifier_begin, identifier_end));
      zs::object identifier = lexer.get_identifier();

      if (auto it = mset.find(identifier); it != mset.end()) {

        std::string_view to = in_params[it->second].get_string_unchecked();

        size_t lindex = std::distance((const char*)output_code.data(), identifier_begin);
        size_t rindex = std::distance((const char*)output_code.data(), identifier_end);

        output_code.replace(output_code.begin() + lindex, output_code.begin() + rindex, to);

        lexer.stream() = zb::utf8_span_stream(output_code);
        lexer.stream()._it = lexer.stream().begin() + lindex + to.size();

        identifier_begin = lexer.stream().ptr();
        tok = lref.lex();
      }
      else {
        identifier_begin = lexer.stream().ptr();
        tok = lref.lex();
      }
    }
    else {
      identifier_begin = lexer.stream().ptr();
      tok = lref.lex();
    }
  }

  //  for (size_t i = 0; i < in_params.size(); i++) {
  //    std::string_view from = macro_params[i].get_string_unchecked();
  //    std::string_view to = in_params[i].get_string_unchecked();
  //    size_t start_pos = 0;
  //    while ((start_pos = output_code.find(from, start_pos)) != zs::string::npos) {
  //      output_code.replace(start_pos, from.length(), to);
  //      start_pos += to.length();
  //    }
  //  }

  return {};
}

} // namespace zs.
