//
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
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#define __ZBASE_DEBUG_BUILD__ 1

//
#define ZBASE_NAMESPACE zb

#ifndef ZBASE_VERSION_MAJOR
#define ZBASE_VERSION_MAJOR 0
#define ZBASE_VERSION_MINOR 1
#define ZBASE_VERSION_PATCH 0
#define ZBASE_VERSION_BUILD 0
#endif

///
#define __ZBASE_VERSION_ID__ \
  ZBASE_CONCAT(ZBASE_CONCAT(ZBASE_VERSION_MAJOR, ZBASE_VERSION_MINOR), ZBASE_VERSION_PATCH)
#define __ZBASE_ABI_NAMESPACE__ ZBASE_CONCAT(__, __ZBASE_VERSION_ID__)

#define __ZBASE_VERSION_STR__ ZBASE_STRINGIFY(ZBASE_VERSION_MAJOR.ZBASE_VERSION_MINOR.ZBASE_VERSION_PATCH)
#define __ZBASE_LONG_VERSION_STR__ \
  ZBASE_STRINGIFY(ZBASE_VERSION_MAJOR.ZBASE_VERSION_MINOR.ZBASE_VERSION_PATCH.ZBASE_VERSION_BUILD)

///
#define ZBASE_BEGIN_NAMESPACE \
  namespace ZBASE_NAMESPACE { \
  inline namespace __ZBASE_ABI_NAMESPACE__ {

///
#define ZBASE_END_NAMESPACE \
  }                         \
  }

#define __ZBASE_BEGIN_SUB_NAMESPACE_LAST_IMPL(name) name {
#define __ZBASE_BEGIN_SUB_NAMESPACE_IMPL(name) name::

#define ZBASE_BEGIN_SUB_NAMESPACE(...) \
  ZBASE_BEGIN_NAMESPACE                \
  namespace ZBASE_FOR_EACH_WITH_LAST(  \
      __ZBASE_BEGIN_SUB_NAMESPACE_IMPL, __ZBASE_BEGIN_SUB_NAMESPACE_LAST_IMPL, __VA_ARGS__)

#define ZBASE_END_SUB_NAMESPACE(...) \
  ZBASE_END_NAMESPACE                \
  }

///
#define __zb ZBASE_NAMESPACE::__ZBASE_ABI_NAMESPACE__

// #if !defined(ZBASE_VERSION) || (ZBASE_VERSION < 15)
#if defined(ZBASE_VERSION)
#undef ZBASE_VERSION
#endif
#define ZBASE_VERSION 1

#ifdef __has_include
#define ZBASE_HAS_INCLUDE(x) __has_include(x)
#else
#define ZBASE_HAS_INCLUDE(x) 0
#endif

#ifdef __has_feature
#define ZBASE_HAS_FEATURE(x) __has_feature(x)
#else
#define ZBASE_HAS_FEATURE(x) 0
#endif

#ifdef __has_extension
#define ZBASE_HAS_EXTENSION(x) __has_extension(x)
#else
#define ZBASE_HAS_EXTENSION(x) 0
#endif

#ifdef __has_attribute
#define ZBASE_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define ZBASE_HAS_ATTRIBUTE(x) 0
#endif

#ifdef __has_builtin
#define ZBASE_HAS_BUILTIN(x) __has_builtin(x)
#else
#define ZBASE_HAS_BUILTIN(x) 0
#endif

///
#define ZBASE_STRINGIFY(s) __ZBASE_STRINGIFY(s)
#define __ZBASE_STRINGIFY(s) #s

#define ZBASE_CONCAT(_X, _Y) __ZBASE_CONCAT(_X, _Y)
#define __ZBASE_CONCAT(_X, _Y) _X##_Y

/////////////////////////////////////////////
#define ZBASE_EXPAND(X) X
#define ZBASE_DEFER(M, ...) M(__VA_ARGS__)

#define ZBASE_IS_MACRO_EMPTY(...) ZBASE_EXPAND(__ZBASE_IS_MACRO_EMPTY(__ZBASE_ARGS_DUMMY(__VA_ARGS__)))
#define __ZBASE_ARGS_DUMMY(...) dummy, ##__VA_ARGS__
#define __ZBASE_SELECT_5(_1, _2, _3, _4, _5, num, ...) num
#define __ZBASE_IS_MACRO_EMPTY(...) ZBASE_EXPAND(__ZBASE_SELECT_5(__VA_ARGS__, 0, 0, 0, 0, 1))

//
// MARK: Platform
//

