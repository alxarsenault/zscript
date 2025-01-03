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

#include <zscript/base/zbase.h>
#include <zscript/base/sys/assert.h>
#include <zscript/base/utility/math.h>
#include <zscript/base/utility/traits.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>

#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#include <intrin.h>

#if defined(_WIN64)
#pragma intrinsic(_BitScanReverse64)
#else
#pragma intrinsic(_BitScanReverse)
#endif
#endif

ZBASE_BEGIN_NAMESPACE

inline constexpr size_t k_platform_alignment = sizeof(void*);
using platform_alignment_t = std::integral_constant<size_t, k_platform_alignment>;

///
inline constexpr size_t k_default_alignment = 16;
inline constexpr size_t k_default_alignment_bit_index = 4;
using default_alignment_t = std::integral_constant<size_t, k_default_alignment>;

///
inline constexpr size_t k_default_vectorized_alignment = 32;
inline constexpr size_t k_default_vectorized_bit_index = 5;
inline constexpr size_t k_default_vectorized_size_threshold = 4096;
using default_vectorized_alignment_t = std::integral_constant<size_t, k_default_vectorized_alignment>;

///
template <class _T>
inline constexpr size_t k_default_alignof
    = alignof(_T) < __zb::k_default_alignment ? __zb::k_default_alignment : alignof(_T);

template <class _T>
using default_alignof_t = std::integral_constant<size_t, k_default_alignof<_T>>;

///
template <class _T>
inline constexpr size_t k_default_vectorized_alignof
    = alignof(_T) < __zb::k_default_vectorized_alignment ? __zb::k_default_vectorized_alignment : alignof(_T);

template <class _T>
using default_vectorized_alignof_t = std::integral_constant<size_t, k_default_vectorized_alignof<_T>>;

///
template <class... Ts>
struct max_align_size_s;

template <class T>
struct max_align_size_s<T> {
  static constexpr size_t size = alignof(T);
};

template <class T, class... Ts>
struct max_align_size_s<T, Ts...> {
  static constexpr size_t size = __zb::maximum(alignof(T), max_align_size_s<Ts...>::size);
};

///
template <class... Ts>
ZB_CHECK inline constexpr size_t max_align_size() {
  return max_align_size_s<Ts...>::size;
}

/// Informs the implementation that the object ptr points to is aligned to at least _Alignment.
/// The implementation may use this information to generate more efficient code, but it might
/// only make this assumption if the object is accessed via the return value of assume_aligned.
///
/// The program is ill-formed if _Alignment is not a power of 2.
/// The behavior is undefined if ptr does not point to an object of type _T
/// (ignoring cv-qualification at every level), or if the object's alignment is not at least _Alignment.
template <size_t _Alignment, class _T>
ZB_CHECK constexpr _T* assume_aligned(_T* ptr) noexcept {
  if (std::is_constant_evaluated()) {
    return ptr;
  }

#if ZBASE_HAS_BUILTIN(__builtin_assume_aligned) || __ZBASE_MSVC__
  return static_cast<_T*>(__builtin_assume_aligned(ptr, _Alignment));
#else
  return ptr;
#endif
}

///
ZB_CHECK ZB_INLINE bool is_aligned(const void* ptr, uintptr_t alignment) noexcept {
  return !(uintptr_t(ptr) & (alignment - 1));
}

///
ZB_CHECK ZB_INLINE constexpr bool is_aligned(uintptr_t addr, size_t alignment) noexcept {
  return !(addr & (alignment - 1));
}

///
template <class T>
ZB_CHECK ZB_INLINE bool is_type_aligned(const void* ptr) noexcept {
  return is_aligned(ptr, alignof(T));
}

