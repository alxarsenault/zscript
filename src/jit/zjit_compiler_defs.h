/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "jit/zjit_compiler_include_guard.h"

#define ZS_JIT_COMPILER_PARSE_OP(name, ...) \
  template <>                               \
  zs::error_result jit_compiler::parse<zs::name>(__VA_ARGS__)

#define ZS_COMPILER_PARSE(exprname, ...) ZS_RETURN_IF_ERROR(parse<exprname>(__VA_ARGS__))

#define ZS_COMPILER_EXPECT(tok)                                         \
  if (!lex_if(tok)) {                                                   \
    return ZS_COMPILER_ERROR(zs::errc::invalid_token, "invalid token ", \
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",    \
        zb::quoted<"'">(zs::token_to_string(tok)));                     \
  }

#define ZS_COMPILER_ERROR_WITH_LINE_INFO(err, linfo, ...) \
  handle_error(err, linfo, ZS_DEVELOPER_SOURCE_LOCATION(), __VA_ARGS__)

#define ZS_COMPILER_ERROR(err, ...) \
  handle_error(err, get_line_info(), ZS_DEVELOPER_SOURCE_LOCATION(), __VA_ARGS__)

#define ZS_COMPILER_RETURN_IF_ERROR(X, ...)                                                  \
  if (zs::error_result err = X) {                                                            \
    return handle_error(err, get_line_info(), ZS_DEVELOPER_SOURCE_LOCATION(X), __VA_ARGS__); \
  }

#define ZS_COMPILER_EXPECT_GET(tok, ret)                                      \
  if (is_not(tok)) {                                                          \
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "invalid token ", \
        zb::quoted<"'">(zs::token_to_string(_token)), ", expected ",          \
        zb::quoted<"'">(zs::token_to_string(tok)));                           \
  }                                                                           \
  else {                                                                      \
    ret = _lexer->get_value();                                                \
    lex();                                                                    \
  }

namespace zs {

ZS_CK_INLINE static bool is_small_string_identifier(const object& identifier) noexcept {
  return identifier.get_string_unchecked().size() <= constants::k_small_string_max_size;
}

using objref_t = zb::ref_wrapper<object>;
using cobjref_t = zb::ref_wrapper<const object>;

#define REF(...) zb::wref(__VA_ARGS__)
#define CREF(...) zb::wcref(__VA_ARGS__)

enum class parse_op : uint8_t {

  //  p_global_function_statement,
  //  p_export_function_statement,

  p_arrow_lamda,
  p_function_call_args,
  p_member_function_call_args,
  p_function_call_args_template,

  //  p_export,
  //  p_export_table,

  p_decl_enum,
  p_enum_table,
  p_table,

  p_module_info,

  p_for,
  p_for_auto,
  p_for_each,

  // 2**n
  p_exponential,

  // '*', '/', '%'
  p_mult,

  // '+', '-'.
  p_plus,

  // '<<', '>>'.
  p_shift,

  // '>', '<', '>=', '<='.
  p_compare,

  // '==', '!=', '===', '<-->', '<==>'.
  p_eq_compare,

  // '&'.
  p_bitwise_and,

  // 'xor'.
  p_bitwise_xor,

  // '|'.
  p_bitwise_or,

  // '&&'.
  p_and,

  // '||'
  p_or,

  // '|||'
  p_triple_or,

  count
};

enum class module_info_type : uint8_t { module, author, brief, version, date, copyright };
enum class identifier_type : uint8_t { normal, exports };

inline constexpr const char* module_info_type_to_string(module_info_type mtype) {
  switch (mtype) {
  case module_info_type::module:
    return "@module";

  case module_info_type::author:
    return "@author";

  case module_info_type::brief:
    return "@brief";

  case module_info_type::version:
    return "@version";

  case module_info_type::date:
    return "@date";
  case module_info_type::copyright:
    return "@copyright";
  }

  return "unknown";
}
using enum parse_op;
using enum object_type;

namespace {
  static std::string_view get_line_content(const zb::utf8_span_stream& stream) {
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
    return line_content;
  }

  static std::string_view get_line_content(const zb::utf8_span_stream& stream, const zs::line_info& linfo) {

    const char* begin = &(*stream._data.begin());
    const char* end = &(*stream._data.end());

    size_t endl_count = 0;

    const char* line_it_begin = begin;
    const char* line_it_end = begin;

    bool found_line = false;
    for (; line_it_end < end; ++line_it_end) {
      if (*line_it_end == '\n') {
        if (++endl_count == linfo.line) {

          found_line = true;
          break;
        }

        line_it_begin = line_it_end;
      }
    }

    if (found_line) {
      std::string_view line_content(line_it_begin, std::distance(line_it_begin, line_it_end));
      return line_content;
    }

    return get_line_content(stream);
  }
} // namespace

//
// MARK: Parse forward declare.
//

ZS_JIT_COMPILER_PARSE_OP(p_table);

// Functions.
ZS_JIT_COMPILER_PARSE_OP(p_arrow_lamda);
ZS_JIT_COMPILER_PARSE_OP(p_function_call_args, bool rawcall, bool table_call);
ZS_JIT_COMPILER_PARSE_OP(p_member_function_call_args);
ZS_JIT_COMPILER_PARSE_OP(p_function_call_args_template, std::string_view meta_code);

// ZS_JIT_COMPILER_PARSE_OP(p_export_function_statement);
// ZS_JIT_COMPILER_PARSE_OP(p_global_function_statement);
ZS_JIT_COMPILER_PARSE_OP(p_module_info, module_info_type mtype);

// ZS_JIT_COMPILER_PARSE_OP(p_export);
// ZS_JIT_COMPILER_PARSE_OP(p_export_table);

} // namespace zs.
