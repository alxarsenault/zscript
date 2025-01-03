#pragma once

#include <zscript/base/zbase.h>
#include <zscript/base/sys/assert.h>

#include <zscript/base/sys/assert.h>
#include <zscript/base/sys/error_code.h>
#include <zscript/base/container/span.h>
#include <zscript/base/utility/integer_enum.h>

#include <zscript/base/crypto/hash.h>
#include <zscript/base/container/small_vector.h>
#include <zscript/base/container/span.h>
#include <zscript/base/container/vector.h>
#include <zscript/base/strings/string_view.h>

#include <sstream>
#include <unordered_map>
#include <unordered_set>

#if ZS_CONFIG_USE_EXCEPTION
#include <exception>
#endif // ZS_CONFIG_USE_EXCEPTION.

#define ZS_DEBUG 1
#define ZS_MEMORY_PROFILER 1
#define ZS_GARBAGE_COLLECTOR 1
#define ZS_USE_ENGINE_GLOBAL_REF_COUNT 1

#define ZS_CONFIG_USE_EXCEPTION 1

#ifndef ZS_VERSION_MAJOR
#define ZS_VERSION_MAJOR 0
#endif // ZS_VERSION_MAJOR.

#ifndef ZS_VERSION_MINOR
#define ZS_VERSION_MINOR 0
#endif // ZS_VERSION_MINOR.

#ifndef ZS_VERSION_PATCH
#define ZS_VERSION_PATCH 0
#endif // ZS_VERSION_PATCH.

#ifndef ZS_VERSION_BUILD
#define ZS_VERSION_BUILD 0
#endif // ZS_VERSION_BUILD.

//-------------------------------------------------------------------

#ifndef ZS_DEBUG
#define ZS_DEBUG 0
#elif ZBASE_IS_MACRO_EMPTY(ZS_DEBUG)
#error "ZS_DEBUG requires a value"
#endif // ZS_DEBUG.

#ifndef ZS_GARBAGE_COLLECTOR
#define ZS_GARBAGE_COLLECTOR 1
#elif ZBASE_IS_MACRO_EMPTY(ZS_GARBAGE_COLLECTOR)
#error "ZS_GARBAGE_COLLECTOR requires a value"
#endif // ZS_GARBAGE_COLLECTOR.

#ifndef ZS_USE_ENGINE_GLOBAL_REF_COUNT
#define ZS_USE_ENGINE_GLOBAL_REF_COUNT 0
#elif ZBASE_IS_MACRO_EMPTY(ZS_USE_ENGINE_GLOBAL_REF_COUNT)
#error "ZS_USE_ENGINE_GLOBAL_REF_COUNT requires a value"
#endif // ZS_USE_ENGINE_GLOBAL_REF_COUNT.

/// For now, adds some TODO warnings.
#ifndef ZS_DEVELOPER_MODE
#define ZS_DEVELOPER_MODE 0
#elif ZBASE_IS_MACRO_EMPTY(ZS_DEVELOPER_MODE)
#error "ZS_DEVELOPER_MODE requires a value"
#endif // ZS_DEVELOPER_MODE.

/// Memory profiling.
#ifndef ZS_MEMORY_PROFILER
#define ZS_MEMORY_PROFILER 0
#elif ZBASE_IS_MACRO_EMPTY(ZS_MEMORY_PROFILER)
#error "ZS_MEMORY_PROFILER requires a value"
#endif // ZS_MEMORY_PROFILER.

/// Exceptions.
#ifndef ZS_CONFIG_USE_EXCEPTION
#define ZS_CONFIG_USE_EXCEPTION 1
#elif ZBASE_IS_MACRO_EMPTY(ZS_CONFIG_USE_EXCEPTION)
#error "ZS_CONFIG_USE_EXCEPTION requires a value"
#endif // ZS_CONFIG_USE_EXCEPTION.

