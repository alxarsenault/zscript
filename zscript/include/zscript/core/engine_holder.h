#pragma once

#include <zscript/core/common.h>
#include <zscript/core/types.h>

namespace zs {
class engine_holder {
public:
  ZB_INLINE_CXPR engine_holder(zs::engine* eng) noexcept
      : _engine(eng) {}

  ZB_CK_INLINE_CXPR zs::engine* get_engine() const noexcept { return _engine; }

protected:
  zs::engine* _engine;
};

static_assert(std::is_trivially_copyable_v<engine_holder>, " AKSJAJKS");
static_assert(std::is_trivially_destructible_v<engine_holder>, " AKSJAJKS");

} // namespace zs.
