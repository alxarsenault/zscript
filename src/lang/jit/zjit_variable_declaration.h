
//
// MARK: - Struct.
//

namespace zs {

ZS_JIT_COMPILER_PARSE_OP(p_decl_var_internal_2, bool is_export, bool is_const) {
  using enum token_type;
  using enum object_type;
  using enum opcode;

  uint32_t mask = 0;
  uint64_t custom_mask = 0;

  if (is_not(tok_identifier)) {
    token_type last_token = _token;
    lex();

    switch (last_token) {
    case tok_char:
    case tok_int:
      mask = zs::get_object_type_mask(k_integer);
      break;
    case tok_float:
      mask = zs::get_object_type_mask(k_float);
      break;
    case tok_string:
      mask = zs::object_base::k_string_mask;
      break;
    case tok_array:
      mask = zs::get_object_type_mask(k_array);
      break;
    case tok_table:
      mask = zs::get_object_type_mask(k_table);
      break;
    case tok_bool:
      mask = zs::get_object_type_mask(k_bool);
      break;
    case tok_exttype:
      mask = zs::get_object_type_mask(k_extension);
      break;

    case tok_null:
      mask = zs::get_object_type_mask(k_null);
      break;

    case tok_var:
      // Parsing a typed var (var<type1, type2, ...>).
      if (is(tok_lt)) {
        if (auto err = parse<p_variable_type_restriction>(REF(mask), REF(custom_mask))) {
          return ZS_COMPILER_ERROR(err, "parsing variable type restriction `var<....>`");
        }
      }
      break;
    }
  }

  zb_loop() {
    if (is_not(tok_identifier)) {
      return ZS_COMPILER_ERROR(zs::error_code::identifier_expected, "expected identifier");
    }

    object var_name(_engine, _lexer->get_identifier_value());

    lex();

    // @code `var name = ...;`
    if (is(tok_eq)) {

      //      if (is_export) {
      //        //
      //        //          _ccs->push_target(0);
      ////        _ccs->push_export_target();
      //////        _ccs->find_local_variable(zs::_ss("__exports__"));
      ////        ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
      //
      //      }
      lex();
      ZS_RETURN_IF_ERROR(parse<p_expression>());

      int_t src = _ccs->pop_target();
      int_t dest = _ccs->new_target(mask, custom_mask, is_const);

      zs::opcode last_op
          = zs::instruction_iterator(&_ccs->_instructions._data[_ccs->get_instruction_index()]).get_opcode();

      bool skip_mask = false;
      if (mask) {
        switch (last_op) {
        case op_load_int:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_integer)))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask", k_integer,
                "expected", zs::object_type_mask_printer{ mask });
          }
          break;

