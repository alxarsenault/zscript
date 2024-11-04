#pragma once

#include <zscript/common.h>
#include <zbase/sys/error_code.h>

#if ZS_CONFIG_USE_EXCEPTION
#include <exception>
#endif // ZS_CONFIG_USE_EXCEPTION.

#define ZS_RETURN_IF_ERROR_1(X)   \
  if (zs::error_result err = X) { \
    return err;                   \
  }

#define ZS_RETURN_IF_ERROR_2(X, R) \
  if (zs::error_result err = X) {  \
    return R;                      \
  }

#define ZS_RETURN_IF_ERROR_3(X, F, R) \
  if (zs::error_result err = X) {     \
    F;                                \
    return R;                         \
  }

#define ZS_RETURN_IF_ERROR_(N, ...) ZBASE_CONCAT(ZS_RETURN_IF_ERROR_, N)(__VA_ARGS__)

///
#define ZS_RETURN_IF_ERROR(...) ZS_RETURN_IF_ERROR_(ZBASE_NARG(__VA_ARGS__), __VA_ARGS__)

namespace zs {

// clang-format off
enum class error_code : int32_t {
  #define ZS_DECL_ERROR_CODE(name, str) name,
  #include <zscript/error_codes_def.h>
  #undef ZS_DECL_ERROR_CODE
};

using errc = error_code;

ZS_CK_INLINE_CXPR const char* error_code_to_string(error_code ec) noexcept {
  switch (ec) {
  #define ZS_DECL_ERROR_CODE(name, str) case error_code::name: return str;
  #include <zscript/error_codes_def.h>
  #undef ZS_DECL_ERROR_CODE
  }

  return "unknown";
}
// clang-format on

ZBASE_ATTRIBUTE_NO_RETURN void throw_error(zs::error_code ec);

namespace detail {
  struct error_result_descriptor {
    using enum_type = error_code;
    static constexpr enum_type default_value = enum_type::success;

    ZS_CK_INLINE_CXPR static bool is_valid(enum_type v) noexcept {
      return v == enum_type::success || v == enum_type::returned;
    }

    ZS_CK_INLINE_CXPR static const char* to_string(enum_type code) noexcept {
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

#if ZS_CONFIG_USE_EXCEPTION
struct exception : std::exception {
  inline exception(error_code error) noexcept
      : _error{ error } {}

  inline exception(const exception&) noexcept = default;

  virtual ~exception() override = default;

  virtual const char* what() const noexcept override { return zs::error_result(_error).message(); }

  ZS_CK_INLINE_CXPR error_code error() const noexcept { return _error; }

private:
  error_code _error;
};
#endif // ZS_CONFIG_USE_EXCEPTION.

} // namespace zs.
