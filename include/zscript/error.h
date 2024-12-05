#pragma once

#include <zscript/common.h>
#include <zscript/error_codes.h>
#include <zbase/sys/error_code.h>

#if ZS_CONFIG_USE_EXCEPTION
#include <exception>
#endif // ZS_CONFIG_USE_EXCEPTION.

#define ZS_RETURN_IF_ERROR(X)     \
  if (zs::error_result err = X) { \
    return err;                   \
  }

namespace zs {

ZBASE_ATTRIBUTE_NO_RETURN void throw_error(zs::error_code ec);

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

#if ZS_CONFIG_USE_EXCEPTION
struct exception : std::exception {
  inline exception(error_code error) noexcept
      : _error{ error } {}

  inline exception(const exception&) noexcept = default;

  virtual ~exception() override = default;

  virtual const char* what() const noexcept override { return zs::error_result(_error).message(); }

  ZB_CK_INLINE_CXPR error_code error() const noexcept { return _error; }

private:
  error_code _error;
};
#endif // ZS_CONFIG_USE_EXCEPTION.

} // namespace zs.
