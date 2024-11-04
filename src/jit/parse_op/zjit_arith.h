/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "jit/zjit_compiler_include_guard.h"

namespace zs {

inline zs::error_result jit_compiler::gen_arith_eq(arithmetic_op aop) {
  expr_type es_type = _estate.type;

  switch (es_type) {
  case expr_type::e_local: {
    add_top_target_instruction<op_arith_eq>(pop_target(), aop);
    return {};
  }

  case expr_type::e_object: {
    if (_ccs->_target_stack.size() < 3) {
      return ZS_COMPILER_ERROR(compile_stack_error, "invalid target stack size for object in add_eq");
    }

    target_t value_idx = pop_target();
    target_t key_idx = pop_target();
    target_t table_idx = pop_target();
    add_new_target_instruction<op_obj_arith_eq>(table_idx, key_idx, value_idx, aop);
    return {};
  }

  default:
    return unimplemented;
  }

  return unimplemented;
}

zs::error_result jit_compiler::parse_arith_eq(token_type op) {
  switch (op) {
  case tok_add_eq:
    return gen_arith_eq(arithmetic_op::aop_add_eq);

  case tok_minus_eq:
    return gen_arith_eq(arithmetic_op::aop_sub_eq);

  case tok_mul_eq:
    return gen_arith_eq(arithmetic_op::aop_mul_eq);

  case tok_div_eq:
    return gen_arith_eq(arithmetic_op::aop_div_eq);

  case tok_mod_eq:
    return gen_arith_eq(arithmetic_op::aop_mod_eq);

  case tok_exp_eq:
    return gen_arith_eq(arithmetic_op::aop_exp_eq);

  case tok_lshift_eq:
    return gen_arith_eq(arithmetic_op::aop_lshift_eq);

  case tok_rshift_eq:
    return gen_arith_eq(arithmetic_op::aop_rshift_eq);

  case tok_bitwise_or_eq:
    return gen_arith_eq(arithmetic_op::aop_bitwise_or_eq);

  case tok_bitwise_and_eq:
    return gen_arith_eq(arithmetic_op::aop_bitwise_and_eq);

  case tok_xor_eq:
    return gen_arith_eq(arithmetic_op::aop_bitwise_xor_eq);

  case tok_double_arrow_eq:
    return unimplemented;

  default:
    break;
  }

  return {};
}

} // namespace zs.
