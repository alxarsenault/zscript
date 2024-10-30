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

#include "lang/zinstruction_vector.h"

namespace zs {

// The custom type_mask value is a uint64_t.
inline constexpr size_t k_max_restricted_type_check = 64;

class function_prototype_object;

struct variable_type_info_t {
  inline variable_type_info_t() noexcept = default;

  inline variable_type_info_t(int_t idx, uint32_t _mask = 0, uint64_t _custom_mask = 0, bool isconst = false)
      : index(idx)
      , mask{ _mask }
      , custom_mask(_custom_mask)
      , is_const(isconst) {}

  int_t index;
  uint32_t mask = 0;
  uint64_t custom_mask = 0;
  bool is_const = false;
};

struct line_info_op_t {
  int_t line;
  int_t column;
  int_t op_index;
  opcode op;

  inline friend std::ostream& operator<<(std::ostream& stream, const line_info_op_t& linfo) {
    return zb::stream_print<"">(stream, "at line", linfo.line, " column ", linfo.column, " op ", linfo.op,
        " op_index ", linfo.op_index);
  }
};

struct local_var_info_t {
  inline local_var_info_t() = default;

  object _name;
  uint_t _start_op = 0;
  uint_t _end_op = 0;
  uint_t _pos = 0;
  uint32_t _mask = 0;
  uint64_t _custom_mask = 0;
  bool _is_const = false;
};

inline std::ostream& operator<<(std::ostream& s, const local_var_info_t& vinfo) {
  s << vinfo._name.convert_to_string();
  return s;
}

struct captured_variable {
  enum class type_t { local, outer };
  using enum type_t;

  captured_variable() = default;
  captured_variable(const captured_variable&) = default;
  captured_variable(captured_variable&&) = default;

  inline captured_variable(const object& capture_name, int_t capture_src, type_t capture_type)
      : type(capture_type)
      , name(capture_name)
      , src(capture_src) {}

  captured_variable& operator=(const captured_variable&) = default;
  captured_variable& operator=(captured_variable&&) = default;

  type_t type;
  object name;
  int_t src;
};

/// Used by the compiler when parsing/compiling a function.
/// The end result is a `function_prototype_object` that can be created by
/// calling `build_function_prototype()`.
class closure_compile_state : public engine_holder {
public:
  closure_compile_state(zs::engine* eng, closure_compile_state* parent = nullptr);

  ~closure_compile_state();

  /// Name of the closure object (i.e. the function name).
  zs::object name;

  /// The name or the path of the source file.
  zs::object source_name;

  /// Add a parameter.
  /// @{
  ZB_CHECK zs::error_result add_parameter(const object& param_name);

  ZB_CHECK zs::error_result add_parameter(
      const object& param_name, uint32_t mask, uint64_t custom_mask, bool is_const = false);
  /// @}

  inline void add_default_param(int_t target) { _default_params.push_back(target); }

  /// Add an instruction to the instruction vector.
  /// @{
  template <opcode Op, class... Args>
  ZB_INLINE void add_instruction(Args... args) {
    _last_instruction_index = _instructions._data.size();
    _instructions.push<Op>(std::forward<Args>(args)...);
  }

  template <class Instruction>
  ZB_INLINE void add_instruction(const Instruction& inst) {
    _last_instruction_index = _instructions._data.size();
    _instructions.push(inst);
  }
  /// @}

  /// Since `new_target()` and `push_target()` throws an exception when the
  /// maximum target stack size is exceeded, this should be called before
  /// calling any of these functions.
  /// @{
  ///
  /// @returns true if a new target can be pushed (i.e. the target stack size
  /// limit won't be reached).
  ZB_CHECK ZB_INLINE bool can_push_target() const noexcept {
    return _vlocals.size() + 1 < k_max_func_stack_size;
  }

  /// @returns true if `n` new target can be pushed (i.e. the target stack size
  /// limit won't be reached).
  ZB_CHECK ZB_INLINE bool can_push_n_target(size_t n) const noexcept {
    return _vlocals.size() + 1 + n < k_max_func_stack_size;
  }
  /// @}

  /// Create a new target.
  /// This is the same as calling `push_target(-1)`.
  ///
  /// @note Avoid calling `push_target(-1)` when creating a new target,
  ///       use `new_target()` for performance reason.
  ZB_CHECK uint8_t new_target();

  /// Create a new target.
  ZB_CHECK uint8_t new_target(uint32_t mask, uint64_t custom_mask, bool is_const = false);

  /// Repush the target at the given index.
  ///
  /// @note Avoid calling `push_target(-1)` when creating a new target,
  ///       use `new_target()` for performance reason.
  int_t push_target(int_t n);

  /// Get the top target index from the target stack.
  ZB_CHECK ZB_INLINE int_t top_target() const noexcept {
    zbase_assert(!_target_stack.empty(), "Trying to get the top target of an empty stack.");
    return _target_stack.back().index;
  }

  /// Get the top target type info from the target stack.
  ZB_CHECK ZB_INLINE variable_type_info_t top_target_type_info() const noexcept {
    return _target_stack.back();
  }

  /// Get the target at the `n` index from the top of the target stack.
  /// Calling `get_up_target(0)`, is the same as calling `top_target()`.
  ZB_CHECK ZB_INLINE int_t get_up_target(int_t n) const noexcept {
    return _target_stack[((_target_stack.size() - 1) - n)].index;
  }