#undef __ZBASE_ANDROID__
#undef __ZBASE_BSD__
#undef __ZBASE_IOS__
#undef __ZBASE_LINUX__
#undef __ZBASE_MACOS__
#undef __ZBASE_APPLE__
#undef __ZBASE_SOLARIS__
#undef __ZBASE_WINDOWS__

// Android.
#if defined(__ANDROID__)
#define __ZBASE_ANDROID__ 1

// Linux.
#elif defined(__linux__) || defined(__linux) || defined(linux)
#define __ZBASE_LINUX__ 1

// Apple macos or ios.
#elif defined(__APPLE__)
#define __ZBASE_APPLE__ 1

// Apple.
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
// IOS.
#define __ZBASE_IOS__ 1

#elif TARGET_OS_MAC
// Mac OS.
#define __ZBASE_MACOS__ 1

#else
#warning "zbase unknown apple platform."
#endif

// BSD.
#elif defined(BSD) || defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) \
    || defined(__OpenBSD__)
#define __ZBASE_BSD__ 1

// Solaris.
#elif defined(__sun) && defined(__SVR4)
#define __ZBASE_SOLARIS__ 1

// Windows.
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
#define __ZBASE_WINDOWS__ 1

// Unknown platform.
#else
#warning "zbase unknown platform."
#endif

///
///
///
#ifndef __ZBASE_ANDROID__
#define __ZBASE_ANDROID__ 0
#endif

#ifndef __ZBASE_BSD__
#define __ZBASE_BSD__ 0
#endif

#ifndef __ZBASE_IOS__
#define __ZBASE_IOS__ 0
#endif

#ifndef __ZBASE_LINUX__
#define __ZBASE_LINUX__ 0
#endif

#ifndef __ZBASE_MACOS__
#define __ZBASE_MACOS__ 0
#endif

#ifndef __ZBASE_SOLARIS__
#define __ZBASE_SOLARIS__ 0
#endif

#ifndef __ZBASE_WINDOWS__
#define __ZBASE_WINDOWS__ 0
#endif

#ifndef __ZBASE_APPLE__
#define __ZBASE_APPLE__ 0
#endif

//
// MARK: Compiler
//

#undef __ZBASE_CLANG__
#undef __ZBASE_GCC__
#undef __ZBASE_INTEL__
#undef __ZBASE_MINGW__
#undef __ZBASE_MSVC__
#undef __ZBASE_WASM__

// Microsoft visual studio.
#if defined(_MSC_VER) && !defined(__clang__)
#define __ZBASE_MSVC__ 1

// Clang.
#elif defined(__clang__)
#define __ZBASE_CLANG__ 1

// GCC.
#elif defined(__GNUC__) || defined(__GNUG__)
#define __ZBASE_GCC__ 1

// Intel.
#elif (defined(SYCL_LANGUAGE_VERSION) && defined(__INTEL_LLVM_COMPILER)) || defined(__INTEL_COMPILER)
#define __ZBASE_INTEL__ 1

// MinGW.
#elif defined(__MINGW32__) || defined(__MINGW64__)
#define __ZBASE_MINGW__ 1

// Web assembly.
#elif defined(__EMSCRIPTEN__)
#define __ZBASE_WASM__ 1

// Unknown compiler.
#else
#error "zbase unsupported compiler."
#endif

#ifndef __ZBASE_CLANG__
#define __ZBASE_CLANG__ 0
#endif
#ifndef __ZBASE_GCC__
#define __ZBASE_GCC__ 0
#endif
#ifndef __ZBASE_INTEL__
#define __ZBASE_INTEL__ 0
#endif
#ifndef __ZBASE_MINGW__
#define __ZBASE_MINGW__ 0
#endif
#ifndef __ZBASE_MSVC__
#define __ZBASE_MSVC__ 0
#endif
#ifndef __ZBASE_WASM__
#define __ZBASE_WASM__ 0
#endif

//
// MARK: __ZBASE_MSVC__
//

#if __ZBASE_MSVC__
#define ZBASE_PRAGMA_(x) __pragma(x)
#define ZBASE_PRAGMA(x) ZBASE_PRAGMA_(x)
#define ZBASE_PRAGMA_PUSH() __pragma(warning(push))
#define ZBASE_PRAGMA_POP() __pragma(warning(pop))
#define ZBASE_PRAGMA_DIAGNOSTIC(TYPE, VALUE) ZBASE_PRAGMA(warning(TYPE : VALUE))
#define ZBASE_DISABLE_ALL_WARNINGS_BEGIN __pragma(warning(push, 0))
#define ZBASE_DISABLE_ALL_WARNINGS_END __pragma(warning(pop))

