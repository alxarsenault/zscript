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

#include <zbase/zbase.h>
#include <zbase/container/small_vector.h>
#include <zbase/strings/string_view.h>
#include <zbase/utility/traits.h>
#include <span>
#include <vector>

ZBASE_BEGIN_NAMESPACE

// using byte_view = std::span<const uint8_t>;
using byte_span = std::span<uint8_t>;

class byte_view : public std::span<const uint8_t> {
public:
  using base = std::span<const uint8_t>;
  using base::base;
  using base::value_type;
  using base::size_type;
  using base::difference_type;
  using base::reference;
  using base::const_reference;
  using base::pointer;
  using base::const_pointer;
  using base::iterator;
  using base::reverse_iterator;
  using base::data;
  static constexpr const size_type npos = -1; // size_type(-1);

  template <class... Args>
  inline constexpr byte_view(Args&&... args) noexcept
      : base(std::forward<Args>(args)...) {}

  inline constexpr byte_view(std::string_view s)
      : base((const uint8_t*)s.data(), s.size()) {}

  inline constexpr byte_view& operator<<=(size_t r_offset) noexcept {
    *this = this->subspan(0, r_offset);
    return *this;
  }

  inline constexpr byte_view& operator>>=(size_t l_offset) noexcept {
    *this = this->subspan(l_offset);
    return *this;
  }

  inline constexpr byte_view operator<<(size_t r_offset) const noexcept { return this->subspan(0, r_offset); }

  inline constexpr explicit operator bool() const noexcept { return !empty(); }

  ZB_CHECK ZB_INLINE const_pointer data(size_type index) const noexcept { return data() + index; }

  ZB_CHECK ZB_INLINE const_reference operator()(difference_type n) const noexcept {
    const size_t sz = size();
    zbase_assert(sz, "call operator[] in an empty string_view");
    return operator[]((n + sz) % sz);
  }

  inline constexpr byte_view subspan(size_type pos = 0, size_type __n = npos) const noexcept {
    size_type p = __zb::minimum(pos, size());
    return __zb::byte_view(data() + p, __zb::minimum(__n, size() - p));
  }

  template <class... Cs>
    requires std::is_convertible_v<std::common_type_t<Cs...>, char> and (sizeof...(Cs) > 1)
  inline constexpr bool contains(Cs... cs) const noexcept {
    for (char c : *this) {
      if (__zb::is_one_of(c, cs...)) {
        return true;
      }
    }
    return false;
  }

  template <auto, class... Ts>
  auto ext(Ts...);
};

enum class pcm_type {
  pcm_8_bit,
  pcm_16_bit,
  pcm_24_bit,
  pcm_32_bit,
};

template <class _VectorType>
class basic_byte_vector : public _VectorType {
public:
  static_assert(std::is_same_v<typename _VectorType::value_type, uint8_t>, //
      "zb::basic_byte_vector must have uint8_t as value_type");

  using vector_type = _VectorType;
  using value_type = uint8_t;
  using reference = typename vector_type::reference;
  using const_reference = typename vector_type::const_reference;
  using pointer = typename vector_type::pointer;
  using const_pointer = typename vector_type::const_pointer;
  using iterator = typename vector_type::iterator;
  using const_iterator = typename vector_type::const_iterator;
  using size_type = typename vector_type::size_type;
  using difference_type = typename vector_type::difference_type;

  using vector_type::vector_type;
  ZBASE_DECLARE_DEFAULT_CTOR(basic_byte_vector);

  inline basic_byte_vector(std::string_view str)
      : vector_type((const_pointer)str.data(), (const_pointer)str.data() + str.size()) {}

  using vector_type::data;

  inline pointer data(size_type __index) noexcept { return vector_type::data() + __index; }
  inline const_pointer data(size_type __index) const noexcept { return vector_type::data() + __index; }

  template <typename T>
  inline T* data() noexcept {
    return std::launder(reinterpret_cast<T*>(vector_type::data()));
  }

