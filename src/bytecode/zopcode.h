#pragma once

#include <zscript.h>

namespace zs {

struct small_string_instruction_data {
  uint64_t value_1;
  uint64_t value_2;

  inline static small_string_instruction_data create(std::string_view s) {
    zbase_assert(s.size() <= constants::k_small_string_max_size);
    small_string_instruction_data sdata = { 0, 0 };
    ::memcpy(&sdata, s.data(), s.size());
    return sdata;
  }

  inline static small_string_instruction_data create(const zs::object& obj) {
    return create(obj.get_string_unchecked());
  }

  ZS_CK_INLINE std::string_view get_string_view() const noexcept {
    return std::string_view((const char*)this);
  }

  ZS_CK_INLINE zs::object get_small_string() const noexcept { return zs::_ss(get_string_view()); }

  inline friend std::ostream& operator<<(std::ostream& stream, const small_string_instruction_data& sdata) {
    return stream << "\"" << (const char*)&sdata << "\"";
  }
};

static_assert(
    std::is_trivial_v<small_string_instruction_data>, "small_string_instruction_data must remain trivial");

using ss_inst_data = small_string_instruction_data;

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

#define ZS_OPCODE_PREFIXED(name) op_##name
#define ZS_OPCODE_ENUM_VALUE(name) zs::opcode::ZS_OPCODE_PREFIXED(name)

enum class opcode : uint8_t {
#define ZS_DECL_OPCODE(name, INST_TYPES) ZS_OPCODE_PREFIXED(name),
#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE
  count
};

inline constexpr const char* opcode_to_string(opcode op) noexcept {
  switch (op) {
#define ZS_DECL_OPCODE(name, INST_TYPES) \
  case ZS_OPCODE_ENUM_VALUE(name):       \
    return #name;
#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE

  case opcode::count:
    return "count";
  }

  return "unknown";
}

} // namespace zs.