#define ZBASE_MSVC_DIAGNOSTIC(TYPE, VALUE) ZBASE_PRAGMA_DIAGNOSTIC(TYPE, VALUE)
#define ZBASE_PRAGMA_DISABLE_WARNING_MSVC(VALUE) ZBASE_PRAGMA_DIAGNOSTIC(disable, VALUE)

#define ZBASE_MESSAGE(msg) ZBASE_PRAGMA(message(msg))
#define ZBASE_ERROR(msg)
//
// MARK: __ZBASE_CLANG__
//

#elif __ZBASE_CLANG__

#define ZBASE_PRAGMA_(x) _Pragma(#x)
#define ZBASE_PRAGMA(x) ZBASE_PRAGMA_(x)
#define ZBASE_PRAGMA_PUSH() ZBASE_PRAGMA(clang diagnostic push)
#define ZBASE_PRAGMA_POP() ZBASE_PRAGMA(clang diagnostic pop)
#define ZBASE_PRAGMA_DIAGNOSTIC(TYPE, VALUE) ZBASE_PRAGMA(clang diagnostic TYPE VALUE)
#define ZBASE_DISABLE_ALL_WARNINGS_BEGIN \
  ZBASE_PRAGMA_PUSH() ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Weverything")
#define ZBASE_DISABLE_ALL_WARNINGS_END ZBASE_PRAGMA_POP()

#define ZBASE_CLANG_DIAGNOSTIC(TYPE, VALUE) ZBASE_PRAGMA_DIAGNOSTIC(TYPE, VALUE)

#define ZBASE_PRAGMA_DISABLE_WARNING_CLANG(VALUE) ZBASE_PRAGMA_DIAGNOSTIC(ignored, VALUE)

#define ZBASE_MESSAGE(msg) ZBASE_PRAGMA(message(msg))

#define ZBASE_ERROR(msg) ZBASE_PRAGMA(GCC error(msg))

//
// MARK: __ZBASE_GCC__
//

#elif __ZBASE_GCC__

#define ZBASE_PRAGMA_(x) _Pragma(#x)
#define ZBASE_PRAGMA(x) ZBASE_PRAGMA_(x)
#define ZBASE_PRAGMA_PUSH() ZBASE_PRAGMA(GCC diagnostic push)
#define ZBASE_PRAGMA_POP() ZBASE_PRAGMA(GCC diagnostic pop)
#define ZBASE_PRAGMA_DIAGNOSTIC(TYPE, VALUE) ZBASE_PRAGMA(GCC diagnostic TYPE VALUE)
#define ZBASE_DISABLE_ALL_WARNINGS_BEGIN    \
  ZBASE_PRAGMA_PUSH()                       \
  ZBASE_PRAGMA_DISABLE_WARNING_GCC("-Wall") \
  ZBASE_PRAGMA_DISABLE_WARNING_GCC("-Wextra") ZBASE_PRAGMA_DISABLE_WARNING_GCC("-Wunused-value")
#define ZBASE_DISABLE_ALL_WARNINGS_END ZBASE_PRAGMA_POP()

#define ZBASE_GCC_DIAGNOSTIC(TYPE, VALUE) ZBASE_PRAGMA_DIAGNOSTIC(TYPE, VALUE)
#define ZBASE_PRAGMA_DISABLE_WARNING_GCC(VALUE) ZBASE_PRAGMA_DIAGNOSTIC(ignored, VALUE)

#define ZBASE_MESSAGE(msg) ZBASE_PRAGMA(GCC message(msg))
#define ZBASE_ERROR(msg) ZBASE_PRAGMA(GCC error(msg))

#else
#define ZBASE_PRAGMA_(x)
#define ZBASE_PRAGMA(x)
#define ZBASE_PRAGMA_PUSH()
#define ZBASE_PRAGMA_POP()
#define ZBASE_PRAGMA_DIAGNOSTIC(TYPE, VALUE)
#define ZBASE_DISABLE_ALL_WARNINGS_BEGIN
#define ZBASE_DISABLE_ALL_WARNINGS_END
#define ZBASE_MESSAGE(msg)
#define ZBASE_ERROR(msg)
#endif

#ifndef ZBASE_PRAGMA_DISABLE_WARNING_MSVC
#define ZBASE_PRAGMA_DISABLE_WARNING_MSVC(X)
#endif

#ifndef ZBASE_PRAGMA_DISABLE_WARNING_CLANG
#define ZBASE_PRAGMA_DISABLE_WARNING_CLANG(X)
#endif

#ifndef ZBASE_PRAGMA_DISABLE_WARNING_GCC
#define ZBASE_PRAGMA_DISABLE_WARNING_GCC(X)
#endif

#ifndef ZBASE_MSVC_DIAGNOSTIC
#define ZBASE_MSVC_DIAGNOSTIC(TYPE, VALUE)
#endif // ZBASE_MSVC_DIAGNOSTIC

