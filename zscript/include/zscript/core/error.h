#pragma once

#include <zscript/core/common.h>
#include <zbase/sys/error_code.h>

// Error codes.
#define ZS_ERROR_CODE_ENUM(X)                                       \
  X(success, "success")                                             \
  X(returned, "returned")                                           \
  X(unknown, "unknown")                                             \
  X(invalid, "invalid")                                             \
  X(invalid_size, "invalid_size")                                   \
  X(invalid_type, "invalid_type")                                   \
  X(invalid_operation, "invalid_operation")                         \
  X(invalid_token, "invalid_token")                                 \
  X(invalid_comma, "invalid_comma")                                 \
  X(invalid_directory, "invalid_directory")                         \
  X(invalid_value_type_assignment, "invalid_value_type_assignment") \
  X(invalid_include_syntax, "invalid_include_syntax")               \
  X(invalid_include_file, "invalid_include_file")                   \
  X(invalid_argument, "invalid_argument")                           \
  X(invalid_parameter_count, "invalid_parameter_count")             \
  X(invalid_parameter_type, "invalid_parameter_type")               \
  X(invalid_native_function_call, "invalid_native_function_call")   \
  X(invalid_name, "invalid_name")                                   \
  X(invalid_include_directory, "invalid_include_directory")         \
  X(invalid_native_array_type, "invalid_native_array_type")         \
  X(already_exists, "already_exists")                               \
  X(conversion_error, "conversion_error")                           \
  X(open_file_error, "open_file_error")                             \
  X(memory_error, "memory_error")                                   \
  X(null_type, "null_type")                                         \
  X(out_of_bounds, "out_of_bounds")                                 \
  X(out_of_memory, "out_of_memory")                                 \
  X(inaccessible, "inaccessible")                                   \
  X(unimplemented, "unimplemented")                                 \
  X(not_a_table, "not_a_table")                                     \
  X(not_a_for_colon, "not_a_for_colon")                             \
  X(identifier_expected, "identifier_expected")                     \
  X(too_many_locals, "too many locals")                             \
  X(duplicated_module, "duplicated_module")                         \
  X(stack_error, "stack_error")                                     \
  X(not_found, "not_found")                                         \
  /* Module */                                                      \
  X(cant_modify_export_table, "cant_modify_export_table")           \
  /* Struct */                                                      \
  X(cant_modify_const_member, "cant_modify_const_member")           \
  X(cant_modify_static_const, "cant_modify_static_const")

#define ZS_RETURN_IF_ERROR(X)     \
  if (zs::error_result err = X) { \
    return err;                   \
  }

namespace zs {
enum class error_code : int32_t {
#define _X(name, str) name,
  ZS_ERROR_CODE_ENUM(_X)
#undef _X
};

ZB_CK_INLINE_CXPR const char* error_code_to_string(error_code ec) noexcept {
  switch (ec) {
#define _X(name, str)    \
  case error_code::name: \
    return str;
    ZS_ERROR_CODE_ENUM(_X)
#undef _X
  }

  return "unknown";
}

namespace detail {
  struct error_result_descriptor {
    using enum_type = error_code;
    static constexpr enum_type default_value = enum_type::success;

    ZB_CK_INLINE_CXPR static bool is_valid(enum_type v) noexcept {
      return v == enum_type::success || v == enum_type::returned;
    }

    ZB_CK_INLINE_CXPR static const char* to_string(enum_type code) noexcept {
      return zs::error_code_to_string(code);
    }
  };
} // namespace detail.

using error_result = zb::generic_error_result<detail::error_result_descriptor>;
using status_result = zb::generic_status_result<detail::error_result_descriptor>;

template <class T>
using optional_result = zb::generic_optional_result<detail::error_result_descriptor, T>;

//
// MARK: - Exceptions
//

struct exception : std::exception {
  virtual ~exception() override = default;
};

enum class error_type { logic, object, lexer, compiler };

ZBASE_ATTRIBUTE_NO_RETURN void throw_exception(zs::error_code ec);
ZBASE_ATTRIBUTE_NO_RETURN void throw_exception(zs::error_code ec, std::string_view msg);
ZBASE_ATTRIBUTE_NO_RETURN void throw_exception(zs::error_type e, std::string_view msg);
} // namespace zs.
