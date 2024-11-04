// Copyright(c) 2024, Meta-Sonic.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.  See the file COPYING included with
// this distribution for more information.
//
// Alternatively, if you have a valid commercial licence for aulib obtained
// by agreement with the copyright holders, you may redistribute and/or modify
// it under the terms described in that licence.
//
// If you wish to distribute code using aulib under terms other than those of
// the GNU General Public License, you must obtain a valid commercial licence
// before doing so.

#pragma once

#include <zscript/zscript.h>
#include "lang/zlexer.h"

namespace zs {

#define ZS_AST_NODE_TYPE_ENUM_VALUE(X, name) X(ast_##name, #name)
#define ZS_AST_NODE_TYPE_ENUM(X)                        \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, empty)                 \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, line)                  \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, root)                  \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, integer_value)         \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, float_value)           \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, bool_value)            \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, null_value)            \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, string_value)          \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, variable_declaration)  \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, parameter_declaration) \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, identifier)            \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, table_declaration)     \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, new_table_field)       \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, dot)                   \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, assignment)            \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, set)                   \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, function_declaration)  \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_add)                \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_sub)                \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_mul)                \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_div)                \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_mod)                \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, eq_eq)                 \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, not_eq)                \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_add_eq)             \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_sub_eq)             \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_mul_eq)             \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_div_eq)             \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_mod_eq)             \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, op_exp_eq)             \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, if_statement)          \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, jump)                  \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, if_block)              \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, else_block)            \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, return_statement)      \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, doc_block)             \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, array_declaration)     \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, array_element)         \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, var_type)              \
  ZS_AST_NODE_TYPE_ENUM_VALUE(X, expr)

enum class ast_node_type {
#define _X(name, str) name,
  ZS_AST_NODE_TYPE_ENUM(_X)
#undef _X
};

/// Get the ast node type name.
ZB_CK_INLINE_CXPR const char* ast_node_name(ast_node_type t) noexcept {
  switch (t) {
#define _X(name, str)           \
  case zs::ast_node_type::name: \
    return str;
    ZS_AST_NODE_TYPE_ENUM(_X)
#undef _X
  }

  zbase_error("invalid type");
  return "unknown";
}

/// Get the ast node type name.
ZB_CK_INLINE ast_node_type ast_node_name_to_type(std::string_view node) noexcept {

#define _X(name, str)               \
  if (node == str) {                \
    return zs::ast_node_type::name; \
  }
  ZS_AST_NODE_TYPE_ENUM(_X)
#undef _X

  zbase_error("invalid type");
  return zs::ast_node_type::ast_empty;
}

ZB_CK_INLINE ast_node_type ast_node_name_to_type(const object& node_name) noexcept {
  return ast_node_name_to_type(node_name.get_string_unchecked());
}

ZB_CHECK ZB_INLINE constexpr bool is_leaf_node(ast_node_type n) noexcept {
  using enum ast_node_type;

  switch (n) {
  case ast_empty:
  case ast_integer_value:
  case ast_float_value:
  case ast_bool_value:
  case ast_null_value:
  case ast_string_value:
  case ast_identifier:
  case ast_doc_block:
    return true;

  default:
    return false;
  }
}

class ast_node : public object {
public:
  using object::object;

  inline ast_node(object&& obj)
      : object(std::move(obj)) {}

  inline ast_node(zs::engine* eng, ast_node_type ntype = ast_node_type::ast_empty,
      const object& value = zs::object::create_none())
      : ast_node(init_tag{}, eng, ntype, value) {}

  inline ast_node(zs::engine* eng, ast_node_type ntype, object&& value)
      : ast_node(init_tag{}, eng, ntype, std::move(value)) {}

  inline zs::error_result add_child(const object& obj) noexcept { return as_node().add_child(obj); }
  inline zs::error_result add_child(object&& obj) noexcept { return as_node().add_child(std::move(obj)); }

  template <class... Children>
  inline void add_children(Children&&... children) {
    (as_node().add_child(std::forward<Children>(children)), ...);
  }

