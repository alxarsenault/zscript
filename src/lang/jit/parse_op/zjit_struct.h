/// This file is part of the `jit_compiler` class and must only be included in `zjit_compiler.cc`.
#include "lang/jit/zjit_compiler_include_guard.h"

namespace zs {

struct jit_compiler::struct_parser {
  inline struct_parser(zs::engine* eng)
      : names((zs::allocator<zs::object>(eng)))
      , constructors((zs::allocator<zs::object>(eng))) {}

  inline zs::error_result add_member(const zs::object& name) {
    if (names.contains(name)) {
      return errc::already_exists;
    }

    names.push_back(name);
    return {};
  }

  inline zs::error_result add_method(const zs::object& name, const zs::function_prototype_object& fpo) {
    return {};
  }

  inline zs::error_result add_constructor(const zs::function_prototype_object& fpo) {
    size_t pcount = fpo._parameter_names.size();

    if (pcount == 1) {
      if (has_default_constructor) {
        return errc::duplicated_default_constructor;
      }

      has_default_constructor = true;
    }

    else if (fpo._parameter_names.size() == fpo._default_params.size() + 1) {
      if (has_default_constructor) {
        return errc::duplicated_default_constructor;
      }

      has_default_constructor = true;
    }

    if (constructors.empty()) {
      constructors.push_back(&fpo);
      return {};
    }

    zs::small_vector<size_t, 8> same((zs::allocator<size_t>(fpo.get_engine())));

    for (size_t i = 0; i < constructors.size(); i++) {
      if (constructors[i]->_parameter_names.size() == pcount) {
        same.push_back(i);
      }
    }

    if (same.empty()) {
      constructors.push_back(&fpo);
      return {};
    }

    for (size_t i = 0; i < same.size(); i++) {
      bool all_same = true;
      const zs::function_prototype_object* f = constructors[same[i]];
      for (size_t k = 0; k < f->_parameter_names.size(); k++) {
        const zs::local_var_info_t* invinfo = fpo.find_local(fpo._parameter_names[k]);
        const zs::local_var_info_t* vinfo = f->find_local(f->_parameter_names[k]);

        if (!invinfo or !vinfo) {
          continue;
        }

        if (invinfo->mask != vinfo->mask or invinfo->custom_mask != vinfo->custom_mask) {
          all_same = false;
          break;
        }
      }

      if (all_same) {
        return errc::ambiguous_constructors;
      }
    }

    constructors.push_back(&fpo);
    return {};
  }

