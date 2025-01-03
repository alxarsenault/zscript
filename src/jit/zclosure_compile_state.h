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
#include "bytecode/zinstruction_vector.h"

namespace zs {

using target_t = uint8_t;

inline constexpr const target_t k_invalid_target = -1;

inline constexpr const size_t k_maximum_target_index = (std::numeric_limits<uint8_t>::max)() - 1;

// The custom type_mask value is a uint64_t.
inline constexpr size_t k_max_restricted_type_check = 64;

inline constexpr uint_t k_captured_end_op = (std::numeric_limits<uint_t>::max)();

class function_prototype_object;

struct scoped_local_var_info_t;
struct local_var_info_t;

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

struct local_var_info_t : named_variable_type_info {
  inline local_var_info_t() = default;
  inline local_var_info_t(const local_var_info_t&) = default;

  inline local_var_info_t(const object& name, uint_t start_op, uint_t end_op, uint_t pos, uint32_t mask,
      uint64_t custom_mask, bool is_const)
      : named_variable_type_info(name, custom_mask, mask)
      , start_op(start_op)
      , end_op(end_op)
      , pos(pos) {
    set_const(is_const);
  }

  uint_t start_op = 0;
  uint_t end_op = 0;
  uint_t pos = 0;
};

inline std::ostream& operator<<(std::ostream& s, const local_var_info_t& vinfo) {
  s << vinfo.name.convert_to_string();
  return s;
}

struct scoped_local_var_info_t : local_var_info_t {
  inline scoped_local_var_info_t() = default;

  inline scoped_local_var_info_t(const object& name, int_t scope_id, uint_t start_op, uint_t end_op,
      uint_t pos, uint32_t mask, uint64_t custom_mask, bool is_const)
      : local_var_info_t(name, start_op, end_op, pos, mask, custom_mask, is_const)
      , scope_id(scope_id) {}

  int_t scope_id = 0;
};

inline std::ostream& operator<<(std::ostream& s, const scoped_local_var_info_t& vinfo) {
  s << vinfo.name.convert_to_string();
  return s;
}

struct captured_variable {
  enum class type_t : uint8_t { local, outer };
  using enum type_t;

  captured_variable() = default;
  captured_variable(const captured_variable&) = default;
  captured_variable(captured_variable&&) = default;

  inline captured_variable(
      const object& capture_name, int_t capture_src, type_t capture_type, bool weak = false)
      : type(capture_type)
      , name(capture_name)
      , src(capture_src)
      , is_weak(weak) {}

  captured_variable& operator=(const captured_variable&) = default;
  captured_variable& operator=(captured_variable&&) = default;

  type_t type;
  object name;
  int_t src;
  bool is_weak;
};

namespace jit {

  class shared_state_data_ref;

  struct shared_state_data : public engine_holder {
    friend class shared_state_data_ref;

    inline shared_state_data(zs::engine* eng)
        : engine_holder(eng)
        , _restricted_types(zs::allocator<object>(eng))
        //        , _exported_names(zs::allocator<object>(eng))
        , _imported_files_set(zs::allocator<object>(eng)) {}

    inline zs::error_result add_module_author(const object& author) {
      ZS_ASSERT(_module_info.is_table());

      object& authors = _module_info.as_table()["authors"];

      if (authors.is_array()) {
        authors.as_array().push(author);
      }
      else {
        authors = zs::_a(_engine, { author });
      }

      return {};
    }

    inline zs::error_result add_module_name(const object& name) {
      ZS_ASSERT(_module_info.is_table());
      return _module_info.as_table().set_no_replace(zs::_ss("name"), name);
    }

    inline zs::error_result add_module_brief(const object& brief) {
      ZS_ASSERT(_module_info.is_table());
      return _module_info.as_table().set_no_replace(zs::_ss("brief"), brief);
    }

    inline zs::error_result add_module_version(const object& version) {
      ZS_ASSERT(_module_info.is_table());
      return _module_info.as_table().set_no_replace(zs::_ss("version"), version);
    }

    inline zs::error_result add_module_date(const object& date) {
      ZS_ASSERT(_module_info.is_table());
      return _module_info.as_table().set_no_replace(zs::_ss("date"), date);
    }

    inline zs::error_result add_module_copyright(const object& copyright) {
      ZS_ASSERT(_module_info.is_table());
      return _module_info.as_table().set_no_replace(zs::_ss("copyright"), copyright);
    }

    /// Type list.
    zs::small_vector<zs::object, 8> _restricted_types;

    /// Used to prevent from importing the same file twice (calling `#import`).
    zs::object_unordered_set _imported_files_set;

    /// The name or the path of the source file.
    zs::object _source_name;

    /// Module name.
    zs::object _module_info;

    /// Exported names.
    //    zs::object_unordered_set _exported_names;

