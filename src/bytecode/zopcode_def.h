
/// op_line.
#define ZS_INSTRUCTION_LINE(X) X(i64, line)
ZS_DECL_OPCODE(line, ZS_INSTRUCTION_LINE)

/// op_load.
#define ZS_INSTRUCTION_LOAD(X) \
  X(u8, target_idx)            \
  X(u32, idx)
ZS_DECL_OPCODE(load, ZS_INSTRUCTION_LOAD)

/// op_load_bool.
#define ZS_INSTRUCTION_LOAD_BOOL(X) \
  X(u8, target_idx)                 \
  X(bool_t, value)
ZS_DECL_OPCODE(load_bool, ZS_INSTRUCTION_LOAD_BOOL)

/// op_load_int.
#define ZS_INSTRUCTION_LOAD_INT(X) \
  X(u8, target_idx)                \
  X(int_t, value)
ZS_DECL_OPCODE(load_int, ZS_INSTRUCTION_LOAD_INT)

/// op_load_float.
#define ZS_INSTRUCTION_LOAD_FLOAT(X) \
  X(u8, target_idx)                  \
  X(float_t, value)
ZS_DECL_OPCODE(load_float, ZS_INSTRUCTION_LOAD_FLOAT)

// op_load_small_string.
#define ZS_INSTRUCTION_LOAD_SMALL_STRING(X) \
  X(u8, target_idx)                         \
  X(ss_inst_data, value)
ZS_DECL_OPCODE(load_small_string, ZS_INSTRUCTION_LOAD_SMALL_STRING)

/// op_load_string.
#define ZS_INSTRUCTION_LOAD_STRING(X) \
  X(u8, target_idx)                   \
  X(u32, idx)
ZS_DECL_OPCODE(load_string, ZS_INSTRUCTION_LOAD_STRING)

/// op_load_null.
#define ZS_INSTRUCTION_LOAD_NULL(X) X(u8, target_idx)
ZS_DECL_OPCODE(load_null, ZS_INSTRUCTION_LOAD_NULL)

/// op_load_nulls.
#define ZS_INSTRUCTION_LOAD_NULLS(X) \
  X(u8, target_idx)                  \
  X(u8, count)
ZS_DECL_OPCODE(load_nulls, ZS_INSTRUCTION_LOAD_NULLS)

/// op_load_none.
#define ZS_INSTRUCTION_LOAD_NONE(X) X(u8, target_idx)
ZS_DECL_OPCODE(load_none, ZS_INSTRUCTION_LOAD_NONE)

/// op_load_object.
#define ZS_INSTRUCTION_LOAD_OBJECT(X) \
  X(u8, target_idx)                   \
  X(object_base, value)
ZS_DECL_OPCODE(load_object, ZS_INSTRUCTION_LOAD_OBJECT)

/// op_load_root.
#define ZS_INSTRUCTION_LOAD_ROOT(X) X(u8, target_idx)
ZS_DECL_OPCODE(load_root, ZS_INSTRUCTION_LOAD_ROOT)

/// op_load_global.
#define ZS_INSTRUCTION_LOAD_GLOBAL(X) X(u8, target_idx)
ZS_DECL_OPCODE(load_global, ZS_INSTRUCTION_LOAD_GLOBAL)

/// op_set_meta_argument.
#define ZS_INSTRUCTION_SET_META_ARGUMENT(X) X(u8, idx)
ZS_DECL_OPCODE(set_meta_argument, ZS_INSTRUCTION_SET_META_ARGUMENT)

/// op_call.
#define ZS_INSTRUCTION_CALL(X) \
  X(u8, target_idx)            \
  X(u8, closure_idx)           \
  X(u8, this_idx)              \
  X(u8, n_params)              \
  X(u64, stack_base)
ZS_DECL_OPCODE(call, ZS_INSTRUCTION_CALL)

/// op_move.
#define ZS_INSTRUCTION_MOVE(X) \
  X(u8, target_idx)            \
  X(u8, idx)
ZS_DECL_OPCODE(move, ZS_INSTRUCTION_MOVE)

