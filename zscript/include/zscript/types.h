#pragma once

#include <zscript/common.h>
#include <zscript/object_type.h>
#include <zbase/utility/integer_enum.h>
#include <zbase/container/span.h>

namespace zs {
struct object_base;
class object;
using var = object;

struct internal;

class engine;
class virtual_machine;
class vm;
class vm_ref;

class delegate_object;
class weak_ref_object;
class array_object;
class struct_object;
class struct_instance_object;
class table_object;
class class_object;
class class_instance_object;
class string_object;
class mutable_string_object;
class native_closure_object;
class function_prototype_object;
class closure_object;
class node_object;
class user_data_object;
class reference_counted_object;
class native_array_object_interface;
class closure_object_t;
struct struct_item;

template <class T>
class native_array_object;

template <class T>
class allocator;

//
// MARK: - Types
//

///
using null_t = std::nullptr_t;

///
using bool_t = bool;

///
using int_t = int64_t;

///
using uint_t = uint64_t;

///
using float_t = double;

///
using raw_pointer_t = void*;

///
enum class object_flags_t : uint8_t {
  none, //
  type_info = 1, //
  enum_table = 2,
  meta_argument = 4,
  array_type = 8
};

ZBASE_ENUM_CLASS_FLAGS(object_flags_t);

namespace constants {
  inline constexpr size_t k_pointer_size = sizeof(void*);
  inline constexpr size_t k_object_size = 16;
  inline constexpr size_t k_small_string_max_size
      = k_object_size - (sizeof(object_flags_t) + sizeof(object_type));
} // namespace constants.

///
using native_cclosure_t = int_t (*)(zs::virtual_machine*);
using native_cpp_closure_t = int_t (*)(zs::vm_ref);

class parameter_list;

using closure_t = zs::object (*)(zs::vm_ref, parameter_list);

//
//
using raw_pointer_release_hook_t = void (*)(zs::engine*, zs::raw_pointer_t);
using native_closure_release_hook_t = void (*)(zs::engine*, zs::raw_pointer_t);
using user_data_release_hook_t = void (*)(zs::engine*, zs::raw_pointer_t);
using release_hook_t = int_t (*)(raw_pointer_t, int_t);

using stream_getter_t = std::ostream& (*)(zs::engine*, zs::raw_pointer_t);

//
using write_function_t = zs::error_result (*)(const uint8_t*, size_t size, void* data);

//
using copy_user_data_to_type_t = zs::error_result (*)(void*, size_t, std::string_view, void* data);

//
using to_string_callback_t = zs::error_result (*)(const zs::object_base&, std::ostream&);

///
using alloc_info_t = uint32_t;

using engine_initializer_t = zs::error_result (*)(zs::engine*);

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

/// The global default allocate function.
extern const allocate_t default_allocate;

extern const stream_getter_t default_stream_getter;
extern const engine_initializer_t default_engine_initializer;

struct config_t {
  size_t stack_size = ZS_DEFAULT_STACK_SIZE;
  allocate_t alloc_callback = ZS_DEFAULT_ALLOCATE;
  raw_pointer_t user_pointer = nullptr;
  raw_pointer_release_hook_t user_release = nullptr;
  stream_getter_t stream_getter = ZS_DEFAULT_STREAM_GETTER;
  engine_initializer_t initializer = ZS_DEFAULT_ENGINE_INITIALIZER;
};

} // namespace zs.