#ifndef ZBASE_CLANG_DIAGNOSTIC
#define ZBASE_CLANG_DIAGNOSTIC(TYPE, VALUE)
#endif // ZBASE_CLANG_DIAGNOSTIC

#ifndef ZBASE_GCC_DIAGNOSTIC
#define ZBASE_GCC_DIAGNOSTIC(TYPE, VALUE)
#endif // ZBASE_GCC_DIAGNOSTIC

//
// ZBASE_DEPRECATED
//

#define ZBASE_DEPRECATED_0(...) [[deprecated]]
#define ZBASE_DEPRECATED_1(msg, ...) [[deprecated(msg)]]
#define ZBASE_DEPRECATED(...) \
  ZBASE_DEFER(ZBASE_CONCAT(ZBASE_DEPRECATED_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)

//
// ZBASE_TODO
//

#define ZBASE_TODO(msg) ZBASE_MESSAGE("[TODO] : " msg)

// #define ZBASE_TODO(desc) ZBASE_MESSAGE("[TODO] : " __FILE__ "(" ZBASE_STRINGIFY(__LINE__) ") : " desc)

//
// ZB_ALWAYS_INLINE
//

#if defined(__GNUC__) || defined(__clang__)
#define ZB_ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define ZB_ALWAYS_INLINE inline
#endif

#define ZB_INLINE ZB_ALWAYS_INLINE

#define ZB_CXPR constexpr
#define ZB_INLINE_CXPR ZB_INLINE constexpr

#if defined(__GNUC__) || defined(__clang__)
#define ZB_NEVER_INLINE __attribute__((__noinline__))
#elif __ZBASE_MSVC__
#define ZB_NEVER_INLINE __declspec(noinline)
#else
#define ZB_NEVER_INLINE
#endif

//
//
//
#define ZB_CHECK [[nodiscard]]

#define ZB_CK_INLINE ZB_CHECK ZB_INLINE
#define ZB_CK_INLINE_CXPR ZB_CHECK ZB_INLINE constexpr
//
// ZBASE_MAYBE_UNUSED
//

#define ZBASE_MAYBE_UNUSED [[maybe_unused]]

#define ZBASE_NO_BREAK [[fallthrough]]

//#define ZBASE_CASE_NO_BREAK(X)                                                                                  \
//  case X:                                                                                                    \
//    [[fallthrough]]

//#define __ZS_OBJECT_INITIALIZER_4(vtype, val, ktype, flags) \
//  { .vtype = val }, {                                       \
//    { { 0 }, { 0 }, object_type::ktype, flags }             \
//  }
//
// #define __ZBASE_CASE_NO_BREAK_1(X) \
//  case X: \
//    [[fallthrough]]
//
// #define __ZBASE_CASE_NO_BREAK_2(X) \
//  case X: \
//    [[fallthrough]]

#define __ZBASE_CASE_NO_BREAK_LAST(name) case name
#define __ZBASE_CASE_NO_BREAK(name) \
  case name:                        \
    [[fallthrough]];
#define ZBASE_CASE_NO_BREAK(...) \
  ZBASE_FOR_EACH_WITH_LAST(__ZBASE_CASE_NO_BREAK, __ZBASE_CASE_NO_BREAK_LAST, __VA_ARGS__)

//#define ZS_OBJECT_INITIALIZER(...) \
//  ZBASE_DEFER(ZBASE_CONCAT(__ZS_OBJECT_INITIALIZER_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)

//
// ZBASE_LIKELY / ZBASE_UNLIKELY
//

#if defined(__GNUC__) || defined(__clang__)
#define ZBASE_LIKELY(c) __builtin_expect((c), 1)
#define ZBASE_UNLIKELY(c) __builtin_expect((c), 0)
#else
#define ZBASE_LIKELY(c) c
#define ZBASE_UNLIKELY(c) c
#endif

//
//
//

/// @macro ZBASE_WARNING_ATTRIBUTE
#define ZBASE_WARNING_ATTRIBUTE(msg) __attribute__((warning(msg)))

/// @macro ZBASE_MACOS_ONLY
#ifdef __APPLE__
#define ZBASE_MACOS_ONLY
#else
#define ZBASE_MACOS_ONLY ZBASE_WARNING_ATTRIBUTE("Only available on MacOS")
#endif // __APPLE__

/// @macro ZBASE_WINDOWS_ONLY
#ifdef __WIN32
#define ZBASE_WINDOWS_ONLY
#else
#define ZBASE_WINDOWS_ONLY ZBASE_WARNING_ATTRIBUTE("Only available on Windows")
#endif // __WIN32

// borrowed from https://stackoverflow.com/a/27054190

#undef __ZBASE_BIG_ENDIAN__
#undef __ZBASE_LITTLE_ENDIAN__

#if (defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN)                                           \
    || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || defined(__BIG_ENDIAN__) \
    || defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || defined(_MIBSEB)       \
    || defined(__MIBSEB) || defined(__MIBSEB__)

#define __ZBASE_BIG_ENDIAN__ 1
#define __ZBASE_LITTLE_ENDIAN__ 0

#elif (defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN)                                            \
    || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || defined(__LITTLE_ENDIAN__) \
    || defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_MIPSEL)             \
    || defined(__MIPSEL) || defined(__MIPSEL__) || defined(_MSC_VER)