  template <typename T>
  inline const T* data() const noexcept {
    return std::launder(reinterpret_cast<const T*>(vector_type::data()));
  }

  template <typename T>
  inline T* data(size_type __index) noexcept {
    return std::launder(reinterpret_cast<T*>(vector_type::data() + __index));
  }

  template <typename T>
  inline const T* data(size_type __index) const noexcept {
    return std::launder(reinterpret_cast<const T*>(vector_type::data() + __index));
  }

  inline std::string_view as_string_view() const noexcept {
    return __zb::string_view((const char*)vector_type::data(), vector_type::size());
  }
  inline std::string to_string() const noexcept {
    return std::string((const char*)vector_type::data(), vector_type::size());
  }

  using vector_type::push_back;
  inline void push(value_type v) { vector_type::push_back(v); }

  inline void push_back(std::string_view str) {
    vector_type::insert(vector_type::end(), str.begin(), str.end());
  }

  inline void push_back(const char* str) {
    for (size_type i = 0; i < std::strlen(str); i++) {
      vector_type::push_back(static_cast<value_type>(str[i]));
    }
  }

  template <class _Container, std::enable_if_t<__zb::is_contiguous_container_v<_Container>, int> = 0>
  inline void push_back(const _Container& bvec) {
    // TODO: change this
    for (const auto& n : bvec) {
      push_back(n);
    }
  }

  template <typename T, bool _IsLittleEndian = true,
      std::enable_if_t<!__zb::is_contiguous_container_v<T>, int> = 0>
  inline void push_back(const T& value) {
    const value_type* other_data = reinterpret_cast<const value_type*>(&value);

    if constexpr (__zb::is_iterable_container_v<T>) {
      for (const auto& n : value) {
        push_back<std::remove_cvref_t<decltype(n)>, _IsLittleEndian>(n);
      }
    }
    else {
      static_assert(std::is_trivially_copyable<T>::value, "Type cannot be serialized.");
      if constexpr (_IsLittleEndian) {
        for (size_t i = 0; i < sizeof(T); i++) {
          vector_type::push_back(other_data[i]);
        }
      }
      else {
        for (int i = sizeof(T) - 1; i >= 0; i--) {
          vector_type::push_back(other_data[i]);
        }
      }
    }
  }

  template <typename T, bool _IsLittleEndian = true>
  inline void push_back(const T* in_data, size_type sz) {
    static_assert(std::is_trivially_copyable<T>::value, "Type cannot be serialized.");

    if constexpr (_IsLittleEndian) {
      size_type last_index = vector_type::size();
      size_type byte_size = sz * sizeof(T);
      vector_type::resize(vector_type::size() + byte_size);
      ::memmove(&vector_type::operator[](last_index), in_data, byte_size);
    }
    else {
      for (size_type i = 0; i < sz; i++) {
        push_back<T, _IsLittleEndian>(in_data[i]);
      }
    }
  }

  template <typename T, pcm_type c_opts>
  inline void push_back(const T& value) {
    static_assert(std::is_floating_point<T>::value, "Type must be a floating point.");

    if constexpr (c_opts == pcm_type::pcm_8_bit) {
      T s = std::clamp<T>(value, (T)-1.0, (T)1.0);
      s = (s + (T)1.0) / (T)2.0;
      push_back(static_cast<value_type>(s * (T)255.0));
    }
    else if constexpr (c_opts == pcm_type::pcm_16_bit) {
      // constexpr T mult = (1 << 15) - 1;
      constexpr T mult = (1 << 15);
      const T s = std::clamp<T>(value, (T)-1.0, (T)1.0);
      push_back<int16_t>(static_cast<int16_t>(s * mult));
    }
    else if constexpr (c_opts == pcm_type::pcm_24_bit) {
      int32_t s_int = static_cast<int32_t>(value * 8388608.0);
      push_back(static_cast<value_type>(s_int & 0xFF));
      push_back(static_cast<value_type>((s_int >> 8) & 0xFF));
      push_back(static_cast<value_type>((s_int >> 16) & 0xFF));
    }
    else if constexpr (c_opts == pcm_type::pcm_32_bit) {
      constexpr T mult_tmp = 1L << 31L;
      constexpr T mult = mult_tmp; // - 1;
      const T s = std::clamp<T>(value, (T)-1.0, (T)1.0);
      push_back<int32_t>(static_cast<int32_t>(s * mult));
    }
  }

