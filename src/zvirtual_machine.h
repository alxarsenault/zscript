// MIT License
//
// Copyright (c) 2024 Alexandre Arsenault
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <zscript/zscript.h>
#include "vm/object_stack.h"
#include "bytecode/zinstruction_vector.h"

namespace zs {

inline constexpr object k_imported_modules_name = zs::_sv("__imported_modules__");
inline constexpr object k_module_loaders_name = zs::_sv("__module_loaders__");
inline constexpr object k_number_delegate_name = zs::_sv("__number_delegate__");
inline constexpr object k_function_delegate_name = zs::_sv("__function_delegate__");
inline constexpr object k_array_delegate_name = zs::_sv("__array_delegate__");
inline constexpr object k_table_delegate_name = zs::_sv("__table_delegate__");
inline constexpr object k_string_delegate_name = zs::_sv("__string_delegate__");
inline constexpr object k_struct_delegate_name = zs::_sv("__struct_delegate__");

inline constexpr object k_delegated_atom_delegates_table_name = zs::_sv("__delegated_atom_delegates_table__");

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

#define ZS_VM_ERROR(err, ...) handle_error(err, { -1, -1 }, ZS_DEVELOPER_SOURCE_LOCATION(), __VA_ARGS__)

/// Backend virtual machine.
class virtual_machine final : public engine_holder {
public:
  ZS_CLASS_COMMON;

  ZS_CHECK static virtual_machine* create(size_t stack_size, allocate_t alloc_cb = ZS_DEFAULT_ALLOCATE,
      raw_pointer_t user_pointer = nullptr, raw_pointer_release_hook_t user_release = nullptr,
      stream_getter_t stream_getter = ZS_DEFAULT_STREAM_GETTER,
      engine_initializer_t initializer = ZS_DEFAULT_ENGINE_INITIALIZER);

  ZS_CHECK static virtual_machine* create(zs::engine* eng, size_t stack_size);
  ZS_CHECK static virtual_machine* create(virtual_machine* vm, size_t stack_size, bool same_global);

  //
  // MARK: Stack.
  //

  void push(const object& obj);
  void push(object&& obj);
  void push_null();
  void push_root();
  void pop();
  void pop(int_t n);
  void remove(int_t n);
  void swap(int_t n1, int_t n2);
  
  ZS_CK_INLINE int_t stack_size() const noexcept { return _stack.stack_size(); }

   ZS_CHECK const object& top() const noexcept;

  ZS_CK_INLINE object& stack_get(int_t idx) { return _stack[idx]; }
  ZS_CK_INLINE const object& stack_get(int_t idx) const { return _stack[idx]; }
 
  ///
   ZS_CK_INLINE zs::object_stack& stack() noexcept { return _stack; }

  /// Get the root table.
  /// @{
   ZS_CK_INLINE const object& get_root() const noexcept { return _global_table; }
  /// @}

  /// Get the global table.
  /// @{
   ZS_CK_INLINE const object& get_global() const noexcept { return _global_table; }
  /// @}

  //
  //
  //

  ZS_CHECK object& get_imported_modules() noexcept;
  ZS_CHECK object& get_module_loaders() noexcept;

  //
  // MARK: Call.
  //
  ZS_CHECK zs::error_result pcall(const object& closure, int_t n_params, int_t stack_base, object& ret_value,
      bool stack_base_relative = true);

  ZS_CHECK zs::error_result pcall(const object& closure, zs::parameter_list params, object& ret_value);

  ZS_CK_INLINE zs::error_result pcall(
      const object& closure, std::initializer_list<const object> params, object& ret_value) {
    zs::parameter_list sparams(std::data(params), params.size());
    return pcall(closure, sparams, ret_value);
  }

  ZS_CHECK zs::error_result call(const object& closure, int_t n_params, int_t stack_base, object& ret_value,
      bool stack_base_relative = true);

  ZS_CHECK zs::error_result call(const object& closure, zs::parameter_list params, object& ret_value);

  ZS_CK_INLINE zs::error_result call(
      const object& closure, std::initializer_list<const object> params, object& ret_value) {
    zs::parameter_list sparams(std::data(params), params.size());
    return call(closure, sparams, ret_value);
  }

  ZS_CK_INLINE zs::error_result call(const object& closure, const object& param, object& ret_value) {
    zs::parameter_list sparams(&param, 1);
    return call(closure, sparams, ret_value);
  }

  ZS_CK_INLINE zs::error_result call_from_top(const object& closure, int_t n_params, object& ret_value) {
    return call(closure, n_params, _stack.stack_size() - n_params, ret_value);
  }

  ZS_CHECK zs::optional_result<zs::object> call_from_top_opt(const object& closure, int_t n_params = 0);

  ZS_CHECK zs::error_result call_from_top(const object& closure, int_t n_params = 0);

  //
  // MARK: Get/Set.
  //

  ZS_CHECK zs::error_result get(const object& obj, const object& key, object& dest);
  ZS_CHECK zs::error_result raw_get(const object& obj, const object& key, object& dest);
  ZS_CHECK zs::error_result set(object& obj, const object& key, const object& value);
  ZS_CHECK zs::error_result set_if_exists(object& obj, const object& key, const object& value);
  ZS_CHECK zs::error_result contains(const object& obj, const object& key, object& dest);
  ZS_CHECK zs::error_result raw_contains(const object& obj, const object& key, object& dest);
 