    bool _is_module = false;
  };

  class shared_state_data_ref {
  public:
    inline shared_state_data_ref(shared_state_data& sdata)
        : _sdata(sdata) {}

    inline zs::engine* get_engine() const noexcept { return _sdata.get_engine(); }

    //    inline zs::error_result add_exported_name(const object& var_name) {
    //      return _sdata._exported_names.insert(var_name).second ? zs::error_code::success
    //                                                            : zs::error_code::already_exists;
    //    }

    ZB_CHECK ZB_INLINE bool is_module() const noexcept { return _sdata._is_module; }

    //    ZB_CHECK ZB_INLINE bool has_exported_name(const object& var_name) const noexcept {
    //      return _sdata._exported_names.contains(var_name);
    //    }

    shared_state_data& _sdata;
  };

  class closure_compile_state_ref;
} // namespace jit.

/// Used by the compiler when parsing/compiling a function.
/// The end result is a `function_prototype_object` that can be created by
/// calling `build_function_prototype()`.
class closure_compile_state : public jit::shared_state_data_ref {
public:
  ZS_CLASS_COMMON;

  struct target_type_info_t : variable_type_info {
    inline target_type_info_t() noexcept = delete;

    inline target_type_info_t(
        int_t index, uint32_t mask = 0, uint64_t custom_mask = 0, bool is_const = false) noexcept
        : variable_type_info{ custom_mask, mask }
        , index(index) {
      set_const(is_const);
    }

    inline target_type_info_t(const scoped_local_var_info_t& lvar) noexcept
        : variable_type_info(lvar)
        , index(lvar.pos) {}

    inline target_type_info_t(const local_var_info_t& lvar) noexcept
        : variable_type_info(lvar)
        , index(lvar.pos) {}

    int_t index;
  };

  closure_compile_state(zs::engine* eng, jit::shared_state_data& sdata);
  closure_compile_state(zs::engine* eng, closure_compile_state& parent);
  closure_compile_state(zs::engine* eng, closure_compile_state& parent, const object& sname);

  ~closure_compile_state();

  /// Name of the closure object (i.e. the function name).
  zs::object name;

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

  template <opcode Op, class... Args>
  ZB_INLINE void replace_last_instruction(Args... args) {
    _instructions._data.resize(_last_instruction_index);
    _last_instruction_index = _instructions._data.size();
    _instructions.push<Op>(std::forward<Args>(args)...);
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
    return _vlocals.size() + n < k_max_func_stack_size;
  }
  /// @}

  /// Create a new target.
  ZB_CHECK target_t new_target();

  /// Create a new target.
  ZB_CHECK target_t new_target(uint32_t mask, uint64_t custom_mask, bool is_const = false);

  /// Repush the target at the given index.
  //  target_t push_target(target_t n);
  target_t push_var_target(target_t n);

  int_t push_export_target();

  /// Get the top target index from the target stack.
  ZB_CHECK ZB_INLINE uint8_t top_target() const noexcept {
    zbase_assert(!_target_stack.empty(), "Trying to get the top target of an empty stack.");
    zbase_assert(_target_stack.back().index >= 0);
    return (uint8_t)_target_stack.back().index;
  }

  /// Get the top target type info from the target stack.
  ZB_CHECK ZB_INLINE target_type_info_t top_target_type_info() const noexcept { return _target_stack.back(); }

  /// Get the target at the `n` index from the top of the target stack.
  /// Calling `get_up_target(0)`, is the same as calling `top_target()`.
  ZB_CHECK ZB_INLINE int_t get_up_target(int_t n) const noexcept {
    return _target_stack[((_target_stack.size() - 1) - n)].index;
  }

  /// Removes the top target from the target stack.
  /// This will also pop it from the `_vlocals` if it was an stack variable
  /// (unnamed).
  target_t pop_target();

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
  ZB_CHECK zs::error_result push_local_variable(const object& name, int_t scope_id, int_t* ret_pos = nullptr,
      uint32_t mask = 0, uint64_t custom_mask = 0, bool is_const = false);

  /// @}

  /// Looks for a literal with the given name or create a new one if it doesn't
  /// already exists. These literals will be accessed with an op_load
  /// instruction.
  ZB_CHECK int_t get_literal(const object& name);

  /// Looks for a capture variable with the given named.
  /// @returns the index of the variable in `_captures` or `-1` if not found.
  ZB_CHECK zs::optional_result<int_t> get_capture(const object& name);

  /// Looks for a local variable with the given named.
  /// @returns the index of the variable in `_vlocals` or `-1` if not found.
  ZB_CHECK zs::optional_result<int_t> find_local_variable(const object& name) const;
  ZB_CHECK const zs::scoped_local_var_info_t* find_local_variable_ptr(const object& name) const noexcept;

