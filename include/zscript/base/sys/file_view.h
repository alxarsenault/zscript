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
#include <zscript/base/sys/error_code.h>
#include <span>
#include <string>
#include <string_view>
#include <assert.h>

ZBASE_BEGIN_NAMESPACE

class file_view {
public:
  using value_type = uint8_t;
  using pointer = value_type*;
  using const_reference = value_type;
  using const_pointer = const value_type*;
  using iterator = const_pointer;
  using const_iterator = const_pointer;
  using size_type = size_t;

  inline file_view() noexcept = default;

  file_view(const file_view&) = delete;

  inline file_view(file_view&& fb) noexcept
      : _data(std::exchange(fb._data, nullptr))
      , _size(std::exchange(fb._size, (size_type)0)) {}

  inline ~file_view() noexcept { close(); }

  file_view& operator=(const file_view&) = delete;

  inline file_view& operator=(file_view&& fb) noexcept {

    if (&fb == this) {
      return *this;
    }

    close();
    _data = std::exchange(fb._data, nullptr);
    _size = std::exchange(fb._size, (size_type)0);
    return *this;
  }

  [[nodiscard]] __zb::error_result open(const char* file_path) noexcept;

  [[nodiscard]] inline __zb::error_result open(const std::string& file_path) noexcept {
    return open(file_path.c_str());
  }

  template <class _Allocator>
  ZB_CHECK ZB_INLINE __zb::error_result open(
      const std::basic_string<char, std::char_traits<char>, _Allocator>& file_path) noexcept {
    return open(file_path.c_str());
  }

  void close() noexcept;

  [[nodiscard]] inline bool is_open() const noexcept { return _data && _size; }

  [[nodiscard]] inline bool empty() const noexcept { return !is_open(); }

  [[nodiscard]] inline size_type size() const noexcept { return _size; }

  [[nodiscard]] inline const_reference operator[](size_type __n) const noexcept {
    assert(__n < size() && "index out of bounds");
    return _data[__n];
  }

  [[nodiscard]] inline const_reference front() const noexcept {
    assert(is_open() && "front() called when empty");
    return *_data;
  }

  [[nodiscard]] inline const_reference back() const noexcept {
    assert(is_open() && "back() called when empty");
    return *(_data + _size - 1);
  }

  [[nodiscard]] inline std::string_view str() const noexcept {
    return std::string_view((const char*)_data, _size);
  }

  [[nodiscard]] inline std::span<const uint8_t> content() const noexcept {
    return std::span<const uint8_t>((const uint8_t*)_data, _size);
  }

  [[nodiscard]] inline iterator begin() const noexcept { return _data; }
  [[nodiscard]] inline iterator end() const noexcept { return _data + _size; }

  [[nodiscard]] inline const_iterator cbegin() const noexcept { return begin(); }
  [[nodiscard]] inline const_iterator cend() const noexcept { return end(); }

  [[nodiscard]] inline const_pointer data() const noexcept { return _data; }

  [[nodiscard]] inline const_pointer data(size_type offset) const noexcept {
    assert(is_open() && "access nullptr");
    assert(offset < size() && "offset out of bounds");
    return data() + offset;
  }

  [[nodiscard]] inline const_reference reversed(size_type index) const noexcept {
    assert(index < size() && "index out of bounds");
    return data()[size() - index - 1];
  }

private:
  pointer _data = nullptr;
  size_type _size = 0;
};

ZBASE_END_NAMESPACE
