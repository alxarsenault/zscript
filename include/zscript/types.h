#pragma once

#include <zscript/common.h>
#include <zbase/sys/assert.h>
#include <zbase/utility/integer_enum.h>
#include <zbase/container/span.h>

/// Object types.
/// @warning: Look at these functions before changing the order:
///           * `is_object_type_ref_counted`
///           * `get_object_type_mask`
#define ZS_OBJECT_TYPE_PREFIX(name) k_##name
#define ZS_OBJECT_TYPE_ENUM_VALUE(X, name, exposed_name) X(ZS_OBJECT_TYPE_PREFIX(name), #name, exposed_name)

#define ZS_OBJECT_TYPE_ENUM(X)                                     \
  /* k_small_string must be zero for small string optimization*/   \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, small_string, string)               \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, null, null)                         \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, none, none)                         \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, bool, bool)                         \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, integer, integer)                   \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, float, float)                       \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, string_view, string)                \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, native_function, closure)           \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, atom, atom)                         \
  /* BEGIN - Reference counted (Everything below k_long_string) */ \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, long_string, string)                \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, closure, closure)                   \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, native_closure, closure)            \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, weak_ref, weak_ref)                 \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, struct, struct)                     \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, struct_instance, struct_instance)   \
  /* BEGIN - Delegable (Everything below k_table) */               \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, table, table)                       \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, array, array)                       \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, user_data, user_data)

/// Exposed object types.
#define ZS_EXPOSED_TYPE_PREFIX(name) ke_##name
#define ZS_EXPOSED_TYPE_ENUM_VALUE(X, name) X(ZS_EXPOSED_TYPE_PREFIX(name), #name)

#define ZS_EXPOSED_TYPE_ENUM(X)            \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, null)      \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, none)      \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, bool)      \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, integer)   \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, float)     \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, atom)      \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, string)    \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, closure)   \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, weak_ref)  \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, table)     \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, array)     \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, struct)    \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, user_data) \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, struct_instance)

/// Atom types.
#define ZS_ATOM_NAME_PREFIX(name) atom_##name
#define ZS_ATOM_TYPE_ENUM_VALUE(X, name) X(ZS_ATOM_NAME_PREFIX(name), #name)

#define ZS_ATOM_TYPE_ENUM(X)                          \
  ZS_ATOM_TYPE_ENUM_VALUE(X, custom)                  \
  ZS_ATOM_TYPE_ENUM_VALUE(X, pointer)                 \
  ZS_ATOM_TYPE_ENUM_VALUE(X, array_iterator)          \
  ZS_ATOM_TYPE_ENUM_VALUE(X, table_iterator)          \
  ZS_ATOM_TYPE_ENUM_VALUE(X, mutable_string_iterator) \
  ZS_ATOM_TYPE_ENUM_VALUE(X, path_iterator)           \
  ZS_ATOM_TYPE_ENUM_VALUE(X, u8)                      \
  ZS_ATOM_TYPE_ENUM_VALUE(X, i8)                      \
  ZS_ATOM_TYPE_ENUM_VALUE(X, u16)                     \
  ZS_ATOM_TYPE_ENUM_VALUE(X, i16)                     \
  ZS_ATOM_TYPE_ENUM_VALUE(X, u32)                     \
  ZS_ATOM_TYPE_ENUM_VALUE(X, i32)                     \
  ZS_ATOM_TYPE_ENUM_VALUE(X, u64)                     \
  ZS_ATOM_TYPE_ENUM_VALUE(X, i64)                     \
  ZS_ATOM_TYPE_ENUM_VALUE(X, f32)                     \
  ZS_ATOM_TYPE_ENUM_VALUE(X, f64)

