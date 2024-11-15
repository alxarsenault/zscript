#pragma once

#include <zscript/common.h>

namespace zs {
class engine;

class engine_holder {
public:
  ZB_INLINE_CXPR engine_holder(zs::engine* eng) noexcept
      : _engine(eng) {}

  ZB_CK_INLINE_CXPR zs::engine* get_engine() const noexcept { return _engine; }

protected:
  zs::engine* _engine;
};

} // namespace zs.
