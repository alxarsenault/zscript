/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "jit/zjit_compiler_include_guard.h"

namespace zs {
zs::error_result jit_compiler::parse_variable_prefix(variable_type_info& vinfo) {

#define SET_ATTRIBUTE(name)                                                                     \
  if (vinfo.is_##name()) {                                                                      \
    return ZS_COMPILER_ERROR(invalid_token, "Duplicated '" ZBASE_STRINGIFY(name) "' keyword."); \
  }                                                                                             \
  vinfo.set_##name()

  while (is_var_decl_prefix_token()) {
    switch (_token) {
    case tok_const:
      SET_ATTRIBUTE(const);
      break;

    case tok_mutable:
      SET_ATTRIBUTE(mutable);
      break;

    case tok_static:
      if (vinfo.is_const()) {
        return ZS_COMPILER_ERROR(invalid_token, "static must be place before const in variable declaration");
      }

      SET_ATTRIBUTE(static);
      break;

    case tok_private:
      SET_ATTRIBUTE(private);
      break;

      //    case tok_export:
      //      SET_ATTRIBUTE(export);
      //      break;
    }

    lex();
  }

#undef SET_ATTRIBUTE
  return {};
}

/// Parsing a variable.
/// `static const var name`
/// `const var name
/// `const name`
/// `var name`
/// `int name`
/// `var<int | float> name`
zs::error_result jit_compiler::parse_variable(variable_type_info& vinfo) {

  ZS_RETURN_IF_ERROR(parse_variable_prefix(vinfo));

  if (!is_var_decl_tok_no_const()) {
    if (is(tok_identifier)) {
      if (_lexer->peek() != tok_identifier) {
        return {};
      }

      object name(_engine, _lexer->get_identifier_value());
      int_t type_index;
      ZS_RETURN_IF_ERROR(_ccs->get_restricted_type_index(name, type_index));

      vinfo.mask |= (uint32_t)zs::create_type_mask(k_table, k_struct_instance, k_user_data, k_array);
      vinfo.custom_mask |= (uint64_t)(1 << type_index);

      lex();
      return {};
    }

    return invalid_token;
  }

  switch (_token) {
  case tok_char:
    ZBASE_NO_BREAK;
  case tok_int:
    vinfo.mask |= zs::get_object_type_mask(k_integer);
    lex();
    break;

  case tok_float:
    vinfo.mask |= zs::get_object_type_mask(k_float);
    lex();
    break;

  case tok_number:
    vinfo.mask |= zs::constants::k_number_mask;
    lex();
    break;

  case tok_bool:
    vinfo.mask |= zs::get_object_type_mask(k_bool);
    lex();
    break;

  case tok_array:
    vinfo.mask |= zs::get_object_type_mask(k_array);
    lex();
    break;

  case tok_table:
    vinfo.mask |= zs::get_object_type_mask(k_table);
    lex();
    break;

  case tok_string:
    vinfo.mask |= zs::object_base::k_string_mask;
    lex();
    break;

  case tok_exttype:
    vinfo.mask |= zs::get_object_type_mask(k_atom);
    lex();
    break;

  case tok_null:
    vinfo.mask |= zs::get_object_type_mask(k_null);
    lex();
    break;

  case tok_var:
    lex();

    // Parsing a typed var (var<type1 | type2 | ...>).
    if (is(tok_lt)) {
      vinfo.mask = 0;
      vinfo.custom_mask = 0;
      int count = 0;
      int comma_count = 0;

      while (lex() != tok_gt) {

        switch (_token) {
        case tok_bitwise_or:
          if (count == 0) {
            zb::print("ERROR: Invalid or");
            return ZS_COMPILER_ERROR(invalid_token, "parsing variable type restriction `var<....>`");
          }

          if (++comma_count != count) {
            zb::print("ERROR: Invalid or");
            return ZS_COMPILER_ERROR(invalid_token, "parsing variable type restriction `var<....>`");
          }
          break;

        case tok_int:
        case tok_char:
          count++;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_integer;
          break;

        case token_type::tok_float:
          count++;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_float;
          break;

        case token_type::tok_number:
          count++;
          vinfo.mask |= (uint32_t)zs::constants::k_number_mask;
          break;

        case token_type::tok_bool:
          count++;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_bool;
          break;

        case token_type::tok_string:
          count++;
          vinfo.mask |= (uint32_t)zs::object_base::k_string_mask;
          break;

        case token_type::tok_array:
          count++;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_array;
          break;

        case token_type::tok_table:
          count++;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_table;
          break;

        case token_type::tok_null:
          count++;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_null;
          break;

        case token_type::tok_exttype:
          count++;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_atom;
          break;

        case token_type::tok_identifier: {
          count++;

          vinfo.mask |= (uint32_t)zs::object_type_mask::k_table;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_struct_instance;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_user_data;
          vinfo.mask |= (uint32_t)zs::object_type_mask::k_array;

          object name(_engine, _lexer->get_identifier_value());
          int_t type_index;

          ZS_RETURN_IF_ERROR(_ccs->get_restricted_type_index(name, type_index));

          vinfo.custom_mask |= (uint64_t)(1 << type_index);
          break;
        }

        default:
          zb::print("ERROR: Invalid token");
          return invalid_type;
        }
      }

      if (is_not(tok_gt)) {
        return ZS_COMPILER_ERROR(invalid_token, "expected var<...>");
      }

      lex();

      return {};
    }
    break;

  default:
    return invalid_token;
  }

  return {};
}

// TODO: Prevent from declaring empty const variable.
zs::error_result jit_compiler::parse_variable_declaration() {

  variable_type_info vinfo;
  ZS_RETURN_IF_ERROR(parse_variable(vinfo));

  zb_loop() {
    if (is_not(tok_identifier)) {
      return ZS_COMPILER_ERROR(identifier_expected, "expected identifier");
    }

    object var_name(_engine, _lexer->get_identifier_value());

    ZS_TRACE("parse_variable_declaration", var_name);

    lex();

    // @code `var name = ...;`
    if (is(tok_eq)) {

      lex();
      ZS_TRACE("parse_variable_declaration (var ", var_name, " = ...)");
      ZS_RETURN_IF_ERROR(parse_expression());

      bool did_compile_time_type_mask = false;
      opcode last_op = get_instruction_opcode();

      ZS_COMPILER_RETURN_IF_ERROR(check_compile_time_mask(last_op, vinfo, did_compile_time_type_mask),
          "wrong type mask '", zs::get_exposed_object_type_name(opcode_to_object_type(last_op)),
          "' expected ", zs::object_type_mask_printer{ vinfo.mask, "'", "'" }, ".");

      target_t src = pop_target();
      target_t dest = new_target(vinfo);

      if (dest != src) {
        add_instruction<op_move>(dest, src);
      }

      if (!did_compile_time_type_mask) {
        if (vinfo.has_custom_mask()) {
          add_top_target_instruction<op_check_custom_type_mask>(vinfo.mask, vinfo.custom_mask);
        }
        else if (vinfo.has_mask()) {
          add_top_target_instruction<op_check_type_mask>(vinfo.mask);
        }
      }
    }

    // @code `var name;`
    else {
      add_new_target_instruction<op_load_null>();
    }

    //    if (vinfo.is_export()) {
    //
    //      if (!_ccs->is_top_level()) {
    //        return ZS_COMPILER_ERROR(
    //            invalid_operation, "export variable declaration is only allowed in the top level scope.\n");
    //      }
    //
    //      ZS_RETURN_IF_ERROR(add_to_export_table(var_name));
    //      if (is_not(tok_comma)) {
    //        break;
    //      }
    //
    //      lex();
    //    }
    //    else {
    pop_target();
    ZS_COMPILER_RETURN_IF_ERROR(
        add_stack_variable(var_name, nullptr, vinfo.mask, vinfo.custom_mask, vinfo.is_const()),
        "Duplicated local variable name ", var_name, ".\n");

    if (is_not(tok_comma)) {
      break;
    }

    lex();
    //    }
  }

  return {};
}

} // namespace zs.