#if ZS_MEMORY_PROFILER
#define ZS_IF_MEMORY_PROFILER(...) __VA_ARGS__
#define ZS_IF_MEMORY_PROFILER_OR(A, B) A
#else
#define ZS_IF_MEMORY_PROFILER(...)
#define ZS_IF_MEMORY_PROFILER_OR(A, B) B
#endif // ZS_MEMORY_PROFILER.

/// Default allocate callback.
#ifndef ZS_DEFAULT_ALLOCATE
#define ZS_DEFAULT_ALLOCATE zs::default_allocate
#elif ZBASE_IS_MACRO_EMPTY(ZS_DEFAULT_ALLOCATE)
#error "ZS_DEFAULT_ALLOCATE requires a value"
#endif // ZS_DEFAULT_ALLOCATE.

/// Default stream getter callback.
#ifndef ZS_DEFAULT_STREAM_GETTER
#define ZS_DEFAULT_STREAM_GETTER zs::default_stream_getter
#elif ZBASE_IS_MACRO_EMPTY(ZS_DEFAULT_STREAM_GETTER)
#error "ZS_DEFAULT_STREAM_GETTER requires a value"
#endif // ZS_DEFAULT_STREAM_GETTER.

/// Default engine initializer callback.
#ifndef ZS_DEFAULT_ENGINE_INITIALIZER
#define ZS_DEFAULT_ENGINE_INITIALIZER zs::default_engine_initializer
#elif ZBASE_IS_MACRO_EMPTY(ZS_DEFAULT_ENGINE_INITIALIZER)
#error "ZS_DEFAULT_ENGINE_INITIALIZER requires a value"
#endif // ZS_DEFAULT_ENGINE_INITIALIZER.

/// @def ZS_DEFAULT_STACK_SIZE
#ifndef ZS_DEFAULT_STACK_SIZE
#define ZS_DEFAULT_STACK_SIZE 1024
#elif ZBASE_IS_MACRO_EMPTY(ZS_DEFAULT_STACK_SIZE)
#error "ZS_DEFAULT_STACK_SIZE requires a value"
#endif // ZS_DEFAULT_STACK_SIZE.

/// @def ZS_DEFAULT_FILE_LOADER
#ifndef ZS_DEFAULT_FILE_LOADER
#define ZS_DEFAULT_FILE_LOADER zs::default_file_loader
#elif ZBASE_IS_MACRO_EMPTY(ZS_DEFAULT_FILE_LOADER)
#error "ZS_DEFAULT_FILE_LOADER requires a value"
#endif // ZS_DEFAULT_FILE_LOADER.

//
//
//

#ifndef ZS_API
#define ZS_API extern
#endif

/// @def ZS_ASSERT.
#ifndef ZS_ASSERT
#define ZS_ASSERT(...) zbase_assert(__VA_ARGS__)
#endif // ZS_ASSERT.

/// @def ZS_WARNING.
#ifndef ZS_WARNING
#define ZS_WARNING(...) zb_warning(__VA_ARGS__)
#endif // ZS_WARNING.

/// @def ZS_ERROR.
#ifndef ZS_ERROR
#define ZS_ERROR(...) zbase_error(__VA_ARGS__)
#endif // ZS_ERROR.

// #define ZS_TRACE(...) zb::print("[trace]:", __VA_ARGS__)

/// @def ZS_TRACE.
#ifndef ZS_TRACE
#define ZS_TRACE(...) \
  do {                \
  } while (false)
#endif // ZS_TRACE.

/// @def ZS_TODO
#if ZS_DEVELOPER_MODE
#define ZS_TODO(msg) ZBASE_TODO(msg)
#else
#define ZS_TODO(msg)
#endif // ZS_DEVELOPER_MODE.

#define ZS_INLINE ZB_INLINE
#define ZS_INLINE_CXPR ZB_INLINE_CXPR
#define ZS_CHECK ZB_CHECK
#define ZS_CK_INLINE ZB_CK_INLINE
#define ZS_CXPR constexpr
#define ZS_CK_INLINE_CXPR ZB_CK_INLINE_CXPR

