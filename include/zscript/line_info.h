#pragma once

#include <zscript/types.h>
#include <ostream>

namespace zs {
/// @struct line_info.
struct line_info {
  line_info() = default;
  inline line_info(int_t l, int_t col)
      : line(l)
      , column(col) {}

  int_t line = 0;
  int_t column = 0;

  inline friend std::ostream& operator<<(std::ostream& stream, const line_info& linfo) {
    return stream << "at line " << linfo.line << " column " << linfo.column;
  }
};

/// @struct statement_info.
struct statement_info {
  inline statement_info(std::string_view lcontent, int_t l, int_t col)
      : content(lcontent)
      , loc(l, col) {}

  inline statement_info(std::string_view lcontent, const zs::line_info& linfo)
      : content(lcontent)
      , loc(linfo) {}
  inline statement_info(const zs::line_info& linfo)

      : loc(linfo) {}

  std::string_view content;
  zs::line_info loc;
};
} // namespace zs.