#define __ZBASE_BIG_ENDIAN__ 0
#define __ZBASE_LITTLE_ENDIAN__ 1

#else
#define __ZBASE_BIG_ENDIAN__ 0
#define __ZBASE_LITTLE_ENDIAN__ 0
#warning Unknown endianness
#endif

//
//
//

#undef __ZBASE_ARCH_ARM__
#undef __ZBASE_ARCH_X86__

#undef __ZBASE_ARCH_X86_32__
#undef __ZBASE_ARCH_X86_64__
#undef __ZBASE_ARCH_ARM_32__
#undef __ZBASE_ARCH_ARM_64__

#if defined(__arm64) || defined(__arm64__) || defined(_M_ARM64)
#define __ZBASE_ARCH_ARM__ 1
#define __ZBASE_ARCH_X86__ 0

#define __ZBASE_ARCH_X86_32__ 0
#define __ZBASE_ARCH_X86_64__ 0
#define __ZBASE_ARCH_ARM_32__ 0
#define __ZBASE_ARCH_ARM_64__ 1

#elif defined(__arm__) || defined(_M_ARM)
#define __ZBASE_ARCH_ARM__ 1
#define __ZBASE_ARCH_X86__ 0

#define __ZBASE_ARCH_X86_32__ 0
#define __ZBASE_ARCH_X86_64__ 0
#define __ZBASE_ARCH_ARM_32__ 1
#define __ZBASE_ARCH_ARM_64__ 0

#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64__) || defined(__amd64) || defined(_M_X64)
#define __ZBASE_ARCH_ARM__ 0
#define __ZBASE_ARCH_X86__ 1

#define __ZBASE_ARCH_X86_32__ 0
#define __ZBASE_ARCH_X86_64__ 1
#define __ZBASE_ARCH_ARM_32__ 0
#define __ZBASE_ARCH_ARM_64__ 0

#elif defined(i386) || defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__) \
    || defined(__i386) || defined(_M_IX86) || defined(_X86_) || defined(__I86__) || defined(__INTEL__)
#define __ZBASE_ARCH_ARM__ 0
#define __ZBASE_ARCH_X86__ 1

#define __ZBASE_ARCH_X86_32__ 1
#define __ZBASE_ARCH_X86_64__ 0
#define __ZBASE_ARCH_ARM_32__ 0
#define __ZBASE_ARCH_ARM_64__ 0

#else
#define __ZBASE_ARCH_ARM__ 0
#define __ZBASE_ARCH_X86__ 0

#define __ZBASE_ARCH_X86_32__ 0
#define __ZBASE_ARCH_X86_64__ 0
#define __ZBASE_ARCH_ARM_32__ 0
#define __ZBASE_ARCH_ARM_64__ 0
#endif

#undef __ZBASE_32BIT__
#undef __ZBASE_64BIT__

#if (defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__amd64) || defined(__aarch64__) \
    || defined(_M_ARM64) || defined(__MINGW64__) || defined(__s390x__)                                       \
    || (defined(__ppc64__) || defined(__PPC64__) || defined(__ppc64le__) || defined(__PPC64LE__))            \
    || defined(__loongarch64))

#define __ZBASE_64BIT__ 1
#define __ZBASE_32BIT__ 0

#elif (defined(__i386) || defined(__i386__) || defined(_M_IX86) || defined(__arm__) || defined(_M_ARM) \
    || defined(__ppc__) || defined(__MINGW32__) || defined(__EMSCRIPTEN__))

#define __ZBASE_64BIT__ 0
#define __ZBASE_32BIT__ 1

