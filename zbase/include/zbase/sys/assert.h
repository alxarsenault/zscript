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
#include <zbase/memory/callable_object.h>

#include <sys/cdefs.h>
#include <iostream>
#include <sstream>
#include <cstdlib>

#ifdef __FILE_NAME__
#define __ZBASE_ASSERT_FILE_NAME__ __FILE_NAME__
#else
#define __ZBASE_ASSERT_FILE_NAME__ __FILE__
#endif // __FILE_NAME__.

///
#ifndef ZBASE_ASSERT_STREAM
#define ZBASE_ASSERT_STREAM() std::cerr
#endif // ZBASE_ASSERT_STREAM.

///
#ifndef ZBASE_ABORT
#define ZBASE_ABORT() ::abort()
#endif // ZBASE_ABORT.

///
/// \p e  (cond string)
/// \p fct
/// \p file
/// \p line
/// \p args...
#ifndef ZBASE_ASSERT_PRINT
#define ZBASE_ASSERT_PRINT(...) zb::assert_print(ZBASE_ASSERT_STREAM(), __VA_ARGS__)
#endif // ZBASE_ASSERT_PRINT.

#ifndef ZBASE_WARNING_PRINT
#define ZBASE_WARNING_PRINT(...) zb::warning_print(ZBASE_ASSERT_STREAM(), __VA_ARGS__)
#endif // ZBASE_ASSERT_PRINT.

///
#ifndef zbase_assert
#define zbase_assert(...) __ZBASE_ASSERT_BASE(__VA_ARGS__, zb::variadic_args_end_tag{})
#endif // zbase_assert.

#ifndef zbase_stream_assert
#define zbase_stream_assert(stream, ...) \
  __ZBASE_STREAM_ASSERT_BASE(stream, __VA_ARGS__, zb::variadic_args_end_tag{})
#endif // zbase_assert.

#ifndef zbase_error
#define zbase_error(...) __ZBASE_ASSERT_BASE(false, __VA_ARGS__, zb::variadic_args_end_tag{})
#endif // zbase_assert.

#ifndef zbase_warning
#define zbase_warning(...) __ZBASE_WARNING_BASE(__VA_ARGS__, zb::variadic_args_end_tag{})
#endif // zbase_assert.

ZBASE_BEGIN_NAMESPACE

inline __zb::callable_object<bool()>::stack_callable_object<256> assert_handler;

inline constexpr size_t k_assert_padding_size = 11;

template <class... Args>
inline std::ostream& assert_print(std::ostream& stream, const char* cond_str, const char* fct_str,
    const char* filename, int line, Args&&... args) {
  stream << "- assert   : " << cond_str;
  stream << "\n  function : " << fct_str;
  stream << "\n  file     : " << filename;
  stream << "\n  line     : " << line;

  if constexpr (sizeof...(Args) > 1) {
    stream << "\n  message  : ";
    (
        [](std::ostream& stream, auto&& item) {
          if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(item)>, __zb::variadic_args_end_tag>) {
            stream << item;
          }
        }(stream, args),
        ...);
  }

  stream << std::endl;

  return stream;
}

template <class... Args>
inline std::ostream& warning_print(std::ostream& stream, const char* cond_str, const char* fct_str,
    const char* filename, int line, Args&&... args) {
  stream << "- warning  : " << cond_str;
  stream << "\n  function : " << fct_str;
  stream << "\n  file     : " << filename;
  stream << "\n  line     : " << line;

  if constexpr (sizeof...(Args) > 1) {
    stream << "\n  message  :";
    (
        [](std::ostream& stream, auto&& item) {
          if constexpr (!std::is_same_v<std::remove_cvref_t<decltype(item)>, __zb::variadic_args_end_tag>) {
            stream << ' ' << item;
          }
        }(stream, args),
        ...);
  }

  stream << std::endl;

  return stream;
}

ZBASE_END_NAMESPACE

#define __ZBASE_ASSERT_BASE(e, ...)                                                                         \
  ((void)((e) ? ((void)0)                                                                                   \
              : ((void)ZBASE_ASSERT_PRINT(#e, __func__, __ZBASE_ASSERT_FILE_NAME__, __LINE__, __VA_ARGS__), \
                  ((!zb::assert_handler || zb::assert_handler.call()) ? ZBASE_ABORT() : ((void)0)))))

#define __ZBASE_STREAM_ASSERT_BASE(stream, e, ...)                                             \
  ((void)((e) ? ((void)0)                                                                      \
              : ((void)zb::assert_print(                                                       \
                     stream, #e, __func__, __ZBASE_ASSERT_FILE_NAME__, __LINE__, __VA_ARGS__), \
                  ((!zb::assert_handler || zb::assert_handler.call()) ? ZBASE_ABORT() : ((void)0)))))

#define __ZBASE_WARNING_BASE(e, ...) \
  ((void)((e)                        \
          ? ((void)0)                \
          : ((void)ZBASE_WARNING_PRINT(#e, __func__, __ZBASE_ASSERT_FILE_NAME__, __LINE__, __VA_ARGS__))))
