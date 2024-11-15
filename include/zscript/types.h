#pragma once

#include <zscript/common.h>
#include <zscript/object_type.h>
#include <zbase/container/span.h>

namespace zs {

//
// MARK: Forward declarations
//

struct internal;
class engine;
class virtual_machine;
class vm;
class vm_ref;
class parameter_list;

template <class T>
class allocator;

struct object_base;
class object;
using var = object;

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

template <class T>
class native_array_object;

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
// MARK: - Types
//

using null_t = std::nullptr_t;
using bool_t = bool;
using int_t = int64_t;
using uint_t = uint64_t;
using float_t = double;
using raw_pointer_t = void*;

///
using native_cclosure_t = int_t (*)(zs::virtual_machine*);
using native_cpp_closure_t = int_t (*)(zs::vm_ref);

using closure_t = zs::object (*)(zs::vm_ref, parameter_list);

using alloc_info_t = uint32_t;

//
//
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

namespace constants {
  inline constexpr size_t k_pointer_size = sizeof(void*);
  inline constexpr size_t k_object_size = 16;
  inline constexpr size_t k_small_string_max_size
      = k_object_size - (sizeof(object_flags_t) + sizeof(object_type));

  inline constexpr alloc_info_t k_engine_deallocation = 80198;
  inline constexpr alloc_info_t k_user_pointer_deallocation = 80199;

} // namespace constants.

} // namespace zs.
