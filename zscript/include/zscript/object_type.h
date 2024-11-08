#pragma once

#include <zscript/common.h>
#include <zbase/sys/assert.h>

#define ZS_OBJECT_TYPE_ENUM_VALUE(X, name, exposed_name) X(k_##name, #name, exposed_name)

#define ZS_OBJECT_TYPE_ENUM(X)                                         \
  /* k_small_string must be zero */                                    \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, small_string, string)                   \
  /* BEGIN - Can be false */                                           \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, null, null)                             \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, bool, bool)                             \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, integer, integer)                       \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, float, float)                           \
  /* END - Can be false */                                             \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, raw_pointer, raw_pointer)               \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, string_view, string)                    \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, native_function, closure)               \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, extension, extension)                   \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, error, error)                           \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, none, null)                             \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, native_function2, closure)              \
  /* BEGIN - reference counted */                                      \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, long_string, string)                    \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, mutable_string, string)                 \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, closure, closure)                       \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, native_closure, closure)                \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, class, class)                           \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, weak_ref, weak_ref)                     \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, struct, struct)                         \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, struct_instance, instance)              \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, function_prototype, function_prototype) \
  /* BEGIN - delegable */                                              \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, table, table)                           \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, array, array)                           \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, native_array, native_array)             \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, node, node)                             \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, user_data, user_data)                   \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, instance, instance)
/* END - delegable */
/* END - reference counted */

#define ZS_EXPOSED_TYPE_ENUM_VALUE(X, name) X(ke_##name, #name)
#define ZS_EXPOSED_TYPE_ENUM(X)                     \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, null)               \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, bool)               \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, integer)            \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, float)              \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, error)              \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, raw_pointer)        \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, string)             \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, closure)            \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, class)              \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, weak_ref)           \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, function_prototype) \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, capture)            \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, table)              \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, array)              \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, native_array)       \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, node)               \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, struct)             \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, user_data)          \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, instance)           \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, extension)

#define ZS_EXTENSION_TYPE_ENUM_VALUE(X, name) X(kext_##name, #name)
#define ZS_EXTENSION_TYPE_ENUM(X)                 \
  ZS_EXTENSION_TYPE_ENUM_VALUE(X, color)          \
  ZS_EXTENSION_TYPE_ENUM_VALUE(X, array_iterator) \
  ZS_EXTENSION_TYPE_ENUM_VALUE(X, table_iterator)

#define ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, name, str, type) X(n_##name, #str, type)
#define ZS_NATIVE_ARRAY_TYPE_ENUM(X)                           \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, uint8, uint8, uint8_t)    \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, int8, int8, int8_t)       \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, uint16, uint16, uint16_t) \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, int16, int16, int16_t)    \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, uint32, uint32, uint32_t) \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, int32, int32, int32_t)    \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, uint64, uint64, uint64_t) \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, int64, int64, int64_t)    \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, float, float32, float)    \
  ZS_NATIVE_ARRAY_TYPE_ENUM_VALUE(X, double, f64, double)

// identifier, name
#define ZS_META_METHOD_ENUM(X)                \
  /* Basic arithmetic */                      \
  X(mt_add, "__operator_add")                 \
  X(mt_sub, "__operator_sub")                 \
  X(mt_mul, "__operator_mul")                 \
  X(mt_div, "__operator_div")                 \
  X(mt_exp, "__operator_exp")                 \
  X(mt_mod, "__operator_mod")                 \
  /* Basic arithmetic equal */                \
  X(mt_add_eq, "__operator_add_eq")           \
  X(mt_sub_eq, "__operator_sub_eq")           \
  X(mt_mul_eq, "__operator_mul_eq")           \
  X(mt_div_eq, "__operator_div_eq")           \
  X(mt_exp_eq, "__operator_exp_eq")           \
  X(mt_mod_eq, "__operator_mod_eq")           \
  /* Right hand size arithmetic */            \
  X(mt_rhs_add, "__operator_rhs_add")         \
  X(mt_rhs_sub, "__operator_rhs_sub")         \
  X(mt_rhs_mul, "__operator_rhs_mul")         \
  X(mt_rhs_div, "__operator_rhs_div")         \
  X(mt_rhs_exp, "__operator_rhs_exp")         \
  X(mt_rhs_mod, "__operator_rhs_mod")         \
  /* Unary */                                 \
  X(mt_unary_minus, "__operator_unm")         \
  X(mt_typeof, "__operator_typeof")           \
  X(mt_tostring, "__operator_tostring")       \
  /* Get / Set */                             \
  X(mt_set, "__operator_set")                 \
  X(mt_get, "__operator_get")                 \
  /* Others */                                \
  X(mt_next, "__operator_next")               \
  X(mt_compare, "__operator_compare")         \
  X(mt_call, "__operator_call")               \
  X(mt_cloned, "__operator_cloned")           \
  X(mt_delete_slot, "__operator_delete_slot") \
  X(mt_new_member, "__operator_new_member")   \
  X(mt_inherited, "__operator_inherited")