ZB_CHECK ZB_INLINE constexpr size_t clip_alignment(size_t alignment) noexcept {
  zbase_assert(__zb::is_power_of_two(alignment), "alignment must be a power of two");
  if (alignment < k_default_alignment) {
    alignment = k_default_alignment;
  }
  zbase_assert(__zb::is_multiple_of_power_of_two(alignment, __zb::k_platform_alignment),
      "alignment must be a multiple of fst::platform_alignment");

  return alignment;
}

ZB_CHECK ZB_INLINE constexpr size_t clip_vectorized_alignment(size_t alignment) noexcept {
  zbase_assert(__zb::is_power_of_two(alignment), "alignment must be a power of two");
  if (alignment < k_default_vectorized_alignment) {
    alignment = k_default_vectorized_alignment;
  }
  zbase_assert(__zb::is_multiple_of_power_of_two(alignment, __zb::k_platform_alignment),
      "alignment must be a multiple of fst::platform_alignment");
  return alignment;
}

///
ZB_CHECK ZB_INLINE constexpr size_t align(uintptr_t ptr, size_t alignment) noexcept {
  return (ptr + (alignment - 1)) & ~(alignment - 1);
}

///
ZB_CHECK ZB_INLINE char* align(char* ptr, size_t alignment) noexcept {
  return (char*)(__zb::align((uintptr_t)ptr, alignment));
}

///
ZB_CHECK ZB_INLINE void* align(void* ptr, size_t alignment) noexcept {
  return (void*)(__zb::align((uintptr_t)ptr, alignment));
}

///
ZB_CHECK ZB_INLINE char* align_range(char* begin, char* end, size_t size, size_t alignment) noexcept {
  zbase_assert(size_t(end - begin) >= size, "size must be greater than range size (end - begin)");

  char* ptr = (char*)(__zb::align((uintptr_t)begin, alignment));
  return ptr + size > end ? nullptr : ptr;
}

///
ZB_CHECK ZB_INLINE void* align_range(void* begin, void* end, size_t size, size_t alignment) noexcept {
  return __zb::align_range((char*)begin, (char*)end, size, alignment);
}

///
/// Given a pointer ptr to a buffer of size avail_space, returns a pointer aligned by the specified alignment
/// for size number of bytes and decreases avail_space argument by the number of bytes used for alignment.
/// The first aligned address is returned.
///
/// The function modifies the pointer only if it would be possible to fit the wanted number of bytes aligned
/// by the given alignment into the buffer. If the buffer is too small, the function does nothing and returns
/// nullptr.
///
/// @note The behavior is undefined if alignment is not a power of two.
///
/// @param alignment the desired alignment
/// @param size the size of the storage to be aligned
/// @param ptr pointer to contiguous storage (a buffer) of at least avail_space bytes
/// @param avail_space the size of the buffer in which to operate
///
/// @returns the adjusted value of ptr, or nullptr if the space provided is too small.
ZB_CHECK ZB_INLINE void* align(size_t alignment, size_t size, void* ptr, size_t& avail_space) noexcept {
  zbase_assert(avail_space >= size);
  zbase_assert(size > 0, "invalid size");
  zbase_assert(alignment && !(alignment & (alignment - 1)), "alignment must be a power of two");
  zbase_assert(!(alignment & (sizeof(void*) - 1)), "alignment must be a multiple of sizeof(void*)");

  if (size_t offset = static_cast<size_t>(reinterpret_cast<uintptr_t>(ptr) & (alignment - 1))) {
    offset = alignment - offset;
    if (avail_space < offset || avail_space - offset < size) {
      return nullptr;
    }

    avail_space -= offset;
    return static_cast<char*>(ptr) + offset;
  }

  return ptr;
}