  template <class... Children>
  inline ast_node& with_children(Children&&... children) {
    (as_node().add_child(std::forward<Children>(children)), ...);
    return *this;
  }

  void add_attribute(const object& key, const object& value) noexcept { as_node().add_attribute(key, value); }

  void add_attribute(std::string_view key, const object& value) noexcept {
    as_node().add_attribute(key, value);
  }

  void add_attribute(const char* key, const object& value) noexcept { as_node().add_attribute(key, value); }

  ast_node& with_attribute(const object& key, const object& value) noexcept {
    as_node().add_attribute(key, value);
    return *this;
  }

  ast_node& with_attribute(std::string_view key, const object& value) noexcept {
    as_node().add_attribute(key, value);
    return *this;
  }

  ast_node& with_attribute(const char* key, const object& value) noexcept {
    as_node().add_attribute(key, value);
    return *this;
  }

  inline void add_attributes(std::initializer_list<node_object::attribute> atts) {
    for (const auto& a : atts) {
      as_node().attributes().push_back(a);
    }
  }

  inline ast_node& with_attributes(std::initializer_list<node_object::attribute> atts) {
    for (const auto& a : atts) {
      as_node().attributes().push_back(a);
    }
    return *this;
  }

private:
  struct init_tag {};

  static inline int_t generate_id() noexcept {
    static int_t __id = 0;
    return __id++;
  }

  template <class Obj>
  inline ast_node(init_tag, zs::engine* eng, ast_node_type ntype, Obj&& value)
      : object(zs::object::create_node(eng, zs::_s(eng, ast_node_name(ntype)), std::forward<Obj>(value))) {
    as_node().attributes().emplace_back(zs::_ss("id"), generate_id());
  }
};

class ast_node_walker {
public:
  inline ast_node_walker(const object& head) noexcept
      : _head(head) {}

  zs::string serialize(zs::engine* eng) const {
    zs::ostringstream stream(zs::create_string_stream(eng));
    ast_node_walker::serialize_sub_tree(_head, stream);
    return stream.str();
  }

private:
  static void serialize_sub_tree(const object& node, zs::ostringstream& stream, int indent = 0) {

    const node_object& nobj = node.as_node();

    zb::stream_print(stream, zb::sfill_t("· ", indent * 2), "[", nobj.get_opt_attribute("id", -1), "] ",
        "ast_", nobj.name().get_string_unchecked());

    //    if (nobj.value().is_none()) {
    //      zb::stream_print(stream, "\n");
    //    }
    //    else {
    //      zb::stream_print(stream, " ", nobj.value().convert_to_string(), "\n");
    //    }
    //
    //    if(nobj.has_attributes()) {
    //      for(const auto& att: nobj.attributes()) {
    //        zb::stream_print(stream, " < ", att.name.get_string_unchecked(), " = ", att.value, ">\n");
    //      }
    //    }

    if (!nobj.value().is_none()) {
      zb::stream_print(stream, " ", nobj.value().convert_to_string());
    }

    if (nobj.has_attributes()) {
      zb::stream_print(stream, " <");
      for (const auto& att : nobj.attributes()) {
        zb::stream_print(stream, att.name.get_string_unchecked(), "=", att.value);
      }
      zb::stream_print(stream, ">");
    }

    zb::stream_print(stream, "\n");

    for (const auto& child : nobj) {
      serialize_sub_tree(child, stream, indent + 1);
    }
  }

  const object& _head;
};

///
class parser : public engine_holder {
public:
  parser(zs::engine* eng);

  ~parser();

  zs::error_result parse(std::string_view content, std::string_view filename, object& output,
      zs::token_type* prepended_token = nullptr);

  //
  // MARK: Parser
  //

  ZB_CHECK zs::error_code expect(token_type tok) noexcept;
  ZB_CHECK zs::error_code expect_get(token_type tok, object& ret);
  ZB_CHECK zs::error_code expect_get_identifier(object& ret);

