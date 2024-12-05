/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "lang/jit/zjit_compiler_include_guard.h"

#define ZS_JIT_COMPILER_PARSE_OP(name, ...) \
  template <>                               \
  zs::error_result jit_compiler::parse<zs::name>(__VA_ARGS__)

#define ZS_COMPILER_PARSE(exprname, ...) ZS_RETURN_IF_ERROR(parse<exprname>(__VA_ARGS__))

//#define ZS_COMPILER_EXPECT(...) \
//  ZBASE_DEFER(ZBASE_CONCAT(__ZS_COMPILER_EXPECT_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)

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

// #define __ZS_COMPILER_ERROR_1(err, msg) handle_error(err, msg, ZB_CURRENT_SOURCE_LOCATION())
//
////#define __ZS_COMPILER_ERROR_MULTIPLE(err, ...) \
////  handle_error(err, zs::strprint(_engine, __VA_ARGS__), ZB_CURRENT_SOURCE_LOCATION())
//
//
// #define __ZS_COMPILER_ERROR_MULTIPLE(err, ...) \
//  handle_error(err,  get_line_info(), ZB_CURRENT_SOURCE_LOCATION(), __VA_ARGS__)

//#define __ZS_COMPILER_ERROR_WITH_LINE_INFO_1(err, linfo, msg) \
//  handle_error(err, linfo, msg, ZB_CURRENT_SOURCE_LOCATION())
//
// #define __ZS_COMPILER_ERROR_WITH_LINE_INFO_MULTIPLE(err, linfo, ...) \
//  handle_error(err, linfo, zs::sstrprint(_engine, __VA_ARGS__), ZB_CURRENT_SOURCE_LOCATION())

//#define ZS_COMPILER_EXPECT_ERROR_MESSAGE_WITH_CODE(tok, ec)                                            \
//  ZS_COMPILER_ERROR(ec, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)), ", expected ", \
//      zb::quoted<"'">(zs::token_to_string(tok)))

//#define ZS_COMPILER_EXPECT_ERROR_MESSAGE(tok) \
//  ZS_COMPILER_EXPECT_ERROR_MESSAGE_WITH_CODE(tok, zs::error_code::invalid_token)

// #define __ZS_COMPILER_EXPECT_1(tok) __ZS_COMPILER_EXPECT_2(tok, zs::error_code::invalid_token)
//
// #d ef ine __ZS _COMPILER_EXPECT_2(tok, ec)                                                        \
//  if (!lex_if(tok)) {                                                                          \
//    return ZS_COMPILER_ERROR(ec,   "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)), \
//            ", expected ", zb::quoted<"'">(zs::token_to_string(tok))); \
  //}

//#define __ZS_COMPILER_EXPECT_2(tok, ec)                                                        \
//  if (!lex_if(tok)) {                                                                          \
//    return handle_error(ec,                                                                    \
//        zs::sstrprint(_engine, "invalid token ", zb::quoted<"'">(zs::token_to_string(_token)), \
//            ", expected ", zb::quoted<"'">(zs::token_to_string(tok))),                         \
//        ZB_CURRENT_SOURCE_LOCATION());                                                         \
//  }

//#define ZS_COMPILER_ERROR_WITH_LINE_INFO(err, linfo, ...)                                                    \
//  ZBASE_DEFER(ZBASE_CONCAT(__ZS_COMPILER_ERROR_WITH_LINE_INFO_, ZBASE_NARG_BINARY(__VA_ARGS__)), err, linfo, \
//      __VA_ARGS__)

namespace zs {

ZS_CK_INLINE static bool is_small_string_identifier(const object& identifier) noexcept {
  return identifier.get_string_unchecked().size() <= constants::k_small_string_max_size;
}

using objref_t = zb::ref_wrapper<object>;
using cobjref_t = zb::ref_wrapper<const object>;

#define REF(...) zb::wref(__VA_ARGS__)
#define CREF(...) zb::wcref(__VA_ARGS__)
enum class parse_op : uint8_t {

  //
  //  p_preprocessor,
  p_statement,
  p_expression,
  p_assign,
  p_replace_assign,
  p_compile_time_mask,

  p_function_statement,
  p_global_function_statement,
  p_export_function_statement,
  p_function,
  p_lamda,
  p_arrow_lamda,
  p_function_call_args,
  p_function_call_args_template,
  p_function_parameters,
  p_create_function,
  p_create_normal_function,
  p_comma,
  p_semi_colon,

  p_decl_var,
  //  p_decl_var_internal,
  //  p_decl_var_internal_2,
  //  p_decl_var_internal_3,

  p_use,
  p_include_or_import_statement,
  p_export,
  p_export_table,

  p_decl_enum,
  p_enum_table,
  p_var_decl_prefix,
  p_variable,
  p_variable_type_restriction,
  p_table,
  p_table_or_class,

  p_module_info,

  p_class_statement,
  p_class,

  p_struct,
  p_struct_constructor,
  p_struct_member,
  p_struct_method,
  p_struct_statement,

