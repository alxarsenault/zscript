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

///
/// \c not_null
///
/// Restricts a pointer or smart pointer to only hold non-null values.
///
/// Has zero size overhead over T.
///
/// If T is a pointer (i.e. T == U*) then
/// - allow construction from U*
/// - disallow construction from nullptr_t
/// - disallow default construction
/// - ensure construction from null U* fails
/// - allow implicit conversion to U*
///

#if 1

template <class T>
class not_null {
public:
  static_assert(std::is_convertible_v<decltype(std::declval<T>() != nullptr), bool>,
      "T cannot be compared to nullptr.");

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
  inline constexpr not_null(U&& u) noexcept
      : _ptr(std::forward<U>(u)) {
    zbase_assert(_ptr, "Pointer is null.");
  }

  template <typename = std::enable_if_t<!std::is_same_v<std::nullptr_t, T>>>
  inline constexpr not_null(T u) noexcept
      : _ptr(std::move(u)) {
    zbase_assert(_ptr, "Pointer is null.");
  }

  template <typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
  inline constexpr not_null(const not_null<U>& other) noexcept
      : not_null(other.get()) {}

  not_null(const not_null& other) noexcept = default;
  not_null(not_null&& other) noexcept = default;

  ~not_null() noexcept = default;

  not_null& operator=(const not_null& other) noexcept = default;
  not_null& operator=(not_null&& other) noexcept = default;

  inline constexpr std::conditional_t<std::is_copy_constructible_v<T>, T, const T&> get() const noexcept {
    zbase_assert(_ptr != nullptr, "Pointer is null.");
    return _ptr;
  }

  inline constexpr operator T() const noexcept { return get(); }
  inline constexpr decltype(auto) operator->() const noexcept { return get(); }
  inline constexpr decltype(auto) operator*() const noexcept { return *get(); }

  // Prevents compilation when someone attempts to assign a null pointer constant.
  not_null(std::nullptr_t) = delete;
  not_null& operator=(std::nullptr_t) = delete;

  // Unwanted operators, pointers only point to single objects.
  not_null& operator++() = delete;
  not_null& operator--() = delete;
  not_null operator++(int) = delete;
  not_null operator--(int) = delete;
  not_null& operator+=(ptrdiff_t) = delete;
  not_null& operator-=(ptrdiff_t) = delete;
  void operator[](ptrdiff_t) const = delete;

private:
  T _ptr;
};

