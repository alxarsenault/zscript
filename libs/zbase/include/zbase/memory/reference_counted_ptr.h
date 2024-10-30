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
#include <zbase/sys/assert.h>
#include <zbase/utility/traits.h>
#include <memory>
#include <utility>

ZBASE_BEGIN_NAMESPACE

//
// MARK: reference_counted_ptr
//

namespace reference_counted_ptr_detail {
ZBASE_DECL_USING_DECLTYPE(meta_ref_count, ref_count);
ZBASE_DECL_HAS_MEMBER(has_ref_count, meta_ref_count);
} // namespace reference_counted_ptr_detail.

template <class _T, class _Handler>
class reference_counted_ptr {
public:
  using value_type = std::remove_pointer_t<_T>;
  using pointer = std::add_pointer_t<value_type>;
  using handler = _Handler;

  inline reference_counted_ptr() noexcept = default;

  inline explicit reference_counted_ptr(pointer ptr) noexcept
      : _ptr(ptr) {}

  inline reference_counted_ptr(pointer ptr, bool should_retain) noexcept
      : _ptr(ptr) {

    if (should_retain && _ptr) {
      handler::retain(_ptr);
    }
  }

  inline reference_counted_ptr(const reference_counted_ptr& rhs) noexcept
      : _ptr(rhs._ptr) {

    if (_ptr) {
      handler::retain(_ptr);
    }
  }

  inline reference_counted_ptr(reference_counted_ptr&& rhs) noexcept
      : _ptr(rhs._ptr) {

    rhs._ptr = nullptr;
  }

  inline ~reference_counted_ptr() noexcept { reset(); }

  inline reference_counted_ptr& operator=(const reference_counted_ptr& rhs) noexcept {

    if (this == &rhs || _ptr == rhs._ptr) {
      return *this;
    }

    reset();
    _ptr = rhs._ptr;
    if (_ptr) {
      handler::retain(_ptr);
    }

    return *this;
  }

  inline reference_counted_ptr& operator=(reference_counted_ptr&& rhs) noexcept {

    if (this == &rhs || _ptr == rhs._ptr) {
      return *this;
    }

    reset();
    _ptr = rhs._ptr;
    rhs._ptr = nullptr;
    return *this;
  }

  inline reference_counted_ptr& operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  inline explicit operator bool() const noexcept { return _ptr != nullptr; }

  inline operator pointer() const&& noexcept = delete;
  inline operator pointer() const& noexcept { return _ptr; }

  inline pointer& ptr_ref() noexcept { return _ptr; }

  inline pointer get() const noexcept { return _ptr; }

  inline pointer operator->() const noexcept { return _ptr; }

  inline void reset() noexcept {
    if (_ptr) {
      handler::release(_ptr);
      _ptr = nullptr;
    }
  }

  inline void reset(pointer ptr, bool should_retain = false) {
    if (_ptr) {
      handler::release(_ptr);
      _ptr = nullptr;
    }

    _ptr = ptr;

    if (should_retain && _ptr) {
      handler::retain(_ptr);
    }
  }

  inline pointer disown() noexcept { return std::exchange(_ptr, nullptr); }

  template <class Dummy = _Handler,
      class = std::enable_if_t<__zb::reference_counted_ptr_detail::has_ref_count<Dummy>::value, int>>
  inline size_t ref_count() const noexcept {
    static_assert(
        __zb::reference_counted_ptr_detail::has_ref_count<handler>::value, "no ref_count in handler");
    return _ptr ? handler::ref_count(_ptr) : 0;
  }

private:
  pointer _ptr = nullptr;
};

ZBASE_END_NAMESPACE
