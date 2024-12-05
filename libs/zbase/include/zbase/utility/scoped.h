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
#include <utility>

ZBASE_BEGIN_NAMESPACE

/// This class template provides a scoped execution of a callable object.
/// The callable object is executed upon the destruction of the scoped object.
///
/// Example:
/// @code
///   void* data = std::malloc(10);
///   auto data_auto_free = scoped([=]() { std::free(data); });
/// @endcode
///
template <class Fct>
struct scoped {
  /// Constructor that takes a callable object and stores it.
  inline constexpr scoped(Fct&& fct)
      : _fct(std::forward<Fct>(fct)) {}

  /// Destructor that executes the stored callable object.
  inline constexpr ~scoped() { _fct(); }

  // Disable copy and move operations.
  scoped(const scoped&) = delete;
  scoped(scoped&&) = delete;
  scoped& operator=(const scoped&) = delete;
  scoped& operator=(scoped&&) = delete;

  void* operator new(size_t) = delete;
  void operator delete(void*) = delete;

  inline operator bool() const noexcept { return true; }

private:
  Fct _fct; /// The stored callable object.
};

// template <class Fct>
// struct scoped<Fct, true> {
//   /// Constructor that takes a callable object and stores it.
//   inline constexpr scoped(Fct&& fct)
//       : _fct(std::forward<Fct>(fct)) {}
//
//   /// Destructor that executes the stored callable object.
//   inline constexpr ~scoped() {
//     if (!_dismiss) {
//       _fct();
//     }
//   }
//
//   // Disable copy and move operations.
//   scoped(const scoped&) = delete;
//   scoped(scoped&&) = delete;
//   scoped& operator=(const scoped&) = delete;
//   scoped& operator=(scoped&&) = delete;
//
//   void* operator new(size_t) = delete;
//   void operator delete(void*) = delete;
//
//   inline void dismiss() noexcept { _dismiss = true; }
//
// private:
//   Fct _fct; /// The stored callable object.
//   bool _dismiss = false;
// };
ZBASE_END_NAMESPACE