  inline void push_padding(size_type count) {
    for (size_type i = 0; i < count; i++) {
      push_back((value_type)0);
    }
  }

  inline size_type push_padding() {
    if (size_type padding = vector_type::size() % 4) {
      push_padding(4 - padding);
      return 4 - padding;
    }

    return 0;
  }

  template <typename T>
  inline T& as_ref_offset(size_type __index) noexcept {
    static_assert(std::is_trivially_copyable<T>::value, "Type cannot be serialized.");
    return *data<T>(__index);
  }

  template <typename T>
  inline const T& as_ref_offset(size_type __index) const noexcept {
    static_assert(std::is_trivially_copyable<T>::value, "Type cannot be serialized.");
    return *data<T>(__index);
  }

  template <typename T, class... _Args>
  inline T as_type_offset() const noexcept {
    return as_offset<T>((sizeof(_Args) + ... + 0));
  }

  template <typename T, bool _IsLittleEndian = true>
  inline T as_offset(size_type __index) const noexcept {
    static_assert(std::is_trivially_copyable<T>::value, "Type cannot be serialized.");

    if constexpr (_IsLittleEndian) {
      if constexpr (sizeof(T) <= 8) {
        T value;
        pointer data = reinterpret_cast<pointer>(&value);
        for (size_type i = __index, j = 0; i < __index + sizeof(T); i++, j++) {
          data[j] = vector_type::operator[](i);
        }
        return value;
      }
      else {
        T value;
        pointer value_data = reinterpret_cast<pointer>(&value);
        ::memmove(value_data, data<T>(__index), sizeof(T));
        return value;
      }
    }
    else {
      T value;
      pointer data = reinterpret_cast<pointer>(&value);
      for (size_type i = __index, j = sizeof(T) - 1; i < __index + sizeof(T); i++, j--) {
        data[j] = vector_type::operator[](i);
      }
      return value;
    }
  }

  template <typename T, bool _IsLittleEndian = true>
  inline T as(iterator pos) const noexcept {
    static_assert(std::is_trivially_copyable<T>::value, "Type cannot be serialized.");
    difference_type index = std::distance(pos, vector_type::begin());
    zbase_assert(index >= 0, "Wrong iterator position.");
    return as_offset<T, _IsLittleEndian>((size_type)index);
  }

  // Get array element at array_index from array starting at index.
  template <typename T, bool _IsLittleEndian = true>
  inline T as_offset(size_type index, size_type array_index) const noexcept {
    static_assert(std::is_trivially_copyable<T>::value, "Type cannot be serialized.");
    return as_offset<T, _IsLittleEndian>(index + array_index * sizeof(T));
  }

  // Get array element at array_index from array starting at pos.
  template <typename T, bool _IsLittleEndian = true>
  inline T as(iterator pos, size_type array_index) const noexcept {
    static_assert(std::is_trivially_copyable<T>::value, "Type cannot be serialized.");
    difference_type index = std::distance(pos, vector_type::begin());
    zbase_assert(index >= 0, "Wrong iterator position.");
    return as_offset<T, _IsLittleEndian>((size_type)index, array_index);
  }