        case op_load_float:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_float)))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask", k_float,
                "expected", zs::object_type_mask_printer{ mask });
          }
          break;

        case op_load_bool:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_bool)))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask", k_bool,
                "expected", zs::object_type_mask_printer{ mask });
          }
          break;

        case op_load_small_string:
          if (!(skip_mask = (mask & zs::get_object_type_mask(k_small_string)))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask",
                k_small_string, "expected", zs::object_type_mask_printer{ mask });
          }
          break;

        case op_load_string:
          if (!(skip_mask = (mask & zs::object_base::k_string_mask))) {
            return ZS_COMPILER_ERROR(zs::error_code::invalid_type_assignment, "wrong type mask",
                k_long_string, "expected", zs::object_type_mask_printer{ mask });
          }
          break;
        }
      }

      if (dest != src) {
        //                          if (_fs->IsLocal(src)) {
        //                              _fs->SnoozeOpt();
        //                          }
        add_instruction<opcode::op_move>((uint8_t)dest, (uint8_t)src);
      }

      if (!skip_mask) {
        if (custom_mask) {
          add_instruction<opcode::op_check_custom_type_mask>((uint8_t)_ccs->top_target(), mask, custom_mask);
        }
        else if (mask) {
          add_instruction<opcode::op_check_type_mask>((uint8_t)_ccs->top_target(), mask);
        }
      }
    }

    // @code `var name;`
    else {
      add_new_target_instruction<opcode::op_load_null>();
    }

    if (is_export) {

      //      _ccs->push_export_target();
      //      int_t table_idx=  _ccs->find_local_variable(zs::_ss("__exports__"));
      ////      ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
      ////      int_t key_idx = _ccs->pop_target();
      //      //
      //      //          _ccs->push_target(0);
      ////      _ccs->push_export_target();
      ////       ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
      //      int_t value_idx = _ccs->pop_target();
      //      int_t key_idx = _ccs->pop_target();
      //      int_t table_idx = _ccs->pop_target();
      //      int_t key_idx = _ccs->pop_target();
      //      int_t table_idx = _ccs->pop_target();
      //      int_t value_idx = _ccs->pop_target();

      //
      //
      if (_ccs->get_parent()) {
        return ZS_COMPILER_ERROR(zs::error_code::invalid_operation,
            "static variable declaration is only allowed in the top level scope.\n");
        //          add_instruction<opcode::op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
      }
      //      add_instruction<opcode::op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);

      ZS_RETURN_IF_ERROR(add_to_export_table(var_name));
      //      if(var_name.get_string_unchecked().size() <= zs::constants::k_small_string_max_size) {
      //        int_t table_idx=  _ccs->find_local_variable(zs::_ss("__exports__"));
      //        int_t value_idx = _ccs->pop_target();
      //
      //        {
      //          struct uint64_t_pair {
      //            uint64_t value_1;
      //            uint64_t value_2;
      //          } spair = {};
      //
      //          std::string_view s = var_name.get_string_unchecked();
      //
      //          ::memcpy(&spair, s.data(), s.size());
      //          if(!_ccs->_exported_names.insert(var_name).second) {
      //            zb::print("duplicated export");
      //          }
      //          add_instruction<opcode::op_rawsets>((uint8_t)table_idx,  (uint8_t)value_idx, spair.value_1,
      //          spair.value_2);
      //        }
      //      }
      //      else {
      //        int_t table_idx=  _ccs->find_local_variable(zs::_ss("__exports__"));
      //        ZS_RETURN_IF_ERROR(add_export_string_instruction(var_name));
      //
      //        int_t key_idx = _ccs->pop_target();
      //        int_t value_idx = _ccs->pop_target();
      //        add_instruction<opcode::op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
      //      }

      if (is_not(tok_comma)) {
        break;
      }

      lex();
    }
    else {
      _ccs->pop_target();
      ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name, nullptr, mask, custom_mask, is_const));

      if (is_not(tok_comma)) {
        break;
      }

      lex();
    }

    //    _ccs->push_target(0);
    //    ZS_RETURN_IF_ERROR(helper::add_string_instruction(this, var_name, _ccs->new_target()));
    ////
    //
    //    int_t value_idx = _ccs->pop_target();
    //    int_t key_idx = _ccs->pop_target();
    //    int_t table_idx = _ccs->pop_target();
    //
    //
    //  if(_ccs->get_parent()) {
    //    add_instruction<opcode::op_set>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
    //  }
    //  else {
    //    add_instruction<opcode::op_rawset>((uint8_t)table_idx, (uint8_t)key_idx, (uint8_t)value_idx);
    //  }
    //
    //
    //
    //  _estate.type = expr_type::e_expr;
    //  _estate.pos = -1;
    //  _estate.no_get = false;
    //  _estate.no_assign = false;

    //    _ccs->pop_target();
    //    ZS_RETURN_IF_ERROR(_ccs->push_local_variable(var_name, nullptr, mask, custom_mask, is_const));
    //
    //    if (is_not(tok_comma)) {
    //      break;
    //    }
    //
    //    lex();
  }

  return {};
}

} // namespace zs.