  ZB_CHECK zs::error_result find_local_variable(const object& name, int_t& pos) const;

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

  //  ZB_CHECK zs::error_result create_export_table();

  ///
  void add_line_infos(const zs::line_info& linfo);

#if ZS_DEBUG
  void add_debug_line_info(const zs::line_info& linfo);
#endif

  ZB_CHECK zs::object build_function_prototype();

  ZB_CHECK ZB_INLINE zs::closure_compile_state* get_parent() const noexcept { return _parent; }

  ZB_CHECK ZB_INLINE bool is_top_level() const noexcept { return _parent == nullptr; }

  //  ZB_CHECK ZB_INLINE bool has_export() const noexcept { return _export_table_target != -1; }
  ZB_CHECK ZB_INLINE bool has_vargs_params() const noexcept { return _has_vargs_params; }

  //  ZB_CHECK ZB_INLINE int_t get_export_target() const noexcept { return _export_table_target; }

  ZB_CHECK ZB_INLINE jit::shared_state_data& state_data() noexcept { return _sdata; }

  ZS_CK_INLINE size_t get_current_capture_count() const noexcept { return _n_capture; }

private:
  friend class jit_compiler;
  friend class parser;
  friend class jit::closure_compile_state_ref;

  struct private_constructor_tag {};
  closure_compile_state(
      private_constructor_tag, zs::engine* eng, jit::shared_state_data& sdata, closure_compile_state* parent);

  /// Parent closure state.
  zs::closure_compile_state* _parent = nullptr;

  /// Parameter names.
  zs::small_vector<zs::object, 8> _parameter_names;

  /// Instruction vector.
  zs::instruction_vector _instructions;

  /// Target stack indexes and type info.
  zs::vector<target_type_info_t> _target_stack;

  /// Literal values.
  /// The object key are strings.
  zs::object_unordered_map<int_t> _literals;

  /// Local variables info.
  /// These can be named or not (when zs::local_var_info_t::_name is not a
  /// string i.e. null). An unnamed variable is a stack value. A named one, is
  /// an actual declared local variable.
  zs::vector<zs::scoped_local_var_info_t> _vlocals;

  /// Only these will be exported to the function prototype.
  zs::vector<zs::local_var_info_t> _local_var_infos;

  /// Line infos.
  zs::vector<zs::line_info_op_t> _line_info;

  /// Functions.
  zs::vector<zs::object> _functions;

  zs::vector<int_t> _default_params;

  zs::vector<size_t> _breaks;
  zs::vector<size_t> _unresolved_breaks;

  zs::vector<size_t> _continues;
  zs::vector<size_t> _unresolved_continues;

  zs::vector<captured_variable> _captures;
  size_t _n_capture = 0;

#if !ZBASE_IS_MACRO_EMPTY(ZS_DEBUG) && ZS_DEBUG
  zs::vector<zs::line_info_op_t> _debug_line_info;
#endif

  int_t _last_line = -1;

  /// Since the instructions have variable lengths,
  /// we keep track of the last instruction start index.
  size_t _last_instruction_index = 0;

  /// This is the maximum stack size that the function will need.
  uint32_t _total_stack_size = 0;

  bool _has_vargs_params = false;

  void mark_local_as_capture(int_t pos);

  ZS_CK_INLINE zs::error_result update_total_stack_size() noexcept {
    return ((_total_stack_size = zb::maximum(_total_stack_size, (uint32_t)_vlocals.size()))
               < k_max_func_stack_size)
        ? zs::errc::success
        : zs::errc::too_many_locals;
  }

  ZB_CHECK int_t alloc_stack_pos();
};

namespace jit {
  class closure_compile_state_ref {
  public:
    ZS_CK_INLINE zs::error_result add_parameter(const object& param_name) {
      return _ccs->add_parameter(param_name);
    }

    ZS_CK_INLINE zs::error_result add_parameter(
        const object& param_name, uint32_t mask, uint64_t custom_mask, bool is_const = false) {
      return _ccs->add_parameter(param_name, mask, custom_mask, is_const);
    }
    /// @}
    ///
    /// Create a new target.
    /// This is the same as calling `push_target(-1)`.
    ///
    /// @note Avoid calling `push_target(-1)` when creating a new target,
    ///       use `new_target()` for performance reason.
    ZS_CK_INLINE target_t new_target() { return _ccs->new_target(); }

    /// Create a new target.
    ZS_CK_INLINE target_t new_target(uint32_t mask, uint64_t custom_mask, bool is_const = false) {
      return _ccs->new_target(mask, custom_mask, is_const);
    }

    ZS_CK_INLINE target_t new_target(const variable_type_info& vinfo) {
      return _ccs->new_target(vinfo.mask, vinfo.custom_mask, vinfo.is_const());
    }