/// Meta methods.
#define ZS_META_METHOD_ENUM(X)       \
  X(mt_none, "__none")               \
  /* Basic arithmetic */             \
  X(mt_add, "__add")                 \
  X(mt_sub, "__sub")                 \
  X(mt_mul, "__mul")                 \
  X(mt_div, "__div")                 \
  X(mt_exp, "__exp")                 \
  X(mt_mod, "__mod")                 \
  X(mt_lshift, "__lshift")           \
  X(mt_rshift, "__rshift")           \
  X(mt_bw_or, "__bw_or")             \
  X(mt_bw_and, "__bw_and")           \
  X(mt_bw_xor, "__bw_xor")           \
  X(mt_compare, "__compare")         \
  /* Basic arithmetic equal */       \
  X(mt_add_eq, "__add_eq")           \
  X(mt_sub_eq, "__sub_eq")           \
  X(mt_mul_eq, "__mul_eq")           \
  X(mt_div_eq, "__div_eq")           \
  X(mt_exp_eq, "__exp_eq")           \
  X(mt_mod_eq, "__mod_eq")           \
  X(mt_lshift_eq, "__lshift_eq")     \
  X(mt_rshift_eq, "__rshift_eq")     \
  X(mt_bw_or_eq, "__bw_or_eq")       \
  X(mt_bw_and_eq, "__bw_and_eq")     \
  X(mt_bw_xor_eq, "__bw_xor_eq")     \
  /* Right hand size arithmetic */   \
  X(mt_rhs_add, "__rhs_add")         \
  X(mt_rhs_sub, "__rhs_sub")         \
  X(mt_rhs_mul, "__rhs_mul")         \
  X(mt_rhs_div, "__rhs_div")         \
  X(mt_rhs_exp, "__rhs_exp")         \
  X(mt_rhs_mod, "__rhs_mod")         \
  X(mt_rhs_lshift, "__rhs_lshift")   \
  X(mt_rhs_rshift, "__rhs_rshift")   \
  X(mt_rhs_bw_or, "___rhs_bw_or")    \
  X(mt_rhs_bw_and, "___rhs_bw_and")  \
  X(mt_rhs_bw_xor, "___rhs_bw_xor")  \
  X(mt_rhs_compare, "__rhs_compare") \
  /* Unary */                        \
  X(mt_unary_minus, "__unm")         \
  X(mt_incr, "__incr")               \
  X(mt_decr, "__decr")               \
  X(mt_pre_incr, "__pre_incr")       \
  X(mt_pre_decr, "__pre_decr")       \
  /* Info */                         \
  X(mt_typeof, "__typeof")           \
  X(mt_tostring, "__tostring")       \
  X(mt_copy, "__copy")               \
  /* Get - Set */                    \
  X(mt_set, "__set")                 \
  X(mt_get, "__get")                 \
  X(mt_contains, "__contains")       \
  /* Others */                       \
  X(mt_next, "__next")               \
  X(mt_call, "__call")               \
  X(mt_cloned, "__cloned")           \
  X(mt_delete_slot, "__delete_slot")

namespace zs {

//
// MARK: Forward declarations
//

template <class T>
class allocator;

struct internal;
struct object_base;
class object;
using var = object;

class engine;
class virtual_machine;
class vm;
class vm_ref;
class parameter_stream;
class parameter_list;
class delegable_object;
class weak_ref_object;
class array_object;
class struct_object;
class struct_instance_object;
class table_object;
class string_object;
class native_closure_object;
class function_prototype_object;
class closure_object;
class user_data_object;
class reference_counted_object;
class capture;
struct user_data_content;

using i8 = int8_t;
using u8 = uint8_t;
using i16 = int16_t;
using u16 = uint16_t;
using i32 = int32_t;
using u32 = uint32_t;
using i64 = int64_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

//
// MARK: Types
//

using null_t = std::nullptr_t;
using bool_t = bool;
using int_t = int64_t;
using uint_t = uint64_t;
using float_t = double;
using raw_pointer_t = void*;

using function_t = int_t (*)(zs::vm_ref);

using alloc_info_t = uint32_t;

/// Allocate function.
///
/// A null ptr with a size greater than zero is equivalent to malloc.
/// A non-null ptr with a size greater than zero is equivalent to realloc.
/// A non-null ptr with a size of zero is equivalent to a free.
///
/// @param eng Engine.
/// @param user_ptr User pointer from engine.
/// @param ptr The pointer.
/// @param size The size to allocate.
/// @param old_size The previous size.
/// @param info Allocation info.
using allocate_t = void* (*)(zs::engine* eng, raw_pointer_t user_ptr, //
    void* ptr, size_t size, size_t old_size, alloc_info_t info);

using raw_pointer_release_hook_t = void (*)(allocate_t, zs::raw_pointer_t);
using native_closure_release_hook_t = void (*)(zs::engine*, zs::raw_pointer_t);
using user_data_release_hook_t = void (*)(zs::engine*, zs::raw_pointer_t);
using release_hook_t = int_t (*)(raw_pointer_t, int_t);

//
using stream_getter_t = std::ostream& (*)(zs::engine*, zs::raw_pointer_t);

//
using write_function_t = zs::error_result (*)(const uint8_t*, size_t size, void* data);

//
using copy_user_data_to_type_t = zs::error_result (*)(void*, size_t, std::string_view, void* data);

//
using to_string_callback_t = zs::error_result (*)(const zs::object_base&, std::ostream&);

///

using engine_initializer_t = zs::error_result (*)(zs::engine*);

//
// MARK: Defaults
//

/// The global default allocate function.
extern const allocate_t default_allocate;
extern const stream_getter_t default_stream_getter;
extern const engine_initializer_t default_engine_initializer;

//
// MARK: Config
//

struct config_t {
  size_t stack_size = ZS_DEFAULT_STACK_SIZE;
  allocate_t alloc_callback = ZS_DEFAULT_ALLOCATE;
  raw_pointer_t user_pointer = nullptr;
  raw_pointer_release_hook_t user_release = nullptr;
  stream_getter_t stream_getter = ZS_DEFAULT_STREAM_GETTER;
  engine_initializer_t initializer = ZS_DEFAULT_ENGINE_INITIALIZER;
};

//
// MARK: Constants
//

///
enum class object_flags_t : uint8_t { //
  f_none,
  f_enum_table = 2,
  f_meta_argument = 4,
  f_char = 8
};

//ZBASE_ENUM_CLASS_FLAGS(object_flags_t);
 
///
enum class delegate_flags_t : uint8_t { //
  df_none,
  df_use_default = 1,
  df_locked = 2
};

//ZBASE_ENUM_CLASS_FLAGS(delegate_flags_t);

///
enum class variable_attribute_t : uint8_t {
  va_none,
  va_const = 1,
  va_static = 2,
  va_private = 4,
  va_mutable = 8,
  va_doc = 16
};

// ZBASE_ENUM_CLASS_FLAGS(variable_attribute_t);

struct variable_type_info {
  using enum variable_attribute_t;