#define ZS_CLASS_COMMON friend struct zs::internal

#define ZS_OBJECT_CLASS_COMMON \
  ZS_CLASS_COMMON;             \
  friend class zs::object;     \
  friend class zs::virtual_machine

#if ZS_GARBAGE_COLLECTOR
#define ZS_IF_GARBAGE_COLLECTOR(...) __VA_ARGS__
#define ZS_IF_NO_GARBAGE_COLLECTOR(...)

#else
#define ZS_IF_GARBAGE_COLLECTOR(...)
#define ZS_IF_NO_GARBAGE_COLLECTOR(...) __VA_ARGS__

#endif // ZS_GARBAGE_COLLECTOR.

#if ZS_USE_ENGINE_GLOBAL_REF_COUNT
#define ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(...) __VA_ARGS__
#define ZS_IF_NO_USE_ENGINE_GLOBAL_REF_COUNT(...)
#else
#define ZS_IF_USE_ENGINE_GLOBAL_REF_COUNT(...)
#define ZS_IF_NO_USE_ENGINE_GLOBAL_REF_COUNT(...) __VA_ARGS__
#endif // ZS_USE_ENGINE_GLOBAL_REF_COUNT.

#define ZS_RETURN_IF_ERROR_1(X)     \
  if (zs::error_result err = (X)) { \
    return err;                     \
  }

#define ZS_RETURN_IF_ERROR_2(X, R)  \
  if (zs::error_result err = (X)) { \
    return R;                       \
  }

#define ZS_RETURN_IF_ERROR_3(X, F, R) \
  if (zs::error_result err = (X)) {   \
    F;                                \
    return R;                         \
  }

#define ZS_RETURN_IF_ERROR_(N, ...) ZBASE_CONCAT(ZS_RETURN_IF_ERROR_, N)(__VA_ARGS__)

///
#define ZS_RETURN_IF_ERROR(...) ZS_RETURN_IF_ERROR_(ZBASE_NARG(__VA_ARGS__), __VA_ARGS__)

/// Object types.
/// @warning: Look at these functions before changing the order:
///           * `is_object_type_ref_counted`
///           * `is_object_type_delegable`
///           * `get_object_type_mask`
#define ZS_OBJECT_TYPE_PREFIX(name) k_##name
#define ZS_OBJECT_TYPE_ENUM_VALUE(X, name, exposed_name) X(name, #name, exposed_name)

#define ZS_OBJECT_TYPE_ENUM(X)                                    \
  /* k_small_string must be zero for small string optimization */ \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, small_string, string)              \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, null, null)                        \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, none, none)                        \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, bool, bool)                        \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, integer, integer)                  \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, float, float)                      \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, string_view, string)               \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, native_function, closure)          \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, atom, atom)                        \
  /* Reference counted (everything >= k_long_string)           */ \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, long_string, string)               \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, closure, closure)                  \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, native_closure, closure)           \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, weak_ref, weak)                    \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, struct, struct)                    \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, struct_instance, struct_instance)  \
  /* Delegable (everything >= k_table)                         */ \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, table, table)                      \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, array, array)                      \
  ZS_OBJECT_TYPE_ENUM_VALUE(X, user_data, user_data)

/// Exposed object types.
#define ZS_EXPOSED_TYPE_PREFIX(name) ke_##name
#define ZS_EXPOSED_TYPE_ENUM_VALUE(X, name) X(ZS_EXPOSED_TYPE_PREFIX(name), #name)

#define ZS_EXPOSED_TYPE_ENUM(X)                  \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, null)            \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, none)            \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, bool)            \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, integer)         \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, float)           \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, string)          \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, atom)            \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, closure)         \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, table)           \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, array)           \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, user_data)       \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, struct)          \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, struct_instance) \
  ZS_EXPOSED_TYPE_ENUM_VALUE(X, weak)

