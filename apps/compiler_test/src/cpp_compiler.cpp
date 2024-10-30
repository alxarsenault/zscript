#include "cpp_compiler.h"
#include <fstream>

namespace zs {

struct dsakjdkasl {
  uint32_t mask;
};

inline std::ostream& operator<<(std::ostream& stream, dsakjdkasl v) {
  bool found_one = false;
  bool has_string_mask = false;
  if ((zs::constants::k_string_mask & v.mask) != 0) {
    has_string_mask = true;
    v.mask &= ~zs::constants::k_string_mask;
  }

#define _X(name, str, exposed_name)                           \
  if (v.mask & zs::get_object_type_mask(object_type::name)) { \
    if (found_one) {                                          \
      stream << ", ";                                         \
    }                                                         \
    stream << ZBASE_STRINGIFY(zs::object_type::name);         \
    found_one = true;                                         \
  }

  ZS_OBJECT_TYPE_ENUM(_X)
#undef _X

  if (has_string_mask) {
    if (found_one) {
      stream << ", ";
    }

    stream << "zs::constants::k_string_mask";
  }

  return stream << "";
}

static inline bool has_special_chars(std::string_view s) {
  for (char c : s) {
    if (zb::is_one_of(c, '"', '\n', '\t')) {
      return true;
    }
  }
  return false;
}

static inline bool has_triple_quote(std::string_view s) { return s.find("\"\"\"") != std::string_view::npos; }

static inline auto triple_quote(const std::string_view& s) {
  return zb::quoted<"R\"\"\"\"\"(", ")\"\"\"\"\"">(s);
}

static inline auto triple_apostrophe(const std::string_view& s) { return zb::quoted<"'''", "'''">(s); }

using enum ast_node_type;

#define _X(name, str) \
  template <>         \
  zs::error_result cpp_compiler::gen<name>(const zs::object& node);
ZS_AST_NODE_TYPE_ENUM(_X)
#undef _X

#define ZS_DECL_UNIMPLEMENTED_COMPILER_GEN(name)                           \
  template <>                                                              \
  zs::error_result cpp_compiler::gen<ast_##name>(const zs::object& node) { \
    return zs::error_code::unimplemented;                                  \
  }

ZS_DECL_UNIMPLEMENTED_COMPILER_GEN(empty);
ZS_DECL_UNIMPLEMENTED_COMPILER_GEN(new_table_field);
ZS_DECL_UNIMPLEMENTED_COMPILER_GEN(op_mod);
ZS_DECL_UNIMPLEMENTED_COMPILER_GEN(jump);
ZS_DECL_UNIMPLEMENTED_COMPILER_GEN(array_element);
ZS_DECL_UNIMPLEMENTED_COMPILER_GEN(root);