#else
// Need to check incrementally, since SIZE_MAX is a size_t, avoid overflow.
// We can never tell the register width, but the SIZE_MAX is a good
// approximation. UINTPTR_MAX and INTPTR_MAX are optional, so avoid them for max
// portability.
#if SIZE_MAX == 0xffff
#error Unknown platform (16-bit, unsupported)
#elif SIZE_MAX == 0xffffffff
#define __ZBASE_64BIT__ 0
#define __ZBASE_32BIT__ 1
#elif SIZE_MAX == 0xffffffffffffffff
#define __ZBASE_64BIT__ 1
#define __ZBASE_32BIT__ 0
#else
#error Unknown platform (not 32-bit, not 64-bit?)
#endif
#endif

#undef __ZBASE_SSE2__
#undef __ZBASE_NEON__

#if ((defined(_WIN32) || defined(_WIN64)) && !defined(__clang__)) \
    || (defined(_M_ARM64) && !defined(__MINGW32__))
#include <intrin.h>
#endif

#if defined(__SSE2__)  \
    || (__ZBASE_MSVC__ \
        && (defined(_M_AMD64) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP == 2)))
#define __ZBASE_SSE2__ 1
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define __ZBASE_NEON__ 1
#endif

#ifndef __ZBASE_SSE2__
#define __ZBASE_SSE2__ 0
#endif

#ifndef __ZBASE_NEON__
#define __ZBASE_NEON__ 0
#endif

#if __ZBASE_SSE2__ || __ZBASE_NEON__
#define __ZBASE_HAS_SIMD__ 1
#else
#define __ZBASE_HAS_SIMD__ 0
#endif

/// @macro ZBASE_PACKED
/// Used to specify a packed structure.
/// ZBASE_PACKED(
///   struct A {
///      int i;
///      int j;
///      int k;
///      long long l;
///   });
///
/// ZBASE_PACKED_START
/// struct B {
///   int i;
///   int j;
///   int k;
///   long long l;
/// };
/// ZBASE_PACKED_END
#if __ZBASE_MSVC__
#define ZBASE_PACKED(d) __pragma(pack(push, 1)) d __pragma(pack(pop))
#define ZBASE_PACKED_START __pragma(pack(push, 1))
#define ZBASE_PACKED_N_START(N) __pragma(pack(push, N))
#define ZBASE_PACKED_END __pragma(pack(pop))

#else
#define ZBASE_PACKED(d) d __attribute__((packed))
#define ZBASE_PACKED_START _Pragma("pack(push, 1)")
#define ZBASE_PACKED_N_START(N) ZBASE_PRAGMA(pack(push, N))
#define ZBASE_PACKED_END _Pragma("pack(pop)")
#endif

/// @macro ZBASE_ATTRIBUTE_RETURNS_NONNULL.
#if ZBASE_HAS_ATTRIBUTE(returns_nonnull)
#define ZBASE_ATTRIBUTE_RETURNS_NONNULL __attribute__((returns_nonnull))
#elif __ZBASE_MSVC__
#define ZBASE_ATTRIBUTE_RETURNS_NONNULL _Ret_notnull_
#else
#define ZBASE_ATTRIBUTE_RETURNS_NONNULL
#endif

/// @macroZBASE_ATTRIBUTE_NO_RETURN.
#if ZBASE_HAS_ATTRIBUTE(noreturn)
#define ZBASE_ATTRIBUTE_NO_RETURN __attribute__((noreturn))
#elif __ZBASE_MSVC__
#define ZBASE_ATTRIBUTE_NO_RETURN __declspec(noreturn)
#else
#define ZBASE_ATTRIBUTE_NO_RETURN
#endif

//
//
//

#define ZBASE_BLOCK(x) \
  do {                 \
    x                  \
  } while (false)

//
//
//

// #define ZBASE_NARG(...) ZBASE_NARG_(__VA_ARGS__, ZBASE_RSEQ_N())
#define ZBASE_NARG(...) ZBASE_NARG_(__VA_ARGS__ __VA_OPT__(, ) ZBASE_RSEQ_N())
#define ZBASE_NARG_(...) ZBASE_ARG_N(__VA_ARGS__)

#define ZBASE_NARG0(...) ZBASE_NARG0_(__VA_ARGS__ __VA_OPT__(, ) ZBASE_RSEQ_N())
#define ZBASE_NARG0_(...) ZBASE_ARG_N0(__VA_ARGS__)

#define ZBASE_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19,    \
    _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
    _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, \
    _62, _63, N, ...)                                                                                        \
  N

#define ZBASE_ARG_N0(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18,    \
    _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
    _61, _62, _63, N, ...)                                                                                   \
  N

#define ZBASE_RSEQ_N()                                                                                    \
  63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, \
      37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, \
      12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define __ZBASE_FE_1(W, LST, x) LST(x)
