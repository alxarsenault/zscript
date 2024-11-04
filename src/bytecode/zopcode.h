#pragma once

#include <zscript/zscript.h>

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
  aop_bitwise_or,
  aop_bitwise_and,
  aop_bitwise_xor,

  aop_compare,

  aop_add_eq,
  aop_sub_eq,
  aop_mul_eq,
  aop_div_eq,
  aop_mod_eq,
  aop_exp_eq,
  aop_lshift_eq,
  aop_rshift_eq,
  aop_bitwise_or_eq,
  aop_bitwise_and_eq,
  aop_bitwise_xor_eq,
};

enum class arithmetic_uop : uint8_t { //
  uop_minus,
  uop_incr,
  uop_decr,
  uop_pre_incr,
  uop_pre_decr
};

inline constexpr bool is_postfix_arithmetic_uop(arithmetic_uop uop) noexcept {
  return zb::is_one_of(uop, arithmetic_uop::uop_incr, arithmetic_uop::uop_decr);
}

inline constexpr arithmetic_uop arithmetic_uop_postfix_to_prefix(arithmetic_uop uop) noexcept {
  ZS_ASSERT(is_postfix_arithmetic_uop(uop));

  switch (uop) {
  case arithmetic_uop::uop_incr:
    return arithmetic_uop::uop_pre_incr;
  case arithmetic_uop::uop_decr:
    return arithmetic_uop::uop_pre_decr;
  default:
    return uop;
  }

  return uop;
}

enum class compare_op : uint8_t { //
  lt,
  le,
  gt,
  ge,
  tw,
  double_arrow,
  double_arrow_eq
};

inline constexpr meta_method arithmetic_op_to_meta_method(arithmetic_op op) noexcept {
  using enum meta_method;
  using enum arithmetic_op;

  // clang-format off
  switch (op) {
  case aop_add: return mt_add;
  case aop_sub: return mt_sub;
  case aop_mul: return mt_mul;
  case aop_div: return mt_div;
  case aop_mod: return mt_mod;
  case aop_exp: return mt_exp;
  case aop_lshift: return mt_lshift;
  case aop_rshift: return mt_rshift;
  case aop_bitwise_or: return mt_bw_or;
  case aop_bitwise_and: return mt_bw_and;
  case aop_bitwise_xor: return mt_bw_xor;

  case aop_compare: return mt_compare;

  case aop_add_eq: return mt_add_eq;
  case aop_sub_eq: return mt_sub_eq;
  case aop_mul_eq: return mt_mul_eq;
  case aop_div_eq: return mt_div_eq;
  case aop_mod_eq: return mt_mod_eq;
  case aop_exp_eq: return mt_exp_eq;
  case aop_lshift_eq: return mt_lshift_eq;
  case aop_rshift_eq: return mt_rshift_eq;
  case aop_bitwise_or_eq: return mt_bw_or_eq;
  case aop_bitwise_and_eq: return mt_bw_and_eq;
  case aop_bitwise_xor_eq: return mt_bw_xor_eq;

  default: break;
  }
  // clang-format on

  return mt_none;
}

inline constexpr const char* arithmetic_op_to_string(arithmetic_op op) noexcept {
  using enum meta_method;
  using enum arithmetic_op;

  // clang-format off
  switch (op) {
  case aop_add: return "+";
  case aop_sub: return "-";
  case aop_mul: return "*";
  case aop_div: return "/";
  case aop_mod: return "%";
  case aop_exp: return "^";
  case aop_lshift: return "<<";
  case aop_rshift: return ">>";
  case aop_bitwise_or: return "|";
  case aop_bitwise_and: return "&";
    case aop_bitwise_xor: return "^^";
      
    case aop_compare: return "<==>";

  case aop_add_eq: return "+=";
  case aop_sub_eq: return "-=";
  case aop_mul_eq: return "*=";
  case aop_div_eq: return "/=";
  case aop_mod_eq: return "%=";
  case aop_exp_eq: return "^=";
  case aop_lshift_eq: return "<<=";
  case aop_rshift_eq: return ">>=";
  case aop_bitwise_or_eq: return "|=";
  case aop_bitwise_and_eq: return "&=";
  case aop_bitwise_xor_eq: return "^^=";
  }
  // clang-format on

  return "unknown";
}

// inline constexpr meta_method arithmetic_op_to_meta_eq_method(arithmetic_op op) noexcept {
//   using enum meta_method;
//   using enum arithmetic_op;
//
//   // clang-format off
//   switch (op) {
//   case aop_add: return mt_add_eq;
//   case aop_sub: return mt_sub_eq;
//   case aop_mul: return mt_mul_eq;
//   case aop_div: return mt_div_eq;
//   case aop_mod: return mt_mod_eq;
//   case aop_exp: return mt_exp_eq;
//   case aop_lshift: return mt_lshift_eq;
//   case aop_rshift: return mt_rshift_eq;
//   case aop_bitwise_or: return mt_bw_or_eq;
//   case aop_bitwise_and: return mt_bw_and_eq;
//   case aop_bitwise_xor: return mt_bw_xor_eq;
//   default: return mt_none;
//   }
//   // clang-format on
//
//   return mt_none;
// }

inline constexpr meta_method arithmetic_op_to_rhs_meta_method(arithmetic_op op) noexcept {
  using enum meta_method;
  using enum arithmetic_op;

  // clang-format off
  switch (op) {
  case aop_add: return mt_rhs_add;
  case aop_sub: return mt_rhs_sub;
  case aop_mul: return mt_rhs_mul;
  case aop_div: return mt_rhs_div;
  case aop_mod: return mt_rhs_mod;
  case aop_exp: return mt_rhs_exp;
  case aop_lshift: return mt_rhs_lshift;
  case aop_rshift: return mt_rhs_rshift;
  case aop_bitwise_or: return mt_rhs_bw_or;
  case aop_bitwise_and: return mt_rhs_bw_and;
  case aop_bitwise_xor: return mt_rhs_bw_xor;
  case aop_compare: return mt_rhs_compare;
  default: break;
  }
  // clang-format on

  return mt_none;
}

inline constexpr meta_method arithmetic_uop_to_meta_method(arithmetic_uop uop) noexcept {
  using enum meta_method;
  using enum arithmetic_uop;

  // clang-format off
  switch (uop) {
  case uop_minus:return mt_unary_minus;
  case uop_incr:return mt_incr;
  case uop_decr:return mt_decr;
  case uop_pre_incr:return mt_pre_incr;
  case uop_pre_decr:return mt_pre_decr;
  }
  // clang-format on

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

} // namespace zs.
