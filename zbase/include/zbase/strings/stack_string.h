// Copyright(c) 2024, Meta-Sonic.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.  See the file COPYING included with
// this distribution for more information.
//
// Alternatively, if you have a valid commercial licence for aulib obtained
// by agreement with the copyright holders, you may redistribute and/or modify
// it under the terms described in that licence.
//
// If you wish to distribute code using aulib under terms other than those of
// the GNU General Public License, you must obtain a valid commercial licence
// before doing so.

#pragma once

#include <zbase/zbase.h>
#include <zbase/sys/assert.h>
#include <zbase/memory/memory.h>
#include <zbase/utility/traits.h>
#include <string>
#include <string_view>

ZBASE_BEGIN_NAMESPACE

template <typename _CharT, size_t _Size>
class basic_stack_string {
public:
  using __self = basic_stack_string;
  using view_type = std::basic_string_view<_CharT>;
  using string_type = std::basic_string<_CharT>;
  using traits_type = std::char_traits<_CharT>;
  using value_type = _CharT;
  using size_type = size_t;
  using difference_type = ::ptrdiff_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = pointer;
  using const_iterator = const_pointer;

  static constexpr size_type maximum_size = _Size;
  static constexpr size_type npos = (std::numeric_limits<size_type>::max)();

  static_assert(!std::is_array_v<value_type>, "Character type of basic_stack_string must not be an array.");
  static_assert(std::is_trivial<value_type>::value, "Character type of basic_stack_string must be trivial.");
  static_assert(std::is_same<value_type, typename traits_type::char_type>::value,
      "traits_type::char_type must be the same type as value_type.");

  constexpr basic_stack_string() noexcept = default;

  inline constexpr basic_stack_string(size_type count, value_type ch) noexcept {
    zbase_assert(count <= maximum_size,
        "basic_stack_string count must be smaller or equal to "
        "maximum_size.");
    ::memset(_data.data(), ch, count * sizeof(value_type));

    _size = count;
    _data[_size] = 0;
  }

  inline constexpr basic_stack_string(const basic_stack_string& other) noexcept
      : _data(other._data)
      , _size(other._size) {}

  inline constexpr basic_stack_string(basic_stack_string&& other) noexcept
      : _data(std::move(other._data))
      , _size(other._size) {

    other._size = 0;
    other._data[0] = 0;
  }

  inline constexpr basic_stack_string(
      const basic_stack_string& other, size_type pos, size_type count = npos) noexcept {
    zbase_assert(pos <= other._size, "basic_stack_string pos must be smaller or equal to size.");
    _size = __zb::minimum(count, other._size - pos);

    ::memcpy(_data.data(), other.data() + pos, _size * sizeof(value_type));

    _data[_size] = 0;
  }

  template <size_t _OtherSize>
  inline constexpr basic_stack_string(const basic_stack_string<value_type, _OtherSize>& other) noexcept {
    zbase_assert(other.size() <= maximum_size,
        "basic_stack_string count must be smaller or equal to "
        "maximum_size.");

    _size = other.size();
    ::memcpy(_data.data(), other.data(), _size * sizeof(value_type));
    _data[_size] = 0;
  }

  template <size_t _OtherSize>
  inline constexpr basic_stack_string(const basic_stack_string<value_type, _OtherSize>& other, size_type pos,
      size_type count = npos) noexcept {
    zbase_assert(other.size() <= maximum_size,
        "basic_stack_string size must be smaller or equal to "
        "maximum_size.");
    zbase_assert(pos <= other.size(), "basic_stack_string pos must be smaller or equal to size.");
    _size = __zb::minimum(count, other.size() - pos);

    ::memcpy(_data.data(), other.data() + pos, _size * sizeof(value_type));

    _data[_size] = 0;
  }

  inline constexpr basic_stack_string(const value_type* s) noexcept {
    _size = c_strlen(s);
    zbase_assert(_size <= maximum_size,
        "basic_stack_string c string must be smaller or equal to "
        "maximum_size.");
    ::memcpy(_data.data(), s, _size * sizeof(value_type));
    _data[_size] = 0;
  }

