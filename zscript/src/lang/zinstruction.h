#pragma once

#include <zscript/zscript.h>
#include "lang/zopcode.h"

namespace zs {

template <opcode Op>
using is_arithmetic_op = std::bool_constant<zb::is_one_of(Op, //
    opcode::op_add, //
    opcode::op_sub, //
    opcode::op_mul, //
    opcode::op_div, //
    opcode::op_mod, //
    opcode::op_exp, //
    opcode::op_bitwise_or, //
    opcode::op_bitwise_and, opcode::op_bitwise_xor)>;

template <opcode Op>
using is_arithmetic_eq_op = std::bool_constant<zb::is_one_of(Op, //
    opcode::op_add_eq, //
    opcode::op_sub_eq, //
    opcode::op_mul_eq, //
    opcode::op_div_eq, //
    opcode::op_mod_eq, //
    opcode::op_exp_eq)>;

template <opcode Op, class = void>
struct instruction_t {
  opcode op = Op;

  inline friend std::ostream& operator<<(std::ostream& stream, const instruction_t& inst) {
    zb::stream_print<" ">(stream, inst.op);
    return stream;
  }
};

#define __ZS_INSTRUCTION_TYPE(type, name) type name;
#define __ZS_INSTRUCTION_PARAM_TYPE(type, name) , type name
#define __ZS_INSTRUCTION_INIT_MEMBER(type, name) , name(name)
#define __ZS_INSTRUCTION_PRINT_TYPE(type, name) , " <", #type, " ", #name, "> = ", inst.name

#define __ZS_DECLARE_INSTRUCTION(name, TYPES_MACRO, size)                                       \
  ZBASE_PACKED_START                                                                            \
  template <>                                                                                   \
  struct instruction_t<ZS_OPCODE_ENUM_VALUE(name)> {                                            \
    static constexpr opcode s_code = ZS_OPCODE_ENUM_VALUE(name);                                \
    inline constexpr instruction_t() noexcept = default;                                        \
                                                                                                \
    inline constexpr instruction_t(opcode op TYPES_MACRO(__ZS_INSTRUCTION_PARAM_TYPE)) noexcept \
        : op(op) TYPES_MACRO(__ZS_INSTRUCTION_INIT_MEMBER) {                                    \
      zbase_assert(op == s_code, "invalid opcode expected op_" #name);                          \
    }                                                                                           \
                                                                                                \
    inline friend std::ostream& operator<<(std::ostream& stream, const instruction_t& inst) {   \
      zb::stream_print<"">(stream, inst.op TYPES_MACRO(__ZS_INSTRUCTION_PRINT_TYPE));           \
      return stream;                                                                            \
    }                                                                                           \
                                                                                                \
    opcode op;                                                                                  \
    TYPES_MACRO(__ZS_INSTRUCTION_TYPE)                                                          \
  };                                                                                            \
  ZBASE_PACKED_END                                                                              \
  static_assert(                                                                                \
      sizeof(instruction_t<ZS_OPCODE_ENUM_VALUE(name)>) == size, "wrong instruction size for op_" #name)

#define ZS_DECLARE_INSTRUCTION(name, TYPES_MACRO_NAME, size) \
  __ZS_DECLARE_INSTRUCTION(name, ZS_INSTRUCTION_##TYPES_MACRO_NAME##_TYPES, size)

/// Arithmetic.
/// Get the stack value at `lhs_idx`.
/// Get the stack value at `rhs_idx`.
/// Compute arithmetic operation.
/// Set the result value in the stack at `target_idx`.
#define ZS_INSTRUCTION_ARITHMETIC_TYPES(X) \
  X(uint8_t, target_idx)                   \
  X(uint8_t, lhs_idx)                      \
  X(uint8_t, rhs_idx)

ZS_DECLARE_INSTRUCTION(add, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(sub, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(mul, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(div, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(mod, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(exp, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(bitwise_or, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(bitwise_and, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(bitwise_xor, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(lshift, ARITHMETIC, 4);
ZS_DECLARE_INSTRUCTION(rshift, ARITHMETIC, 4);

#define ZS_INSTRUCTION_ARITH_TYPES(X) \
  X(uint8_t, target_idx)              \
  X(uint8_t, lhs_idx)                 \
  X(uint8_t, rhs_idx)                 \
  X(arithmetic_op, aop)
ZS_DECLARE_INSTRUCTION(arith, ARITH, 5);

/// Get the stack value at `target_idx`.
/// Get the stack value at `rhs_idx`.
/// Compute arithmetic equal operation.
/// Set the result value in the stack at `target_idx`.
#define ZS_INSTRUCTION_ARITHMETIC_EQ_TYPES(X) \
  X(uint8_t, target_idx)                      \
  X(uint8_t, rhs_idx)

ZS_DECLARE_INSTRUCTION(add_eq, ARITHMETIC_EQ, 3);
ZS_DECLARE_INSTRUCTION(sub_eq, ARITHMETIC_EQ, 3);
ZS_DECLARE_INSTRUCTION(mul_eq, ARITHMETIC_EQ, 3);
ZS_DECLARE_INSTRUCTION(div_eq, ARITHMETIC_EQ, 3);
ZS_DECLARE_INSTRUCTION(mod_eq, ARITHMETIC_EQ, 3);
ZS_DECLARE_INSTRUCTION(exp_eq, ARITHMETIC_EQ, 3);

///
#define ZS_INSTRUCTION_LINE_TYPES(X) X(int64_t, line)
ZS_DECLARE_INSTRUCTION(line, LINE, 9);

///
#define ZS_INSTRUCTION_LOAD_TYPES(X) \
  X(uint8_t, target_idx)             \
  X(uint32_t, idx)
ZS_DECLARE_INSTRUCTION(load, LOAD, 6);

/// op_move.
#define ZS_INSTRUCTION_MOVE_TYPES(X) \
  X(uint8_t, target_idx)             \
  X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(move, MOVE, 3);

///
#define ZS_INSTRUCTION_LOAD_BOOL_TYPES(X) \
  X(uint8_t, target_idx)                  \
  X(bool_t, value)
ZS_DECLARE_INSTRUCTION(load_bool, LOAD_BOOL, 3);

///
#define ZS_INSTRUCTION_LOAD_INT_TYPES(X) \
  X(uint8_t, target_idx)                 \
  X(int_t, value)
ZS_DECLARE_INSTRUCTION(load_int, LOAD_INT, 10);

///
#define ZS_INSTRUCTION_LOAD_FLOAT_TYPES(X) \
  X(uint8_t, target_idx)                   \
  X(float_t, value)
ZS_DECLARE_INSTRUCTION(load_float, LOAD_FLOAT, 10);

///
#define ZS_INSTRUCTION_LOAD_SMALL_STRING_TYPES(X) \
  X(uint8_t, target_idx)                          \
  X(uint64_t, value_1)                            \
  X(uint64_t, value_2)
ZS_DECLARE_INSTRUCTION(load_small_string, LOAD_SMALL_STRING, 18);

///
#define ZS_INSTRUCTION_LOAD_STRING_TYPES(X) \
  X(uint8_t, target_idx)                    \
  X(uint32_t, idx)
ZS_DECLARE_INSTRUCTION(load_string, LOAD_STRING, 6);

///
#define ZS_INSTRUCTION_LOAD_NULL_TYPES(X) X(uint8_t, target_idx)
ZS_DECLARE_INSTRUCTION(load_null, LOAD_NULL, 2);

///
#define ZS_INSTRUCTION_LOAD_NULLS_TYPES(X) \
  X(uint8_t, target_idx)                   \
  X(uint8_t, count)
ZS_DECLARE_INSTRUCTION(load_nulls, LOAD_NULLS, 3);

///
#define ZS_INSTRUCTION_LOAD_NONE_TYPES(X) X(uint8_t, target_idx)
ZS_DECLARE_INSTRUCTION(load_none, LOAD_NONE, 2);

///
#define ZS_INSTRUCTION_LOAD_OBJECT_TYPES(X) \
  X(uint8_t, target_idx)                    \
  X(object_base, value)
ZS_DECLARE_INSTRUCTION(load_object, LOAD_OBJECT, 18);

///
#define ZS_INSTRUCTION_SET_META_ARGUMENT_TYPES(X) X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(set_meta_argument, SET_META_ARGUMENT, 2);

///
#define ZS_INSTRUCTION_GET_TYPES(X) \
  X(uint8_t, target_idx)            \
  X(uint8_t, table_idx)             \
  X(uint8_t, key_idx)               \
  X(bool, look_in_root)

ZS_DECLARE_INSTRUCTION(get, GET, 5);

///
#define ZS_INSTRUCTION_SET_TYPES(X) \
  X(uint8_t, table_idx)             \
  X(uint8_t, key_idx)               \
  X(uint8_t, value_idx)
ZS_DECLARE_INSTRUCTION(set, SET, 4);

///
#define ZS_INSTRUCTION_RAWSET_TYPES(X) \
  X(uint8_t, table_idx)                \
  X(uint8_t, key_idx)                  \
  X(uint8_t, value_idx)
ZS_DECLARE_INSTRUCTION(rawset, RAWSET, 4);

///
#define ZS_INSTRUCTION_RAWSETS_TYPES(X) \
  X(uint8_t, table_idx)                 \
  X(uint8_t, value_idx)                 \
  X(uint64_t, value_1)                  \
  X(uint64_t, value_2)
ZS_DECLARE_INSTRUCTION(rawsets, RAWSETS, 19);

///
#define ZS_INSTRUCTION_EQUALS_TYPES(X) \
  X(uint8_t, target_idx)               \
  X(uint8_t, lhs_idx)                  \
  X(uint32_t, rhs_idx)                 \
  X(bool, is_literal)
ZS_DECLARE_INSTRUCTION(eq, EQUALS, 8);

///
#define ZS_INSTRUCTION_NOT_EQUALS_TYPES(X) \
  X(uint8_t, target_idx)                   \
  X(uint8_t, lhs_idx)                      \
  X(uint32_t, rhs_idx)                     \
  X(bool, is_literal)
ZS_DECLARE_INSTRUCTION(ne, NOT_EQUALS, 8);

///
#define ZS_INSTRUCTION_LOAD_ROOT_TYPES(X) X(uint8_t, target_idx)
ZS_DECLARE_INSTRUCTION(load_root, LOAD_ROOT, 2);

///
#define ZS_INSTRUCTION_LOAD_GLOBAL_TYPES(X) X(uint8_t, target_idx)
ZS_DECLARE_INSTRUCTION(load_global, LOAD_GLOBAL, 2);

///
#define ZS_INSTRUCTION_JMP_TYPES(X) X(int32_t, offset)
ZS_DECLARE_INSTRUCTION(jmp, JMP, 5);

///
// #d efine ZS_INSTRUCTION_JCMP_TYPES(X) \
//  X(uint32_t, offset)                \
//  X(compare_op, cmp_op)              \
//  X(uint8_t, lhs_idx)                \
//  X(uint8_t, rhs_idx)
// ZS_DECLARE_INSTRUCTION(jcmp, JCMP, 8);

///
#define ZS_INSTRUCTION_JZ_TYPES(X) \
  X(int32_t, offset)               \
  X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(jz, JZ, 6);

///
#define ZS_INSTRUCTION_CLOSE_TYPES(X) X(uint32_t, stack_size)
ZS_DECLARE_INSTRUCTION(close, CLOSE, 5);

///
#define ZS_INSTRUCTION_CMP_TYPES(X) \
  X(uint8_t, target_idx)            \
  X(compare_op, cmp_op)             \
  X(uint8_t, lhs_idx)               \
  X(uint8_t, rhs_idx)
ZS_DECLARE_INSTRUCTION(cmp, CMP, 5);

///
#define ZS_INSTRUCTION_NOT_TYPES(X) \
  X(uint8_t, target_idx)            \
  X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(not, NOT, 3);

///
#define ZS_INSTRUCTION_INCR_TYPES(X) \
  X(uint8_t, target_idx)             \
  X(uint8_t, idx)                    \
  X(bool, is_incr)
ZS_DECLARE_INSTRUCTION(incr, INCR, 4);

///
#define ZS_INSTRUCTION_PINCR_TYPES(X) \
  X(uint8_t, target_idx)              \
  X(uint8_t, idx)                     \
  X(bool, is_incr)
ZS_DECLARE_INSTRUCTION(pincr, PINCR, 4);

///
#define ZS_INSTRUCTION_POBJINCR_TYPES(X) \
  X(uint8_t, target_idx)                 \
  X(uint8_t, table_ix)                   \
  X(uint8_t, key_ix)                     \
  X(bool, is_incr)
ZS_DECLARE_INSTRUCTION(pobjincr, POBJINCR, 5);

///
///
///
#define ZS_INSTRUCTION_EXISTS_TYPES(X) \
  X(uint8_t, target_idx)               \
  X(uint8_t, table_idx)                \
  X(uint8_t, key_idx)
ZS_DECLARE_INSTRUCTION(exists, EXISTS, 4);

///
#define ZS_INSTRUCTION_TYPEOF_TYPES(X) \
  X(uint8_t, target_idx)               \
  X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(typeof, TYPEOF, 3);

///
#define ZS_INSTRUCTION_CLONE_TYPES(X) \
  X(uint8_t, target_idx)              \
  X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(clone, CLONE, 3);

///
#define ZS_INSTRUCTION_GET_BASE_TYPES(X) X(uint8_t, target_idx)
ZS_DECLARE_INSTRUCTION(get_base, GET_BASE, 2);

///
#define ZS_INSTRUCTION_RETURN_TYPES(X) \
  X(uint8_t, idx)                      \
  X(bool, has_value)
ZS_DECLARE_INSTRUCTION(return, RETURN, 3);

///
#define ZS_INSTRUCTION_RETURN_EXPORT_TYPES(X) X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(return_export, RETURN_EXPORT, 2);

/// Check type.
/// Makes sure that the object at stack index `idx` is of `obj_type` type.
#define ZS_INSTRUCTION_CHECK_TYPE_TYPES(X) \
  X(uint8_t, idx)                          \
  X(zs::object_type, obj_type)
ZS_DECLARE_INSTRUCTION(check_type, CHECK_TYPE, 3);

/// Check type mask.
/// Makes sure that the object at stack index `idx` has a type (type mask) in
/// the ones given in `mask`.
#define ZS_INSTRUCTION_CHECK_TYPE_MASK_TYPES(X) \
  X(uint8_t, idx)                               \
  X(uint32_t, mask)
ZS_DECLARE_INSTRUCTION(check_type_mask, CHECK_TYPE_MASK, 6);

/// Check custom type mask.
/// Same as `op_check_type_mask` but a an added `custom_mask` possibility.
/// When parsing/compiling a variable like this one: `var<int, float, MyClass>`.
/// The builtin types are store in the mask value.
/// For the custom types, like `MyClass`, they are stored in an array of custom
/// type somewhere in the function state. The `custom_mask` value represent the
/// index in that array.
///
/// By example, given this code:
///   @code
///     var a;
///     int b;
///     var<int, string> c;
///     var<MyClass> d;
///     var<MyClass2, MyClass3> e;
///   @endcode
///
///   * The variable `a` would have no extra instruction for type checking.
///   * The variable `b` would have an extra `op_check_type` instruction for
///   type checking.
///   * The variable `c` would have an extra `op_check_type_mask` instruction
///   for type checking.
///   * The variables `d` and `e` would have an extra
///   `op_check_custom_type_mask` instruction for type
///     checking.
///
///     While parsing variable `d` and `e`, the function state will end up with
///     an array of custom types that would look like this `["MyClass",
///     "MyClass2", "MyClass3"]`.
///
///     Then for variable `d` the `custom_mask` value would the index of
///     `MyClass` converted to a mask (see zs::index_to_mask` in `internal.h`).
///     Basically the `custom_mask` would be `0b001`.
///
///     For the variable `e`, the custom mask would be `0b110`.
///
/// Given that, it is only possible to declare up to 64 different custom type
/// restriction in a function.
///
/// Having a custom type in a variable declaration will automatically add the
/// table, instance and user_data types to the (builtin) mask value.
///
ZS_TODO("The automatically added mask is not 100% sure yet.");

#define ZS_INSTRUCTION_CHECK_CUSTOM_TYPE_MASK_TYPES(X) \
  X(uint8_t, idx)                                      \
  X(uint32_t, mask)                                    \
  X(uint64_t, custom_mask)
ZS_DECLARE_INSTRUCTION(check_custom_type_mask, CHECK_CUSTOM_TYPE_MASK, 14);

/// Call.
#define ZS_INSTRUCTION_CALL_TYPES(X) \
  X(uint8_t, target_idx)             \
  X(uint8_t, closure_idx)            \
  X(uint8_t, this_idx)               \
  X(uint8_t, n_params)               \
  X(uint64_t, stack_base)
ZS_DECLARE_INSTRUCTION(call, CALL, 13);

/// new_obj.
#define ZS_INSTRUCTION_NEW_OBJ_TYPES(X) \
  X(uint8_t, target_idx)                \
  X(object_type, type)
ZS_DECLARE_INSTRUCTION(new_obj, NEW_OBJ, 3);

/// new_slot.
#define ZS_INSTRUCTION_NEW_SLOT_TYPES(X) \
  X(uint8_t, table_idx)                  \
  X(uint8_t, key_idx)                    \
  X(uint8_t, value_idx)
ZS_DECLARE_INSTRUCTION(new_slot, NEW_SLOT, 4);

/// op_new_struct_slot.
#define ZS_INSTRUCTION_NEW_STRUCT_SLOT_TYPES(X) \
  X(uint8_t, table_idx)                         \
  X(uint8_t, key_idx)                           \
  X(uint8_t, value_idx)                         \
  X(uint32_t, mask)                             \
  X(bool, is_static)                            \
  X(bool, has_value)                            \
  X(bool, is_const)
ZS_DECLARE_INSTRUCTION(new_struct_slot, NEW_STRUCT_SLOT, 11);

/// op_new_struct_constructor.
#define ZS_INSTRUCTION_NEW_STRUCT_CONSTRUCTOR_TYPES(X) \
  X(uint8_t, table_idx)                                \
  X(uint8_t, value_idx)
ZS_DECLARE_INSTRUCTION(new_struct_constructor, NEW_STRUCT_CONSTRUCTOR, 3);

/// new_class_slot.
#define ZS_INSTRUCTION_NEW_CLASS_SLOT_TYPES(X) \
  X(uint8_t, table_idx)                        \
  X(uint8_t, key_idx)                          \
  X(uint8_t, value_idx)                        \
  X(bool, is_static)
ZS_DECLARE_INSTRUCTION(new_class_slot, NEW_CLASS_SLOT, 5);

/// new_enum_slot.
#define ZS_INSTRUCTION_NEW_ENUM_SLOT_TYPES(X) \
  X(uint8_t, table_idx)                       \
  X(uint8_t, key_idx)                         \
  X(uint8_t, value_idx)
ZS_DECLARE_INSTRUCTION(new_enum_slot, NEW_ENUM_SLOT, 4);

/// array_append.
#define ZS_INSTRUCTION_ARRAY_APPEND_TYPES(X) \
  X(uint8_t, array_idx)                      \
  X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(array_append, ARRAY_APPEND, 3);

/// .
#define ZS_INSTRUCTION_TYPEID_TYPES(X) \
  X(uint8_t, target_idx)               \
  X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(typeid, TYPEID, 3);

/// closure
#define ZS_INSTRUCTION_NEW_CLOSURE_TYPES(X) \
  X(uint8_t, target_idx)                    \
  X(uint32_t, fct_idx)                      \
  X(uint8_t, bounded_target)
ZS_DECLARE_INSTRUCTION(new_closure, NEW_CLOSURE, 7);

/// capture
#define ZS_INSTRUCTION_GET_CAPTURE_TYPES(X) \
  X(uint8_t, target_idx)                    \
  X(uint32_t, idx)
ZS_DECLARE_INSTRUCTION(get_capture, GET_CAPTURE, 6);

/// close_enum
#define ZS_INSTRUCTION_CLOSE_ENUM_TYPES(X) X(uint8_t, idx)
ZS_DECLARE_INSTRUCTION(close_enum, CLOSE_ENUM, 2);

inline constexpr size_t k_instruction_count = (size_t)opcode::count;

inline constexpr std::array<uint8_t, k_instruction_count> k_instruction_sizes = {
#define ZS_DECL_OPCODE(name) (uint8_t)sizeof(instruction_t<ZS_OPCODE_ENUM_VALUE(name)>),
#include "lang/zopcode_def.h"
#undef ZS_DECL_OPCODE
};

inline constexpr size_t k_biggest_instruction_size = zb::maximum(
#define ZS_DECL_OPCODE(name) sizeof(instruction_t<ZS_OPCODE_ENUM_VALUE(name)>),
#include "lang/zopcode_def.h"
#undef ZS_DECL_OPCODE
    0);

template <opcode Op>
ZB_CHECK ZB_INLINE constexpr size_t get_instruction_size() noexcept {
  return sizeof(instruction_t<Op>);
}

ZB_CHECK ZB_INLINE constexpr size_t get_instruction_size(opcode op) noexcept {
  zbase_assert((size_t)op < k_instruction_count, "out of bounds instruction");
  return k_instruction_sizes[(size_t)op];
}

} // namespace zs.