#define __ZBASE_FE_2(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_1(W, LST, __VA_ARGS__)
#define __ZBASE_FE_3(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_2(W, LST, __VA_ARGS__)
#define __ZBASE_FE_4(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_3(W, LST, __VA_ARGS__)
#define __ZBASE_FE_5(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_4(W, LST, __VA_ARGS__)
#define __ZBASE_FE_6(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_5(W, LST, __VA_ARGS__)
#define __ZBASE_FE_7(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_6(W, LST, __VA_ARGS__)
#define __ZBASE_FE_8(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_7(W, LST, __VA_ARGS__)
#define __ZBASE_FE_9(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_8(W, LST, __VA_ARGS__)
#define __ZBASE_FE_10(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_9(W, LST, __VA_ARGS__)
#define __ZBASE_FE_11(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_10(W, LST, __VA_ARGS__)
#define __ZBASE_FE_12(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_11(W, LST, __VA_ARGS__)
#define __ZBASE_FE_13(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_12(W, LST, __VA_ARGS__)
#define __ZBASE_FE_14(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_13(W, LST, __VA_ARGS__)
#define __ZBASE_FE_15(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_14(W, LST, __VA_ARGS__)
#define __ZBASE_FE_16(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_15(W, LST, __VA_ARGS__)
#define __ZBASE_FE_17(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_16(W, LST, __VA_ARGS__)
#define __ZBASE_FE_18(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_17(W, LST, __VA_ARGS__)
#define __ZBASE_FE_19(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_18(W, LST, __VA_ARGS__)
#define __ZBASE_FE_20(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_19(W, LST, __VA_ARGS__)
#define __ZBASE_FE_21(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_20(W, LST, __VA_ARGS__)
#define __ZBASE_FE_22(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_21(W, LST, __VA_ARGS__)
#define __ZBASE_FE_23(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_22(W, LST, __VA_ARGS__)
#define __ZBASE_FE_24(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_23(W, LST, __VA_ARGS__)
#define __ZBASE_FE_25(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_24(W, LST, __VA_ARGS__)
#define __ZBASE_FE_26(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_25(W, LST, __VA_ARGS__)
#define __ZBASE_FE_27(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_26(W, LST, __VA_ARGS__)
#define __ZBASE_FE_28(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_27(W, LST, __VA_ARGS__)
#define __ZBASE_FE_29(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_28(W, LST, __VA_ARGS__)
#define __ZBASE_FE_30(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_29(W, LST, __VA_ARGS__)
#define __ZBASE_FE_31(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_30(W, LST, __VA_ARGS__)
#define __ZBASE_FE_32(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_31(W, LST, __VA_ARGS__)
#define __ZBASE_FE_33(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_32(W, LST, __VA_ARGS__)
#define __ZBASE_FE_34(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_33(W, LST, __VA_ARGS__)
#define __ZBASE_FE_35(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_34(W, LST, __VA_ARGS__)
#define __ZBASE_FE_36(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_35(W, LST, __VA_ARGS__)
#define __ZBASE_FE_37(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_36(W, LST, __VA_ARGS__)
#define __ZBASE_FE_38(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_37(W, LST, __VA_ARGS__)
#define __ZBASE_FE_39(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_38(W, LST, __VA_ARGS__)
#define __ZBASE_FE_40(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_39(W, LST, __VA_ARGS__)
#define __ZBASE_FE_41(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_40(W, LST, __VA_ARGS__)
#define __ZBASE_FE_42(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_41(W, LST, __VA_ARGS__)
#define __ZBASE_FE_43(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_42(W, LST, __VA_ARGS__)
#define __ZBASE_FE_44(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_43(W, LST, __VA_ARGS__)
#define __ZBASE_FE_45(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_44(W, LST, __VA_ARGS__)
#define __ZBASE_FE_46(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_45(W, LST, __VA_ARGS__)
#define __ZBASE_FE_47(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_46(W, LST, __VA_ARGS__)
#define __ZBASE_FE_48(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_47(W, LST, __VA_ARGS__)
#define __ZBASE_FE_49(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_48(W, LST, __VA_ARGS__)
#define __ZBASE_FE_50(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_49(W, LST, __VA_ARGS__)
#define __ZBASE_FE_51(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_50(W, LST, __VA_ARGS__)
#define __ZBASE_FE_52(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_51(W, LST, __VA_ARGS__)
#define __ZBASE_FE_53(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_52(W, LST, __VA_ARGS__)
#define __ZBASE_FE_54(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_53(W, LST, __VA_ARGS__)
#define __ZBASE_FE_55(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_54(W, LST, __VA_ARGS__)
#define __ZBASE_FE_56(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_55(W, LST, __VA_ARGS__)
#define __ZBASE_FE_57(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_56(W, LST, __VA_ARGS__)
#define __ZBASE_FE_58(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_57(W, LST, __VA_ARGS__)
#define __ZBASE_FE_59(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_58(W, LST, __VA_ARGS__)
#define __ZBASE_FE_60(W, LST, x, ...) ZBASE_DEFER(W, x) __ZBASE_FE_59(W, LST, __VA_ARGS__)