  p_if,
  p_if_block,
  p_factor,
  p_for,
  p_for_auto,
  p_for_each,
  p_factor_identifier,
  p_factor_at,
  p_bind_env,
  p_define,

  p_as_table,
  p_load_json_file,
  p_as_string,
  p_as_value,

  p_macro,
  p_macro_call,

  // '.', '[', '++', '--', '('.
  p_prefixed,
  p_prefixed_incr,
  p_prefixed_lbracket,
  p_prefixed_lbracket_template,

  // 2^n
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

ZS_JIT_COMPILER_PARSE_OP(p_statement, bool close_frame);
ZS_JIT_COMPILER_PARSE_OP(p_include_or_import_statement, token_type tok);
ZS_JIT_COMPILER_PARSE_OP(p_table_or_class, token_type separator, token_type terminator);
ZS_JIT_COMPILER_PARSE_OP(p_table);
ZS_JIT_COMPILER_PARSE_OP(p_semi_colon);
ZS_JIT_COMPILER_PARSE_OP(p_prefixed);
ZS_JIT_COMPILER_PARSE_OP(p_macro, token_type);
ZS_JIT_COMPILER_PARSE_OP(p_macro_call);
// ZS_JIT_COMPILER_PARSE_OP(p_decl_var_internal_2, bool is_export, bool is_const);
ZS_JIT_COMPILER_PARSE_OP(p_decl_var);
ZS_JIT_COMPILER_PARSE_OP(p_struct);

ZS_JIT_COMPILER_PARSE_OP(p_struct_statement);
ZS_JIT_COMPILER_PARSE_OP(p_struct_constructor, struct_parser* sparser);
ZS_JIT_COMPILER_PARSE_OP(p_struct_member, struct_parser* sparser, zs::var_decl_flags_t vdcl_flags);
ZS_JIT_COMPILER_PARSE_OP(p_struct_method, struct_parser* sparser, zs::var_decl_flags_t vdcl_flags);

ZS_JIT_COMPILER_PARSE_OP(p_class_statement);
ZS_JIT_COMPILER_PARSE_OP(p_variable_type_restriction, zb::ref_wrapper<uint32_t>, zb::ref_wrapper<uint64_t>);
ZS_JIT_COMPILER_PARSE_OP(
    p_variable, uint32_t* obj_type_mask, uint64_t* custom_mask, bool* is_static, bool* is_const);

ZS_JIT_COMPILER_PARSE_OP(p_var_decl_prefix, zs::var_decl_flags_t* flags);

ZS_JIT_COMPILER_PARSE_OP(p_expression);
ZS_JIT_COMPILER_PARSE_OP(p_assign, expr_state estate);

ZS_JIT_COMPILER_PARSE_OP(p_bind_env, zb::ref_wrapper<int_t> target);
ZS_JIT_COMPILER_PARSE_OP(p_factor, object* name);
ZS_JIT_COMPILER_PARSE_OP(p_factor_identifier, identifier_type itype);
ZS_JIT_COMPILER_PARSE_OP(p_factor_at);
ZS_JIT_COMPILER_PARSE_OP(p_as_table);
ZS_JIT_COMPILER_PARSE_OP(p_as_string);
ZS_JIT_COMPILER_PARSE_OP(p_as_value);
ZS_JIT_COMPILER_PARSE_OP(p_load_json_file);
ZS_JIT_COMPILER_PARSE_OP(p_if_block);
ZS_JIT_COMPILER_PARSE_OP(p_if);

// Functions.
ZS_JIT_COMPILER_PARSE_OP(p_function_statement);
ZS_JIT_COMPILER_PARSE_OP(p_function, bool lambda);
ZS_JIT_COMPILER_PARSE_OP(p_arrow_lamda);
ZS_JIT_COMPILER_PARSE_OP(p_function_call_args, bool rawcall);
ZS_JIT_COMPILER_PARSE_OP(p_function_call_args_template, std::string_view meta_code);
ZS_JIT_COMPILER_PARSE_OP(
    p_create_function, zb::ref_wrapper<const object> name, int_t boundtarget, bool lambda);

ZS_JIT_COMPILER_PARSE_OP(p_create_normal_function, cobjref_t name);

ZS_JIT_COMPILER_PARSE_OP(p_function_parameters, zs::closure_compile_state* fct_state, int_t* def_params);
ZS_JIT_COMPILER_PARSE_OP(p_export_function_statement);
ZS_JIT_COMPILER_PARSE_OP(p_global_function_statement);
ZS_JIT_COMPILER_PARSE_OP(p_module_info, module_info_type mtype);

ZS_JIT_COMPILER_PARSE_OP(p_export);
ZS_JIT_COMPILER_PARSE_OP(p_export_table);
ZS_JIT_COMPILER_PARSE_OP(p_use);
ZS_JIT_COMPILER_PARSE_OP(
    p_compile_time_mask, zs::opcode last_op, uint32_t mask, uint64_t custom_mask, bool* procesed);

ZS_JIT_COMPILER_PARSE_OP(p_replace_assign, int_t dst, bool* did_replace);
} // namespace zs.