/// Atom types.
#define ZS_ATOM_NAME_PREFIX(name) atom_##name
#define ZS_ATOM_TYPE_ENUM_VALUE(X, name) X(ZS_ATOM_NAME_PREFIX(name), #name)

#define ZS_ATOM_TYPE_ENUM(X)                          \
  ZS_ATOM_TYPE_ENUM_VALUE(X, user)                    \
  ZS_ATOM_TYPE_ENUM_VALUE(X, pointer)                 \
  ZS_ATOM_TYPE_ENUM_VALUE(X, array_iterator)          \
  ZS_ATOM_TYPE_ENUM_VALUE(X, table_iterator)          \
  ZS_ATOM_TYPE_ENUM_VALUE(X, mutable_string_iterator) \
  ZS_ATOM_TYPE_ENUM_VALUE(X, path_iterator)

/// Meta methods.
#define ZS_META_METHOD_ENUM(X)       \
  X(mt_none, "__none")               \
  /* Basic arithmetics */            \
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
  /* Basic arithmetics equal */      \
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
  /* Right hand size arithmetics */  \
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
  X(mt_unary_bw_not, "__bw_not")     \
  X(mt_incr, "__incr")               \
  X(mt_decr, "__decr")               \
  X(mt_pre_incr, "__pre_incr")       \
  X(mt_pre_decr, "__pre_decr")       \
  /* Info and Accessor */            \
  X(mt_typeof, "__typeof")           \
  X(mt_tostring, "__tostring")       \
  X(mt_copy, "__copy")               \
  X(mt_set, "__set")                 \
  X(mt_get, "__get")                 \
  X(mt_has, "__has")                 \
  X(mt_call, "__call")

namespace zs {

//
// MARK: Forward declarations
//

enum class object_type : uint8_t;
enum class object_type_mask : uint32_t;
enum class exposed_object_type : uint8_t;
enum class atom_type : uint8_t;
enum class meta_method : uint32_t;
enum class object_flags_t : uint8_t;
enum class delegate_flags_t : uint8_t;
enum class variable_attribute_t : uint8_t;

struct internal;
struct object_base;
class object;
class engine;
class virtual_machine;
class vm;
class vm_ref;
struct parameter_stream;
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
struct variable_type_info;

using parameter_list = zb::span<const zs::object>;

template <class T>
class allocator;

// clang-format off
enum class error_code : int32_t {
  #define ZS_DECL_ERROR_CODE(name, str) name,
  #include <zscript/detail/error_codes_def.h>
  #undef ZS_DECL_ERROR_CODE
};

using errc = error_code;

ZS_CK_INLINE_CXPR const char* error_code_to_string(error_code ec) noexcept {
  switch (ec) {
  #define ZS_DECL_ERROR_CODE(name, str) case error_code::name: return str;
  #include <zscript/detail/error_codes_def.h>
  #undef ZS_DECL_ERROR_CODE
  }

  return "unknown";
}
// clang-format on

ZBASE_ATTRIBUTE_NO_RETURN void throw_error(zs::error_code ec);

namespace detail {
  struct error_result_descriptor {
    using enum_type = error_code;
    static constexpr enum_type default_value = enum_type::success;

    ZS_CK_INLINE_CXPR static bool is_valid(enum_type v) noexcept {
      return v == enum_type::success || v == enum_type::returned;
    }

    ZS_CK_INLINE_CXPR static const char* to_string(enum_type code) noexcept {
      return zs::error_code_to_string(code);
    }
  };
} // namespace detail.

using error_result = zb::generic_error_result<detail::error_result_descriptor>;
using status_result = zb::generic_status_result<detail::error_result_descriptor>;

template <class T>
using optional_result = zb::generic_optional_result<detail::error_result_descriptor, T>;

//
// MARK: - Exceptions
//

#if ZS_CONFIG_USE_EXCEPTION
struct exception : std::exception {
  inline exception(error_code error) noexcept
      : _error{ error } {}