/// Get the minimum required size to allocate one element of size `size` aligned as `alignment`
/// given an a pointer aligned as `input_alignment`.
///
/// @param input_alignment The minimum alignment of the input pointer.
/// @param size The size of each element.
/// @param alignment The required alignment of each element.
/// @param align_end When true, the total size will be aligned on both size.
///
/// @returns the minimum size required.
ZB_CHECK ZB_INLINE size_t required_aligned_size(
    size_t input_alignment, size_t size, size_t alignment, bool align_end) noexcept {
  zbase_assert(input_alignment > 0, "invalid input alignment");
  zbase_assert(size > 0, "invalid size");
  zbase_assert(alignment && !(alignment & (alignment - 1)), "alignment must be a power of two");
  zbase_assert(!(alignment & (sizeof(void*) - 1)), "alignment must be a multiple of sizeof(void*)");

  const size_t sz = input_alignment >= alignment ? size : alignment - input_alignment + size;
  return align_end ? __zb::align(sz, alignment) : sz;
}

/// Get the minimum required size to allocate `count` elements of size `size` aligned as `alignment`
/// given an a pointer aligned as `input_alignment`.
///
/// @param input_alignment The minimum alignment of the input pointer.
/// @param size The size of each element.
/// @param alignment The required alignment of each element.
/// @param count The number of elements required.
/// @param align_end When true, the total size will be aligned on both size.
///
/// @returns the minimum size required.
///
/// @note A count of one, calls `required_aligned_size(size_t, size_t, size_t, bool)` internally.
/// @note A count of zero, is an invalid value and will assert.
ZB_CHECK ZB_INLINE size_t required_aligned_size(
    size_t input_alignment, size_t size, size_t alignment, size_t count, bool align_end) noexcept {
  zbase_assert(input_alignment > 0, "invalid input alignment");
  zbase_assert(size > 0, "invalid size");
  zbase_assert(alignment && !(alignment & (alignment - 1)), "alignment must be a power of two");
  zbase_assert(!(alignment & (sizeof(void*) - 1)), "alignment must be a multiple of sizeof(void*)");
  zbase_assert(count > 0, "invalid count");

  if (count == 1) {
    return __zb::required_aligned_size(input_alignment, size, alignment, align_end);
  }

  const size_t sz = __zb::required_aligned_size(input_alignment, size, alignment, true)
      + __zb::align(size, alignment) * count;
  return align_end ? __zb::align(sz, alignment) : sz;
}

struct aligned_type_storage_construct_tag {};

template <class T>
struct aligned_type_storage {
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  aligned_type_storage() noexcept = default;

  template <class... Args>
  inline aligned_type_storage(aligned_type_storage_construct_tag, Args&&... args) noexcept {
    if constexpr (std::is_trivial_v<T>) {
      get() = T(std::forward<Args>(args)...);
    }
    else {
      zb_placement_new(_data) T(std::forward<Args>(args)...);
    }
  }

  ZB_CHECK static inline constexpr size_type size() noexcept { return sizeof(T); }
  ZB_CHECK static inline constexpr size_type alignment() noexcept { return alignof(T); }
  ZB_CHECK static inline constexpr size_type aligned_size() noexcept { return sizeof(T); }

  ZB_CHECK inline T* data() noexcept { return (T*)_data; }
  ZB_CHECK inline const T* data() const noexcept { return (const T*)_data; }

  template <class... Args>
  inline T* construct(Args&&... args) noexcept {
    if constexpr (std::is_trivial_v<T>) {
      get() = T(std::forward<Args>(args)...);
      return data();
    }
    else {
      return zb_placement_new(_data) T(std::forward<Args>(args)...);
    }
  }

  inline void destroy() noexcept {
    if constexpr (!std::is_trivial_v<T>) {
      get().~T();
    }
  }

  ZB_CHECK inline T& get() noexcept { return *data(); }
  ZB_CHECK inline const T& get() const noexcept { return *data(); }

  ZB_CHECK inline operator T&() noexcept { return get(); }
  ZB_CHECK inline operator const T&() const noexcept { return get(); }

private:
  alignas(alignof(T)) uint8_t _data[sizeof(T)];
};

