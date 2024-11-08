#pragma once

#include <zscript.h>

namespace zs {

inline constexpr size_t k_max_func_stack_size = 0xFF;
inline constexpr size_t k_max_literals = 0x7FFFFFFF;

enum class arithmetic_op : uint8_t {
  add,
  sub,
  mul,
  div,
  mod,
  exp,
  lshift,
  rshift,
  bitwise_or,
  bitwise_and,
  bitwise_xor,

};

enum class arithmetic_uop : uint8_t { incr, decr, pincr, pdecr };

enum class compare_op : uint8_t { //
  lt,
  le,
  gt,
  ge,
  tw,
  double_arrow,
  double_arrow_eq
};

#define ZS_OPCODE_ENUM_VALUE(name) zs::opcode::op_##name

enum class opcode : uint8_t {
#define ZS_DECL_OPCODE(name) op_##name,
#include "lang/zopcode_def.h"
#undef ZS_DECL_OPCODE
  count
};

inline constexpr const char* opcode_to_string(opcode op) noexcept {
  switch (op) {
#define ZS_DECL_OPCODE(name)       \
  case ZS_OPCODE_ENUM_VALUE(name): \
    return #name;
#include "lang/zopcode_def.h"
#undef ZS_DECL_OPCODE

  case opcode::count:
    return "count";
  }

  return "unknown";
}

} // namespace zs.