  inline exception(const exception&) noexcept = default;

  virtual ~exception() override = default;

  virtual const char* what() const noexcept override { return zs::error_result(_error).message(); }

  ZS_CK_INLINE_CXPR error_code error() const noexcept { return _error; }

private:
  error_code _error;
};
#endif // ZS_CONFIG_USE_EXCEPTION.

using var = object;
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
struct none {};
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

///
using stream_getter_t = std::ostream& (*)(zs::engine*, zs::raw_pointer_t);

///
using write_function_t = zs::error_result (*)(const uint8_t*, size_t size, void* data);

///
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

/// Object types.
///
/// An enum of all possible raw types an object can be.
enum class object_type : uint8_t {
#define _X(name, str, exposed_name) ZS_OBJECT_TYPE_PREFIX(name),
  ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
};

/// Exposed object types.
///
/// We use the exposed object types to map all object types to a reduced set of
/// types (the exposed object types).
///
/// In other words, we don't want the user to know about any implementation
/// specifics for strings and functions.
///
/// For now, we map these to 'string':
///   * k_small_string
///   * k_string_view
///   * k_long_string
///
/// And these to 'closure':
///   * k_native_function
///   * k_closure
///   * k_native_closure
enum class exposed_object_type : uint8_t {
#define _X(name, str) name,
  ZS_EXPOSED_TYPE_ENUM(_X)
#undef _X
};

/// Atom types.
enum class atom_type : uint8_t {
#define _X(name, str) name,
  ZS_ATOM_TYPE_ENUM(_X)
#undef _X
};

/// Meta methods.
enum class meta_method : uint32_t {
#define _X(name, str) name,
  ZS_META_METHOD_ENUM(_X)
#undef _X
};

static_assert(uint8_t(zs::object_type::k_small_string) == 0, "object_type::k_small_string must be zero.");
static_assert(zb::enum_count<zs::object_type>() < 32, "object_type maximum value should remain under 32.");

///
enum class object_flags_t : uint8_t { //
  f_none,
  f_char = 1
};

///
enum class delegate_flags_t : uint8_t { //
  df_none,
  df_use_default = 1,
  df_locked = 2
};

//
// MARK: Allocation interface.
//

/// Allocate memory.
void* allocate(zs::engine* eng, size_t sz, alloc_info_t ainfo = alloc_info_t{});

/// Reallocate memory.
void* reallocate(
    zs::engine* eng, void* ptr, size_t size, size_t old_size, alloc_info_t ainfo = alloc_info_t{});

/// Deallocate memory.
void deallocate(zs::engine* eng, void* ptr, alloc_info_t ainfo = alloc_info_t{});

enum class memory_tag {
  nt_unknown,
  nt_engine,
  nt_vm,
  nt_array,
  nt_table,
  nt_struct,
  nt_class,
  nt_string,
  nt_user_data,
  nt_native_closure,
  nt_weak_ptr,
  nt_capture,
  nt_allocator
};

struct internal {
  template <class T, class... Args>
  ZS_INLINE static T* zs_new(zs::engine* eng, Args&&... args) {
    T* ptr = (T*)zs::allocate(eng, sizeof(T));
    ptr = zb_placement_new(ptr) T(std::forward<Args>(args)...);
    return ptr;
  }

  template <memory_tag Tag, class T, class... Args>
  ZS_INLINE static T* zs_new(zs::engine* eng, Args&&... args) {
    T* ptr = (T*)zs::allocate(eng, sizeof(T), (alloc_info_t)Tag);
    ptr = zb_placement_new(ptr) T(std::forward<Args>(args)...);
    return ptr;
  }

  template <class T>
  ZS_INLINE static void zs_delete(zs::engine* eng, T* ptr) {
    ptr->~T();
    zs::deallocate(eng, ptr);
  }

  template <class T>
  ZS_INLINE static void zs_delete(zs::engine* eng, T* ptr, memory_tag tag) {
    ptr->~T();
    zs::deallocate(eng, ptr, (alloc_info_t)tag);
  }