/// op_assign.
#define ZS_INSTRUCTION_ASSIGN(X) \
  X(u8, target_idx)            \
  X(u8, idx)
ZS_DECL_OPCODE(assign, ZS_INSTRUCTION_ASSIGN)

/// op_assign_w_mask.
#define ZS_INSTRUCTION_ASSIGN_W_MASK(X) \
  X(u8, target_idx)              \
  X(u8, idx)                     \
  X(u32, mask)
ZS_DECL_OPCODE(assign_w_mask, ZS_INSTRUCTION_ASSIGN_W_MASK)

/// op_assign_custom.
#define ZS_INSTRUCTION_ASSIGN_CUSTOM(X) \
  X(u8, target_idx)                     \
  X(u8, idx)                            \
  X(u32, mask)                          \
  X(u64, custom_mask)
ZS_DECL_OPCODE(assign_custom, ZS_INSTRUCTION_ASSIGN_CUSTOM)

/// op_set_capture.
#define ZS_INSTRUCTION_SET_CAPTURE(X) \
  X(u8, target_idx)                   \
  X(u32, capture_idx)                 \
  X(u8, value_idx)
ZS_DECL_OPCODE(set_capture, ZS_INSTRUCTION_SET_CAPTURE)

/// op_set.
#define ZS_INSTRUCTION_SET(X) \
  X(u8, target_idx)           \
  X(u8, table_idx)            \
  X(u8, key_idx)              \
  X(u8, value_idx)            \
  X(bool, can_create)
ZS_DECL_OPCODE(set, ZS_INSTRUCTION_SET)

/// op_set_ss.
#define ZS_INSTRUCTION_SET_SS(X) \
  X(u8, target_idx)              \
  X(u8, table_idx)               \
  X(ss_inst_data, key)           \
  X(u8, value_idx)               \
  X(bool, can_create)
ZS_DECL_OPCODE(set_ss, ZS_INSTRUCTION_SET_SS)

/// op_rawset.
#define ZS_INSTRUCTION_RAWSET(X) \
  X(u8, target_idx)              \
  X(u8, table_idx)               \
  X(u8, key_idx)                 \
  X(u8, value_idx)               \
  X(bool, can_create)
ZS_DECL_OPCODE(rawset, ZS_INSTRUCTION_RAWSET)

/// op_rawset_ss.
#define ZS_INSTRUCTION_RAWSET_SS(X) \
  X(u8, target_idx)                 \
  X(u8, table_idx)                  \
  X(ss_inst_data, key)              \
  X(u8, value_idx)                  \
  X(bool, can_create)
ZS_DECL_OPCODE(rawset_ss, ZS_INSTRUCTION_RAWSET_SS)

/// op_get.
#define ZS_INSTRUCTION_GET(X) \
  X(u8, target_idx)           \
  X(u8, table_idx)            \
  X(u8, key_idx)              \
  X(get_op_flags_t, flags)
ZS_DECL_OPCODE(get, ZS_INSTRUCTION_GET)

/// op_eq.
#define ZS_INSTRUCTION_EQUALS(X) \
  X(u8, target_idx)              \
  X(u8, lhs_idx)                 \
  X(u32, rhs_idx)                \
  X(bool, is_literal)
ZS_DECL_OPCODE(eq, ZS_INSTRUCTION_EQUALS) // ==

/// op_ne.
#define ZS_INSTRUCTION_NOT_EQUALS(X) \
  X(u8, target_idx)                  \
  X(u8, lhs_idx)                     \
  X(u32, rhs_idx)                    \
  X(bool, is_literal)
ZS_DECL_OPCODE(ne, ZS_INSTRUCTION_NOT_EQUALS)

/// Arithmetic.
/// Get the stack value at `lhs_idx`.
/// Get the stack value at `rhs_idx`.
/// Compute arithmetic operation.
/// Set the result value in the stack at `target_idx`.
#define ZS_INSTRUCTION_ARITHMETIC(X) \
  X(u8, target_idx)                  \
  X(u8, lhs_idx)                     \
  X(u8, rhs_idx)