namespace zs {
struct object_base;
class object;
using var = object;

//
// MARK: - Object
//

///
enum class exposed_object_type : uint8_t {
#define _X(name, str) name,
  ZS_EXPOSED_TYPE_ENUM(_X)
#undef _X
};

enum class native_array_type : uint8_t {
  n_invalid,
#define _X(name, str, type) name,
  ZS_NATIVE_ARRAY_TYPE_ENUM(_X)
#undef _X
};

/// List of object types.
///
/// @warning: Look at these functions before changing the order:
///           * `is_object_type_ref_counted`
///           * `is_object_type_convertible_to_false`,
///           * `get_object_type_mask`
///
enum class object_type : uint8_t {
#define _X(name, str, exposed_name) name,
  ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
};

static_assert((uint8_t)object_type::k_instance < 32, "object_type maximum value should remain under 32");

using var_type = object_type;

enum class extension_type : uint8_t {
#define _X(name, str) name,
  ZS_EXTENSION_TYPE_ENUM(_X)
#undef _X
};

/// Get the object_type name.
ZB_CK_INLINE_CXPR const char* get_object_type_name(object_type t) noexcept {
  switch (t) {
#define _X(name, str, exposed_name) \
  case object_type::name:           \
    return str;
    ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
  }

  zbase_error("invalid type");
  return "unknown";
}

/// Get the object_type mask.
ZB_CK_INLINE_CXPR uint32_t get_object_type_mask(object_type t) noexcept {
  return 1 << static_cast<uint32_t>(t);
}

ZB_CK_INLINE_CXPR exposed_object_type to_exposed_object_type(object_type t) noexcept {
  switch (t) {
#define _X(name, str, exposed_name) \
  case object_type::name:           \
    return exposed_object_type::ke_##exposed_name;
    ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
  }

  zbase_error("invalid type");
  return exposed_object_type::ke_null;
}

/// Get the exposed_object_type name.
ZB_CK_INLINE_CXPR const char* get_exposed_object_type_name(exposed_object_type t) noexcept {
  switch (t) {
#define _X(name, str)             \
  case exposed_object_type::name: \
    return str;
    ZS_EXPOSED_TYPE_ENUM(_X)
#undef _X
  }

  zbase_error("invalid type");
  return "unknown";
}

ZB_CK_INLINE_CXPR const char* get_exposed_object_type_name(object_type t) noexcept {
  switch (t) {
#define _X(name, str, exposed_name) \
  case object_type::name:           \
    return #exposed_name;
    ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
  }

  zbase_error("invalid type");
  return "unknown";
}

// ZS_CK_INLINE_CXPR bool is_object_type_ref_counted(object_type t) noexcept {
//   return static_cast<uint8_t>(t) >= static_cast<uint8_t>(object_type::k_long_string) or (t ==
//   object_type::k_native_function2);
// }

ZS_CK_INLINE_CXPR bool is_object_type_convertible_to_false(object_type t) noexcept {
  return static_cast<uint8_t>(t) && static_cast<uint8_t>(t) <= static_cast<uint8_t>(object_type::k_float);
}

ZS_CK_INLINE_CXPR bool is_object_type_delegable(object_type t) noexcept {
  return static_cast<uint8_t>(t) >= static_cast<uint8_t>(object_type::k_table);
}

template <class T>
ZB_CK_INLINE_CXPR native_array_type to_native_array_type() noexcept {
  if constexpr (std::is_same_v<void, T>) {
    zb_static_error("Invalid type");
    return native_array_type::n_invalid;
  }

#define _X(name, str, type)                     \
  else if constexpr (std::is_same_v<type, T>) { \
    return native_array_type::name;             \
  }

  ZS_NATIVE_ARRAY_TYPE_ENUM(_X)
#undef _X

  else {
    zb_static_error("Invalid type");
    return native_array_type::n_invalid;
  }
}

ZB_CK_INLINE_CXPR native_array_type to_native_array_type(std::string_view nm) noexcept {
  if (nm == "void") {
    zbase_error("Invalid type");
    return native_array_type::n_invalid;
  }

#define _X(name, str, type)         \
  else if (nm == str) {             \
    return native_array_type::name; \
  }

  ZS_NATIVE_ARRAY_TYPE_ENUM(_X)
#undef _X

  else {
    zbase_error("Invalid type");
    return native_array_type::n_invalid;
  }
}

template <native_array_type Type>
struct native_array_type_t {};

#define _X(name, str, arr_type)                         \
  template <>                                           \
  struct native_array_type_t<native_array_type::name> { \
    using type = arr_type;                              \
  };

ZS_NATIVE_ARRAY_TYPE_ENUM(_X)
#undef _X

enum class object_type_mask : uint32_t {
#define _X(name, str, exposed_name) name = zs::get_object_type_mask(object_type::name),
  ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
};

template <class T, class... Args>
ZB_CK_INLINE_CXPR uint32_t create_type_mask(T t, Args... args) noexcept {
  if constexpr (sizeof...(Args) == 0) {
    if constexpr (std::is_same_v<T, object_type>) {
      return get_object_type_mask(t);
    }
    else if constexpr (std::is_same_v<T, object_type_mask>) {
      return static_cast<uint32_t>(t);
    }
    else if constexpr (std::is_same_v<T, uint32_t>) {
      return t;
    }
    else if constexpr (std::is_same_v<T, object_base>) {
      return get_object_type_mask(t._type);
    }
    else if constexpr (std::is_same_v<T, object>) {
      return get_object_type_mask(t._type);
    }
    else {
      zb_static_error("Should never happend");
    }
  }
  else {
    uint32_t mask = 0;

    if constexpr (std::is_same_v<T, object_type>) {
      mask |= get_object_type_mask(t);
    }
    else if constexpr (std::is_same_v<T, object_type_mask>) {
      mask |= static_cast<uint32_t>(t);
    }
    else if constexpr (std::is_same_v<T, uint32_t>) {
      mask |= t;
    }
    else if constexpr (std::is_same_v<T, object_base>) {
      mask |= get_object_type_mask(t._type);
    }
    else if constexpr (std::is_same_v<T, object>) {
      mask |= get_object_type_mask(t._type);
    }
    else {
      zb_static_error("Should never happend");
    }

    return mask | create_type_mask(args...);
  }
}

enum class meta_method : uint32_t {
#define _X(name, str) name,
  ZS_META_METHOD_ENUM(_X)
#undef _X
};

namespace constants {
  using enum object_type;