/// Construct a 64-bit literal by a pair of 32-bit integer.
///
/// 64-bit literal with or without ULL suffix is prone to compiler warnings.
/// UINT64_C() is C macro which cause compilation problems.
/// Use this macro to define 64-bit constants by a pair of 32-bit integer.
#define ZBASE_UINT64_C2(high32, low32) ((static_cast<uint64_t>(high32) << 32) | static_cast<uint64_t>(low32))

// template <size_t _Len, size_t _Align = __find_max_align<__all_types, _Len>::value>
// struct _LIBCPP_DEPRECATED_IN_CXX23 _LIBCPP_TEMPLATE_VIS aligned_storage {
//   typedef typename __find_pod<__all_types, _Align>::type _Aligner;
//   union type {
//     _Aligner __align;
//     unsigned char __data[(_Len + _Align - 1) / _Align * _Align];
//   };
// };

void* aligned_allocate(size_t size, size_t alignment) noexcept;
// void* aligned_reallocate(void* ptr, size_t size, size_t old_size, size_t alignment) noexcept;
void aligned_deallocate(void* ptr) noexcept;

/// Copies `size` bytes from the object pointed to by `src` to the object pointed to by `dst`.
/// Both objects are reinterpreted as arrays of `unsigned char`.
///
/// @param dst pointer to the memory location to copy to
/// @param src pointer to the memory location to copy from
/// @param size number of bytes to copy
///
/// @note If the objects overlap, the behavior is undefined.
///.@note If either dest or src is an invalid or null pointer, the behavior is undefined, even if count is
/// zero.
ZB_INLINE void memcpy(void* dst, const void* src, size_t size) noexcept;

/// Copies `size` bytes from the object pointed to by `src` to the object pointed to by `dst`.
/// Both objects are reinterpreted as arrays of unsigned char.
///
/// @param dst pointer to the memory location to copy to
/// @param src pointer to the memory location to copy from
/// @param size number of bytes to copy
///
/// @note The objects may overlap: copying takes place as if the bytes were copied to a temporary byte
///       array and then the bytes were copied from the array to `dst`.
///
/// @note If either `dst` or src is an invalid or null pointer, the behavior is undefined, even if count is
/// zero.
ZB_INLINE void memmove(void* dst, const void* src, size_t size) noexcept;

///
ZB_CHECK ZB_INLINE int memcmp(const void* lhs, const void* rhs, size_t size) noexcept;

///
ZB_INLINE void memset(void* dst, int value, size_t size) noexcept;

///
ZB_CHECK ZB_INLINE constexpr size_t strlen(const char* str) noexcept;

/// Returns the number of characters in the string, not including the terminating null character.
/// If there's no null terminator within the first `max_length` bytes of the string,
/// then `max_length` is returned to indicate the error condition.
/// null-terminated strings have lengths that are strictly less than `max_length`.
///
/// strnlen is not a replacement for strlen, strnlen is intended to be used only to calculate
/// the size of incoming untrusted data in a buffer of known size�for example, a network packet.
/// strnlen calculates the length but doesn't walk past the end of the buffer if the string is unterminated.
/// For other situations, use strlen.
ZB_CHECK ZB_INLINE size_t strnlen(const char* str, size_t max_length) noexcept;

///
ZB_CHECK ZB_INLINE int strncmp(const char* a, const char* b, size_t size) noexcept;

///
ZB_CHECK ZB_INLINE int strcmp(const char* a, const char* b) noexcept;

///
ZB_CHECK ZB_INLINE char* strncpy(char* dst, const char* src, size_t count) noexcept;

///
ZB_CHECK ZB_INLINE char* strncat(char* dst, const char* src, size_t count) noexcept;

/// Returns the number of leading 0-bits in x, starting at the most significant bit position.
ZB_CHECK ZB_INLINE uint32_t clzll(uint64_t x) noexcept;

