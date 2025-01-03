/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "jit/zjit_compiler_include_guard.h"

namespace zs {

zs::error_result jit_compiler::parse_arith_eq(token_type op) {
  arithmetic_op aop;

  // clang-format off
  switch (op) {
  case tok_add_eq: aop = aop_add_eq; break;
  case tok_sub_eq: aop = aop_sub_eq; break;
  case tok_mul_eq: aop = aop_mul_eq; break;
  case tok_div_eq: aop = aop_div_eq; break;
  case tok_mod_eq: aop = aop_mod_eq; break;
  case tok_exp_eq: aop = aop_exp_eq; break;
  case tok_lshift_eq: aop = aop_lshift_eq; break;
  case tok_rshift_eq: aop = aop_rshift_eq; break;
  case tok_bitwise_or_eq: aop = aop_bitwise_or_eq; break;
  case tok_bitwise_and_eq: aop = aop_bitwise_and_eq; break;
  case tok_bitwise_xor_eq: aop = aop_bitwise_xor_eq; break;
  default: return {};
  }
  // clang-format on

  switch (_estate.type) {
  case expr_type::e_local: {
    add_top_target_instruction<op_arith_eq>(aop, pop_target());
    return {};
  }

  case expr_type::e_object: {
    if (_ccs->_target_stack.size() < 3) {
      return ZS_COMPILER_ERROR(compile_stack_error, "invalid target stack size for object in add_eq");
    }

    target_t value_idx = pop_target();
    target_t key_idx = pop_target();
    target_t table_idx = pop_target();
    add_new_target_instruction<op_obj_arith_eq>(aop, table_idx, key_idx, value_idx);
    return {};
  }

  default:
    return unimplemented;
  }

  return unimplemented;
}

} // namespace zs.