  template <typename T, pcm_type c_opts>
  inline T as(size_type __index) const noexcept {
    static_assert(std::is_floating_point<T>::value, "Type must be a floating point.");

    if constexpr (c_opts == pcm_type::pcm_8_bit) {
      constexpr T div = 1 << 7;
      return static_cast<T>(int(vector_type::operator[](__index)) - 128) / div;
    }
    else if constexpr (c_opts == pcm_type::pcm_16_bit) {
      constexpr T denom = T(1.0) / (T)(1 << 15);
      return (T)as_offset<int16_t>(__index) * denom;
    }
    else if constexpr (c_opts == pcm_type::pcm_24_bit) {
      constexpr T denom = 1.0 / (T)8388608.0;
      int32_t value = (static_cast<int32_t>(vector_type::operator[](__index + 2)) << 16)
          | (static_cast<int32_t>(vector_type::operator[](__index + 1)) << 8)
          | static_cast<int32_t>(vector_type::operator[](__index));

      // If the 24th bit is set, this is a negative number in 24-bit world.
      // Make sure sign is extended to the 32 bit float.
      return (value & 0x800000 ? (value | ~0xFFFFFF) : value) * denom;
    }
    else if constexpr (c_opts == pcm_type::pcm_32_bit) {
      constexpr T div = 1L << 31L;
      return as_offset<int32_t>(__index) / div;
    }
  }

  template <typename T, bool _IsLittleEndian = true>
  inline void copy_as(T* buffer, size_type index, size_type array_size) const noexcept {
    static_assert(std::is_trivially_copyable<T>::value, "Type cannot be serialized.");

    if constexpr (_IsLittleEndian) {
      ::memmove(buffer, data<T>(index), array_size * sizeof(T));
    }
    else {
      for (size_type i = 0; i < array_size; i++) {
        buffer[i] = as_offset<T, _IsLittleEndian>(index, i);
      }
    }
  }

  inline iterator find(const_pointer data, size_type size) noexcept {
    return std::search(vector_type::begin(), vector_type::end(), data, data + size);
  }

  inline const_iterator find(const_pointer data, size_type size) const noexcept {
    return std::search(vector_type::cbegin(), vector_type::cend(), data, data + size);
  }

  template <class T>
  inline iterator find(std::span<const T> data) noexcept {
    return std::search(vector_type::begin(), vector_type::end(), static_cast<const_pointer>(data.data()),
        static_cast<const_pointer>(data.data()) + data.size_bytes());
  }

  template <class T>
  inline const_iterator find(std::span<const T> data) const noexcept {
    return std::search(vector_type::cbegin(), vector_type::cend(), static_cast<const_pointer>(data.data()),
        static_cast<const_pointer>(data.data()) + data.size_bytes());
  }

  inline iterator find(size_type offset, const_pointer data, size_type size) noexcept {
    return std::search(vector_type::begin() + offset, vector_type::end(), data, data + size);
  }

  inline const_iterator find(size_type offset, const_pointer data, size_type size) const noexcept {
    return std::search(vector_type::cbegin() + offset, vector_type::cend(), data, data + size);
  }

  template <class T>
  inline iterator find(size_type offset, std::span<const T> data) noexcept {
    return std::search(vector_type::begin() + offset, vector_type::end(),
        static_cast<const_pointer>(data.data()), static_cast<const_pointer>(data.data()) + data.size_bytes());
  }

  template <class T>
  inline const_iterator find(size_type offset, std::span<const T> data) noexcept {
    return std::search(vector_type::cbegin() + offset, vector_type::cend(),
        static_cast<const_pointer>(data.data()), static_cast<const_pointer>(data.data()) + data.size_bytes());
  }
};

//

template <class _Allocator>
using byte_vector_t = __zb::basic_byte_vector<std::vector<uint8_t, _Allocator>>;

using byte_vector = __zb::basic_byte_vector<std::vector<uint8_t>>;

template <size_t _Size>
using small_byte_vector = __zb::basic_byte_vector<__zb::small_vector<uint8_t, _Size>>;

ZBASE_END_NAMESPACE