template <size_t _Size, size_t _Alignment>
struct aligned_storage {
  using value_type = uint8_t;
  using pointer = uint8_t*;
  using const_pointer = const uint8_t*;
  using reference = uint8_t&;
  using const_reference = uint8_t;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  ///
  ZB_CHECK static inline constexpr size_type size() noexcept { return _Size; }
  ZB_CHECK static inline constexpr size_type alignment() noexcept { return _Alignment; }
  ZB_CHECK static inline constexpr size_type aligned_size() noexcept { return sizeof(aligned_storage); }

  ZB_CHECK inline constexpr pointer data() noexcept { return &_data[0]; }
  ZB_CHECK inline constexpr const_pointer data() const noexcept { return &_data[0]; }

  ZB_CHECK inline constexpr pointer data(size_type index) noexcept {
    zbase_assert(index < aligned_size(), "index out of bounds");
    return &_data[index];
  }

  ZB_CHECK inline constexpr const_pointer data(size_type index) const noexcept {
    zbase_assert(index < aligned_size(), "index out of bounds");
    return &_data[index];
  }

  template <class T>
  ZB_CHECK inline constexpr T* data() noexcept {
    return reinterpret_cast<T*>(data());
  }

  template <class T>
  ZB_CHECK inline constexpr const T* data() const noexcept {
    return reinterpret_cast<const T*>(data());
  }

  template <class T>
  ZB_CHECK inline constexpr T* data(size_type offset) noexcept {
    zbase_assert(offset + sizeof(T) <= aligned_size(), "offset out of bounds");
    return reinterpret_cast<T*>(data(offset));
  }

  template <class T>
  ZB_CHECK inline constexpr const T* data(size_type offset) const noexcept {
    zbase_assert(offset + sizeof(T) <= aligned_size(), "offset out of bounds");
    return reinterpret_cast<const T*>(data(offset));
  }

  template <class T>
  ZB_CHECK inline constexpr T& as(size_type offset = 0) noexcept {
    zbase_assert(offset + sizeof(T) <= aligned_size(), "offset out of bounds");
    return *reinterpret_cast<T*>(data() + offset);
  }

  template <class T>
  ZB_CHECK inline constexpr const T& as(size_type offset = 0) const noexcept {
    zbase_assert(offset + sizeof(T) <= aligned_size(), "offset out of bounds");
    return *reinterpret_cast<const T*>(data() + offset);
  }

  ZB_CHECK inline constexpr reference operator[](size_type index) noexcept {
    zbase_assert(index < aligned_size(), "index out of bounds");
    return _data[index];
  }

  ZB_CHECK inline constexpr value_type operator[](size_type index) const noexcept {
    zbase_assert(index < aligned_size(), "index out of bounds");
    return _data[index];
  }

  template <size_t _OtherAlignment>
  ZB_CHECK inline bool operator==(const aligned_storage<_Size, _OtherAlignment>& s) const noexcept {
    return !__zb::memcmp(data(), s.data(), size());
  }

  template <size_t _OtherAlignment>
  ZB_CHECK inline bool operator!=(const aligned_storage<_Size, _OtherAlignment>& s) const noexcept {
    return __zb::memcmp(data(), s.data(), size());
  }

  alignas(_Alignment) value_type _data[_Size];
};

template <class T, size_t Alignment = __zb::maximum(alignof(T), alignof(void*))>
struct aligned_allocator {
public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = size_t;
  using difference_type = ptrdiff_t;

  template <class U>
  struct rebind {
    using other = aligned_allocator<U, Alignment>;
  };

  inline aligned_allocator() noexcept = default;
  inline aligned_allocator(const aligned_allocator&) noexcept = default;

  template <typename U>
  inline aligned_allocator(const aligned_allocator<U, Alignment>&) noexcept {}

  inline ~aligned_allocator() noexcept = default;

