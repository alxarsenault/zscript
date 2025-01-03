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
#include <zscript/base/utility/traits.h>

namespace zb {

/// The `ref_wrapper<T>` class is a utility that wraps a reference to an object of type T,
/// allowing the reference to be passed and stored by value.
/// It also prevents binding to temporary objects (rvalues) to avoid dangling references.
///
/// This class provides a safe and efficient way to handle references in situations
/// where they need to behave like regular objects, such as in containers or function arguments.
///
/// Compare to `std::reference_wrapper<T>`:
/// * No invoke.
/// * No copy assignment operator.
template <class T>
class ref_wrapper {
  static void __fun(T&) noexcept;
  static void __fun(T&&) = delete;

public:
  using value_type = T;
  using pointer = T*;
  using reference = T&;

  template <class _Up,
      class = std::enable_if_t<!zb::is_same_rcvref_v<_Up, ref_wrapper>, decltype(__fun(std::declval<_Up>()))>>
  ZB_INLINE_CXPR explicit ref_wrapper(_Up&& __u) noexcept
      : _ptr(std::addressof(static_cast<_Up&&>(__u))) {}

  ref_wrapper(const ref_wrapper&) = default;
  ref_wrapper& operator=(const ref_wrapper&) = default;

  ZB_INLINE_CXPR operator reference() const noexcept { return *_ptr; }

  ZB_CK_INLINE_CXPR reference get() const noexcept { return *_ptr; }

  ZB_INLINE_CXPR reference operator*() const noexcept { return *_ptr; }

  ZB_INLINE_CXPR pointer operator->() const noexcept { return _ptr; }

private:
  value_type* _ptr;
};

template <class T>
ref_wrapper(T&) -> ref_wrapper<T>;

#define ZB_REF(...) zb::wref(__VA_ARGS__)
#define ZB_CREF(...) zb::wcref(__VA_ARGS__)

template <class T>
ZB_CK_INLINE_CXPR ref_wrapper<T> wref(T& t) noexcept {
  return ref_wrapper<T>(t);
}

template <class T>
ZB_CK_INLINE_CXPR ref_wrapper<T> wref(ref_wrapper<T> t) noexcept {
  return t;
}

template <class T>
ZB_CK_INLINE_CXPR ref_wrapper<const T> wcref(const T& t) noexcept {
  return ref_wrapper<const T>(t);
}

template <class T>
ZB_CK_INLINE_CXPR ref_wrapper<const T> wcref(ref_wrapper<T> t) noexcept {
  return t;
}

template <class T>
void wref(const T&&) = delete;

template <class T>
void wcref(const T&&) = delete;

} // namespace zb.