  inline constexpr basic_stack_string(const value_type* s, size_type count) noexcept {
    zbase_assert(count <= maximum_size, "basic_stack_string count must be smaller or equal to maximum_size.");
    zbase_assert(count <= c_strlen(s), "basic_stack_string count must be smaller or equal to c string size.");
    _size = count;
    ::memcpy(_data.data(), s, _size * sizeof(value_type));

    _data[_size] = 0;
  }

  template <class InputIt>
  inline constexpr basic_stack_string(InputIt first, InputIt last) noexcept {
    _size = last - first;
    zbase_assert(_size <= maximum_size,
        "basic_stack_string iteration distance must be smaller or "
        "equal to maximum_size.");

    ::memcpy(_data.data(), first, _size * sizeof(value_type));
    _data[_size] = 0;
  }

  inline constexpr basic_stack_string(view_type v) noexcept {
    zbase_assert(
        v.size() <= maximum_size, "basic_stack_string view size must be smaller or equal to maximum_size.");
    _size = v.size();
    ::memcpy(_data.data(), v.data(), _size * sizeof(value_type));

    _data[_size] = 0;
  }

  inline constexpr basic_stack_string(view_type v, size_type pos, size_type count = npos) noexcept {
    zbase_assert(pos <= v.size(), "basic_stack_string pos must be smaller or equal to view size.");
    _size = __zb::minimum(count, v.size() - pos);
    zbase_assert(_size <= maximum_size, "basic_stack_string size must be smaller or equal to maximum_size.");

    ::memcpy(_data.data(), v.data() + pos, _size * sizeof(value_type));

    _data[_size] = 0;
  }

  inline constexpr basic_stack_string(const string_type& s) noexcept
      : basic_stack_string(view_type(s)) {}

  inline constexpr basic_stack_string(const string_type& s, size_type pos, size_type count = npos) noexcept
      : basic_stack_string(view_type(s), pos, count) {}

  inline constexpr basic_stack_string& operator=(const basic_stack_string& other) noexcept {
    _data = other._data;
    _size = other._size;
    return *this;
  }

  inline constexpr basic_stack_string& operator=(basic_stack_string&& other) noexcept {
    _data = std::move(other._data);
    _size = other._size;
    other._size = 0;
    return *this;
  }