  template <memory_tag Tag, class T>
  ZS_INLINE static void zs_delete(zs::engine* eng, T* ptr) {
    ptr->~T();
    zs::deallocate(eng, ptr, (alloc_info_t)Tag);
  }

  template <class T>
  struct proxy {};

  template <class T>
  struct test_helper {};
};

template <class T, class... Args>
ZS_INLINE static T* zs_new(zs::engine* eng, Args&&... args) {
  return internal::zs_new<T, Args...>(eng, std::forward<Args>(args)...);
}

template <memory_tag Tag, class T, class... Args>
ZS_INLINE T* zs_new(zs::engine* eng, Args&&... args) {
  return internal::zs_new<Tag, T, Args...>(eng, std::forward<Args>(args)...);
}

template <class T>
ZS_INLINE void zs_delete(zs::engine* eng, T* ptr) {
  internal::zs_delete<T>(eng, ptr);
}

template <memory_tag Tag, class T>
ZS_INLINE void zs_delete(zs::engine* eng, T* ptr) {
  internal::zs_delete<Tag, T>(eng, ptr);
}

class engine_holder {
public:
  ZS_INLINE_CXPR engine_holder(zs::engine* eng) noexcept
      : _engine(eng) {}

  ZS_CK_INLINE_CXPR zs::engine* get_engine() const noexcept { return _engine; }

protected:
  zs::engine* _engine;
};

class reference_counted : public engine_holder {
public:
  ZS_CLASS_COMMON;

  reference_counted(zs::engine* eng) noexcept;

  reference_counted(reference_counted&&) = delete;
  reference_counted(const reference_counted&) = delete;
  reference_counted& operator=(reference_counted&&) = delete;
  reference_counted& operator=(const reference_counted&) = delete;

  void retain() noexcept;
  virtual bool release() noexcept;

  ZS_CK_INLINE size_t ref_count() const noexcept { return _ref_count; }

protected:
  virtual ~reference_counted() = 0;

  size_t _ref_count = 1;
};

namespace detail {
  template <class T>
  struct unique_ptr_deleter : public engine_holder {
    static_assert(!std::is_function_v<T>, "unique_ptr_deleter cannot be instantiated for function types");

    ZS_INLINE unique_ptr_deleter(zs::engine* eng)
        : engine_holder(eng) {}

    template <class U>
    ZS_INLINE_CXPR unique_ptr_deleter(
        const unique_ptr_deleter<U>&, std::enable_if_t<std::is_convertible_v<U*, T*>>* = 0) noexcept {}

    ZS_INLINE_CXPR void operator()(T* ptr) const noexcept;
  };
} // namespace detail.

//
// MARK: Allocator
//

template <class T>
class allocator : public engine_holder {
public:
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::false_type;

  ZS_INLINE_CXPR allocator(zs::engine* eng) noexcept
      : engine_holder(eng) ZS_IF_MEMORY_PROFILER(, _tag(memory_tag::nt_allocator)) {}

  ZS_INLINE_CXPR allocator(zs::engine* eng, memory_tag tag) noexcept
      : engine_holder(eng) ZS_IF_MEMORY_PROFILER(, _tag(tag)) {}

  inline constexpr allocator(const allocator&) noexcept = default;
  inline constexpr allocator(allocator&&) noexcept = default;

  template <class U>
  ZS_INLINE_CXPR allocator(const allocator<U>& a) noexcept
      : engine_holder(a.get_engine()) ZS_IF_MEMORY_PROFILER(, _tag(a._tag)) {}

  inline constexpr allocator& operator=(const allocator&) noexcept = default;
  inline constexpr allocator& operator=(allocator&&) noexcept = default;

  ZS_CK_INLINE_CXPR T* allocate(size_t n);

  ZS_INLINE_CXPR void deallocate(T* ptr, size_t) noexcept;