  uint64_t custom_mask = 0;
  uint32_t mask = 0;
  variable_attribute_t flags = va_none;

  ZS_CK_INLINE_CXPR bool has_mask() const noexcept { return mask != 0 or custom_mask != 0; }
  ZS_CK_INLINE_CXPR bool has_custom_mask() const noexcept { return custom_mask != 0; }

  ZS_INLINE_CXPR void set_const(bool value) noexcept { zb::set_flag<va_const>(flags, value); }
  ZS_INLINE_CXPR void set_static(bool value) noexcept { zb::set_flag<va_static>(flags, value); }
  ZS_INLINE_CXPR void set_private(bool value) noexcept { zb::set_flag<va_private>(flags, value); }
  ZS_INLINE_CXPR void set_mutable(bool value) noexcept { zb::set_flag<va_mutable>(flags, value); }
  ZS_INLINE_CXPR void set_doc(bool value) noexcept { zb::set_flag<va_doc>(flags, value); }

  ZS_INLINE_CXPR void set_const() noexcept { zb::set_flag<va_const>(flags); }
  ZS_INLINE_CXPR void set_static() noexcept { zb::set_flag<va_static>(flags); }
  ZS_INLINE_CXPR void set_private() noexcept { zb::set_flag<va_private>(flags); }
  ZS_INLINE_CXPR void set_mutable() noexcept { zb::set_flag<va_mutable>(flags); }
  ZS_INLINE_CXPR void set_doc() noexcept { zb::set_flag<va_doc>(flags); }

  ZS_CK_INLINE_CXPR bool is_const() const noexcept { return zb::has_flag<va_const>(flags); }
  ZS_CK_INLINE_CXPR bool is_static() const noexcept { return zb::has_flag<va_static>(flags); }
  ZS_CK_INLINE_CXPR bool is_private() const noexcept { return zb::has_flag<va_private>(flags); }
  ZS_CK_INLINE_CXPR bool is_mutable() const noexcept { return zb::has_flag<va_mutable>(flags); }
  ZS_CK_INLINE_CXPR bool is_doc() const noexcept { return zb::has_flag<va_doc>(flags); }
};

//
// MARK: - Object
//

/// Atom types.
enum class atom_type : uint8_t {
#define _X(name, str) name,
  ZS_ATOM_TYPE_ENUM(_X)
#undef _X
};

/// Exposed object types.
enum class exposed_object_type : uint16_t {
#define _X(name, str) name,
  ZS_EXPOSED_TYPE_ENUM(_X)
#undef _X
};

/// Object types.
enum class object_type : uint8_t {
#define _X(name, str, exposed_name) name,
  ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
};

// Meta methods.
enum class meta_method : uint32_t {
#define _X(name, str) name,
  ZS_META_METHOD_ENUM(_X)
#undef _X
};

static_assert((uint8_t)object_type::k_user_data < 32, "object_type maximum value should remain under 32");

/// Get the object_type name.
ZB_CK_INLINE_CXPR const char* get_object_type_name(object_type t) noexcept {
  switch (t) {
#define _X(name, str, exposed_name) \
  case object_type::name:           \
    return str;

    ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
  }

  ZS_ERROR("invalid type");
  return "unknown";
}

/// Get the object_type mask.
ZB_CK_INLINE_CXPR uint32_t get_object_type_mask(object_type t) noexcept {
  return 1 << static_cast<uint32_t>(t);
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

  ZS_ERROR("invalid type");
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

  ZS_ERROR("invalid type");
  return "unknown";
}

ZS_CK_INLINE_CXPR bool is_object_type_ref_counted(object_type t) noexcept {
  return static_cast<uint8_t>(t) >= static_cast<uint8_t>(object_type::k_long_string);
}

ZS_CK_INLINE_CXPR bool is_object_type_delegable(object_type t) noexcept {
  return static_cast<uint8_t>(t) >= static_cast<uint8_t>(object_type::k_table);
}

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

namespace constants {
  using enum object_type;