  ZB_CHECK inline pointer allocate(size_type n, [[maybe_unused]] const void* hint = nullptr) {
    size_t alignment = Alignment;
    n = __zb::maximum(n, alignment);
    pointer p = reinterpret_cast<pointer>(__zb::aligned_allocate((sizeof(value_type) * n), alignment));
    zbase_assert(p);
    return p;
  }

  inline void deallocate(pointer p, size_type) noexcept { __zb::aligned_deallocate(p); }

  inline constexpr size_type max_size() const noexcept {
    return std::numeric_limits<size_type>::max() / sizeof(value_type);
  }
};

//
// MARK: Implementation.
//

ZB_INLINE void memcpy(void* dst, const void* src, size_t size) noexcept {
#if __ZBASE_CLANG__
  __builtin_memcpy(dst, src, size);
#else

#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma intrinsic(memcpy)
#endif

  ::memcpy(dst, src, size);

#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma function(memcpy)
#endif

#endif
}

ZB_INLINE void memmove(void* dst, const void* src, size_t size) noexcept {
#if __ZBASE_CLANG__
  __builtin_memmove(dst, src, size);
#else

#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma intrinsic(memmove)
#endif

  ::memmove(dst, src, size);

#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma function(memmove)
#endif
#endif
}

ZB_CHECK ZB_INLINE int memcmp(const void* lhs, const void* rhs, size_t size) noexcept {
#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma intrinsic(memcmp)
#endif

  return ::memcmp(lhs, rhs, size);

#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma function(memcmp)
#endif
}

///
ZB_INLINE void memset(void* dst, int value, size_t size) noexcept {

#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma intrinsic(memset)
#endif

  ::memset(dst, value, size);

#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma function(memset)
#endif
}

///
ZB_CHECK ZB_INLINE constexpr size_t strlen(const char* str) noexcept {
#if __ZBASE_MSVC__ || __ZBASE_CLANG__
  return __builtin_strlen(str);
#else
  return ::strlen(str);
#endif
}

ZB_CK_INLINE_CXPR size_t constexpr_strlen(const char* str) noexcept {
  size_t i = 0;
  for (; str[i] != '\0'; ++i) {
  }

  return i;
}

ZB_CHECK ZB_INLINE size_t strnlen(const char* str, size_t max_length) noexcept {
#if __ZBASE_WINDOWS__
  return ::strnlen(str, max_length);

#elif __ZBASE_CLANG__
  if (const char* end = __builtin_char_memchr(str, 0, max_length)) {
    return (size_t)(end - str);
  }
  return max_length;
#else
  if (const char* end = ::memchr(str, 0, max_length)) {
    return (size_t)(end - str);
  }
  return max_length;
#endif
}

///
ZB_CHECK ZB_INLINE int strncmp(const char* a, const char* b, size_t size) noexcept {
#if __ZBASE_CLANG__
  return __builtin_strncmp(a, b, size);
#else
  return ::strncmp(a, b, size);
#endif
}

///
ZB_CHECK ZB_INLINE int strcmp(const char* a, const char* b) noexcept {
#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma intrinsic(strcmp)
#endif

  return ::strcmp(a, b);

#if __ZBASE_MSVC__ && __ZBASE_INTEL__
#pragma function(strcmp)
#endif
}

ZB_CHECK ZB_INLINE char* strncpy(char* dst, const char* src, size_t count) noexcept {
  return ::strncpy(dst, src, count);
}

ZB_CHECK ZB_INLINE char* strncat(char* dst, const char* src, size_t count) noexcept {
  return ::strncat(dst, src, count);
}

