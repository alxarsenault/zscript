#pragma once

#include <zscript/common.h>
#include <ostream>

namespace zs {

/// Version struct.
struct version_t {
  uint8_t major, minor, patch, build;

  inline friend std::ostream& operator<<(std::ostream& stream, const version_t& v) {
    return stream << "zscript " << (int)v.major << "." << (int)v.minor << "." << (int)v.patch;
  }
};

inline constexpr version_t k_version
    = version_t{ ZS_VERSION_MAJOR, ZS_VERSION_MINOR, ZS_VERSION_PATCH, ZS_VERSION_BUILD };

ZB_CK_INLINE_CXPR version_t version() noexcept {
  return version_t{ ZS_VERSION_MAJOR, ZS_VERSION_MINOR, ZS_VERSION_PATCH, ZS_VERSION_BUILD };
}
} // namespace zs.
