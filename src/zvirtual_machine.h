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

#include <zscript.h>
#include <zscript/object_stack.h>
#include "bytecode/zinstruction_vector.h"

namespace zs {

struct call_info {
  inline call_info() noexcept = default;

  template <class Object>
  ZS_INLINE call_info(Object&& clo, int_t stack_base, int_t top_index) noexcept
      : closure(std::forward<Object>(clo))
      , previous_stack_base(stack_base)
      , previous_top_index(top_index) {}

  template <class Object>
  ZS_INLINE call_info(Object&& clo, const zb::execution_stack_state& state) noexcept
      : closure(std::forward<Object>(clo))
      , previous_stack_base((int_t)state.base)
      , previous_top_index((int_t)state.top) {}

  inline call_info(const call_info&) noexcept = default;
  inline call_info(call_info&&) noexcept = default;
  inline call_info& operator=(const call_info&) noexcept = default;
  inline call_info& operator=(call_info&&) noexcept = default;

  zs::object closure;
  int_t previous_stack_base;
  int_t previous_top_index;

  friend std::ostream& operator<<(std::ostream& s, const call_info& ci);
};

/// Backend virtual machine.
class virtual_machine final : public engine_holder {
public:
  ZS_CLASS_COMMON;

  ZS_CHECK static virtual_machine* create(size_t stack_size, allocate_t alloc_cb = ZS_DEFAULT_ALLOCATE,
      raw_pointer_t user_pointer = nullptr, raw_pointer_release_hook_t user_release = nullptr,
      stream_getter_t stream_getter = ZS_DEFAULT_STREAM_GETTER,
      engine_initializer_t initializer = ZS_DEFAULT_ENGINE_INITIALIZER);

  ZS_CHECK static virtual_machine* create(zs::engine* eng, size_t stack_size);

  void push(const object& obj);
  void push(object&& obj);
  void push_null();
  void push_root();
  void pop();
  void pop(int_t n);
  void remove(int_t n);
  void swap(int_t n1, int_t n2);

  ZS_CHECK object& get_up(int_t n);
  ZS_CHECK object& get_at(int_t n);

  ZS_CHECK object pop_get();

  ZS_CK_INLINE int_t stack_size() const noexcept { return _stack.stack_size(); }

  ZS_CHECK object& top() noexcept;
  ZS_CHECK const object& top() const noexcept;

  ZS_CHECK object& get_imported_module_cache() noexcept;

  ZS_CHECK zs::error_result call(const object& closure, int_t n_params, int_t stack_base, object& ret_value,
      bool stack_base_relative = true);

  ZS_CHECK zs::error_result call(const object& closure, std::span<const object> params, object& ret_value);

  ZS_CK_INLINE zs::error_result call(
      const object& closure, std::initializer_list<const object> params, object& ret_value) {
    std::span<const object> sparams(std::data(params), params.size());
    return call(closure, sparams, ret_value);
  }

  ZS_CK_INLINE zs::error_result call_from_top(const object& closure, int_t n_params, object& ret_value) {
    return call(closure, n_params, _stack.stack_size() - n_params, ret_value);
  }

  ZS_CHECK zs::optional_result<zs::object> call_from_top_opt(const object& closure, int_t n_params = 0);

  ZS_CHECK zs::error_result call_from_top(const object& closure, int_t n_params = 0);

  ZS_CK_INLINE object& stack_get(int_t idx) { return _stack[idx]; }
  ZS_CK_INLINE const object& stack_get(int_t idx) const { return _stack[idx]; }

  ZS_CK_INLINE object& operator[](int_t idx) noexcept { return _stack[idx]; }
  ZS_CK_INLINE const object& operator[](int_t idx) const noexcept { return _stack[idx]; }

  ZS_CHECK zs::error_result get(const object& obj, const object& key, object& dest);
  ZS_CHECK zs::error_result set(object& obj, const object& key, const object& value);