ZB_CHECK ZB_INLINE uint32_t clzll(uint64_t x) noexcept {

  // Passing 0 to __builtin_clzll is UB in GCC and results in an
  // infinite loop in the software implementation.
  if (x == 0) {
    return 64;
  }

#if defined(_MSC_VER) && !defined(UNDER_CE)
  unsigned long r = 0;
#if defined(_WIN64)

  _BitScanReverse64(&r, x);
#else
  // Scan the high 32 bits.
  if (_BitScanReverse(&r, static_cast<uint32_t>(x >> 32))) {
    return 63 - (r + 32);
  }

  // Scan the low 32 bits.
  _BitScanReverse(&r, static_cast<uint32_t>(x & 0xFFFFFFFF));
#endif // _WIN64

  return 63 - r;

#elif (defined(__GNUC__) && __GNUC__ >= 4) || ZBASE_HAS_BUILTIN(__builtin_clzll)
  // __builtin_clzll wrapper
  return static_cast<uint32_t>(__builtin_clzll(x));

#else
  // naive version
  uint32_t r = 0;
  while (!(x & (static_cast<uint64_t>(1) << 63))) {
    x <<= 1;
    ++r;
  }

  return r;
#endif // _MSC_VER
}

template <class _T,
    std::enable_if_t<std::is_move_constructible_v<_T> && std::is_move_assignable_v<_T>, int> = 0>
ZB_INLINE constexpr void mem_swap(_T& _Left, _T& _Right) noexcept {
  _T _Tmp = std::move(_Left);
  _Left = std::move(_Right);
  _Right = std::move(_Tmp);
}

///
template <class T>
ZB_INLINE void mem_fill(T* dst, __zb::cref_t<T> value, size_t size) noexcept {
  for (size_t i = 0; i < size; i++) {
    dst[i] = value;
  }
}

///
template <class T>
ZB_INLINE void mem_copy(T* dst, const T* src, size_t size) noexcept {
  if constexpr (std::is_trivially_copyable_v<T>) {
    __zb::memcpy((void*)dst, (const void*)src, size * sizeof(T));
  }
  else {
    for (size_t i = 0; i < size; i++) {
      dst[i] = src[i];
    }
  }
}

template <class T>
ZB_INLINE void mem_zero(T& dst) noexcept {
  __zb::memset((void*)&dst, 0, sizeof(T));
}

template <class T>
ZB_INLINE void mem_zero(T* dst, size_t n) noexcept {
  __zb::memset((void*)dst, 0, n * sizeof(T));
}

template <class _T>
ZB_INLINE void relocate(_T* dst, _T* src, size_t size) noexcept {
  if constexpr (std::is_trivially_copyable_v<_T>) {
    __zb::memcpy(dst, src, size * sizeof(_T));
  }
  else if constexpr (std::is_trivially_destructible_v<_T>) {
    for (size_t i = 0; i < size; i++) {
      zb_placement_new(dst + i) _T(std::move(src[i]));
    }
  }
  else {
    for (size_t i = 0; i < size; i++) {
      zb_placement_new(dst + i) _T(std::move(src[i]));
      src[i].~_T();
    }
  }
}

template <class _T>
ZB_INLINE void default_construct_range([[maybe_unused]] _T* dst, [[maybe_unused]] size_t size) noexcept {
  if constexpr (!std::is_trivially_default_constructible_v<_T>) {
    for (size_t i = 0; i < size; i++) {
      zb_placement_new(dst + i) _T();
    }
  }
}

template <class _T>
ZB_INLINE void copy_construct_range(_T* dst, const _T* src, size_t size) noexcept {
  if constexpr (std::is_trivially_copyable_v<_T>) {
    __zb::memcpy(dst, src, size * sizeof(_T));
  }
  else {
    for (size_t i = 0; i < size; i++) {
      zb_placement_new(dst + i) _T(src[i]);
    }
  }
}

template <class _T>
ZB_INLINE void copy_range(_T* dst, const _T* src, size_t size) noexcept {
  if constexpr (std::is_trivially_copyable_v<_T>) {
    __zb::memcpy(dst, src, size * sizeof(_T));
  }
  else {
    for (size_t i = 0; i < size; i++) {
      dst[i] = src[i];
    }
  }
}