  /// Removes the top target from the target stack.
  /// This will also pop it from the `_vlocals` if it was an stack variable
  /// (unnamed).
  int_t pop_target();

  /// Get the last instruction index.
  /// Since the instructions have variable lengths, the last instruction cannot
  /// be accessed by simply calling `_instructions.back()`.
  ///
  /// @note By 'instruction index' we mean: the offset of the instruction in the
  /// instruction
  ///       vector in bytes size.
  ///
  /// @warning The last instruction index is not the number of instructions - 1.
  ZB_CHECK ZB_INLINE int_t get_instruction_index() const noexcept { return (int_t)_last_instruction_index; }

  /// @returns the index of the next instruction or in other words, the size in
  /// bytes
  ///          of the instruction vector.
  ZB_CHECK ZB_INLINE int_t get_next_instruction_index() const noexcept {
    return (int_t)_instructions._data.size();
  }

  /// Add a new named local variable.
  /// @{
  ZB_CHECK zs::error_result push_local_variable(const object& name, int_t* ret_pos = nullptr,
      uint32_t mask = 0, uint64_t custom_mask = 0, bool is_const = false);

  ZB_CHECK ZB_INLINE zs::error_result push_local_variable(
      const object& name, int_t* ret_pos, const variable_type_info_t& type_info) {
    return push_local_variable(name, ret_pos, type_info.mask, type_info.custom_mask, type_info.is_const);
  }
  /// @}

  /// Looks for a literal with the given name or create a new one if it doesn't
  /// already exists. These literals will be accessed with an op_load
  /// instruction.
  ZB_CHECK int_t get_literal(const object& name);

  /// Looks for a capture variable with the given named.
  /// @returns the index of the variable in `_captures` or `-1` if not found.
  ZB_CHECK int_t get_capture(const object& name);

  /// Looks for a local variable with the given named.
  /// @returns the index of the variable in `_vlocals` or `-1` if not found.
  ZB_CHECK int_t find_local_variable(const object& name) const;

  /// @returns true if the variable in `_vlocals` at the given index is a named
  /// local variable.
  ZB_CHECK bool is_local(size_t pos) const;

  /// Get the current number of local variables (stack or named).
  /// @warning Not the same as `_total_stack_size`.
  ZB_CHECK ZB_INLINE int_t get_stack_size() const noexcept { return (int_t)_vlocals.size(); }

  /// TODO: Add some doc.
  void set_stack_size(int_t n);

  ZB_CHECK zs::error_result get_restricted_type_index(const object& name, int_t& index);

  ZB_CHECK zs::error_result get_restricted_type_mask(const object& name, int_t& mask) const noexcept;

  ///
  void add_line_infos(const zs::line_info& linfo);

#if ZS_DEBUG
  void add_debug_line_info(const zs::line_info& linfo);
#endif

  ZB_CHECK zs::function_prototype_object* build_function_prototype();

  ZB_CHECK ZB_INLINE zs::closure_compile_state* get_parent() const noexcept { return _parent; }

  ZB_CHECK zs::closure_compile_state* push_child_state();
  void pop_child_state();

private:
  friend class jit_compiler;
  friend class parser;

  /// Parent closure state.
  zs::closure_compile_state* _parent = nullptr;

  /// Parameter names.
  zs::small_vector<zs::object, 8> _parameter_names;

  /// Child closure state.
  zs::vector<zs::unique_ptr<closure_compile_state>> _children;

  /// Instruction vector.
  zs::instruction_vector _instructions;

  /// Type list.
  zs::small_vector<zs::object, 8> _restricted_types;

  /// Target stack indexes and type info.
  zs::vector<variable_type_info_t> _target_stack;

  /// Literal values.
  /// The object key are strings.
  zs::object_unordered_map<int_t> _literals;

  /// Local variables info.
  /// These can be named or not (when zs::local_var_info_t::_name is not a
  /// string i.e. null). An unnamed variable is a stack value. A named one, is
  /// an actual declared local variable.
  zs::vector<zs::local_var_info_t> _vlocals;

  /// Only these will be exported to the function prototype.
  zs::vector<zs::local_var_info_t> _local_var_infos;

  /// Line infos.
  zs::vector<zs::line_info_op_t> _line_info;

  /// Functions.
  zs::vector<zs::object> _functions;

  zs::vector<int_t> _default_params;

  zs::vector<captured_variable> _captures;
  size_t _n_capture = 0;

  /// Used to prevent from importing the same file twice (calling `#import`).
  zs::object_unordered_set _imported_files_set;

#if !ZBASE_IS_MACRO_EMPTY(ZS_DEBUG) && ZS_DEBUG
  zs::vector<zs::line_info_op_t> _debug_line_info;
#endif

  int_t _last_line = -1;

  /// Since the instructions have variable lengths,
  /// we keep track of the last instruction start index.
  size_t _last_instruction_index = 0;

  /// This is the maximum stack size that the function will need.
  uint32_t _total_stack_size = 0;

  bool _vargs_params = false;

  void mark_local_as_capture(int_t pos);

  ZB_CHECK int_t alloc_stack_pos();
};

} // namespace zs.