template <class T, class U>
auto operator==(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> decltype(lhs.get() == rhs.get()) {
  return lhs.get() == rhs.get();
}

template <class T, class U>
auto operator!=(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> decltype(lhs.get() != rhs.get()) {
  return lhs.get() != rhs.get();
}

template <class T, class U>
auto operator<(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> decltype(lhs.get() < rhs.get()) {
  return lhs.get() < rhs.get();
}

template <class T, class U>
auto operator<=(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> decltype(lhs.get() <= rhs.get()) {
  return lhs.get() <= rhs.get();
}

template <class T, class U>
auto operator>(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> decltype(lhs.get() > rhs.get()) {
  return lhs.get() > rhs.get();
}

template <class T, class U>
auto operator>=(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> decltype(lhs.get() >= rhs.get()) {
  return lhs.get() >= rhs.get();
}

// More unwanted operators.
template <class T, class U>
ptrdiff_t operator-(const not_null<T>&, const not_null<U>&) = delete;

template <class T>
not_null<T> operator-(const not_null<T>&, ptrdiff_t) = delete;

template <class T>
not_null<T> operator+(const not_null<T>&, ptrdiff_t) = delete;

template <class T>
not_null<T> operator+(ptrdiff_t, const not_null<T>&) = delete;

#else

template <class T>
using not_null = T;

#endif

template <class T>
auto make_not_null(T&& t) noexcept {
  return not_null<std::remove_cv_t<std::remove_reference_t<T>>>{ std::forward<T>(t) };
}

//
// MARK: packed_pointer
//

/// packed_pointer
class packed_pointer {
public:
  packed_pointer() noexcept = default;
  packed_pointer(const packed_pointer&) noexcept = default;
  packed_pointer(packed_pointer&&) noexcept = default;

  template <typename TPointer, typename TInt>
  inline packed_pointer(TPointer ptr, TInt value) noexcept {
    set_pointer(ptr);
    set_int(value);
  }

  ~packed_pointer() = default;

  packed_pointer& operator=(const packed_pointer&) noexcept = default;
  packed_pointer& operator=(packed_pointer&&) noexcept = default;

  template <typename T>
  [[nodiscard]] inline T get_int() const noexcept {
    return static_cast<T>(_data & k_int_mask);
  }

  template <typename T>
  [[nodiscard]] inline T get_pointer() const noexcept {
    return static_cast<T>(reinterpret_cast<void*>(_data & k_pointer_mask));
  }

  template <typename T>
  inline void set_pointer(T ptr) noexcept {
    intptr_t ptrWord = reinterpret_cast<intptr_t>((void*)(ptr));
    zbase_assert((ptrWord & ~k_pointer_mask) == 0, "Pointer is not sufficiently aligned");

    // Preserve all low bits, just update the pointer.
    _data = (intptr_t)(ptrWord | (_data & ~k_pointer_mask));
  }

  template <typename T>
  inline T exchange_pointer(T ptr) noexcept {
    T last = static_cast<T>(reinterpret_cast<void*>(_data & k_pointer_mask));

    intptr_t ptrWord = reinterpret_cast<intptr_t>((void*)(ptr));
    zbase_assert((ptrWord & ~k_pointer_mask) == 0, "Pointer is not sufficiently aligned");

    // Preserve all low bits, just update the pointer.
    _data = (intptr_t)(ptrWord | (_data & ~k_pointer_mask));
    return last;
  }

  template <typename T>
  inline void set_int(T value) noexcept {
    intptr_t intWord = static_cast<intptr_t>(value);
    zbase_assert((intWord & ~k_int_mask) == 0, "Integer too large for field");

    // Preserve all bits other than the ones we are updating.
    _data = (intptr_t)((_data & ~k_int_mask) | intWord);
  }

  [[nodiscard]] inline constexpr bool operator==(packed_pointer other) const noexcept {
    return _data == other._data;
  }

  [[nodiscard]] inline constexpr bool operator!=(packed_pointer other) const noexcept {
    return _data != other._data;
  }

private:
  intptr_t _data = 0;

  static constexpr uintptr_t k_pointer_mask = (uintptr_t)-4;
  static constexpr uintptr_t k_int_mask = (uintptr_t)3;
};

//
// MARK: optional_ptr
//

template <class _T>
struct default_deleter {
  static inline void deallocate(_T* ptr) { delete ptr; }
};

template <class T>
struct empty_deleter {
  inline void operator()(T*) const {}
};

///
template <class _T, class _Dp = __zb::default_deleter<_T>>
class optional_ptr {
public:
  using element_type = _T;
  using pointer = _T*;
  using storage_type = __zb::packed_pointer;
  using deleter_type = _Dp;

  inline constexpr optional_ptr() noexcept
      : _data{ nullptr, false } {}

  inline constexpr optional_ptr(std::nullptr_t) noexcept
      : _data{ nullptr, false } {}

  inline explicit optional_ptr(pointer ptr, bool powned) noexcept
      : _data{ ptr, powned } {}

  inline optional_ptr(optional_ptr&& _Right) noexcept
      : _data{ _Right.get(), _Right.owned() } {
    (void)_Right.release();
  }

  inline optional_ptr(std::unique_ptr<element_type>&& uptr) noexcept
      : _data{ uptr.get(), (bool)uptr } {
    (void)uptr.release();
  }

  optional_ptr(const optional_ptr&) = delete;

  inline ~optional_ptr() noexcept {
    if (pointer ptr = _data.get_pointer<pointer>(); ptr && owned()) {
      deleter_type::deallocate(ptr);
    }
  }

  optional_ptr& operator=(const optional_ptr&) = delete;

  inline constexpr optional_ptr& operator=(std::nullptr_t) noexcept {
    reset();
    return *this;
  }

  optional_ptr& operator=(optional_ptr&& _Right) noexcept {
    if (this != std::addressof(_Right)) {
      reset(_Right.get(), _Right.owned());
      (void)_Right.release();
    }
    return *this;
  }

  /// Returns true if the pointer is owned.
  [[nodiscard]] inline bool owned() const noexcept { return _data.get_int<bool>(); }

  [[nodiscard]] std::add_lvalue_reference_t<_T> operator*() const noexcept {
    return *(_data.get_pointer<pointer>());
  }

  [[nodiscard]] pointer operator->() const noexcept { return _data.get_pointer<pointer>(); }

  [[nodiscard]] pointer get() const noexcept { return _data.get_pointer<pointer>(); }

  [[nodiscard]] inline explicit operator bool() const noexcept {
    return static_cast<bool>(_data.get_pointer<pointer>());
  }

  [[nodiscard]] inline pointer release() noexcept {
    _data.set_int(false);
    return _data.exchange_pointer<pointer>(nullptr);
  }

  inline void reset() noexcept {
    if (pointer old_ptr = _data.exchange_pointer<pointer>(nullptr); old_ptr && owned()) {
      deleter_type::deallocate(old_ptr);
    }

    _data.set_int(false);
  }

  inline void reset(pointer ptr, bool powned) noexcept {
    if (pointer old_ptr = _data.exchange_pointer<pointer>(ptr); old_ptr && owned()) {
      deleter_type::deallocate(old_ptr);
    }

    _data.set_int(powned && ptr != nullptr);
  }

private:
  storage_type _data;
};

template <class T, class D>
inline bool operator==(const optional_ptr<T, D>& x, const optional_ptr<T, D>& y) noexcept {
  return x.get() == y.get();
}

template <class T, class D>
inline bool operator!=(const optional_ptr<T, D>& x, const optional_ptr<T, D>& y) noexcept {
  return x.get() != y.get();
}

template <class T, class D>
inline bool operator==(const optional_ptr<T, D>& x, std::nullptr_t) noexcept {
  return x.get() == nullptr;
}

template <class T, class D>
inline bool operator==(std::nullptr_t, const optional_ptr<T, D>& x) noexcept {
  return x.get() == nullptr;
}

template <class T, class D>
inline bool operator!=(const optional_ptr<T, D>& x, std::nullptr_t) noexcept {
  return x.get() != nullptr;
}

template <class T, class D>
inline bool operator!=(std::nullptr_t, const optional_ptr<T, D>& x) noexcept {
  return x.get() != nullptr;
}

template <class T, class D>
inline bool operator==(const optional_ptr<T, D>& x, const T* y) noexcept {
  return x.get() == y;
}

template <class T, class D>
inline bool operator==(const T* x, const optional_ptr<T, D>& y) noexcept {
  return y.get() == x;
}

template <class T, class D>
inline bool operator!=(const optional_ptr<T, D>& x, const T* y) noexcept {
  return x.get() != y;
}

template <class T, class D>
inline bool operator!=(const T* x, const optional_ptr<T, D>& y) noexcept {
  return y.get() != x;
}

template <class T>
inline optional_ptr<T> make_owned(T* ptr) {
  return optional_ptr<T>(ptr, true);
}

template <class T, class... Args>
inline optional_ptr<T> make_owned(Args&&... args) {
  return optional_ptr<T>(new T(std::forward<Args>(args)...), true);
}
ZBASE_END_NAMESPACE
