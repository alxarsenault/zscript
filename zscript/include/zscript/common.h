#pragma once

#include <zbase/zbase.h>
#include <zbase/sys/assert.h>

#define ZS_DEBUG 1
#define ZS_MEMORY_PROFILER 1
#define ZS_GARBAGE_COLLECTOR 1
#define ZS_USE_ENGINE_GLOBAL_REF_COUNT 1

#define ZS_VERSION_MAJOR 0
#define ZS_VERSION_MINOR 0
#define ZS_VERSION_PATCH 1
#define ZS_VERSION_BUILD 1

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
  friend class zs::object

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
