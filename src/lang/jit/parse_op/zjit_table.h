/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "lang/jit/zjit_compiler_include_guard.h"

namespace zs {

// Table.
ZS_JIT_COMPILER_PARSE_OP(p_table) {

  while (is_not(tok_rcrlbracket)) {
    zs::object key;
    bool is_small_string_key = false;

    int_t inst_idx = -1;
    int_t nextinst_idx = -1;

    switch (_token) {
    case tok_constructor:
      return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "A table cannot have a constructor.");

    case tok_function: {
      lex();
      ZS_COMPILER_EXPECT_GET(tok_identifier, key);

      if (!(is_small_string_key = is_small_string_identifier(key))) {
        add_string_instruction(key);
      }

      int_t bound_target = 0xFF;
      if (is(tok_lsqrbracket)) {
        ZS_COMPILER_PARSE(p_bind_env, REF(bound_target));
      }

      ZS_COMPILER_EXPECT(tok_lbracket);
      ZS_COMPILER_PARSE(p_create_function, CREF(key), bound_target, false);

      add_new_target_instruction<op_new_closure>(get_last_function_index(), (uint8_t)bound_target);
      break;
    }

    case tok_lsqrbracket: {
      lex();
      ZS_COMPILER_PARSE(p_comma);
      ZS_COMPILER_EXPECT(tok_rsqrbracket);
      ZS_COMPILER_EXPECT(tok_eq);

      inst_idx = get_instruction_index();
      nextinst_idx = get_next_instruction_index();
      ZS_COMPILER_PARSE(p_expression);
      break;
    }

    case tok_string_value:
    case tok_escaped_string_value: {
      key = _lexer->get_value();
      lex();

      if (!(is_small_string_key = is_small_string_identifier(key))) {
        add_string_instruction(key);
      }

      ZS_COMPILER_EXPECT(tok_colon);
      inst_idx = get_instruction_index();
      nextinst_idx = get_next_instruction_index();
      ZS_COMPILER_PARSE(p_expression);
      break;
    }

    default:
      ZS_COMPILER_EXPECT_GET(tok_identifier, key);

      if (!(is_small_string_key = is_small_string_identifier(key))) {
        add_string_instruction(key);
      }

      ZS_COMPILER_EXPECT(tok_eq);

      inst_idx = get_instruction_index();
      nextinst_idx = get_next_instruction_index();
      ZS_COMPILER_PARSE(p_expression);
    }

    // Optional comma.
    lex_if(tok_comma);

    if (is_small_string_key) {
      zs::ss_inst_data key_ss_data = zs::ss_inst_data::create(key);
      target_t value_target = pop_target();
      target_t top_trgt = top_target();

      if (get_instruction_index() == nextinst_idx) {
        zs::opcode last_op = get_instruction_opcode(nextinst_idx);

        switch (last_op) {
        case op_load_int: {
          // Get the value from the instruction before deleting it.
          int_t int_value = get_instruction_value<op_load_int>(nextinst_idx);

          // Remove the last instruction.
          _ccs->_instructions._data.resize(nextinst_idx);

          add_instruction<op_new_slot_ss_integer>(top_trgt, key_ss_data, int_value);
          continue;
        }

        case op_load_float: {
          // Get the value from the instruction before deleting it.
          float_t float_value = get_instruction_value<op_load_float>(nextinst_idx);

          // Remove the last instruction.
          _ccs->_instructions._data.resize(nextinst_idx);

          add_instruction<op_new_slot_ss_float>(top_trgt, key_ss_data, float_value);
          continue;
        }

        case op_load_bool: {
          // Get the value from the instruction before deleting it.
          bool_t bool_value = get_instruction_value<op_load_bool>(nextinst_idx);

          // Remove the last instruction.
          _ccs->_instructions._data.resize(nextinst_idx);

          add_instruction<op_new_slot_ss_bool>(top_trgt, key_ss_data, bool_value);
          continue;
        }

        case op_load_small_string: {
          // Get the value from the instruction before deleting it.
          zs::ss_inst_data ss_value = get_instruction_value<op_load_small_string>(nextinst_idx);

          // Remove the last instruction.
          _ccs->_instructions._data.resize(nextinst_idx);

          add_instruction<op_new_slot_ss_small_string>(top_trgt, key_ss_data, ss_value);
          continue;
        }
        }
      }

      add_instruction<op_new_slot_ss>(top_trgt, key_ss_data, value_target);
    }
    else {
      target_t val = pop_target();
      target_t key = pop_target();
      add_top_target_instruction<op_new_slot>(key, val);
    }
  }

  lex();

  return {};
}
} // namespace zs.
