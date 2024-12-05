/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "lang/jit/zjit_compiler_include_guard.h"

namespace zs {

ZS_JIT_COMPILER_PARSE_OP(p_var_decl_prefix, zs::var_decl_flags_t* flags) {
  ZS_ASSERT(flags, "Invalid flags pointer");

  zs::var_decl_flags_t flgs = zs::var_decl_flags_t::vdf_none;

  while (is_var_decl_prefix_token()) {
    switch (_token) {
    case tok_const:
      flgs |= var_decl_flags_t::vdf_const;
      break;
    case tok_mutable:
      flgs |= var_decl_flags_t::vdf_mutable;
      break;
    case tok_static:
      flgs |= var_decl_flags_t::vdf_static;
      break;
    case tok_private:
      flgs |= var_decl_flags_t::vdf_private;
      break;
    case tok_export:
      flgs |= var_decl_flags_t::vdf_export;
      break;
    }

    lex();
  }

  *flags |= flgs;

  return {};
}

/// Parsing a variable.
/// `static const var name`
/// `const var name
/// `const name`
/// `var name`
/// `int name`
/// `var<int, float> name`
ZS_JIT_COMPILER_PARSE_OP(
    p_variable, uint32_t* obj_type_mask, uint64_t* custom_mask, bool* is_static, bool* is_const) {

  *is_static = lex_if(tok_static);
  *is_const = lex_if(tok_const);

  if (is(tok_static)) {
    return ZS_COMPILER_ERROR(
        zs::error_code::invalid_token, "staric must be place before const in variable declaration");
  }

  if (!is_var_decl_tok_no_const()) {
    if (is(tok_identifier)) {
      return {};
    }

    return zs::error_code::invalid_token;
  }

  switch (_token) {
  case tok_char:
    ZBASE_NO_BREAK;
  case tok_int:
    *obj_type_mask |= zs::get_object_type_mask(k_integer);
    lex();
    break;

  case tok_float:
    *obj_type_mask |= zs::get_object_type_mask(k_float);
    lex();
    break;

  case tok_number:
    *obj_type_mask |= zs::constants::k_number_mask;
    lex();
    break;

  case tok_bool:
    *obj_type_mask |= zs::get_object_type_mask(k_bool);
    lex();
    break;

  case tok_array:
    *obj_type_mask |= zs::get_object_type_mask(k_array);
    lex();
    break;

  case tok_table:
    *obj_type_mask |= zs::get_object_type_mask(k_table);
    lex();
    break;

  case tok_string:
    *obj_type_mask |= zs::object_base::k_string_mask;
    lex();
    break;

  case tok_exttype:
    *obj_type_mask |= zs::get_object_type_mask(k_extension);
    lex();
    break;

  case tok_null:
    *obj_type_mask |= zs::get_object_type_mask(k_null);
    lex();
    break;

  case tok_var:
    lex();

    // Parsing a typed var (var<type1, type2, ...>).
    if (is(tok_lt)) {
      if (auto err = parse<p_variable_type_restriction>(REF(*obj_type_mask), REF(*custom_mask))) {
        return ZS_COMPILER_ERROR(err, "parsing variable type restriction `var<....>`");
      }
    }
    break;

  default:
    return zs::error_code::invalid_token;
  }

  return {};
}

template <>
zs::error_result jit_compiler::parse<p_variable_type_restriction>(
    zb::ref_wrapper<uint32_t> mask, zb::ref_wrapper<uint64_t> custom_mask) {
  using enum token_type;

  if (is_not(tok_lt)) {
    return zs::error_code::invalid;
  }

  mask.get() = 0;
  custom_mask.get() = 0;
  //  mask = 0;
  //  custom_mask = 0;
  int count = 0;
  int comma_count = 0;

  while (lex() != tok_gt) {

    switch (_token) {
    case tok_comma:
      if (count == 0) {
        zb::print("ERROR: Invalid comma");
        return zs::error_code::invalid_comma;
      }

      if (++comma_count != count) {
        zb::print("ERROR: Invalid comma");
        return zs::error_code::invalid_comma;
      }
      break;

    case tok_int:
    case tok_char:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_integer;
      break;

    case token_type::tok_float:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_float;
      break;

    case token_type::tok_number:
      count++;
      mask |= (uint32_t)zs::constants::k_number_mask;
      break;

    case token_type::tok_bool:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_bool;
      break;

    case token_type::tok_string:
      count++;
      mask |= (uint32_t)zs::object_base::k_string_mask;
      break;

    case token_type::tok_array:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_array;
      break;

    case token_type::tok_table:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_table;
      break;

    case token_type::tok_null:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_null;
      break;

    case token_type::tok_exttype:
      count++;
      mask |= (uint32_t)zs::object_type_mask::k_extension;
      break;

    case token_type::tok_identifier: {
      count++;

      mask |= (uint32_t)zs::object_type_mask::k_table;
      mask |= (uint32_t)zs::object_type_mask::k_instance;
      mask |= (uint32_t)zs::object_type_mask::k_struct_instance;
      mask |= (uint32_t)zs::object_type_mask::k_user_data;
      mask |= (uint32_t)zs::object_type_mask::k_array;
      mask |= (uint32_t)zs::object_type_mask::k_native_array;
      mask |= (uint32_t)zs::object_type_mask::k_node;
      mask |= (uint32_t)zs::object_type_mask::k_mutable_string;

      object name(_engine, _lexer->get_identifier_value());
      int_t type_index;

      ZS_RETURN_IF_ERROR(_ccs->get_restricted_type_index(name, type_index));

      custom_mask |= (uint64_t)(1 << type_index);
      break;
    }

    default:
      zb::print("ERROR: Invalid token");
      return zs::error_code::invalid_type;
    }
  }

  if (is_not(tok_gt)) {
    return ZS_COMPILER_ERROR(zs::error_code::invalid_token, "expected var<...>");
  }

  lex();

  return {};
}

// TODO: Prevent from declaring empty const variable.
ZS_JIT_COMPILER_PARSE_OP(p_decl_var) {

  bool is_export = lex_if(tok_export);

  uint32_t obj_type_mask = 0;
  uint64_t custom_mask = 0;

  bool is_const = false;
  bool is_static_dummy = false;
  ZS_COMPILER_PARSE(p_variable, &obj_type_mask, &custom_mask, &is_static_dummy, &is_const);

  zb_loop() {
    if (is_not(tok_identifier)) {
      return ZS_COMPILER_ERROR(zs::errc::identifier_expected, "expected identifier");
    }

    object var_name(_engine, _lexer->get_identifier_value());

    ZS_TRACE("p_decl_var", var_name);

    lex();

    // @code `var name = ...;`
    if (is(tok_eq)) {

      lex();
      ZS_TRACE("p_decl_var (var ", var_name, " = ...)");
      ZS_RETURN_IF_ERROR(parse<p_expression>());

      target_t src = pop_target();
      target_t dest = new_target(obj_type_mask, custom_mask, is_const);

      zs::opcode last_op = get_instruction_opcode(get_instruction_index());
      //          =
      //          zs::instruction_iterator(&_ccs->_instructions._data[_ccs->get_instruction_index()]).get_opcode();

      bool skip_mask = false;
      if (obj_type_mask) {
        switch (last_op) {
        case op_load_int:
          if (!(skip_mask = (obj_type_mask & zs::get_object_type_mask(k_integer)))) {
            return ZS_COMPILER_ERROR(zs::errc::invalid_type_assignment, "wrong type mask", k_integer,
                "expected", zs::object_type_mask_printer{ obj_type_mask });
          }
          break;

        case op_load_float:
          if (!(skip_mask = (obj_type_mask & zs::get_object_type_mask(k_float)))) {
            return ZS_COMPILER_ERROR(zs::errc::invalid_type_assignment, "wrong type mask", k_float,
                "expected", zs::object_type_mask_printer{ obj_type_mask });
          }
          break;

        case op_load_bool:
          if (!(skip_mask = (obj_type_mask & zs::get_object_type_mask(k_bool)))) {
            return ZS_COMPILER_ERROR(zs::errc::invalid_type_assignment, "wrong type mask", k_bool, "expected",
                zs::object_type_mask_printer{ obj_type_mask });
          }
          break;

        case op_load_small_string:
          if (!(skip_mask = (obj_type_mask & zs::get_object_type_mask(k_small_string)))) {
            return ZS_COMPILER_ERROR(zs::errc::invalid_type_assignment, "wrong type mask", k_small_string,
                "expected", zs::object_type_mask_printer{ obj_type_mask });
          }
          break;

        case op_load_string:
          if (!(skip_mask = (obj_type_mask & zs::object_base::k_string_mask))) {
            return ZS_COMPILER_ERROR(zs::errc::invalid_type_assignment, "wrong type mask", k_long_string,
                "expected", zs::object_type_mask_printer{ obj_type_mask });
          }
          break;
        }
      }

      if (dest != src) {
        add_instruction<op_move>(dest, src);
      }

      if (!skip_mask) {
        if (custom_mask) {
          add_top_target_instruction<op_check_custom_type_mask>(obj_type_mask, custom_mask);
        }
        else if (obj_type_mask) {
          add_top_target_instruction<op_check_type_mask>(obj_type_mask);
        }
      }
    }

    // @code `var name;`
    else {
      add_new_target_instruction<op_load_null>();
    }

    if (is_export) {

      if (!_ccs->is_top_level()) {
        return ZS_COMPILER_ERROR(zs::errc::invalid_operation,
            "export variable declaration is only allowed in the top level scope.\n");
      }

      ZS_RETURN_IF_ERROR(add_to_export_table(var_name));
      if (is_not(tok_comma)) {
        break;
      }

      lex();
    }
    else {
      pop_target();
      ZS_COMPILER_RETURN_IF_ERROR(add_stack_variable(var_name, nullptr, obj_type_mask, custom_mask, is_const),
          "Duplicated local variable name ", var_name, ".\n");

      if (is_not(tok_comma)) {
        break;
      }

      lex();
    }
  }

  return {};
}

} // namespace zs.
