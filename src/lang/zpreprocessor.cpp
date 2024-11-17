#include "lang/zpreprocessor.h"
#include "lang/preprocessor/zmacro_parser.h"
#include "lang/preprocessor/zinclude_parser.h"
#include "lang/preprocessor/zcounter_parser.h"
#include "lang/preprocessor/zuuid_parser.h"
#include "lang/preprocessor/zstringify_parser.h"
#include "lang/preprocessor/zmacro_def_parser.h"

namespace zs {

preprocessor::preprocessor(zs::engine* eng)
    : engine_holder(eng)
    , _macros(zs::allocator<zs::macro>(eng))
    , _imported_files_set(zs::allocator<zs::object>(eng))
    , _error_message(eng) {

  _uuid_map = zs::_t(_engine);
}

zs::error_result preprocessor::handle_error(
    const zs::lexer_ref& lx, zs::error_code ec, std::string_view msg, const zb::source_location& loc) {
  zs::line_info linfo = lx._lexer->get_last_line_info();

  const auto& stream = lx._lexer->stream();
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

zs::error_code preprocessor::expect(
    zs::lexer_ref& lx, token_type tok, const zb::source_location& loc) noexcept {
  return expect(lx, tok, zs::error_code::invalid_token, loc);
}

zs::error_code preprocessor::expect(
    zs::lexer_ref& lx, token_type tok, zs::error_code err, const zb::source_location& loc) noexcept {
  if (lx.is_not(tok)) {

    return handle_error(lx, err,
        zs::strprint(_engine, "invalid token ", zb::quoted<"'">(zs::token_to_string(lx._token)),
            ", expected ", zb::quoted<"'">(zs::token_to_string(tok))),
        loc);
  }

  lx.lex();
  return zs::error_code::success;
}

zs::error_result preprocessor::preprocess(
    std::string_view content, std::string_view filename, object& output, zs::virtual_machine* vm) {

  zs::string input_code(content, _engine);
  zs::string output_code(content, _engine);

  bool found_macro_def = false;
  bool found_macro = false;
  bool found_include = false;

  //
  if (auto err = parse<zs::macro_def_parser>(input_code, output_code, found_macro_def)) {
    return err;
  }

  if (auto err = parse<zs::include_parser>(input_code, output_code, found_include)) {
    return err;
  }

  if (auto err = parse<zs::macro_parser>(input_code, output_code, found_macro)) {
    return err;
  }

  output = zs::_s(_engine, output_code);

  //  zs::macro_def_parser mdefparser(this);
  //  if (auto err = mdefparser.parse(scode, filename, output, found_macro)) {
  //    _error_message = mdefparser.get_error();
  //    return err;
  //  }

  //
  //  do {
  //
  //    found_macro = false;
  //    did_include = false;
  //    bool dummy = false;
  //    {
  //      zs::counter_parser cparser(this);
  //      if (auto err = cparser.parse(scode, filename, output, dummy)) {
  //        _error_message = cparser.get_error();
  //        return err;
  //      }
  //
  //      scode = output.get_string_unchecked();
  //    }
  //
  //    {
  //      zs::uuid_parser uparser(this);
  //      if (auto err = uparser.parse(scode, filename, output, dummy)) {
  //        _error_message = uparser.get_error();
  //        return err;
  //      }
  //
  //      scode = output.get_string_unchecked();
  //    }
  //
  //    {
  //      zs::stringify_parser sparser(this);
  //      if (auto err = sparser.parse(scode, filename, output, dummy)) {
  //        _error_message = sparser.get_error();
  //        return err;
  //      }
  //
  //      scode = output.get_string_unchecked();
  //    }
  //
  //    zs::macro_parser mparser(this);
  //    if (auto err = mparser.parse(scode, filename, output, found_macro)) {
  //      _error_message = mparser.get_error();
  //      return err;
  //    }
  //
  //    zs::include_parser iparser(this);
  //    scode = output.get_string_unchecked();
  //
  //    if (auto err = iparser.parse(scode, filename, output, did_include)) {
  //      _error_message = iparser.get_error();
  //      return err;
  //    }
  //    scode = output.get_string_unchecked();
  //
  //    {
  //      zs::counter_parser cparser(this);
  //      if (auto err = cparser.parse(scode, filename, output, dummy)) {
  //        _error_message = cparser.get_error();
  //        return err;
  //      }
  //      scode = output.get_string_unchecked();
  //    }
  //    {
  //      zs::uuid_parser uparser(this);
  //      if (auto err = uparser.parse(scode, filename, output, dummy)) {
  //        _error_message = uparser.get_error();
  //        return err;
  //      }
  //
  //      scode = output.get_string_unchecked();
  //    }
  //
  //  } while (found_macro or did_include);

  return {};
}

} // namespace zs.