  inline constexpr const uint32_t k_number_mask = zs::create_type_mask(k_integer, k_float);

  inline constexpr const uint32_t k_number_or_bool_mask = zs::create_type_mask(k_number_mask, k_bool);

  inline constexpr const uint32_t k_string_mask
      = zs::create_type_mask(k_long_string, k_small_string, k_string_view, k_mutable_string);

  inline constexpr const uint32_t k_function_mask
      = zs::create_type_mask(k_closure, k_native_closure, k_native_function);

#define _X(name, str) inline constexpr const std::string_view k_##name##_string = str;
  ZS_META_METHOD_ENUM(_X)
#undef _X

  template <meta_method MetaMethod>
  inline zs::object get() noexcept;

} // namespace constants

inline constexpr const char* meta_method_name(meta_method mm) noexcept {
  switch (mm) {
#define _X(name, str)     \
  case meta_method::name: \
    return str;
    ZS_META_METHOD_ENUM(_X)
#undef _X
  }

  zbase_error("invalid meta method");
  return "unknown";
}
struct object_type_mask_printer {
  uint32_t mask;

  inline friend std::ostream& operator<<(std::ostream& stream, object_type_mask_printer v) {

#define _STREAM_OBJECT_TYPE_MASK(name, str, exposed_name)     \
  if (v.mask & zs::get_object_type_mask(object_type::name)) { \
    if (found_one) {                                          \
      stream << ", ";                                         \
    }                                                         \
    stream << str;                                            \
    found_one = true;                                         \
  }

    bool found_one = false;
    const bool has_string_mask = (bool)(zs::constants::k_string_mask & v.mask);
    v.mask &= ~zs::constants::k_string_mask;

    stream << "[";
    ZS_OBJECT_TYPE_ENUM(_STREAM_OBJECT_TYPE_MASK)

    return stream << (has_string_mask ? (found_one ? ", string" : "string") : "") << "]";
#undef _STREAM_OBJECT_TYPE_MASK
  }
};

// struct object_type_mask_printer2 {
//   uint32_t mask;
//
//   inline friend std::ostream& operator<<(std::ostream& stream, object_type_mask_printer2 v);
// };

} // namespace zs.