template <>
zs::error_result cpp_compiler::gen<ast_line>(const zs::object& node) {
  //  iwritee("// ", node.as_node().value());
  _line_comment = node;

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_return_statement>(const zs::object& node) {

  iwrite("return ");
  for (const auto& child : node.as_node()) {
    ZS_RETURN_IF_ERROR(gen<ast_expr>(child));
  }
  writee(";");
  return {};
}

zs::error_result cpp_compiler::arith_eq_op(
    const zs::object& node, std::string_view op, std::string_view ops) {
  zbase_assert(node.as_node().size() == 2);

  if (_line_comment.is_node()) {
    eiwritee("// ", _line_comment.as_node().value());
  }
  else {
    endl();
  }

  iwrite("if(zs::error_result err = __vm->", op, "(");
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  write(", ");
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  writee(")) {");
  iwritee<1>("return __vm.set_error(err, \"Invalid '", ops, "' operation.\");");
  iwritee<-1>("}");

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_op_add_eq>(const zs::object& node) {
  return arith_eq_op(node, "add_eq", "+=");
}

template <>
zs::error_result cpp_compiler::gen<ast_op_sub_eq>(const zs::object& node) {
  return arith_eq_op(node, "sub_eq", "-=");
}

template <>
zs::error_result cpp_compiler::gen<ast_op_mul_eq>(const zs::object& node) {
  return arith_eq_op(node, "mul_eq", "*=");
}

template <>
zs::error_result cpp_compiler::gen<ast_op_div_eq>(const zs::object& node) {
  return arith_eq_op(node, "div_eq", "/=");
}

template <>
zs::error_result cpp_compiler::gen<ast_op_mod_eq>(const zs::object& node) {
  return arith_eq_op(node, "mod_eq", "%=");
}

template <>
zs::error_result cpp_compiler::gen<ast_op_exp_eq>(const zs::object& node) {
  return arith_eq_op(node, "exp_eq", "^=");
}

template <>
zs::error_result cpp_compiler::gen<ast_eq_eq>(const zs::object& node) {
  zbase_assert(node.as_node().size() == 2);

  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  _stream << " == ";
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_not_eq>(const zs::object& node) {
  zbase_assert(node.as_node().size() == 2);
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  _stream << " != ";
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_assignment>(const zs::object& node) {
  zbase_assert(node.as_node().size() == 2);

  iwrite();
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  write(" = ");
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  writee(";");

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_set>(const zs::object& node) {
  zbase_assert(node.as_node().size() == 3);

  //  _stream << _indent;
  //  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  //  _stream << " = ";
  //  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  //  _stream << ";\n";

  if (_line_comment.is_node()) {
    eiwritee("// ", _line_comment.as_node().value());
  }
  else {
    endl();
  }

  if (zs::is_leaf_node(ast_node_name_to_type(node.as_node()[0].as_node().name()))) {
    iwrite("if(zs::error_result err = __vm->set(");
  }
  else {
    iwrite("if(zs::error_result err = __vm->set(");
  }

  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  write(", ");
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  write(", ");
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[2]));
  writee(")) {");
  iwritee<1>("return __vm.set_error(err, \"Invalid 'set' operation.\");");
  iwritee<-1>("}");

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_op_add>(const zs::object& node) {

  zbase_assert(node.as_node().size() == 2);

  _stream << "__vm->add(";

  //  zb::print(node[0]);

  //  {
  //    zs::ast_node_walker walker(node[0]);
  //    zs::string f = walker.serialize(_engine);
  //    zb::print(f );
  //  }

  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  _stream << ", ";
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  _stream << ")";

  //  {
  //    zs::ast_node_walker walker(node[1]);
  //    zs::string f = walker.serialize(_engine);
  //    zb::print(f);
  //  }
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_op_sub>(const zs::object& node) {

  zbase_assert(node.as_node().size() == 2);

  _stream << "__vm->sub(";

  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  _stream << ", ";
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  _stream << ")";

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_op_mul>(const zs::object& node) {

  zbase_assert(node.as_node().size() == 2);

  _stream << "__vm->mul(";

  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  _stream << ", ";
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  _stream << ")";

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_op_div>(const zs::object& node) {

  zbase_assert(node.as_node().size() == 2);

  _stream << "__vm->div(";

  //  zs::ast_node_walker w(node);
  //  zb::print("klklklkk", w.serialize(_engine));

  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  _stream << ", ";
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  _stream << ")";

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_array_declaration>(const zs::object& node) {

  if (!node.as_node().has_children()) {
    _stream << "zs::var::create_array(__eng, 0);\n";
    return {};
  }

  _stream << "zs::var::create_array(__eng, {";

  const size_t sz = node.as_node().size();
  size_t count = 0;

  for (const auto& child : node.as_node()) {
    if (zs::ast_node_name_to_type(child.as_node().name()) == zs::ast_array_element) {
      count++;

      for (const auto& elem : child.as_node()) {
        ZS_RETURN_IF_ERROR(gen<ast_expr>(elem));

        if (count != sz) {
          _stream << ", ";
        }
      }
    }
  }

  _stream << "})";

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_table_declaration>(const zs::object& node) {

  if (!node.as_node().has_children()) {
    writee("zs::var::create_table(__eng);");
    return {};
  }

  writee("zs::var::create_table(__eng, {");
  ++_indent;
  const size_t sz = node.as_node().size();
  size_t count = 0;

  for (const auto& child : node.as_node()) {
    if (zs::ast_node_name_to_type(child.as_node().name()) == zs::ast_new_table_field) {
      count++;

      iwrite("{");
      ZS_RETURN_IF_ERROR(gen<ast_expr>(child.as_node()[0]));
      write(", ");
      ZS_RETURN_IF_ERROR(gen<ast_expr>(child.as_node()[1]));
      write("}");

      if (count != sz) {
        writee(",");
      }
      else {
        endl();
      }
    }
  }

  iwrite<-1>("})");

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_var_type>(const zs::object& node) {
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_parameter_declaration>(const zs::object& node) {
  zbase_assert(zs::ast_node_name_to_type(node.as_node().name()) == ast_parameter_declaration);

  zs::object_type obj_type = (zs::object_type)node.as_node()[0].as_node().value().to_int();

  switch (obj_type) {
  default:
    write("zs::var ");
    break;
  }

  write(node.as_node().value().get_string_unchecked());
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_function_declaration>(const zs::object& node) {

  eiwrite("auto ", node.as_node().get_opt_attribute("name").get_string_unchecked(), " = [](");

  //  ast_node_walker w(node);
  //  zb::print(w.serialize(_engine));

  const zs::vector<zs::object>& children = node.as_node().children();
  const size_t sz = children.size();
  if (sz) {

    ZS_RETURN_IF_ERROR(gen<ast_parameter_declaration>(children[0]));

    for (size_t i = 1; i < sz; i++) {

      write(", ");

      const object& child = children[i];
      ZS_RETURN_IF_ERROR(gen<ast_parameter_declaration>(child));
    }
  }

  writee(") {");
  iwritee("};\n");
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_variable_declaration>(const zs::object& node) {

  endl();

  if (_line_comment.is_node()) {
    iwritee("// ", _line_comment.as_node().value());
  }
  const object* name = node.as_node().get_attribute("name");

  if (!name) {
    return zs::error_code::invalid_name;
  }

  if (!node.as_node().has_children()) {
    iwritee("zs::var ", name->get_string_unchecked(), ";");
    return {};
  }

  //  if (node.as_node()[0].as_node().value().is_type_info()) {
  //    if (node.as_node()[0].as_node().value().is_integer()) {
  //
  //      switch ((zs::object_type)node.as_node()[0].as_node().value()._int) {
  //      case zs::object_type::k_bool:
  //        iwrite("zs::bool_t");
  //        break;
  //
  //      case zs::object_type::k_integer:
  //        iwrite("zs::int_t");
  //        break;
  //
  //      case zs::object_type::k_float:
  //        iwrite("zs::float_t");
  //        break;
  //
  //      case zs::object_type::k_null:
  //        zs::throw_exception(zs::error_code::invalid);
  //        break;
  //
  //      case zs::object_type::k_none:
  //        iwrite("zs::var");
  //        break;
  //
  //      case zs::object_type::k_array:
  //        iwrite("zs::var");
  //        break;
  //
  //      default:
  //        iwrite("zs::var");
  //        break;
  //      }
  //    }
  //  }
  //  else {
  //    iwrite("zs::var");
  //  }

  if (const object* cpptype = node.as_node().get_attribute("cpp-type")) {
    iwrite(cpptype->get_string_unchecked());
  }
  else {
    iwrite("zs::var");
  }

  _stream << " " << name->get_string_unchecked();

  if (node.as_node().has_children()) {

    _stream << " = ";

    auto end_it = node.as_node().end();
    for (auto it = node.as_node().begin(); it != end_it; ++it) {
      ZS_RETURN_IF_ERROR(gen<ast_expr>(*it));
    }
  }

  writee(";");

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_if_block>(const zs::object& node) {

  _stream << " {\n";
  ++_indent;
  for (const auto& child : node.as_node()) {
    ZS_RETURN_IF_ERROR(gen<ast_expr>(child));
  }
  --_indent;
  _stream << _indent << "}\n";

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_else_block>(const zs::object& node) {

  _stream << _indent << "else {\n";
  ++_indent;
  for (const auto& child : node.as_node()) {
    ZS_RETURN_IF_ERROR(gen<ast_expr>(child));
  }
  --_indent;
  _stream << _indent << "}\n";

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_if_statement>(const zs::object& node) {

  _stream << _indent << "if (";
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  _stream << ")";

  for (auto it = node.as_node().begin() + 1; it != node.as_node().end(); ++it) {
    ZS_RETURN_IF_ERROR(gen<ast_expr>(*it));
  }

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_null_value>(const zs::object& node) {
  _stream << "nullptr";
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_integer_value>(const zs::object& node) {
  _stream << node.as_node().value()._int;
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_float_value>(const zs::object& node) {
  _stream << node.as_node().value()._float;
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_bool_value>(const zs::object& node) {
  _stream << std::boolalpha << node.as_node().value()._bool;
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_string_value>(const zs::object& node) {
  zb::string_view s = node.as_node().value().get_string_unchecked();
  zb::string_view pre = node.as_node().value().is_small_string() ? "zs::_ss(" : "zs::_s(__eng, ";

  if (has_triple_quote(s)) {
    _stream << pre << triple_apostrophe(s) << ")";
    return {};
  }
  else if (has_special_chars(s)) {
    _stream << pre << triple_quote(s) << ")";
    return {};
  }

  _stream << pre << zb::quoted(s) << ")";

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_identifier>(const zs::object& node) {
  _stream << node.as_node().value().get_string_unchecked();
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_dot>(const zs::object& node) {
  zbase_assert(node.as_node().size() == 2);

  //  zs::error_result get(const object& obj, const object& key, object& dest);

  // eiwrite("if(zs::error_result err = __vm->get(");
  //  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  //  write(", ");
  //  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  //  write(", ");
  //  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  //  writee(")) {");
  //  iwrite<1>("return __vm.set_error(err, \"Invalid '+=' operation.\");\n");
  //  iwrite<-1>("}\n");

  write("__vm->get(");

  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[0]));
  _stream << ", ";
  ZS_RETURN_IF_ERROR(gen<ast_expr>(node.as_node()[1]));
  _stream << ")";

  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_doc_block>(const zs::object& node) {
  constexpr zb::string_view comment = "///";
  constexpr zb::string_view scomment = "/// ";

  zb::string_view s = node.as_node().value().get_string_unchecked();
  bool last_line_empty = false;

  _stream << "\n";

  size_t sz = s.size();

  for (size_t i = 0; i < sz;) {
    if (s[i] == '\n') {

      size_t line_sz = i + 1;
      if (zb::string_view line = zb::strip_trailing_spaces(s << i); !line or line[0] == '\n') {
        if (!last_line_empty) {
          last_line_empty = true;
          write(_indent, comment, line, "\n");
        }
      }
      else {
        write(_indent, scomment, line, "\n");
        last_line_empty = false;
      }

      s >>= line_sz;
      i = 0;
      sz -= line_sz;
    }
    else {
      ++i;
    }
  }

  if (s) {
    write(_indent, scomment, s, "\n");
  }
  return {};
}

template <>
zs::error_result cpp_compiler::gen<ast_expr>(const zs::object& node) {

  zs::ast_node_type type = zs::ast_node_name_to_type(node.as_node().name());

  //  if(type == ast_expr){
  //    return gen<ast_expr>(node.as_node().children()[0]);
  //  }
  switch (type) {
#define _X(name, str) \
  case zs::name:      \
    return gen<zs::name>(node);
    ZS_AST_NODE_TYPE_ENUM(_X)
#undef _X

    //  case zs::ast_variable_declaration:
    //    return gen<ast_variable_declaration>(node);
    //
    //  case zs::ast_integer_value:
    //    return gen<ast_integer_value>(node);
    //
    //  case zs::ast_float_value:
    //    return gen<ast_float_value>(node);
    //
    //  case zs::ast_bool_value:
    //    return gen<ast_bool_value>(node);
    //
    //  case zs::ast_string_value:
    //    return gen<ast_string_value>(node);
    //
    //  case zs::ast_identifier:
    //    return gen<ast_identifier>(node);
    //
    //  case zs::ast_eq_eq:
    //    return gen<ast_eq_eq>(node);
    //
    //  case zs::ast_not_eq:
    //    return gen<ast_not_eq>(node);
    //
    //  case zs::ast_add:
    //    return gen<ast_add>(node);
    //
    //  case zs::ast_sub:
    //    return gen<ast_sub>(node);
    //
    //    case zs::ast_mul:
    //      return gen<ast_mul>(node);
    //
    //
    //    case zs::ast_div:
    //      return gen<ast_div>(node);
    //
    //  case zs::ast_array:
    //    return gen<ast_array>(node);
    //
    //  case zs::ast_table:
    //    return gen<ast_table>(node);
    //
    //  case zs::ast_if:
    //    return gen<ast_if>(node);
    //
    //  case zs::ast_if_block:
    //    return gen<ast_if_block>(node);
    //
    //  case zs::ast_else_block:
    //    return gen<ast_else_block>(node);
    //
    //  case zs::ast_assignment:
    //    return gen<ast_assignment>(node);
    //
    //  case zs::ast_return:
    //    return gen<ast_return>(node);
    //
    //  case zs::ast_dot:
    //    return gen<ast_dot>(node);
    //
    //  case zs::ast_function_decl:
    //    return gen<ast_function_decl>(node);
    //
    //  case zs::ast_doc_block:
    //    return gen<ast_doc_block>(node);
    //
    //  case zs::ast_var_type:
    //    return gen<ast_var_type>(node);

  default:
    write(node._type, "\n");
    return zs::error_code::unimplemented;
  }

  return {};
}

// template <>
// zs::error_result cpp_compiler::gen<ast_statement>(const zs::object& node) {
//   size_t sz = node.as_node().size();
//   zbase_assert(sz >= 1);
//
//   if(sz == 1)
//   {
//     return gen<ast_expr>(node.as_node().children()[0]);
//   }else {
//     zbase_assert(node.as_node().children()[0].is_node() and node.as_node().children()[0].as_node().type()
//     == (int)ast_line); return gen<ast_expr>(node.as_node().children()[1]);
//   }
// }

cpp_compiler::cpp_compiler(zs::engine* eng, zs::parser* p)
    : engine_holder(eng)
    , _parser(p)
    , _filename(zs::allocator<char>(eng))
    , _stream(std::ios_base::out, zs::allocator<char>(eng))
    , _header_stream(std::ios_base::out, zs::allocator<char>(eng))
    , _pre_stream(std::ios_base::out, zs::allocator<char>(eng))
    , _post_stream(std::ios_base::out, zs::allocator<char>(eng)) {}

cpp_compiler::~cpp_compiler() {}

zs::error_result cpp_compiler::generate(zb::string_view filename, zb::string_view fct_name,
    zb::string_view namespc, std::initializer_list<zs::parameter_info> vars) {

  zb::vector<zs::parameter_info> vvars(vars);
  return generate(filename, fct_name, namespc, vvars);
}

zs::error_result cpp_compiler::generate(zb::string_view filename, zb::string_view fct_name,
    zb::string_view namespc, std::span<zs::parameter_info> vars) {

  _filename = filename;

  _header_stream << "// zscript auto generated.\n";
  _header_stream << "#pragma once\n\n";
  _header_stream << "#include <zscript/zscript.h>\n\n";

  _pre_stream << "#include \"" << _filename << ".h\"\n";
  _pre_stream << "#include \"zvirtual_machine.h\"\n";
  _pre_stream << "\n";

  if (namespc) {
    _pre_stream << "namespace " << namespc << " {\n";
    _header_stream << "namespace " << namespc << " {\n";
  }

  writee("zs::object ", fct_name, "(zs::vm_ref __vm, zs::parameter_list __params) {");
  _header_stream << "zs::object " << fct_name << "(zs::vm_ref vm, zs::parameter_list params);\n";

  ++_indent;

  const size_t vars_sz = vars.size();

  size_t min_req_args = 1;
  size_t first_optional_arg_index = -1;
  for (size_t i = 0; i < vars_sz; i++) {

    if (vars[i].optional) {
      first_optional_arg_index = i;
      break;
    }

    min_req_args++;
  }

  iwrite("ZBASE_MAYBE_UNUSED zs::engine* __eng = __vm->get_engine();\n\n");

  iwrite("const size_t nargs = __params.size();\n");
  iwrite("if (nargs < ", min_req_args, ") {\n");
  iwrite<1>("__vm.set_error(\"Invalid paramater count\");\n");
  iwrite("return zs::error_code::invalid_parameter_count;\n");
  iwrite<-1>("}\n\n");

  iwrite("ZBASE_MAYBE_UNUSED const zs::var& __this = __params[0];\n");
  //
  bool did_print_required_params = false;
  bool did_print_optional = false;
  for (size_t i = 0; i < vars_sz; i++) {
    const zs::parameter_info& vinfo = vars[i];
    if (i < first_optional_arg_index) {
      if (!did_print_required_params) {
        did_print_required_params = true;
        eiwrite("// Required parameters.");
      }
      eiwrite("zs::var ", vinfo.name.get_string_unchecked(), " = __params[", i + 1, "];");
    }
    else {

      if (!did_print_optional) {
        did_print_optional = true;

        if (did_print_required_params) {
          endl();
        }
        eiwrite("// Optional parameters.");
      }

      eiwrite("zs::var ", vinfo.name.get_string_unchecked(), " = __params.opt_get(", i + 1, ");");
    }
  }
  endl();

  for (size_t i = 0; i < vars_sz; i++) {
    const zs::parameter_info& vinfo = vars[i];
    if (uint32_t mask = static_cast<uint32_t>(vinfo.mask); mask != uint32_t(-1)) {

      eiwrite("if (");

      if (i >= first_optional_arg_index) {
        write("nargs > ", i + 1, " && ");
      }

      write("!", vinfo.name.get_string_unchecked(), ".is_type(", dsakjdkasl{ mask }, ")) {\n");

      iwrite<1>("__vm.set_error(\"Invalid paramater type for '", vinfo.name.get_string_unchecked(),
          "' at position '", i + 1, "'.\",\n");

      iwrite<2>("\"", zs::object_type_mask_printer{ mask }, " was required, got\", zs::get_object_type_name(",
          vinfo.name.get_string_unchecked(), ".get_type()), \".\");\n");

      iwrite<-2>("return zs::error_code::invalid_parameter_type;\n");
      iwrite<-1>("}\n");
    }
  }

  //

  const zs::object& root = _parser->root();
  for (const auto& node : root.as_node()) {
    ZS_RETURN_IF_ERROR(gen<ast_expr>(node));
  }

  if (root.as_node().has_children()
      and zs::ast_node_name_to_type(root.as_node().children().back().as_node().name())
          != ast_return_statement) {

    iwrite("return nullptr;\n");
  }

  _stream << "}\n";

  if (namespc) {
    _post_stream << "} // namespace " << namespc << ".\n";
    _header_stream << "} // namespace " << namespc << ".\n";
  }

  return {};
}

//
// zs::error_result cpp_compiler::generate(zb::string_view filename, zb::string_view fct_name,
//    zb::string_view namespc, std::span<zs::var_info> vars) {
//
//  _filename = filename;
//
//  _header_stream << "// zscript auto generated.\n";
//  _header_stream << "#pragma once\n\n";
//  _header_stream << "#include <zscript/zscript.h>\n\n";
//
//  _pre_stream << "#include \"" << _filename << ".h\"\n";
//  // _pre_stream << "\n";
//  _pre_stream << "#include \"zvirtual_machine.h\"\n";
//  _pre_stream << "\n";
//
//  if (namespc) {
//    _pre_stream << "namespace " << namespc << " {\n";
//    _header_stream << "namespace " << namespc << " {\n";
//  }
//
//  _stream << "zs::int_t " << fct_name << "(zs::vm_ref __vm) {\n";
//  _header_stream << "zs::int_t " << fct_name << "(zs::vm_ref vm);\n";
//
//  ++_indent;
//
//  const size_t vars_sz = vars.size();
//
//  size_t min_req_args = 1;
//  size_t first_optional_arg_index = -1;
//  for (size_t i = 0; i < vars_sz; i++) {
//
//    if (vars[i].optional) {
//      first_optional_arg_index = i;
//      break;
//    }
//
//    min_req_args++;
//  }
//
//  _stream << _indent << "ZBASE_MAYBE_UNUSED zs::engine* __eng = __vm->get_engine();\n\n";
//
//
//  _stream << _indent << "const zs::int_t nargs = __vm.stack_size();\n";
//  _stream << _indent << "if (__vm.stack_size() < " << min_req_args << ") {\n";
//
//  ++_indent;
//  _stream << _indent << "__vm.set_error(\"Invalid paramater count\");\n";
//  _stream << _indent << "return -1;\n";
//  --_indent;
//  _stream << _indent << "}\n\n";
//
//
//
//  _stream << _indent << "ZBASE_MAYBE_UNUSED zs::var& __this = __vm[0];\n";
//
//  for (size_t i = 0; i < vars_sz; i++) {
//    const zs::var_info& vinfo = vars[i];
//
//    if (i < first_optional_arg_index) {
//      _stream << "\n" << _indent << "zs::var " << vinfo.name.get_string_unchecked() << " = __vm[" << i + 1
//      << "];\n";
//
//      uint32_t mask = static_cast<uint32_t>(vinfo.mask);
//      if (mask != uint32_t(-1)) {
//        _stream << _indent << "if (!" << vinfo.name.get_string_unchecked() << ".is_type(" << dsakjdkasl{
//        mask }
//                << ")) {\n";
//        ++_indent;
//        _stream << _indent << "__vm.set_error(\"Invalid paramater type (" << i + 1 << ")\");\n";
//        _stream << _indent << "return -1;\n";
//        --_indent;
//        _stream << _indent << "}\n";
//      }
//    }
//    else if (uint32_t mask = static_cast<uint32_t>(vinfo.mask); mask != uint32_t(-1)) {
//      _stream << "\n" << _indent << "zs::var " << vinfo.name.get_string_unchecked() << ";\n";
//      _stream << _indent << "if (nargs >= " << i + 1 << ") {\n";
//      ++_indent;
//      _stream << _indent << vinfo.name.get_string_unchecked() << " = __vm[" << i + 1 << "];\n";
//
//      _stream << _indent << "if (!" << vinfo.name.get_string_unchecked() << ".is_type(" << dsakjdkasl{ mask
//      }
//              << ")) {\n";
//      ++_indent;
//      _stream << _indent << "__vm.set_error(\"Invalid paramater type (" << i + 1 << ")\");\n";
//
//      _stream << _indent << "return -1;\n";
//      --_indent;
//      _stream << _indent << "}\n";
//
//      --_indent;
//      _stream << _indent << "}\n";
//    }
//    else {
//      _stream << "\n"
//              << _indent << "zs::var " << vinfo.name.get_string_unchecked() << " = (nargs >= " << i + 1 <<
//              ") ? __vm[" << i + 1
//              << "] : nullptr;\n";
//    }
//  }
//
//  //
//
//  const zs::object& root = _parser->_root;
//  for (const auto& node : root) {
//    ZS_RETURN_IF_ERROR(gen<ast_expr>(node));
//  }
//
//  if (root.has_children() and root._children.back()._type != ast_return) {
//    _stream << _indent << "return 0;\n";
//  }
//
//  _stream << "}\n";
//
//  if (namespc) {
//    _post_stream << "} // namespace " << namespc << ".\n";
//    _header_stream << "} // namespace " << namespc << ".\n";
//  }
//
//  return {};
//}

zs::error_result cpp_compiler::export_code(const std::filesystem::path& directory) const {
  //  std::string_view generated_header_code = cpp_comp.header_stream().view();
  //  zb::print(generated_header_code);

  std::string generated_header_code = get_header_content();
  std::string generated_source_code = get_source_content();

  //  std::string_view generated_source_code = cpp_comp.stream().view();
  //  zb::print(generated_source_code);

  //  std::string formated_header_code = format_code(generated_header_code);
  //  zb::print(formated_header_code);
  //
  //  std::string formated_source_code = format_code(generated_source_code);
  //  zb::print(formated_source_code);

  {
    std::error_code ec;
    if (!std::filesystem::create_directories(directory, ec) && ec) {
      zb::print("Could not create output directory", zb::quoted(directory));
      return zs::error_code::invalid;
    }
  }

  std::ofstream source_file((directory / _filename).replace_extension(".cpp"));

  if (!source_file.is_open()) {
    return zs::error_code::open_file_error;
  }

  source_file << generated_source_code;
  source_file.close();

  std::ofstream header_file((directory / _filename).replace_extension(".h"));

  if (!header_file.is_open()) {
    return zs::error_code::open_file_error;
  }
  header_file << generated_header_code;
  //      file << formated_header_code;
  header_file.close();
  return {};
}

std::string cpp_compiler::get_header_content() const { return std::string(_header_stream.str()); }

std::string cpp_compiler::get_source_content() const {
  std::string content;
  content += _pre_stream.str();
  content += _stream.str();
  content += _post_stream.str();
  return content;
}
} // namespace zs.