#define ZBASE_FOR_EACH_NARG(...) ZBASE_FOR_EACH_NARG_(__VA_ARGS__, ZBASE_FOR_EACH_RSEQ_N())
#define ZBASE_FOR_EACH_NARG_(...) ZBASE_FOR_EACH_ARG_N(__VA_ARGS__)
#define ZBASE_FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17,     \
    _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, \
    _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, \
    _60, N, ...)                                                                                             \
  N

#define ZBASE_FOR_EACH_RSEQ_N()                                                                              \
  60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35,    \
      34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, \
      8, 7, 6, 5, 4, 3, 2, 1, 0

#define __ZBASE_FOR_EACH_(N, W, LST, ...) ZBASE_CONCAT(__ZBASE_FE_, N)(W, LST, __VA_ARGS__)

#define ZBASE_FOR_EACH_WITH_LAST(W, LST, ...) \
  __ZBASE_FOR_EACH_(ZBASE_FOR_EACH_NARG(__VA_ARGS__), W, LST, __VA_ARGS__)

#define ZBASE_FOR_EACH(W, ...) ZBASE_FOR_EACH_WITH_LAST(W, W, __VA_ARGS__)

/// zb_alloca
#if defined(__clang__)
#define zb_alloca(size) __builtin_alloca(size)

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
#include <malloc.h>
#define zb_alloca(size) _alloca(size)

#else
#define zb_alloca(size) alloca(size)
#endif

#define zb_loop() while (true)

#define zb_bit_offsetof(t, f) \
  ({                          \
    union {                   \
      unsigned long long raw; \
      t typ;                  \
    };                        \
    raw = 0;                  \
    ++typ.f;                  \
    std::countr_zero(raw);    \
  })

#define zb_bit_sizeof(t, f)                                          \
  ({                                                                 \
    union {                                                          \
      unsigned long long raw;                                        \
      t typ;                                                         \
    };                                                               \
    raw = 0;                                                         \
    --typ.f;                                                         \
    8 * sizeof(raw) - std::countl_zero(raw) - std::countr_zero(raw); \
  })

#ifdef __cplusplus
#define ZBASE_BEGIN_EXTERN_C extern "C" {
#define ZBASE_END_EXTERN_C } // extern "C"
#else
#define ZBASE_BEGIN_EXTERN_C
#define ZBASE_END_EXTERN_C
#endif // __cplusplus.

#define ZBASE_LINE (__LINE__ - 1)

#define ZBASE_DECLARE_DEFAULT_CTOR(NAME)                  \
  inline NAME() noexcept = default;                       \
  inline NAME(const NAME&) noexcept = default;            \
  inline NAME(NAME&&) noexcept = default;                 \
  inline NAME& operator=(const NAME&) noexcept = default; \
  inline NAME& operator=(NAME&&) noexcept = default

#define ZBASE_DECLARE_DEFAULT_CEXPR_CTOR(NAME)                      \
  inline constexpr NAME() noexcept = default;                       \
  inline constexpr NAME(const NAME&) noexcept = default;            \
  inline constexpr NAME(NAME&&) noexcept = default;                 \
  inline constexpr NAME& operator=(const NAME&) noexcept = default; \
  inline constexpr NAME& operator=(NAME&&) noexcept = default

ZBASE_BEGIN_NAMESPACE

template <class... Args>
ZB_INLINE constexpr void unused(Args&&...) noexcept {}

#define zb_placement_new(mem) ::new (mem, zb::placement_new_tag{})
struct placement_new_tag {};

struct variadic_args_begin_tag {};
struct variadic_args_end_tag {};

struct nocopy_nomove {
  nocopy_nomove() = default;
  ~nocopy_nomove() = default;
  nocopy_nomove(const nocopy_nomove&) = delete;
  nocopy_nomove& operator=(const nocopy_nomove&) = delete;
  nocopy_nomove(nocopy_nomove&&) = delete;
  nocopy_nomove& operator=(nocopy_nomove&&) = delete;
};

ZBASE_END_NAMESPACE

[[nodiscard]] ZBASE_ATTRIBUTE_RETURNS_NONNULL ZB_ALWAYS_INLINE void* operator new(
    size_t, void* memory, zb::placement_new_tag) noexcept {
  return memory;
}

ZB_ALWAYS_INLINE void operator delete(void*, void*, zb::placement_new_tag) noexcept {}