  ZS_INLINE_CXPR bool operator==(const allocator& a) const noexcept { return _engine == a.get_engine(); }

  template <class U>
  ZS_CK_INLINE_CXPR bool operator==(const allocator<U>& a) const noexcept {
    return _engine == a.get_engine();
  }

  template <class U>
  struct rebind {
    using other = allocator<U>;
  };

private:
  template <class U>
  friend class allocator;

  ZS_IF_MEMORY_PROFILER(memory_tag _tag);
};

/// unique_ptr.
template <class T>
using unique_ptr = std::unique_ptr<T, zs::detail::unique_ptr_deleter<T>>;

using string_allocator = zs::allocator<char>;

/// string.
using string = std::basic_string<char, std::char_traits<char>, zs::string_allocator>;

/// vector.
template <class T>
using vector = zb::vector<T, zs::allocator<T>>;

/// small_vector.
template <class _T, size_t _Size>
using small_vector = zb::small_vector<_T, _Size, zs::allocator<_T>>;

/// unordered_map_allocator.
template <class Key, class Value>
using unordered_map_allocator = zs::allocator<std::pair<const Key, Value>>;

/// unordered_map.
template <class Key, class T, class Hash = zb::rapid_hasher<Key>, class Pred = std::equal_to<Key>>
using unordered_map = std::unordered_map<Key, T, Hash, Pred, zs::unordered_map_allocator<Key, T>>;

/// unordered_set.
template <class T, class Hash = zb::rapid_hasher<T>, class Pred = std::equal_to<T>>
using unordered_set = std::unordered_set<T, Hash, Pred, zs::allocator<T>>;

using ostringstream = std::basic_ostringstream<char, std::char_traits<char>, zs::string_allocator>;

namespace detail {
  template <class T>
  inline T create_string_stream(zs::engine* eng) {
    if constexpr (std::is_constructible_v<T, std::ios_base::openmode, zs::string_allocator>) {
      return T(std::ios_base::out, zs::string_allocator(eng));
    }
    else {
      return T(zs::string("", zs::string_allocator(eng)), std::ios_base::out);
    }
  }
} // namespace detail.

inline zs::ostringstream create_string_stream(zs::engine* eng) {
  return detail::create_string_stream<zs::ostringstream>(eng);
}

using unordered_object_map_allocator = zs::allocator<std::pair<const object, object>>;

struct object_table_hash {
  using is_transparent = void;

  ZS_CHECK size_t operator()(const object_base& obj) const noexcept;
  ZS_CHECK size_t operator()(std::string_view s) const noexcept;
  ZS_CK_INLINE size_t operator()(const std::string& s) const noexcept {
    return operator()(std::string_view(s));
  }
  ZS_CK_INLINE size_t operator()(const char* s) const noexcept { return operator()(std::string_view(s)); }
};

struct object_table_equal_to {
  using is_transparent = void;
  ZS_CK_INLINE bool operator()(const object_base& lhs, const object_base& rhs) const noexcept;
  ZS_CK_INLINE bool operator()(const object_base& lhs, std::string_view rhs) const noexcept;
  ZS_CK_INLINE bool operator()(std::string_view lhs, const object_base& rhs) const noexcept;
};

template <class T>
using object_unordered_map = zs::unordered_map<object, T, object_table_hash, object_table_equal_to>;

using object_map = zs::object_unordered_map<object>;

using object_unordered_set = zs::unordered_set<object, object_table_hash, object_table_equal_to>;

class reference_counted_object : public engine_holder {
public:
  ZS_OBJECT_CLASS_COMMON;
  friend class weak_ref_object;

  reference_counted_object(zs::engine* eng, object_type obj_type) noexcept;

  reference_counted_object(reference_counted_object&&) = delete;
  reference_counted_object(const reference_counted_object&) = delete;

  reference_counted_object& operator=(reference_counted_object&&) = delete;
  reference_counted_object& operator=(const reference_counted_object&) = delete;

  void retain() noexcept;