  zs::small_vector<zs::object, 8> names;
  zs::small_vector<const zs::function_prototype_object*, 8> constructors;
  bool has_default_constructor = false;
};

ZS_JIT_COMPILER_PARSE_OP(p_struct_statement) {

  lex();
  expr_state es = _estate;
  _estate.no_get = true;

  bool had_doc = _has_doc_block;
  _has_doc_block = false;

  //  zb::scoped remove_doc_block([&]() {
  //    if(_has_doc_block) {
  ////      zb::print("DOC-BLOCK");
  //      _has_doc_block = false;
  //    }
  //  });

  // Check if the struct is declared as `struct var_name {` or `struct something.var_name`.
  const bool is_local = is(tok_identifier) and _lexer->peek() == tok_lcrlbracket;

  if (is_local) {
    zs::object var_name;
    ZS_COMPILER_EXPECT_GET(tok_identifier, var_name);

    ZS_COMPILER_PARSE(p_struct);

    add_string_instruction(var_name);
    add_instruction<op_set_struct_name>(up_target(-2), top_target());
    pop_target();

    if (had_doc) {
      add_struct_doc_instruction(top_target());
      //      add_string_instruction(_doc_block);
      //      target_t doc_idx = pop_target();
      //      add_top_target_instruction<op_set_struct_doc>(  doc_idx);
    }
    pop_target();

    ZS_COMPILER_RETURN_IF_ERROR(
        add_stack_variable(var_name), "Duplicated local variable name ", var_name, ".\n");
  }
  else {
    ZS_COMPILER_PARSE(p_prefixed);

    switch (_estate.type) {
    case expr_type::e_expr:
      return ZS_COMPILER_ERROR(errc::invalid_operation, "Invalid class name");

    case expr_type::e_base:
      ZBASE_NO_BREAK;
    case expr_type::e_object: {
      ZS_COMPILER_PARSE(p_struct);

      add_top_target_instruction<op_set_struct_name>(up_target(-2));
      ///
      if (had_doc) {
        add_struct_doc_instruction(top_target());

        //        add_string_instruction(_doc_block);
        //        target_t doc_idx = pop_target();
        //        add_top_target_instruction<op_set_struct_doc>(  doc_idx);
      }

      target_t val = pop_target();
      target_t key = pop_target();
      target_t table = pop_target();
      add_instruction<op_new_slot>(table, key, val);
      break;
    }

    case expr_type::e_local:
      ZBASE_NO_BREAK;
    case expr_type::e_capture:
      return ZS_COMPILER_ERROR(
          errc::invalid_operation, "Cannot create a class in a local with the syntax(class <local>)");
    }
  }

  _estate = es;
  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_struct) {

  if (is(tok_identifier)) {
    return ZS_COMPILER_ERROR(errc::unexpected_struct_name,
        "A struct expression cannot be named `var a = struct {...};`.\n"
        "Only struct statement can be named `struct Name {};`.");
  }

  ZS_COMPILER_EXPECT(tok_lcrlbracket);

  add_new_target_instruction<op_new_obj>(k_struct);

  struct_parser sparser(_engine);

  [[maybe_unused]] int_t nitems = 0;

  while (!is(tok_rcrlbracket)) {
    bool had_doc = is(tok_doc_block);
    size_t doc_count = _doc_blocks.size();
    if (had_doc) {
      //      std::string_view val =  _lexer->get_string_value() ;
      std::string_view val = zb::strip_leading_and_trailing_endlines(_lexer->get_escaped_string_value());
      //      zb::print("DOOCOCOCOC", val);
      _has_doc_block = false;
      _doc_blocks.push_back(zs::_s(_engine, val));
      lex();
    }

    if (is(tok_constructor)) {
      ZS_COMPILER_PARSE(p_struct_constructor, &sparser);
    }
    else {
      zs::var_decl_flags_t vdcl_flags = zs::var_decl_flags_t::vdf_none;
      if (had_doc) {
        vdcl_flags = zs::var_decl_flags_t::vdf_doc;
      }
      ZS_COMPILER_PARSE(p_var_decl_prefix, &vdcl_flags);

      bool is_private = lex_if(tok_private);
      if (is(tok_function, tok_mutable)) {
        ZS_COMPILER_PARSE(p_struct_method, &sparser, vdcl_flags);
      }
      else {
        ZS_COMPILER_PARSE(p_struct_member, &sparser, vdcl_flags);
      }
    }

    if (had_doc and doc_count != _doc_blocks.size()) {
      //      add_doc_instruction(top_target());
      _doc_blocks.pop_back();
      //        add_string_instruction(_doc_block);
      //        target_t doc_idx = pop_target();
      //        add_top_target_instruction<op_set_struct_doc>(  doc_idx);
    }
    //    else if (is(tok_function, tok_mutable)) {
    //      ZS_COMPILER_PARSE(p_struct_method, &sparser);
    //    }
    //    else {
    //      ZS_COMPILER_PARSE(p_struct_content, &sparser);
    //    }
    nitems++;
  }

  lex();

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_struct_method, struct_parser* sparser, zs::var_decl_flags_t vdcl_flags) {
  zs::line_info linfo = get_line_info();

  bool is_mutable = lex_if(tok_mutable);

  ZS_COMPILER_EXPECT(tok_function);

  zs::object identifier;
  ZS_COMPILER_EXPECT_GET(tok_identifier, identifier);

  ZS_COMPILER_EXPECT(tok_lbracket);

  ZS_COMPILER_PARSE(p_create_normal_function, CREF(identifier));

  if (auto err = sparser->add_method(identifier, get_last_function_proto())) {
    return ZS_COMPILER_ERROR_WITH_LINE_INFO(err, linfo, "Invalid method.");
  }

  add_top_target_instruction<op_new_struct_method>(get_last_function_index(), vdcl_flags);

  if ((vdcl_flags & zs::var_decl_flags_t::vdf_doc) != 0) {
    add_struct_doc_member_instruction(top_target(), identifier);
  }

  //  add_top_target_instruction<op_new_struct_method>(get_last_function_index(),
  //      (vdcl_flags & var_decl_flags_t::vdf_private) == var_decl_flags_t::vdf_private);

  // Optional semi-colon.
  lex_if(tok_semi_colon);

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_struct_member, struct_parser* sparser, zs::var_decl_flags_t vdcl_flags) {
  uint32_t obj_type_mask = 0;
  uint64_t custom_mask = 0;
  bool is_static = false;
  bool is_const = false;

  ZS_COMPILER_PARSE(p_variable, &obj_type_mask, &custom_mask, &is_static, &is_const);

  zs::line_info linfo = get_line_info();

  zb_loop() {
    zs::object identifier;
    ZS_COMPILER_EXPECT_GET(tok_identifier, identifier);

    ZS_COMPILER_RETURN_IF_ERROR(
        sparser->add_member(identifier), "struct member variable", identifier, "already exists.\n");

    if (!is_small_string_identifier(identifier)) {
      add_string_instruction(identifier);
    }

    target_t val = k_invalid_target;

    // Default value?
    if (lex_if(tok_eq)) {
      ZS_COMPILER_PARSE(p_expression);
      val = pop_target();
    }
    else if (zs::is_var_decl_flags_static(vdcl_flags)) {
      return ZS_COMPILER_ERROR(errc::invalid, "struct static variable requires a default value.\n");
    }

    if (is_small_string_identifier(identifier)) {
      add_top_target_instruction<op_new_struct_slot_ss>(
          zs::ss_inst_data::create(identifier), val, obj_type_mask, vdcl_flags);

      //      add_top_target_instruction<op_new_struct_slot_ss>(zs::ss_inst_data::create(identifier), val,
      //          obj_type_mask, zs::is_var_decl_flags_static(vdcl_flags), (vdcl_flags &
      //          var_decl_flags_t::vdf_private) == var_decl_flags_t::vdf_private, val != k_invalid_target,
      //          zs::is_var_decl_flags_const(vdcl_flags));
    }
    else {
      add_top_target_instruction<op_new_struct_slot>(pop_target(), val, obj_type_mask, vdcl_flags);

      //      add_top_target_instruction<op_new_struct_slot>(
      //          pop_target(), val, obj_type_mask, zs::is_var_decl_flags_static(vdcl_flags), (vdcl_flags &
      //          var_decl_flags_t::vdf_private) == var_decl_flags_t::vdf_private, val != k_invalid_target,
      //          zs::is_var_decl_flags_const(vdcl_flags));
    }

    if ((vdcl_flags & zs::var_decl_flags_t::vdf_doc) != 0) {
      add_struct_doc_member_instruction(top_target(), identifier);
    }

    if (is_not(tok_comma)) {
      break;
    }

    linfo = get_line_info();
    lex();
  }

  // Mandatory semi-colon.
  if (!lex_if(tok_semi_colon)) {
    return ZS_COMPILER_ERROR_WITH_LINE_INFO(
        errc::invalid_token, linfo, "A semi-colon `;` is mandatory after a struct member declaration.");
  }

  return {};
}

ZS_JIT_COMPILER_PARSE_OP(p_struct_constructor, struct_parser* sparser) {
  ZS_ASSERT(is(tok_constructor));

  zs::line_info linfo = get_line_info();

  // Skip `tok_constructor`.
  lex();

  ZS_COMPILER_EXPECT(tok_lbracket);

  // No parameter for `constructor()`?
  if (is(tok_rbracket)) {

    // Only one default constructor can be declared.
    // If there's already a default constructor, we return an error.
    if (sparser->has_default_constructor) {
      return ZS_COMPILER_ERROR_WITH_LINE_INFO(
          errc::duplicated_default_constructor, linfo, "Duplicated default constructor.");
    }

    // Check if it is either `constructor() = default` or `constructor() {}`.
    // When a constructor with no content is declared, i.e. `constructor() {}`, we don't
    // actually create the closure, we just add a default constructor to the struct.
    if (peek_compare({ ZS_TOK(eq, default) }) or peek_compare({ ZS_TOK(lcrlbracket, rcrlbracket) })) {
      sparser->has_default_constructor = true;
      add_top_target_instruction<op_new_struct_default_constructor>();

      // Lex `{ tok_rbracket, tok_eq, tok_default }` or `{ tok_rbracket, tok_lcrlbracket, tok_rcrlbracket }`.
      lex_n(3);
      lex_if(tok_semi_colon);
      return {};
    }

    // We're not setting the `sparser->has_default_constructor` value to true here
    // since it will be handle in the `sparser->add_constructor()`.
  }

  zs::object var_name = zs::_ss("constructor");
  ZS_COMPILER_PARSE(p_create_normal_function, CREF(var_name));

  if (auto err = sparser->add_constructor(get_last_function_proto())) {
    return ZS_COMPILER_ERROR_WITH_LINE_INFO(err, linfo,
        (err == errc::ambiguous_constructors ? "Ambiguous constructors."
                                             : "Duplicated default constructor."));
  }

  add_top_target_instruction<op_new_struct_constructor>(get_last_function_index());

  lex_if(tok_semi_colon);

  return {};
}

} // namespace zs.