  inline constexpr basic_stack_string& operator=(view_type v) noexcept {
    zbase_assert(
        v.size() <= maximum_size, "basic_stack_string view size must be smaller or equal to maximum_size.");
    _size = v.size();
    ::memcpy(_data.data(), v.data(), _size * sizeof(value_type));

    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& operator=(const string_type& s) noexcept {
    return operator=(view_type(s));
  }

  inline constexpr basic_stack_string& operator=(const value_type* s) noexcept {
    _size = c_strlen(s);
    zbase_assert(
        _size <= maximum_size, "basic_stack_string c string must be smaller or equal to maximum_size.");

    ::memcpy(_data.data(), s, _size * sizeof(value_type));

    _data[_size] = 0;
    return *this;
  }

  // Iterators.
  inline constexpr iterator begin() noexcept { return iterator(_data.data()); }
  inline constexpr const_iterator begin() const noexcept { return const_iterator(_data.data()); }

  inline constexpr iterator end() noexcept { return iterator(_data.data() + _size); }
  inline constexpr const_iterator end() const noexcept { return const_iterator(_data.data() + _size); }

  inline constexpr const_iterator cbegin() const noexcept { return begin(); }
  inline constexpr const_iterator cend() const noexcept { return end(); }

  // Capacity.
  [[nodiscard]] inline constexpr size_type size() const noexcept { return _size; }
  [[nodiscard]] inline constexpr size_type length() const noexcept { return _size; }
  [[nodiscard]] inline constexpr size_type max_size() const noexcept { return maximum_size; }
  [[nodiscard]] inline constexpr size_type capacity() const noexcept { return maximum_size; }
  [[nodiscard]] inline constexpr bool empty() const noexcept { return _size == 0; }

  // Element access.
  inline constexpr reference operator[](size_type n) noexcept {
    zbase_assert(n < maximum_size, "Index out of bounds");
    return _data[n];
  }

  inline constexpr const_reference operator[](size_type n) const noexcept {
    zbase_assert(n < maximum_size, "Index out of bounds");
    return _data[n];
  }

  inline constexpr reference front() noexcept {
    zbase_assert(_size > 0, "basic_stack_string::front when empty.");
    return _data[0];
  }

  inline constexpr const_reference front() const noexcept {
    zbase_assert(_size > 0, "basic_stack_string::front when empty.");
    return _data[0];
  }

  inline constexpr reference back() noexcept {
    zbase_assert(_size > 0, "basic_stack_string::back when empty.");
    return _data[_size - 1];
  }

  inline constexpr const_reference back() const noexcept {
    zbase_assert(_size > 0, "basic_stack_string::back when empty.");
    return _data[_size - 1];
  }

  inline constexpr pointer data() noexcept { return _data.data(); }
  inline constexpr const_pointer data() const noexcept { return _data.data(); }

  inline constexpr const_pointer c_str() const noexcept { return _data.data(); }

  //
  // Operations.
  //

  inline constexpr void clear() noexcept {
    _data[0] = 0;
    _size = 0;
  }

  inline constexpr void push_back(value_type c) noexcept {
    zbase_assert(_size + 1 <= maximum_size,
        "basic_stack_string::push_back size would end up greather than maximum_size.");
    _data[_size++] = c;
    _data[_size] = 0;
  }

  inline constexpr void pop_back() noexcept {
    zbase_assert(_size, "basic_stack_string::pop_back when empty.");
    _size--;
    _data[_size] = 0;
  }

  //
  // Append.
  //

  inline constexpr bool is_appendable(const basic_stack_string& other) const noexcept {
    return _size + other.size() <= maximum_size;
  }

  inline constexpr bool is_appendable(
      const basic_stack_string& other, size_type pos, size_type count = npos) const noexcept {
    return (pos <= other.size()) && (_size + __zb::minimum(count, other.size() - pos) <= maximum_size);
  }

  inline constexpr bool is_appendable(size_type count) const noexcept {
    return _size + count <= maximum_size;
  }

  inline constexpr bool is_appendable(view_type v) const noexcept { return _size + v.size() <= maximum_size; }

  inline constexpr bool is_appendable(view_type v, size_type pos, size_type count = npos) const noexcept {
    return (pos <= v.size()) && (_size + __zb::minimum(count, v.size() - pos) <= maximum_size);
  }

  inline constexpr basic_stack_string& append(value_type c) noexcept {
    zbase_assert(_size + 1 <= maximum_size,
        "basic_stack_string::push_back size would end up greather than maximum_size.");
    _data[_size++] = c;
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& append(size_type count, value_type c) noexcept {
    zbase_assert(_size + count <= maximum_size,
        "basic_stack_string::append size would end up greather than maximum_size.");

    __zb::mem_fill(_data.data() + _size, c, count);
    _size += count;
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& append(const basic_stack_string& other) noexcept {
    zbase_assert(_size + other.size() <= maximum_size,
        "basic_stack_string::append size would end up greather than maximum_size.");

    ::memcpy(_data.data() + _size, other.data(), other.size() * sizeof(value_type));

    _size += other.size();
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& append(
      const basic_stack_string& other, size_type pos, size_type count = npos) noexcept {
    zbase_assert(pos <= other.size(), "basic_stack_string pos must be smaller or equal to string size.");
    size_type o_size = __zb::minimum(count, other.size() - pos);
    zbase_assert(_size + o_size <= maximum_size,
        "basic_stack_string::append size would end up greather than maximum_size.");

    ::memcpy(_data.data() + _size, other.data() + pos, o_size * sizeof(value_type));

    _size += o_size;
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& append(view_type v) noexcept {
    zbase_assert(_size + v.size() <= maximum_size,
        "basic_stack_string::append size would end up greather than "
        "maximum_size.");

    ::memcpy(_data.data() + _size, v.data(), v.size() * sizeof(value_type));

    _size += v.size();
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& append(view_type v, size_type pos, size_type count = npos) noexcept {
    zbase_assert(pos <= v.size(),
        "basic_stack_string pos must be smaller or equal to view "
        "size.");
    size_type o_size = __zb::minimum(count, v.size() - pos);
    zbase_assert(_size + o_size <= maximum_size,
        "basic_stack_string::append size would end up greather than "
        "maximum_size.");

    ::memcpy(_data.data() + _size, v.data() + pos, o_size * sizeof(value_type));

    _size += o_size;
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& append(const value_type* s) noexcept { return append(view_type(s)); }

  inline constexpr basic_stack_string& append(
      const value_type* s, size_type pos, size_type count = npos) noexcept {
    return append(view_type(s), pos, count);
  }

  inline constexpr basic_stack_string& append(const string_type& s) noexcept { return append(view_type(s)); }

  inline constexpr basic_stack_string& append(
      const string_type& s, size_type pos, size_type count = npos) noexcept {
    return append(view_type(s), pos, count);
  }

  template <class InputIt>
  constexpr basic_stack_string& append(InputIt first, InputIt last) noexcept {
    return append(view_type(first, last));
  }

  inline constexpr basic_stack_string& operator+=(const basic_stack_string& other) noexcept {
    return append(other);
  }

  inline constexpr basic_stack_string& operator+=(view_type v) noexcept { return append(v); }

  inline constexpr basic_stack_string& operator+=(const value_type* s) noexcept {
    return append(view_type(s));
  }

  inline constexpr basic_stack_string& clipped_append(view_type v) noexcept {

    if (size_t msize = zb::minimum(maximum_size - _size, v.size())) {
      ::memcpy(_data.data() + _size, v.data(), msize * sizeof(value_type));
      _size += msize;
      _data[_size] = 0;
    }

    return *this;
  }

  //
  // Insert.
  //
  inline constexpr basic_stack_string& insert(size_type index, size_type count, value_type c) {
    zbase_assert(index <= _size, "basic_stack_string::insert index out of bounds.");
    zbase_assert(count + _size <= maximum_size,
        "basic_stack_string::insert size would end up greather than "
        "maximum_size.");
    size_type delta = _size - index;
    ::memmove((void*)(_data.data() + index + count), (const void*)(_data.data() + index),
        delta * sizeof(value_type));
    // std::fill_n(_data.data() + index, count, c);
    __zb::mem_fill(_data.data() + index, c, count);
    _size += count;
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& insert(size_type index, view_type v) {
    zbase_assert(index <= _size, "basic_stack_string::insert index out of bounds.");
    zbase_assert(v.size() + _size <= maximum_size,
        "basic_stack_string::insert size would end up greather than "
        "maximum_size.");
    size_type delta = _size - index;
    ::memmove((void*)(_data.data() + index + v.size()), (const void*)(_data.data() + index),
        delta * sizeof(value_type));

    ::memcpy(_data.data() + index, v.data(), v.size() * sizeof(value_type));

    _size += v.size();
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& insert(size_type index, view_type v, size_type count) {
    zbase_assert(index <= _size, "basic_stack_string::insert index out of bounds.");
    zbase_assert(count <= v.size(), "basic_stack_string::insert count out of bounds.");
    zbase_assert(count + _size <= maximum_size,
        "basic_stack_string::insert size would end up greather than "
        "maximum_size.");
    size_type delta = _size - index;
    ::memmove((void*)(_data.data() + index + count), (const void*)(_data.data() + index),
        delta * sizeof(value_type));

    ::memcpy(_data.data() + index, v.data(), count * sizeof(value_type));

    _size += count;
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& insert(
      size_type index, view_type v, size_type index_str, size_type count) {
    zbase_assert(index <= _size, "basic_stack_string::insert index out of bounds.");
    zbase_assert(index_str <= v.size(), "basic_stack_string::insert index_str out of bounds.");
    size_type s_size = __zb::minimum(count, v.size() - index_str);
    zbase_assert(s_size + _size <= maximum_size,
        "basic_stack_string::insert size would end up greather than "
        "maximum_size.");

    size_type delta = _size - index;
    ::memmove((void*)(_data.data() + index + s_size), (const void*)(_data.data() + index),
        delta * sizeof(value_type));

    ::memcpy(_data.data() + index, v.data() + index_str, s_size * sizeof(value_type));

    _size += s_size;
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& insert(size_type index, const value_type* s) {
    return insert(index, view_type(s));
  }

  inline constexpr basic_stack_string& insert(size_type index, const value_type* s, size_type count) {
    return insert(index, view_type(s), count);
  }

  inline constexpr basic_stack_string& insert(size_type index, const basic_stack_string& str) {
    zbase_assert(index <= _size, "basic_stack_string::insert index out of bounds.");
    zbase_assert(str.size() + _size <= maximum_size,
        "basic_stack_string::insert size would end up greather than "
        "maximum_size.");
    size_type delta = _size - index;
    ::memmove((void*)(_data.data() + index + str.size()), (const void*)(_data.data() + index),
        delta * sizeof(value_type));

    ::memcpy(_data.data() + index, str.data(), str.size() * sizeof(value_type));

    _size += str.size();
    _data[_size] = 0;
    return *this;
  }

  inline constexpr basic_stack_string& insert(
      size_type index, const basic_stack_string& str, size_type index_str, size_type count = npos) {
    zbase_assert(index <= _size, "basic_stack_string::insert index out of bounds.");
    zbase_assert(index_str <= str.size(), "basic_stack_string::insert index_str out of bounds.");
    size_type s_size = __zb::minimum(count, str.size() - index_str);
    zbase_assert(s_size + _size <= maximum_size,
        "basic_stack_string::insert size would end up greather than "
        "maximum_size.");

    size_type delta = _size - index;
    ::memmove((void*)(_data.data() + index + s_size), (const void*)(_data.data() + index),
        delta * sizeof(value_type));

    ::memcpy(_data.data() + index, str.data() + index_str, s_size * sizeof(value_type));

    _size += s_size;
    _data[_size] = 0;
    return *this;
  }

  //
  //
  //
  inline constexpr basic_stack_string& erase(size_type index = 0, size_type count = npos) {
    zbase_assert(index <= _size, "basic_stack_string::insert index out of bounds.");
    size_type s_size = __zb::minimum(count, _size - index);
    size_type delta = _size - s_size;
    ::memmove((void*)(_data.data() + index), (const void*)(_data.data() + index + s_size),
        delta * sizeof(value_type));
    _size -= s_size;
    _data[_size] = 0;
    return *this;
  }

  //
  // Resize.
  //
  inline constexpr void resize(size_type count, value_type c) {
    zbase_assert(count <= maximum_size,
        "basic_stack_string::resize count must be smaller or equal to "
        "maximum_size.");
    if (count < _size) {
      _size = count;
      _data[_size] = 0;
    }
    else if (count > _size) {
      __zb::mem_fill(_data.data() + _size, c, (count - _size));
      _size = count;
      _data[_size] = 0;
    }
  }
  inline constexpr void resize(size_type count) {
    zbase_assert(count <= maximum_size,
        "basic_stack_string::resize count must be smaller or equal to "
        "maximum_size.");
    if (count < _size) {
      _size = count;
      _data[_size] = 0;
    }
    else if (count > _size) {
      _size = count;
      _data[_size] = 0;
    }
  }

  //
  //
  //
  inline constexpr basic_stack_string& to_upper_case() {
    for (size_t i = 0; i < _size; i++) {
      _data[i] = (value_type)::toupper(_data[i]);
    }
    return *this;
  }

  inline constexpr basic_stack_string& to_lower_case() {
    for (size_t i = 0; i < _size; i++) {
      _data[i] = (value_type)::tolower(_data[i]);
    }

    return *this;
  }

  //
  // Convert.
  //
  inline string_type to_string() const { return string_type(data(), size()); }
  inline operator string_type() const { return to_string(); }

  inline constexpr view_type view() const { return view_type(data(), size()); }
  inline constexpr operator view_type() const { return view(); }

  template <class T, size_t N>
  friend inline std::ostream& operator<<(std::ostream& s, const __zb::basic_stack_string<T, N>& str) {
    return s << str.view();
  }

  template <auto, class... Ts>
  auto ext(Ts...);

private:
  static constexpr size_type maximum_size_with_escape_char = maximum_size + 1;
  std::array<value_type, maximum_size_with_escape_char> _data;
  size_t _size = 0;

  inline constexpr size_type c_strlen(const value_type* str) noexcept {
    return *str ? 1 + c_strlen(str + 1) : 0;
  }
};

//
// Operator ==
//
template <class _CharT, size_t _Size>
inline bool operator==(
    const basic_stack_string<_CharT, _Size>& __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  using view_type = typename basic_stack_string<_CharT, _Size>::view_type;
  return (__lhs.size() == __rhs.size()) && (view_type(__lhs) == view_type(__rhs));
}

template <class _CharT, size_t _LSize, size_t _RSize>
inline bool operator==(const basic_stack_string<_CharT, _LSize>& __lhs,
    const basic_stack_string<_CharT, _RSize>& __rhs) noexcept {
  using view_type = typename basic_stack_string<_CharT, _LSize>::view_type;
  return (__lhs.size() == __rhs.size()) && (view_type(__lhs) == view_type(__rhs));
}

template <class _CharT, size_t _Size>
inline bool operator==(const basic_stack_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
  using view_type = typename basic_stack_string<_CharT, _Size>::view_type;
  return view_type(__lhs) == view_type(__rhs);
}

template <class _CharT, size_t _Size>
inline bool operator==(const _CharT* __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  using view_type = typename basic_stack_string<_CharT, _Size>::view_type;
  return view_type(__lhs) == view_type(__rhs);
}

template <class _CharT, size_t _Size>
inline bool operator==(const basic_stack_string<_CharT, _Size>& __lhs,
    const typename basic_stack_string<_CharT, _Size>::string_type& __rhs) noexcept {
  using view_type = typename basic_stack_string<_CharT, _Size>::view_type;
  return view_type(__lhs) == view_type(__rhs);
}

template <class _CharT, size_t _Size>
inline bool operator==(const typename basic_stack_string<_CharT, _Size>::string_type& __lhs,
    const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  using view_type = typename basic_stack_string<_CharT, _Size>::view_type;
  return view_type(__lhs) == view_type(__rhs);
}

//
// Operator !=
//
template <class _CharT, size_t _LSize, size_t _RSize>
inline bool operator!=(const basic_stack_string<_CharT, _LSize>& __lhs,
    const basic_stack_string<_CharT, _RSize>& __rhs) noexcept {
  return !(__lhs == __rhs);
}

template <class _CharT, size_t _Size>
inline bool operator!=(const basic_stack_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
  return !(__lhs == __rhs);
}

template <class _CharT, size_t _Size>
inline bool operator!=(const _CharT* __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  return !(__lhs == __rhs);
}

//
// Operator <
//
template <class _CharT, size_t _LSize, size_t _RSize>
inline bool operator<(const basic_stack_string<_CharT, _LSize>& __lhs,
    const basic_stack_string<_CharT, _RSize>& __rhs) noexcept {
  using view_type = typename basic_stack_string<_CharT, _LSize>::view_type;
  return view_type(__lhs).compare(view_type(__rhs)) < 0;
}

template <class _CharT, size_t _Size>
inline bool operator<(const basic_stack_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
  using view_type = typename basic_stack_string<_CharT, _Size>::view_type;
  return view_type(__lhs).compare(view_type(__rhs)) < 0;
}

template <class _CharT, size_t _Size>
inline bool operator<(const _CharT* __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  using view_type = typename basic_stack_string<_CharT, _Size>::view_type;
  return view_type(__lhs).compare(view_type(__rhs)) < 0;
}

//
// Operator >
//
template <class _CharT, size_t _LSize, size_t _RSize>
inline bool operator>(const basic_stack_string<_CharT, _LSize>& __lhs,
    const basic_stack_string<_CharT, _RSize>& __rhs) noexcept {
  return __rhs < __lhs;
}

template <class _CharT, size_t _Size>
inline bool operator>(const basic_stack_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
  return __rhs < __lhs;
}

template <class _CharT, size_t _Size>
inline bool operator>(const _CharT* __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  return __rhs < __lhs;
}

//
// Operator <=
//
template <class _CharT, size_t _LSize, size_t _RSize>
inline bool operator<=(const basic_stack_string<_CharT, _LSize>& __lhs,
    const basic_stack_string<_CharT, _RSize>& __rhs) noexcept {
  return !(__rhs < __lhs);
}

template <class _CharT, size_t _Size>
inline bool operator<=(const basic_stack_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
  return !(__rhs < __lhs);
}

template <class _CharT, size_t _Size>
inline bool operator<=(const _CharT* __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  return !(__rhs < __lhs);
}

//
// Operator >=
//
template <class _CharT, size_t _LSize, size_t _RSize>
inline bool operator>=(const basic_stack_string<_CharT, _LSize>& __lhs,
    const basic_stack_string<_CharT, _RSize>& __rhs) noexcept {
  return !(__lhs < __rhs);
}

template <class _CharT, size_t _Size>
inline bool operator>=(const basic_stack_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
  return !(__lhs < __rhs);
}

template <class _CharT, size_t _Size>
inline bool operator>=(const _CharT* __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  return !(__lhs < __rhs);
}

// operator +
template <class _CharT, size_t _LSize, size_t _RSize>
inline basic_stack_string<_CharT, _LSize> operator+(const basic_stack_string<_CharT, _LSize>& __lhs,
    const basic_stack_string<_CharT, _RSize>& __rhs) noexcept {
  basic_stack_string<_CharT, _LSize> s = __lhs;
  s += __rhs;
  return s;
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    _CharT __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  basic_stack_string<_CharT, _Size> s;
  s.push_back(__lhs);
  s.append(__rhs);
  return s;
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    const basic_stack_string<_CharT, _Size>& __lhs, _CharT __rhs) noexcept {
  basic_stack_string<_CharT, _Size> s = __lhs;
  s.push_back(__rhs);
  return s;
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    const _CharT* __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  basic_stack_string<_CharT, _Size> s = __lhs;
  s += __rhs;
  return s;
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    const basic_stack_string<_CharT, _Size>& __lhs, const _CharT* __rhs) noexcept {
  basic_stack_string<_CharT, _Size> s = __lhs;
  s += __rhs;
  return s;
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    basic_stack_string<_CharT, _Size>&& __lhs, const basic_stack_string<_CharT, _Size>& __rhs) noexcept {
  return std::move(__lhs.append(__rhs));
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    const basic_stack_string<_CharT, _Size>& __lhs, basic_stack_string<_CharT, _Size>&& __rhs) noexcept {
  return std::move(__rhs.insert(0, __lhs));
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    basic_stack_string<_CharT, _Size>&& __lhs, basic_stack_string<_CharT, _Size>&& __rhs) noexcept {
  return std::move(__lhs.append(__rhs));
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    basic_stack_string<_CharT, _Size>&& __lhs, const _CharT* __rhs) noexcept {
  return std::move(__lhs.append(__rhs));
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    const _CharT* __lhs, basic_stack_string<_CharT, _Size>&& __rhs) noexcept {
  return std::move(__rhs.insert(0, __lhs));
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    _CharT __lhs, basic_stack_string<_CharT, _Size>&& __rhs) noexcept {
  __rhs.insert(__rhs.begin(), __lhs);
  return std::move(__rhs);
}

template <class _CharT, size_t _Size>
inline basic_stack_string<_CharT, _Size> operator+(
    basic_stack_string<_CharT, _Size>&& __lhs, _CharT __rhs) noexcept {
  __lhs.push_back(__rhs);
  return std::move(__lhs);
}

template <size_t N>
using stack_string = basic_stack_string<char, N>;

ZBASE_END_NAMESPACE
