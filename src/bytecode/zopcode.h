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

  inline static small_string_instruction_data create(const char* s) { return create(std::string_view(s)); }

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

enum class get_op_flags_t : uint8_t {
  gf_none = 0,
  gf_look_in_root = 1,
  gf_will_call = 2,
};

ZBASE_ENUM_CLASS_FLAGS(get_op_flags_t);

inline constexpr get_op_flags_t make_get_op_flags(bool look_in_root, bool will_call = false) noexcept {

  get_op_flags_t flg = get_op_flags_t::gf_none;

  if (look_in_root) {
    flg |= get_op_flags_t::gf_look_in_root;
  }

  if (will_call) {
    flg |= get_op_flags_t::gf_will_call;
  }
  return flg;
}

enum class arithmetic_op : uint8_t {
  aop_add,
  aop_sub,
  aop_mul,
  aop_div,
  aop_mod,
  aop_exp,

  aop_lshift,
  aop_rshift,

  //  aop_add_eq,
  //  aop_sub_eq,
  //  aop_mul_eq,
  //  aop_div_eq,

  aop_bitwise_or,
  aop_bitwise_and,
  aop_bitwise_xor,
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

inline constexpr bool arithmetic_op_has_meta_method(arithmetic_op op) noexcept {
  return op < arithmetic_op::aop_bitwise_or;
}

inline constexpr bool arithmetic_op_has_rhs_meta_method(arithmetic_op op) noexcept {
  return op <= arithmetic_op::aop_exp;
}

inline constexpr meta_method arithmetic_op_to_meta_method(arithmetic_op op) noexcept {
  using enum meta_method;
  using enum arithmetic_op;

  ZS_ASSERT(arithmetic_op_has_meta_method(op));

  switch (op) {
  case aop_add:
    return mt_add;
  case aop_sub:
    return mt_sub;
  case aop_mul:
    return mt_mul;
  case aop_div:
    return mt_div;
  case aop_mod:
    return mt_mod;
  case aop_exp:
    return mt_exp;
  case aop_lshift:
    return mt_lshift;
  case aop_rshift:
    return mt_rshift;
    //    case aop_add_eq:
    //      return mt_add_eq;
    //    case aop_sub_eq:
    //      return mt_sub_eq;
    //    case aop_mul_eq:
    //      return mt_mul_eq;
    //    case aop_div_eq:
    //      return mt_div_eq;

  default:
    return mt_none;
  }

  return mt_none;
}

inline constexpr meta_method arithmetic_op_to_meta_eq_method(arithmetic_op op) noexcept {
  using enum meta_method;
  using enum arithmetic_op;

  ZS_ASSERT(arithmetic_op_has_meta_method(op));

  switch (op) {
  case aop_add:
    return mt_add_eq;
  case aop_sub:
    return mt_sub_eq;
  case aop_mul:
    return mt_mul_eq;
  case aop_div:
    return mt_div_eq;
  case aop_mod:
    return mt_mod_eq;
  case aop_exp:
    return mt_exp_eq;
  case aop_lshift:
    return mt_lshift_eq;
  case aop_rshift:
    return mt_rshift_eq;
  default:
    return mt_none;
  }

  return mt_none;
}

inline constexpr meta_method arithmetic_op_to_rhs_meta_method(arithmetic_op op) noexcept {
  using enum meta_method;
  using enum arithmetic_op;

  ZS_ASSERT(arithmetic_op_has_rhs_meta_method(op));

  switch (op) {
  case aop_add:
    return mt_rhs_add;
  case aop_sub:
    return mt_rhs_sub;
  case aop_mul:
    return mt_rhs_mul;
  case aop_div:
    return mt_rhs_div;
  case aop_mod:
    return mt_rhs_mod;
  case aop_exp:
    return mt_rhs_exp;

  default:
    return mt_none;
  }

  return mt_none;
}

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

inline constexpr bool is_arithmetic_op(opcode op) noexcept {
  using enum opcode;

  switch (op) {
  case op_add:
  case op_sub:
  case op_mul:
  case op_div:
  case op_mod:
  case op_exp:
  case op_bitwise_or:
  case op_bitwise_and:
  case op_bitwise_xor:
  case op_lshift:
  case op_rshift:
    return true;

  default:
    return false;
    return false;
  }
}

} // namespace zs.