  ZS_CHECK zs::error_result compare(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result lt(object& target, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result eq(object& target, const object& lhs, const object& rhs);

  /// Get the typeof of obj.
  ZS_CHECK zs::error_result type_of(const object& obj, object& dest);
  ZS_CHECK zs::error_result to_string(const object& obj, object& dest);
  ZS_CHECK zs::error_result copy(const object& obj, object& dest);
  ZS_CHECK zs::object get_delegate(const object& obj);

  /// @{
  ZS_CHECK zs::error_result add(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result sub(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result mul(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result div(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result mod(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result exp(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result lshift(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result rshift(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result bitwise_or(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result bitwise_and(object& dest, const object& lhs, const object& rhs);
  ZS_CHECK zs::error_result bitwise_xor(object& dest, const object& lhs, const object& rhs);

  ZS_CHECK zs::error_result add_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result sub_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result mul_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result div_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result mod_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result exp_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result lshift_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result rshift_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result bitwise_or_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result bitwise_and_eq(object& obj, const object& rhs);
  ZS_CHECK zs::error_result bitwise_xor_eq(object& obj, const object& rhs);
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

  /// Compile a code buffer.
  /// On success, the `closure_result` is a closure object.

  ZS_CHECK zs::error_result compile_buffer(
      std::string_view content, object&& source_name, zs::object& closure_result, bool with_vargs = false);

  ZS_CHECK zs::error_result compile_buffer(std::string_view content, const object& source_name,
      zs::object& closure_result, bool with_vargs = false);

  template <class String>
    requires zb::is_string_view_convertible_v<String>
  ZS_CK_INLINE zs::error_result compile_buffer(
      std::string_view content, String&& source_name, zs::object& closure_result, bool with_vargs = false) {
    return compile_buffer(content, zs::_s(_engine, source_name), closure_result, with_vargs);
  }

  /// Compile a code buffer.
  /// On success, the `closure_result` is a closure object.
  /// @{
  ZS_CHECK zs::error_result compile_file(const char* filepath, std::string_view source_name,
      zs::object& closure_result, bool with_vargs = false);

  ZS_CHECK zs::error_result compile_file(const zs::object& filepath, std::string_view source_name,
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
 
  ZS_CHECK zs::string get_error() const noexcept;

  ZS_CK_INLINE const zs::vector<call_info>& get_call_stack() const noexcept { return _call_stack; }

  // An enum, is an empty table with the data in it's delegate.
  ZS_CHECK zs::error_result make_enum_table(object& obj);

  template <class... Args>
  inline void set_error(Args&&... args) {
    set_error(std::string_view(zs::strprint(_engine, std::forward<Args>(args)...)));
  }

  inline void set_error(std::string_view msg) { 
    handle_error(zs::errc::unknown, { -1, -1 }, msg, ZS_DEVELOPER_SOURCE_LOCATION());
  }

  zs::error_result handle_error(zs::error_code ec, const zs::line_info& linfo, std::string_view msg,
      const zs::developer_source_location& loc);

  template <class... Args>
  inline zs::error_result handle_error(zs::error_code ec, const zs::line_info& linfo,
      const zs::developer_source_location& loc, const Args&... args) {
    if constexpr (sizeof...(Args) == 1
        and zb::is_string_view_convertible_v<std::tuple_element_t<0, std::tuple<Args...>>>) {
      return handle_error(ec, linfo, std::string_view(args...), loc);
    }
    else {
      return handle_error(ec, linfo, std::string_view(zs::strprint(_engine, args...)), loc);
    }
  }

  inline table_object& globals() const noexcept { return _global_table.as_table(); }

  const object& get_default_table_delegate() const noexcept;
  const object& get_default_number_delegate() const;
  const object& get_default_array_delegate() const;
  const object& get_default_string_delegate() const;

  const object& get_default_struct_delegate() const;
  const object& get_default_function_delegate() const;
  const object& get_delegated_atom_delegates_table() const;

  object get_default_delegate_for_type(object_type t) const;

private:
  using enum opcode;
  using enum exposed_object_type;
  using enum object_type;
  using enum arithmetic_uop;
  friend class vm_ref;

  object _global_table;
  zs::object_stack _stack;
  zs::vector<call_info> _call_stack;
  zs::vector<object> _open_captures;

  zs::string _error_message;
  zs::error_stack _errors;

  bool _owns_engine;

  virtual_machine(zs::engine* eng, size_t stack_size, bool owns_engine);
  virtual_machine(virtual_machine* vm, size_t stack_size, bool same_global);
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

    int_t get_iterator_index(const zs::instruction_vector::iterator& it) const noexcept;
    zs::instruction_vector::iterator get_instruction(size_t index) const noexcept;
  };

  template <opcode Op>
  zs::error_code exec_op_wrapper(zs::instruction_iterator& it, exec_op_data_t& op_data);

  template <opcode Op>
  zs::error_code exec_op(zs::instruction_iterator& it, exec_op_data_t& op_data);

  struct proxy;
  struct runtime_table_proxy;

  template <auto Code, class... Args>
  ZS_CHECK zs::error_result runtime_action(Args... args);

  zs::error_result delegate_table_contains(
      const object& obj, const object& key, const object& delegate, object& dest, bool use_meta_get = true);

  zs::error_result call_closure(const object& cobj, int_t n_params, int_t absolute_stack_base,
      object& ret_value, bool is_pcall = false);

  //
  // MARK: Arithmetics.
  //

  object start_delegate_chain(const object& obj);

  zs::error_result arithmetic_operation(arithmetic_op op, object& target, const object lhs, const object rhs);

  zs::error_result unary_arithmetic_operation(arithmetic_uop uop, object& target, object& src);

  error_result unary_delegate_operation(meta_method mt, const object src, object& dest,
      uint32_t valid_objects_mask = 0, error_code error_code_if_not_found = errc::invalid_operation);

  error_result binary_delegate_operation(meta_method mt, const object lhs, const object rhs, object& dest);

  error_result find_meta_method(meta_method mt, const object& obj, object& delegate_out, object& dest,
      meta_method alt = meta_method::mt_none);
};

} // namespace zs.
