/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "lang/jit/zjit_compiler_include_guard.h"

namespace zs {

ZS_JIT_COMPILER_PARSE_OP(p_module_info, module_info_type mtype) {

  if (mtype == module_info_type::module) {
    if (!_ccs->is_top_level() or _is_header < 2) {
      return ZS_COMPILER_ERROR(zs::errc::invalid_module_tag,
          "The @module statement can only happen once in the top level scope, before any other "
          "statements.\n");
    }

    if (_ccs->is_module()) {
      return ZS_COMPILER_ERROR(zs::errc::duplicated_module_tag, "Duplicated @module tag.\n");
    }

    _ccs->_sdata._is_module = true;
    _ccs->_sdata._module_info = zs::_t(_engine);
  }

  if (!_ccs->is_top_level() or !_ccs->is_module() or _is_header < 2) {
    return ZS_COMPILER_ERROR(zs::errc::invalid_module_info_tag, "The ", module_info_type_to_string(mtype),
        " statement can only happen in the top level scope after a @module statement.\n");
  }

  const char* begin_ptr = stream_ptr() + 1;
  lex(true);

  if (is(tok_endl)) {
    return ZS_COMPILER_ERROR(zs::errc::invalid_module_info_tag, "No end line is allowed be a ",
        module_info_type_to_string(mtype), " statement value.\n");
  }

  object content;

  bool kkk = zb::is_one_of(_lexer->peek(true), tok_endl, tok_semi_colon);
  if (is(tok_string_value) and kkk) {
    content = zs::_s(_engine, zb::strip_all(_lexer->get_string_value()));
    lex();
  }
  else if (is(tok_escaped_string_value) and kkk) {
    content = zs::_s(_engine, zb::strip_all(_lexer->get_escaped_string_value()));
    lex();
  }
  else {
    while (is_not(tok_endl, tok_semi_colon, tok_eof)) {
      lex(true);
    }

    if (is(tok_endl, tok_semi_colon)) {
      content = zs::_s(_engine, zb::strip_all(std::string_view(begin_ptr, stream_ptr() - 1)));

      lex();
    }

    else {
      return ZS_COMPILER_ERROR(zs::errc::invalid_module_info_tag, "The ", module_info_type_to_string(mtype),
          " statement can only happen once in the top level scope after a @module statement.\n");
    }
  }

  if (content.get_string_unchecked().empty()) {
    return ZS_COMPILER_ERROR(zs::errc::invalid_module_info_tag, "The ", module_info_type_to_string(mtype),
        " statement requires a value.\n");
  }

  while (lex_if(tok_semi_colon)) {
  }

  switch (mtype) {

  case module_info_type::module:
    ZS_COMPILER_RETURN_IF_ERROR(_ccs->_sdata.add_module_name(content), "The ",
        module_info_type_to_string(mtype), " statement failed.\n");
    return _ccs->create_export_table();

  case module_info_type::author:
    ZS_COMPILER_RETURN_IF_ERROR(_ccs->_sdata.add_module_author(content), "The ",
        module_info_type_to_string(mtype), " statement failed.\n");
    return {};

  case module_info_type::brief:
    ZS_COMPILER_RETURN_IF_ERROR(_ccs->_sdata.add_module_brief(content), "The ",
        module_info_type_to_string(mtype), " statement can only happen once.\n");
    return {};

  case module_info_type::copyright:
    ZS_COMPILER_RETURN_IF_ERROR(_ccs->_sdata.add_module_copyright(content), "The ",
        module_info_type_to_string(mtype), " statement can only happen once.\n");
    return {};

  case module_info_type::date:
    ZS_COMPILER_RETURN_IF_ERROR(_ccs->_sdata.add_module_date(content), "The ",
        module_info_type_to_string(mtype), " statement can only happen once.\n");
    return {};

  case module_info_type::version:
    ZS_COMPILER_RETURN_IF_ERROR(_ccs->_sdata.add_module_version(content), "The ",
        module_info_type_to_string(mtype), " statement can only happen once.\n");
    return {};

  default:
    break;
  }

  return {};
}
} // namespace zs.
