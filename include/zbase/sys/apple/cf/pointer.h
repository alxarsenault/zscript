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

#if __ZBASE_APPLE__
#include <zbase/memory/reference_counted_ptr.h>
#include <zbase/sys/apple/cf/type.h>
#include <memory>
#include <type_traits>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)

namespace detail {
template <class _T>
struct reference_counted_handler {
  using value_type = std::remove_pointer_t<_T>;
  using pointer = std::add_pointer_t<value_type>;

  static inline void retain(pointer ptr) { cf::retain(ptr); }

  static inline void release(pointer ptr) { cf::release(ptr); }
};
} // namespace detail.

template <class _CFType>
using pointer = __zb::reference_counted_ptr<_CFType, detail::reference_counted_handler<_CFType>>;

ZBASE_END_SUB_NAMESPACE(apple, cf)
#endif // __ZBASE_APPLE__