ZS_DECL_OPCODE(add, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(sub, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(mul, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(div, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(mod, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(exp, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(bitwise_or, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(bitwise_and, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(bitwise_xor, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(lshift, ZS_INSTRUCTION_ARITHMETIC)
ZS_DECL_OPCODE(rshift, ZS_INSTRUCTION_ARITHMETIC)

/// Get the stack value at `target_idx`.
/// Get the stack value at `rhs_idx`.
/// Compute arithmetic equal operation.
/// Set the result value in the stack at `target_idx`.
#define ZS_INSTRUCTION_ARITHMETIC_EQ(X) \
  X(u8, target_idx)                     \
  X(u8, rhs_idx)

ZS_DECL_OPCODE(add_eq, ZS_INSTRUCTION_ARITHMETIC_EQ)
ZS_DECL_OPCODE(sub_eq, ZS_INSTRUCTION_ARITHMETIC_EQ)
ZS_DECL_OPCODE(mul_eq, ZS_INSTRUCTION_ARITHMETIC_EQ)
ZS_DECL_OPCODE(div_eq, ZS_INSTRUCTION_ARITHMETIC_EQ)
ZS_DECL_OPCODE(mod_eq, ZS_INSTRUCTION_ARITHMETIC_EQ)
ZS_DECL_OPCODE(exp_eq, ZS_INSTRUCTION_ARITHMETIC_EQ)

/// op_object_add_eq.
#define ZS_INSTRUCTION_OBJECT_ADD_EQ(X) \
  X(u8, target_idx)                     \
  X(u8, obj_idx)                        \
  X(u8, key_idx)                        \
  X(u8, value_idx)
ZS_DECL_OPCODE(object_add_eq, ZS_INSTRUCTION_OBJECT_ADD_EQ)

/// op_object_mul_eq.
#define ZS_INSTRUCTION_OBJECT_MUL_EQ(X) \
  X(u8, target_idx)                     \
  X(u8, obj_idx)                        \
  X(u8, key_idx)                        \
  X(u8, value_idx)
ZS_DECL_OPCODE(object_mul_eq, ZS_INSTRUCTION_OBJECT_MUL_EQ)

/// op_return.
#define ZS_INSTRUCTION_RETURN(X) \
  X(u8, idx)                     \
  X(bool, has_value)
ZS_DECL_OPCODE(return, ZS_INSTRUCTION_RETURN)

/// op_return_export.
#define ZS_INSTRUCTION_RETURN_EXPORT(X) X(u8, idx)
ZS_DECL_OPCODE(return_export, ZS_INSTRUCTION_RETURN_EXPORT)

/// op_use.
#define ZS_INSTRUCTION_USE(X) \
  X(u8, target_idx)           \
  X(u8, src_idx)
ZS_DECL_OPCODE(use, ZS_INSTRUCTION_USE)

/// op_jmp.
#define ZS_INSTRUCTION_JMP(X) X(i32, offset)
ZS_DECL_OPCODE(jmp, ZS_INSTRUCTION_JMP)

/// op_jz.
#define ZS_INSTRUCTION_JZ(X) \
  X(i32, offset)             \
  X(u8, idx)
ZS_DECL_OPCODE(jz, ZS_INSTRUCTION_JZ)

/// op_if_null.
#define ZS_INSTRUCTION_IF_NULL(X) \
  X(u8, target_idx)               \
  X(u8, value_idx)                \
  X(i32, offset)                  \
  X(bool, null_only)
ZS_DECL_OPCODE(if_null, ZS_INSTRUCTION_IF_NULL)

/// op_get_capture.
#define ZS_INSTRUCTION_GET_CAPTURE(X) \
  X(u8, target_idx)                   \
  X(u32, idx)
ZS_DECL_OPCODE(get_capture, ZS_INSTRUCTION_GET_CAPTURE)

/// op_new_obj.
#define ZS_INSTRUCTION_NEW_OBJ(X) \
  X(u8, target_idx)               \
  X(object_type, type)
ZS_DECL_OPCODE(new_obj, ZS_INSTRUCTION_NEW_OBJ)

/// op_load_lib_ss.
#define ZS_INSTRUCTION_LOAD_LIB_SS(X) \
  X(u8, target_idx)                   \
  X(ss_inst_data, key)
ZS_DECL_OPCODE(load_lib_ss, ZS_INSTRUCTION_LOAD_LIB_SS)

/// op_new_slot.
#define ZS_INSTRUCTION_NEW_SLOT(X) \
  X(u8, table_idx)                 \
  X(u8, key_idx)                   \
  X(u8, value_idx)
ZS_DECL_OPCODE(new_slot, ZS_INSTRUCTION_NEW_SLOT)

/// op_new_slot_ss.
#define ZS_INSTRUCTION_NEW_SLOT_SS(X) \
  X(u8, table_idx)                    \
  X(ss_inst_data, key)                \
  X(u8, value_idx)
ZS_DECL_OPCODE(new_slot_ss, ZS_INSTRUCTION_NEW_SLOT_SS)

/// op_new_slot_ss_integer.
#define ZS_INSTRUCTION_NEW_SLOT_SS_INTEGER(X) \
  X(u8, table_idx)                            \
  X(ss_inst_data, key)                        \
  X(int_t, value)
ZS_DECL_OPCODE(new_slot_ss_integer, ZS_INSTRUCTION_NEW_SLOT_SS_INTEGER)

/// op_new_slot_ss_float.
#define ZS_INSTRUCTION_NEW_SLOT_SS_FLOAT(X) \
  X(u8, table_idx)                          \
  X(ss_inst_data, key)                      \
  X(float_t, value)
ZS_DECL_OPCODE(new_slot_ss_float, ZS_INSTRUCTION_NEW_SLOT_SS_FLOAT)

/// op_new_slot_ss_bool.
#define ZS_INSTRUCTION_NEW_SLOT_SS_BOOL(X) \
  X(u8, table_idx)                         \
  X(ss_inst_data, key)                     \
  X(bool_t, value)
ZS_DECL_OPCODE(new_slot_ss_bool, ZS_INSTRUCTION_NEW_SLOT_SS_BOOL)

/// op_new_slot_ss_small_string.
#define ZS_INSTRUCTION_NEW_SLOT_SS_SMALL_STRING(X) \
  X(u8, table_idx)                                 \
  X(ss_inst_data, key)                             \
  X(ss_inst_data, value)
ZS_DECL_OPCODE(new_slot_ss_small_string, ZS_INSTRUCTION_NEW_SLOT_SS_SMALL_STRING)

/// op_new_class_slot.
#define ZS_INSTRUCTION_NEW_CLASS_SLOT(X) \
  X(u8, table_idx)                       \
  X(u8, key_idx)                         \
  X(u8, value_idx)                       \
  X(bool, is_static)
ZS_DECL_OPCODE(new_class_slot, ZS_INSTRUCTION_NEW_CLASS_SLOT)

/// op_new_struct_slot.
#define ZS_INSTRUCTION_NEW_STRUCT_SLOT(X) \
  X(u8, struct_idx)                       \
  X(u8, key_idx)                          \
  X(u8, value_idx)                        \
  X(u32, mask)                            \
  X(var_decl_flags_t, vdecl_flags)
ZS_DECL_OPCODE(new_struct_slot, ZS_INSTRUCTION_NEW_STRUCT_SLOT)

/// op_new_struct_slot_ss
#define ZS_INSTRUCTION_NEW_STRUCT_SLOT_SS(X) \
  X(u8, struct_idx)                          \
  X(ss_inst_data, key)                       \
  X(u8, value_idx)                           \
  X(u32, mask)                               \
  X(var_decl_flags_t, vdecl_flags)
ZS_DECL_OPCODE(new_struct_slot_ss, ZS_INSTRUCTION_NEW_STRUCT_SLOT_SS)

/// op_new_struct_default_constructor.
#define ZS_INSTRUCTION_NEW_STRUCT_DEFAULT_CONSTRUCTOR(X) X(u8, struct_idx)
ZS_DECL_OPCODE(new_struct_default_constructor, ZS_INSTRUCTION_NEW_STRUCT_DEFAULT_CONSTRUCTOR)

/// op_new_struct_constructor.
#define ZS_INSTRUCTION_NEW_STRUCT_CONSTRUCTOR(X) \
  X(u8, struct_idx)                              \
  X(u32, fct_idx)
ZS_DECL_OPCODE(new_struct_constructor, ZS_INSTRUCTION_NEW_STRUCT_CONSTRUCTOR)

/// op_new_struct_method.
#define ZS_INSTRUCTION_NEW_STRUCT_METHOD(X) \
  X(u8, struct_idx)                         \
  X(u32, fct_idx)                           \
  X(var_decl_flags_t, decl_flag)
ZS_DECL_OPCODE(new_struct_method, ZS_INSTRUCTION_NEW_STRUCT_METHOD)

/// op_set_struct_name.
#define ZS_INSTRUCTION_SET_STRUCT_NAME(X) \
  X(u8, struct_idx)                       \
  X(u8, name_idx)
ZS_DECL_OPCODE(set_struct_name, ZS_INSTRUCTION_SET_STRUCT_NAME)

/// op_set_struct_doc.
#define ZS_INSTRUCTION_SET_STRUCT_DOC(X) \
  X(u8, struct_idx)                      \
  X(u8, doc_idx)
ZS_DECL_OPCODE(set_struct_doc, ZS_INSTRUCTION_SET_STRUCT_DOC)

/// op_set_struct_member_doc.
#define ZS_INSTRUCTION_SET_STRUCT_MEMBER_DOC(X) \
  X(u8, struct_idx)                             \
  X(u8, name_idx)                               \
  X(u8, doc_idx)
ZS_DECL_OPCODE(set_struct_member_doc, ZS_INSTRUCTION_SET_STRUCT_MEMBER_DOC)

/// op_array_append.
#define ZS_INSTRUCTION_ARRAY_APPEND(X) \
  X(u8, array_idx)                     \
  X(u8, idx)
ZS_DECL_OPCODE(array_append, ZS_INSTRUCTION_ARRAY_APPEND)

/// op_incr.
/// i++ or i--
#define ZS_INSTRUCTION_INCR(X) \
  X(u8, target_idx)            \
  X(u8, idx)                   \
  X(bool, is_incr)
ZS_DECL_OPCODE(incr, ZS_INSTRUCTION_INCR)

/// op_pincr.
/// ++i or --i
#define ZS_INSTRUCTION_PINCR(X) \
  X(u8, target_idx)             \
  X(u8, idx)                    \
  X(bool, is_incr)
ZS_DECL_OPCODE(pincr, ZS_INSTRUCTION_PINCR)

/// op_pobjincr.
/// ++i or --i
#define ZS_INSTRUCTION_POBJINCR(X) \
  X(u8, target_idx)                \
  X(u8, table_ix)                  \
  X(u8, key_ix)                    \
  X(bool, is_incr)
ZS_DECL_OPCODE(pobjincr, ZS_INSTRUCTION_POBJINCR)

/// op_cmp.
#define ZS_INSTRUCTION_CMP(X) \
  X(u8, target_idx)           \
  X(compare_op, cmp_op)       \
  X(u8, lhs_idx)              \
  X(u8, rhs_idx)
ZS_DECL_OPCODE(cmp, ZS_INSTRUCTION_CMP)

/// op_exists.
#define ZS_INSTRUCTION_EXISTS(X) \
  X(u8, target_idx)              \
  X(u8, table_idx)               \
  X(u8, key_idx)
ZS_DECL_OPCODE(exists, ZS_INSTRUCTION_EXISTS)

/// op_not.
#define ZS_INSTRUCTION_NOT(X) \
  X(u8, target_idx)           \
  X(u8, idx)
ZS_DECL_OPCODE(not, ZS_INSTRUCTION_NOT)

/// op_obj_not.
#define ZS_INSTRUCTION_OBJ_NOT(X) \
  X(u8, target_idx)               \
  X(u8, table_idx)                \
  X(u8, key_idx)
ZS_DECL_OPCODE(obj_not, ZS_INSTRUCTION_OBJ_NOT)

/// op_and.
#define ZS_INSTRUCTION_AND(X) \
  X(u8, target_idx)           \
  X(u8, src_idx)              \
  X(i32, offset)
ZS_DECL_OPCODE(and, ZS_INSTRUCTION_AND)

/// op_or.
#define ZS_INSTRUCTION_OR(X) \
  X(u8, target_idx)          \
  X(u8, src_idx)             \
  X(i32, offset)
ZS_DECL_OPCODE(or, ZS_INSTRUCTION_OR)

/// op_new_closure.
#define ZS_INSTRUCTION_NEW_CLOSURE(X) \
  X(u8, target_idx)                   \
  X(u32, fct_idx)                     \
  X(u8, bounded_target)
ZS_DECL_OPCODE(new_closure, ZS_INSTRUCTION_NEW_CLOSURE)

/// op_clone.
#define ZS_INSTRUCTION_CLONE(X) \
  X(u8, target_idx)             \
  X(u8, idx)
ZS_DECL_OPCODE(clone, ZS_INSTRUCTION_CLONE)

/// op_typeof.
#define ZS_INSTRUCTION_TYPEOF(X) \
  X(u8, target_idx)              \
  X(u8, idx)
ZS_DECL_OPCODE(typeof, ZS_INSTRUCTION_TYPEOF)

/// op_typeid.
#define ZS_INSTRUCTION_TYPEID(X) \
  X(u8, target_idx)              \
  X(u8, idx)
ZS_DECL_OPCODE(typeid, ZS_INSTRUCTION_TYPEID)

/// op_get_base.
#define ZS_INSTRUCTION_GET_BASE(X) X(u8, target_idx)
ZS_DECL_OPCODE(get_base, ZS_INSTRUCTION_GET_BASE)

/// op_close.
#define ZS_INSTRUCTION_CLOSE(X) X(u32, stack_size)
ZS_DECL_OPCODE(close, ZS_INSTRUCTION_CLOSE)

/// op_check_type.
/// Makes sure that the object at stack index `idx` is of `obj_type` type.
#define ZS_INSTRUCTION_CHECK_TYPE(X) \
  X(u8, idx)                         \
  X(object_type, obj_type)
ZS_DECL_OPCODE(check_type, ZS_INSTRUCTION_CHECK_TYPE)

/// op_check_type_mask.
/// Makes sure that the object at stack index `idx` has a type (type mask) in
/// the ones given in `mask`.
#define ZS_INSTRUCTION_CHECK_TYPE_MASK(X) \
  X(u8, idx)                              \
  X(u32, mask)
ZS_DECL_OPCODE(check_type_mask, ZS_INSTRUCTION_CHECK_TYPE_MASK)

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
#define ZS_INSTRUCTION_CHECK_CUSTOM_TYPE_MASK(X) \
  X(u8, idx)                                     \
  X(u32, mask)                                   \
  X(u64, custom_mask)
ZS_DECL_OPCODE(check_custom_type_mask, ZS_INSTRUCTION_CHECK_CUSTOM_TYPE_MASK)

/// op_new_enum_slot.
#define ZS_INSTRUCTION_NEW_ENUM_SLOT(X) \
  X(u8, table_idx)                      \
  X(u8, key_idx)                        \
  X(u8, value_idx)
ZS_DECL_OPCODE(new_enum_slot, ZS_INSTRUCTION_NEW_ENUM_SLOT)

/// op_close_enum.
#define ZS_INSTRUCTION_CLOSE_ENUM(X) X(u8, idx)
ZS_DECL_OPCODE(close_enum, ZS_INSTRUCTION_CLOSE_ENUM)

///// op_arith.
// #d efine ZS_INSTRUCTION_ARITH(X) \
//  X(u8, target_idx)             \
//  X(u8, lhs_idx)                \
//  X(u8, rhs_idx)                \
//  X(arithmetic_op, aop)
// ZS_DECL_OPCODE(arith, ZS_INSTRUCTION_ARITH)