    /// Repush the target at the given index.
    //    ZS_INLINE target_t push_target(int_t n) { return _ccs->push_target(n); }

    ZS_INLINE target_t push_var_target(int_t n) noexcept { return _ccs->push_var_target(n); }

    ZS_INLINE target_t push_this_target() noexcept { return _ccs->push_var_target(0); }

    /// Removes the top target from the target stack.
    /// This will also pop it from the `_vlocals` if it was an stack variable
    /// (unnamed).
    ZS_INLINE target_t pop_target() noexcept { return _ccs->pop_target(); }

    ZS_INLINE target_t pop_target_if(bool cond) noexcept {
      return cond ? _ccs->pop_target() : k_invalid_target;
    }

    ZS_INLINE void pop_n_target(size_t n) noexcept {
      for (size_t i = 0; i < n; i++) {
        _ccs->pop_target();
      }
    }

    ZS_CK_INLINE target_t top_target() const noexcept { return _ccs->top_target(); }

    ZS_CK_INLINE target_t up_target(int_t n) const noexcept {
      ZS_ASSERT(n < 0);

      return (target_t)(_ccs->_target_stack[_ccs->_target_stack.size() + n].index);
    }

    //    /// Add a new named local variable.
    //    /// @{
    //    ZS_CK_INLINE zs::error_result push_local_variable(const object& name, int_t* ret_pos = nullptr,
    //        uint32_t mask = 0, uint64_t custom_mask = 0, bool is_const = false) {
    //      return _ccs->push_local_variable(name, ret_pos, mask, custom_mask, is_const);
    //    }
    //
    //    ZS_CK_INLINE zs::error_result push_local_variable(
    //        const object& name, int_t* ret_pos, const variable_type_info_t& type_info) {
    //      return _ccs->push_local_variable(
    //          name, ret_pos, type_info.mask, type_info.custom_mask, type_info.is_const);
    //    }
    //    /// @}

    ZS_CK_INLINE uint32_t get_functions_count() const noexcept { return (uint32_t)_ccs->_functions.size(); }

    ZS_CK_INLINE uint32_t get_last_function_index() const noexcept {
      ZS_ASSERT(
          !_ccs->_functions.empty(), "Can't call get_last_function_index() when no functions were declared.");
      return (uint32_t)(_ccs->_functions.size() - 1);
    }

    ZS_CK_INLINE zs::vector<zs::object>& get_functions() noexcept { return _ccs->_functions; }

    ZS_CK_INLINE const zs::vector<zs::object>& get_functions() const noexcept { return _ccs->_functions; }

    ZS_CK_INLINE zs::object& get_function(size_t index) noexcept { return _ccs->_functions[index]; }

    ZS_CK_INLINE const zs::object& get_function(size_t index) const noexcept {
      return _ccs->_functions[index];
    }

    ZS_CHECK zs::function_prototype_object& get_function_proto(size_t index) noexcept;
    ZS_CHECK const zs::function_prototype_object& get_function_proto(size_t index) const noexcept;

    ZS_CHECK zs::function_prototype_object& get_last_function_proto() noexcept;
    ZS_CHECK const zs::function_prototype_object& get_last_function_proto() const noexcept;

    ZS_CK_INLINE int_t get_instruction_index() const noexcept { return _ccs->get_instruction_index(); }

    ZS_CK_INLINE int_t get_next_instruction_index() const noexcept {
      return _ccs->get_next_instruction_index();
    }

    template <opcode Op>
    ZS_CK_INLINE instruction_t<Op>& get_instruction_ref(size_t index) noexcept {
      return _ccs->_instructions.get_ref<Op>(index);
    }

    template <opcode Op>
    ZS_CK_INLINE instruction_t<Op>& get_instruction_ref() noexcept {
      return _ccs->_instructions.get_ref<Op>(get_instruction_index());
    }

    template <opcode Op>
    ZS_CK_INLINE auto get_instruction_value(size_t index) const noexcept {
      return _ccs->_instructions.get_ref<Op>(index).value;
    }

    ZS_CK_INLINE zs::opcode get_instruction_opcode(size_t index) const noexcept {
      return _ccs->_instructions.get_opcode(index);
    }

    ZS_CK_INLINE zs::opcode get_instruction_opcode() const noexcept {
      return _ccs->_instructions.get_opcode(get_instruction_index());
    }

    ZS_CK_INLINE zs::instruction_vector& get_instructions() noexcept { return _ccs->_instructions; }

    ZS_CK_INLINE zs::vector<uint8_t>& get_instructions_internal_vector() noexcept {
      return _ccs->_instructions._data;
    }
    zs::closure_compile_state* _ccs = nullptr;
  };
} // namespace jit.

} // namespace zs.