  ZS_CK_INLINE zs::error_result set(const object& obj, const object& key, const object& value) {
    return set(obj, key, value);
  }

  /// Get the typeof of obj.
  ZS_CHECK zs::error_result type_of(const object& obj, object& dest);

  /// @{
  ZS_CHECK zs::error_result add(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result sub(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result mul(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result div(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result mod(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result exp(object& target, const object& lhs, const object& rhs);

  ZS_CHECK zs::error_result bitwise_or(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result bitwise_and(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result bitwise_xor(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result lshift(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result rshift(object& target, const object& lhs, const object& rhs);

  ZS_CHECK zs::error_result add_eq(object& target, const object& rhs);
  ZS_CHECK zs::error_result sub_eq(object& target, const object& rhs);
  ZS_CHECK zs::error_result mul_eq(object& target, const object& rhs);
  ZS_CHECK zs::error_result div_eq(object& target, const object& rhs);
  ZS_CHECK zs::error_result mod_eq(object& target, const object& rhs);
  ZS_CHECK zs::error_result exp_eq(object& target, const object& rhs);
  /// @}

  //
  // Unsafe.
  //

  ZS_CHECK object get(const object& obj, const object& key);

  ZS_CHECK object add(const object& lhs, const object& rhs);
  ZS_CHECK object sub(const object& lhs, const object& rhs);
  ZS_CHECK object mul(const object& lhs, const object& rhs);
  ZS_CHECK object div(const object& lhs, const object& rhs);
  ZS_CHECK object mod(const object& lhs, const object& rhs);
  ZS_CHECK object exp(const object& lhs, const object& rhs);

  //
  //
  //

  ZS_CK_INLINE zs::object create_this_table_from_root() {
    return zs::object::create_table_with_delegate(_engine, _root_table);
  }

  /// Compile a code buffer.
  /// On success, the `closure_result` is a closure object.
  ZS_CHECK zs::error_result compile_buffer(std::string_view content, std::string_view source_name,
      zs::object& closure_result, bool with_vargs = false);

  /// Compile a code buffer.
  /// On success, the `closure_result` is a closure object.
  /// @{
  ZS_CHECK zs::error_result compile_file(const char* filepath, std::string_view source_name,
      zs::object& closure_result, bool with_vargs = false);

  ZS_CK_INLINE zs::error_result compile_file(const std::filesystem::path& filepath,
      std::string_view source_name, zs::object& closure_result, bool with_vargs = false) {
    return compile_file(filepath.c_str(), source_name, closure_result, with_vargs);
  }
  /// @}
  ///
  ///

  ZS_CHECK zs::error_result call_file(const std::filesystem::path& filepath, std::string_view source_name,
      zs::object& ret_value, bool with_vargs = false);

  ZS_CHECK zs::error_result call_buffer(
      std::string_view content, std::string_view source_name, zs::object& ret_value, bool with_vargs = false);

  ZS_CHECK zs::error_result call_buffer(std::string_view content, std::string_view source_name,
      zs::object& this_table, zs::object& ret_value, std::span<const object> args, bool with_vargs = false);

  ///
  ZS_CHECK zs::error_result load_buffer_as_value(
      std::string_view content, std::string_view source_name, zs::object& value);

  ///
  ZS_CHECK zs::error_result load_buffer_as_array(
      std::string_view content, std::string_view source_name, zs::object& value, std::string_view sep = ",");

  ///
  ZS_CHECK zs::error_result load_file_as_value(
      const char* filepath, std::string_view source_name, zs::object& result_value);

  ZS_CK_INLINE zs::error_result load_file_as_value(
      const std::filesystem::path& filepath, std::string_view source_name, zs::object& result_value) {
    return load_file_as_value(filepath.c_str(), source_name, result_value);
  }

  ///
  ZS_CHECK zs::error_result load_file_as_array(const char* filepath, std::string_view source_name,
      zs::object& result_value, std::string_view sep = ",");

  ZS_CK_INLINE zs::error_result load_file_as_array(const std::filesystem::path& filepath,
      std::string_view source_name, zs::object& result_value, std::string_view sep = ",") {
    return load_file_as_array(filepath.c_str(), source_name, result_value, sep);
  }

  ZS_CHECK zs::error_result load_json_table(std::string_view content, const object& table, object& output);

  ZS_CHECK zs::error_result load_json_file(const char* filepath, const object& table, object& output);

  ZS_CK_INLINE zs::error_result load_json_file(const char* filepath, object& output) {
    return load_json_file(filepath, nullptr, output);
  }

  ZS_CHECK zs::error_result load_json_file(std::string_view filepath, const object& table, object& output);

  ZS_CK_INLINE zs::error_result load_json_file(std::string_view filepath, object& output) {
    return load_json_file(filepath, nullptr, output);
  }

  ///
  ZS_CK_INLINE zs::object_stack& get_stack() noexcept { return _stack; }
  ZS_CK_INLINE zs::object_stack& stack() noexcept { return _stack; }

  /// Get the root table.
  /// @{
  ZS_CK_INLINE object& get_root() noexcept { return _root_table; }
  ZS_CK_INLINE const object& get_root() const noexcept { return _root_table; }
  /// @}

  ZS_CK_INLINE const zs::string& get_error() const noexcept { return _error_message; }
  ZS_CK_INLINE const zs::vector<call_info>& get_call_stack() const noexcept { return _call_stack; }

  // An enum, is an empty table with the data in it's delegate.
  ZS_CHECK zs::error_result make_enum_table(object& obj);

  template <class... Args>
  inline void set_error(Args&&... args) {
    set_error(std::string_view(zs::strprint(_engine, std::forward<Args>(args)...)));
  }

  inline void set_error(std::string_view s) { _error_message += s; }

private:
  using enum opcode;
  friend class vm_ref;

  object _root_table;
  object _imported_module_cache;

  object _last_error;
  object _error_handler;
  zs::object_stack _stack;
  zs::vector<call_info> _call_stack;
  zs::string _error_message;
  zs::vector<object> _constexpr_variables;

  object _default_array_delegate;
  object _default_native_array_delegate;
  object _default_table_delegate;
  object _default_string_delegate;
  object _default_color_delegate;
  object _default_array_iterator_delegate;
  object _default_table_iterator_delegate;
  object _default_struct_delegate;

  bool _owns_engine;

  //  struct helper;

  virtual_machine(zs::engine* eng, size_t stack_size, bool owns_engine);
  ~virtual_machine() = default;

  zs::error_result init();
  void release() noexcept;

  friend void close_virtual_machine(virtual_machine* v);

  struct executor;

  struct exec_op_data_t {
    closure_object* closure;
    zs::function_prototype_object* fct;
    object& ret_value;
    zs::instruction_vector& instructions() const noexcept;
  };

  template <opcode Op>
  zs::error_code exec_op_wrapper(zs::instruction_iterator& it, exec_op_data_t& op_data);

  template <opcode Op>
  zs::error_code exec_op(zs::instruction_iterator& it, exec_op_data_t& op_data);

  template <auto Code, class... Args>
  ZS_CHECK zs::error_result runtime_action(Args... args);

  template <exposed_object_type LType, exposed_object_type RType>
  zs::error_result arithmetic_operation(
      enum arithmetic_op op, object& target, const object& lhs, const object& rhs);

  template <exposed_object_type Type>
  zs::error_result arithmetic_operation(enum arithmetic_uop op, object& target, object& src);

  zs::error_result runtime_arith_operation(arithmetic_uop op, object& target, object& src);

  zs::error_result runtime_arith_operation(
      arithmetic_op op, object& target, const object& lhs, const object& rhs);
};

} // namespace zs.