template <class _T>
ZB_INLINE void move_construct_range(_T* dst, _T* src, size_t size) noexcept {
  if constexpr (std::is_trivially_copyable_v<_T>) {
    __zb::memcpy(dst, src, size * sizeof(_T));
  }
  else {
    for (size_t i = 0; i < size; i++) {
      zb_placement_new(dst + i) _T(std::move(src[i]));
    }
  }
}

template <class _T>
ZB_INLINE void move_range(_T* dst, _T* src, size_t size) noexcept {
  if constexpr (std::is_trivially_copyable_v<_T>) {
    __zb::memcpy(dst, src, size * sizeof(_T));
  }
  else {
    for (size_t i = 0; i < size; i++) {
      dst[i] = _T(std::move(src[i]));
    }
  }
}

template <class _T>
ZB_INLINE void destruct_range([[maybe_unused]] _T* dst, [[maybe_unused]] size_t size) noexcept {
  if constexpr (!std::is_trivially_destructible_v<_T>) {
    for (size_t i = 0; i < size; i++) {
      dst[i].~_T();
    }
  }
}

template <class _T, class _U>
inline void copy_construct_element(_T& dst, _U&& src) noexcept {
  if constexpr (std::is_trivially_copyable_v<_T>) {

    if constexpr (__zb::is_trivial_cref_v<_T> || !std::is_same_v<_T, std::remove_cv_t<_U>>) {
      dst = src;
    }
    else {
      __zb::memcpy((void*)&dst, (const void*)&src, sizeof(_T));
    }
  }
  else {
    zb_placement_new(&dst) _T(src);
  }
}

template <class _T, class _U>
inline void move_construct_element(_T& dst, _U&& src) noexcept {
  if constexpr (std::is_trivially_copyable_v<_T>) {

    if constexpr (__zb::is_trivial_cref_v<_T>) {
      dst = src;
    }
    else if constexpr (!std::is_same_v<_T, std::remove_cv_t<_U>>) {
      zb_placement_new(&dst) _T(std::forward<_U>(src));
    }
    else {
      __zb::memcpy((void*)&dst, (const void*)&src, sizeof(_T));
    }
  }
  else {
    zb_placement_new(&dst) _T(std::forward<_U>(src));
  }
}

template <class _T>
ZB_INLINE void copy_element(_T& dst, __zb::cref_t<_T> src) noexcept {
  if constexpr (__zb::is_trivial_cref_v<_T> || !std::is_trivially_copyable_v<_T>) {
    dst = src;
  }
  else {
    __zb::memcpy((void*)&dst, (const void*)&src, sizeof(_T));
  }
}

template <class _T>
ZB_INLINE void move_element(_T& dst, _T&& src) noexcept {
  if constexpr (__zb::is_trivial_cref_v<_T>) {
    dst = src;
  }
  else if constexpr (std::is_trivially_copyable_v<_T>) {
    __zb::memcpy((void*)&dst, (const void*)&src, sizeof(_T));
  }
  else {
    dst = std::move(src);
  }
}

template <class _SrcIterator, class _DstIterator>
inline constexpr _DstIterator swap_ranges(
    const _SrcIterator src_first, const _SrcIterator src_last, _DstIterator dst) noexcept {
  const size_t size = src_last - src_first;
  for (size_t i = 0; i < size; i++) {
    __zb::mem_swap(dst[i], src_first[i]);
  }

  return dst;
}

template <class T>
ZB_CHECK inline constexpr T byte_swap(T value) noexcept {
  static_assert(std::is_trivially_copyable_v<T>, "zb::byte_swap T must be trivially copyable");

  constexpr size_t end_offset = sizeof(T) - 1;
  uint8_t* data = (uint8_t*)&value;
  for (uint8_t *begin = data, *end = data + end_offset; begin < end;) {
    __zb::mem_swap(*begin++, *end--);
  }

  return *(T*)data;
}

ZBASE_END_NAMESPACE