  inline constexpr const uint32_t k_number_mask = zs::create_type_mask(k_integer, k_float);

  inline constexpr const uint32_t k_number_or_bool_mask = zs::create_type_mask(k_number_mask, k_bool);

  inline constexpr const uint32_t k_string_mask
      = zs::create_type_mask(k_long_string, k_small_string, k_string_view);

  inline constexpr const uint32_t k_cstring_mask = zs::create_type_mask(k_long_string, k_small_string);

  inline constexpr const uint32_t k_function_mask
      = zs::create_type_mask(k_closure, k_native_closure, k_native_function);

  inline constexpr const uint32_t k_delegate_mask = zs::create_type_mask(k_table, k_null, k_none);

  inline constexpr const uint32_t k_meta_type_mask = zs::create_type_mask(k_table, k_struct_instance);

  inline constexpr const uint32_t k_unary_arithmetic_type_mask
      = zs::create_type_mask(k_bool, k_integer, k_float, k_atom, k_table, k_struct_instance, k_user_data);

  /// Atom delegate ids.
  inline constexpr const uint8_t k_atom_array_iterator_delegate_id = 1;
  inline constexpr const uint8_t k_atom_table_iterator_delegate_id = 2;
  inline constexpr const uint8_t k_atom_mutable_string_iterator_delegate_id = 3;
  inline constexpr const uint8_t k_atom_path_iterator_delegate_id = 4;

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

  ZS_ERROR("invalid meta method");
  return "unknown";
}

inline constexpr meta_method get_rhs_meta_method(meta_method mm) noexcept {
  using enum meta_method;

  // clang-format off
  switch (mm) {
  case mt_add: return mt_rhs_add;
  case mt_sub: return mt_rhs_sub;
  case mt_mul: return mt_rhs_mul;
  case mt_div: return mt_rhs_div;
  case mt_mod: return mt_rhs_mod;
  case mt_exp: return mt_rhs_exp;
  case mt_lshift: return mt_rhs_lshift;
  case mt_rshift: return mt_rhs_rshift;
  case mt_bw_or: return mt_rhs_bw_or;
  case mt_bw_and: return mt_rhs_bw_and;
  case mt_bw_xor: return mt_rhs_bw_xor;
    default: return mt_none;
  }
  // clang-format on

  return mt_none;
}

struct object_type_mask_printer {
  uint32_t mask;
  const char* l = "[";
  const char* r = "]";

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
    stream << v.l;
    ZS_OBJECT_TYPE_ENUM(_STREAM_OBJECT_TYPE_MASK)

    return stream << (has_string_mask ? (found_one ? ", string" : "string") : "") << v.r;
#undef _STREAM_OBJECT_TYPE_MASK
  }
};

namespace constants {
  inline constexpr size_t k_pointer_size = sizeof(void*);
  inline constexpr size_t k_object_size = 16;
  inline constexpr size_t k_small_string_max_size
      = k_object_size - (sizeof(object_flags_t) + sizeof(object_type));

  inline constexpr alloc_info_t k_engine_deallocation = 80198;
  inline constexpr alloc_info_t k_user_pointer_deallocation = 80199;

} // namespace constants.

/// @struct line_info.
struct line_info {
  line_info() = default;
  inline line_info(int_t l, int_t col)
      : line(l)
      , column(col) {}

  int_t line = 0;
  int_t column = 0;

  inline friend std::ostream& operator<<(std::ostream& stream, const line_info& linfo) {
    return stream << "at line " << linfo.line << " column " << linfo.column;
  }
};
/// Version struct.
struct version_t {
  uint8_t major, minor, patch, build;

  inline friend std::ostream& operator<<(std::ostream& stream, const version_t& v) {
    return stream << "zscript " << (int)v.major << "." << (int)v.minor << "." << (int)v.patch;
  }
};

inline constexpr version_t k_version
    = version_t{ ZS_VERSION_MAJOR, ZS_VERSION_MINOR, ZS_VERSION_PATCH, ZS_VERSION_BUILD };

ZB_CK_INLINE_CXPR version_t version() noexcept {
  return version_t{ ZS_VERSION_MAJOR, ZS_VERSION_MINOR, ZS_VERSION_PATCH, ZS_VERSION_BUILD };
}
} // namespace zs.