  ZB_CHECK inline const zs::string& get_error() const noexcept { return _error_message; }

  ZB_CHECK inline bool is_end_of_statement() const noexcept {
    using enum token_type;
    return (is_tok(_lexer->last_token(), tok_endl, tok_doc_block))
        || is(tok_eof, tok_rcrlbracket, tok_semi_colon);
  }

  //
  // MARK: Lexer
  //

  token_type lex();

  //
  // MARK: Token helpers
  //

  ZB_CHECK ZB_INLINE bool is(token_type t) const noexcept { return _token == t; }

  template <class... Tokens>
  ZB_CHECK ZB_INLINE bool is(Tokens... tokens) const noexcept {
    return zb::is_one_of(_token, tokens...);
  }

  template <class... Tokens>
  ZB_CHECK static ZB_INLINE bool is_tok(token_type t, Tokens... tokens) noexcept {
    return zb::is_one_of(t, tokens...);
  }

  template <class... Tokens>
  ZB_CHECK ZB_INLINE bool is_not(Tokens... tokens) const noexcept {
    return !zb::is_one_of(_token, tokens...);
  }

  ZB_INLINE void push_node(const object& node) noexcept { _stack.push_back(node); }

  ZB_INLINE void push_node(object&& node) noexcept { _stack.push_back(std::move(node)); }

  template <class... Args>
  ZB_INLINE void push_node(ast_node_type ntype) noexcept {
    _stack.push_back(ast_node(_engine, ntype));
  }

  template <class... Args>
  ZB_INLINE void push_node(ast_node_type ntype, const object& value) noexcept {
    _stack.push_back(ast_node(_engine, ntype, value));
  }

  template <class... Args>
  ZB_INLINE void push_node(ast_node_type ntype, object&& value) noexcept {
    _stack.push_back(ast_node(_engine, ntype, std::move(value)));
  }

  ZB_INLINE void push_arith_expr_node(ast_node_type ntype) noexcept {
    ast_node expr(_engine, ntype);
    _stack(-2).as_node().add_attribute("role", zs::_ss("lhs"));
    _stack(-1).as_node().add_attribute("role", zs::_ss("rhs"));
    expr.add_children(std::move(_stack(-2)), std::move(_stack(-1)));
    _stack.pop_back();
    _stack.pop_back();
    _stack.push_back(std::move(expr));
  }

  ZB_INLINE ast_node get_arith_expr_node(ast_node_type ntype) noexcept {
    ast_node expr(_engine, ntype);
    expr.add_children(std::move(_stack(-2)), std::move(_stack(-1)));
    _stack.pop_back();
    _stack.pop_back();
    return expr;
  }

  ZB_CHECK ZB_NEVER_INLINE ast_node get_pop_back() noexcept;

  ZB_CK_INLINE zs::node_object& top() { return _stack.back().as_node(); }

  ZB_CK_INLINE zs::object& root() { return _root; }
  ZB_CK_INLINE const zs::object& root() const { return _root; }

  ZB_CK_INLINE zs::vector<zs::object>& stack() { return _stack; }
  ZB_CK_INLINE const zs::vector<zs::object>& stack() const { return _stack; }

  ZB_INLINE void add_child_to_top(object&& node) { top().add_child(std::move(node)); }

  ZB_NEVER_INLINE void add_top_to_previous();

  //
  struct helper;

  using enum token_type;
  enum class parse_op : uint8_t;

private:
  //
  // MARK: Members
  //

  zs::lexer* _lexer = nullptr;
  zs::token_type _token = token_type::tok_none;
  zs::string _error_message;
  zs::object _root;
  zs::vector<zs::object> _stack;
  zs::string _filename;
  const char* _last_ptr = nullptr;
  bool _output_lines = true;

  template <parse_op Op, class... Args>
  ZB_CHECK zs::error_result parse(Args... args);

  zs::error_result parse_include_or_import_statement(token_type tok);
};
} // namespace zs.
