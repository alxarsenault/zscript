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

#if __ZBASE_APPLE__
#include <zscript/base/sys/apple/cf/type.h>
#include <zscript/base/sys/apple/cf/pointer.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)

cf::pointer<CFDictionaryRef> create_dictionary(const void** keys, const void** values, size_t size) noexcept;

template <size_t N>
inline cf::pointer<CFDictionaryRef> create_dictionary(
    CFStringRef const (&keys)[N], CFTypeRef const (&values)[N]) noexcept {
  return create_dictionary(reinterpret_cast<const void**>(&keys), reinterpret_cast<const void**>(&values), N);
}

ZBASE_END_SUB_NAMESPACE(apple, cf)
#endif // __ZBASE_APPLE__
