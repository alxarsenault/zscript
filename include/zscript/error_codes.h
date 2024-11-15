#pragma once

#include <zscript/common.h>

namespace zs {

// clang-format off
enum class error_code : int32_t {
  #define ZS_DECL_ERROR_CODE(name, str) name,
  #include <zscript/error_codes_def.h>
  #undef ZS_DECL_ERROR_CODE
};

ZB_CK_INLINE_CXPR const char* error_code_to_string(error_code ec) noexcept {
  switch (ec) {
  #define ZS_DECL_ERROR_CODE(name, str) case error_code::name: return str;
  #include <zscript/error_codes_def.h>
  #undef ZS_DECL_ERROR_CODE
  }

  return "unknown";
}
// clang-format on
} // namespace zs.
