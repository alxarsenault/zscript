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
#include <iostream>
#include <string>
#include <string_view>
#include <memory>
#include <type_traits>
#include <utility>

using CFTypeRef = const void*;
using CFStringRef = const struct __CFString*;
using CFURLRef = const struct __CFURL*;
using CFDictionaryRef = const struct __CFDictionary*;
using CFRunLoopRef = struct __CFRunLoop*;

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)
void retain(const void* obj) noexcept;
void release(const void* obj) noexcept;
size_t ref_count(const void* obj) noexcept;

template <class T, std::enable_if_t<std::is_pointer_v<T>, int> = 0>
inline void retain(T obj) noexcept {
  cf::retain((const void*)obj);
}

template <class T, std::enable_if_t<std::is_pointer_v<T>, int> = 0>
inline void release(T obj) noexcept {
  cf::release((const void*)obj);
}

template <class T, std::enable_if_t<std::is_pointer_v<T>, int> = 0>
inline size_t ref_count(T obj) noexcept {
  return cf::ref_count((const void*)obj);
}

std::string cf_string_to_std_string(CFStringRef s);

ZBASE_END_SUB_NAMESPACE(apple, cf)
#endif // __ZBASE_APPLE__