  ZS_CK_INLINE size_t ref_count() const noexcept { return _ref_count; }

  ZS_CK_INLINE object_type get_object_type() const noexcept { return _obj_type; }

  ZS_CHECK object clone() const noexcept;

protected:
  ZS_INLINE ~reference_counted_object() noexcept {
    ZS_ASSERT(_ref_count == 0, "~reference_counted: ref_count should be zero");
  }

  bool release() noexcept;

private:
  ZBASE_PRAGMA_PUSH()
  ZBASE_CLANG_DIAGNOSTIC(ignored, "-Wgnu-anonymous-struct")
  struct {
    size_t _ref_count : 56;
    object_type _obj_type : 8;
  };
  ZBASE_PRAGMA_POP()

  weak_ref_object* _weak_ref_object = nullptr;

  object get_weak_ref(const object_base& obj) noexcept;
};

template <zb::separator Separator = " ", class... Args>
ZS_INLINE std::ostream& print(const Args&... args) {
  return zb::print<Separator>(args...);
}

template <zb::separator Separator = "", class... Args>
ZS_INLINE zs::string strprint(zs::engine* eng, const Args&... args) {
  zs::ostringstream stream(zs::create_string_stream(eng));
  zb::stream_print<Separator>(stream, args...);
  return zs::string(stream.str(), zs::string_allocator(eng));
}

template <class... Args>
ZS_INLINE zs::string sstrprint(zs::engine* eng, const Args&... args) {
  zs::ostringstream stream(zs::create_string_stream(eng));
  zb::stream_print<" ">(stream, args...);
  return zs::string(stream.str(), zs::string_allocator(eng));
}
/// Get the object_type name.
ZB_CK_INLINE_CXPR const char* get_object_type_name(object_type t) noexcept {
  switch (t) {
#define _X(name, str, exposed_name)              \
  case object_type::ZS_OBJECT_TYPE_PREFIX(name): \
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
#define _X(name, str, exposed_name)              \
  case object_type::ZS_OBJECT_TYPE_PREFIX(name): \
    return #exposed_name;

    ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
  }

  ZS_ERROR("invalid type");
  return "unknown";
}

ZS_CK_INLINE_CXPR bool is_object_type_ref_counted(object_type t) noexcept {
  return t >= object_type::k_long_string;
}

ZS_CK_INLINE_CXPR bool is_object_type_delegable(object_type t) noexcept { return t >= object_type::k_table; }

enum class object_type_mask : uint32_t {
#define _X(name, str, exposed_name) \
  ZBASE_CONCAT(otm_, name) = zs::get_object_type_mask(object_type::ZS_OBJECT_TYPE_PREFIX(name)),
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
  case mt_add:    return mt_rhs_add;
  case mt_sub:    return mt_rhs_sub;
  case mt_mul:    return mt_rhs_mul;
  case mt_div:    return mt_rhs_div;
  case mt_mod:    return mt_rhs_mod;
  case mt_exp:    return mt_rhs_exp;
  case mt_lshift: return mt_rhs_lshift;
  case mt_rshift: return mt_rhs_rshift;
  case mt_bw_or:  return mt_rhs_bw_or;
  case mt_bw_and: return mt_rhs_bw_and;
  case mt_bw_xor: return mt_rhs_bw_xor;
  default:        return mt_none;
  }
  // clang-format on

  return mt_none;
}

struct object_type_mask_printer {
  uint32_t mask;
  const char* l = "[";
  const char* r = "]";

  inline friend std::ostream& operator<<(std::ostream& stream, object_type_mask_printer v) {

#define _STREAM_OBJECT_TYPE_MASK(name, str, exposed_name)                            \
  if (v.mask & zs::get_object_type_mask(object_type::ZS_OBJECT_TYPE_PREFIX(name))) { \
    if (found_one) {                                                                 \
      stream << ", ";                                                                \
    }                                                                                \
    stream << str;                                                                   \
    found_one = true;                                                                \
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
