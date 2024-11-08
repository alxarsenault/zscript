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
#include <zbase/container/vector.h>
#include <zbase/container/span.h>
#include <span>

ZBASE_BEGIN_NAMESPACE

template <class T>
class stack_view;

/// Represents the state of the execution stack.
struct execution_stack_state {
  using size_type = size_t;
  size_type base;
  size_type top;
};

/// @brief A stack implementation that operates on a vector.
///
/// This class provides a stack-like interface over a vector,
/// allowing push and pop operations, with support for a customizable allocator.
///
/// @tparam T The type of elements stored in the stack.
/// @tparam Allocator The allocator used for memory management (defaults to std::allocator<T>).
///
template <class T, class Allocator = std::allocator<T>, bool CanResize = true>
class execution_stack : __zb::vector<T, Allocator> {
public:
  using value_type = T;
  using vector_type = __zb::vector<T, Allocator>;
  using size_type = size_t;
  using difference_type = typename vector_type::difference_type;
  using reference = typename vector_type::reference;
  using const_reference = typename vector_type::const_reference;
  using pointer = typename vector_type::pointer;
  using const_pointer = typename vector_type::const_pointer;
  using allocator_type = typename vector_type::allocator_type;
  using state = execution_stack_state;

  static constexpr bool can_resize = CanResize;

  /// @brief Constructs an empty stack or a stack with a pre-allocated size.
  /// @param n The number of elements to pre-allocate (defaults to 0).
  ZB_INLINE explicit execution_stack(size_type n = 0)
      : vector_type(n)
      , _base_idx(0)
      , _top_idx(0) {}

  /// @brief Constructs a stack with a pre-allocated size using a custom allocator.
  /// @param n The number of elements to pre-allocate.
  /// @param a The allocator to use.
  ZB_INLINE explicit execution_stack(size_type n, const allocator_type& a)
      : vector_type(n, a)
      , _base_idx(0)
      , _top_idx(0) {}

  /// @brief Pushes a copy of the element onto the stack.
  /// @param v The value to push onto the stack (defaults to a default-constructed value).
  inline void push(const value_type& v = value_type{}) noexcept {
    if (_top_idx >= vector_type::size()) {
      if constexpr (can_resize) {
        vector_type::push_back(v);
        ++_top_idx;
      }
      else {
        zbase_error("stack is full");
      }
      return;
    }

    vector_type::operator[](_top_idx++) = v;
  }

  /// @brief Pushes a moved element onto the stack.
  /// @param v The value to move onto the stack.
  inline void push(value_type&& v) noexcept {
    if (_top_idx >= vector_type::size()) {
      if constexpr (can_resize) {
        vector_type::push_back(std::move(v));
        ++_top_idx;
      }
      else {
        zbase_error("stack is full");
      }
      return;
    }

    vector_type::operator[](_top_idx++) = std::move(v);
  }

  /// @brief Pushes a default-constructed element onto the stack multiple times.
  /// @param n The number of elements to push.
  inline void push_n(size_t n) noexcept {
    const size_t new_size = _top_idx + n;
    if (new_size > vector_type::size()) {
      if constexpr (can_resize) {
        vector_type::resize(new_size);
      }
      else {
        zbase_error("stack is full");
      }
    }

    for (size_t i = _top_idx; i < new_size; i++) {
      vector_type::operator[](i) = value_type{};
    }
    _top_idx += n;
  }

  /// @brief Pushes a copy of the provided value onto the stack multiple times.
  /// @param n The number of elements to push.
  /// @param v The value to push onto the stack.
  inline void push_n(size_t n, const value_type& v) noexcept {
    const size_t new_size = _top_idx + n;
    if (new_size > vector_type::size()) {
      if constexpr (can_resize) {
        vector_type::resize(new_size);
      }
      else {
        zbase_error("stack is full");
      }
    }

    for (size_t i = _top_idx; i < new_size; i++) {
      vector_type::operator[](i) = v;
    }

    _top_idx += n;
  }

  /// @brief Pops the top element off the stack.
  ZB_INLINE void pop() noexcept {
    zbase_assert(!vector_type::empty() and _top_idx > _base_idx, "zb::execution_stack::pop out of bounds");
    vector_type::operator[](--_top_idx) = value_type{};
  }

  /// @brief Pops the top element off the stack and returns it.
  /// @return The popped value.
  ZB_CHECK ZB_INLINE value_type pop_get() noexcept {
    zbase_assert(
        !vector_type::empty() and _top_idx > _base_idx, "zb::execution_stack::pop_get out of bounds");
    return std::move(vector_type::operator[](--_top_idx));
  }

  /// @brief Pops multiple elements from the stack.
  /// @param n The number of elements to pop.
  /// @param reinit Whether to reinitialize popped elements to default values.
  inline void pop(size_type n, bool reinit = true) noexcept {
    zbase_assert(n <= (size_type)vector_type::size(), "zb::execution_stack::pop n out of bounds");
    zbase_assert(n <= stack_size(), "zb::execution_stack::pop n out of bounds");

    if (reinit) {
      for (size_type i = 0; i < n; i++) {
        vector_type::operator[](--_top_idx) = value_type{};
      }
    }
    else {
      _top_idx -= n;
    }
  }

  /// @brief Pops elements until the stack reaches the specified size.
  /// @param new_top_size The new size of the top of the stack.
  inline void pop_to(size_t new_top_size) noexcept {
    zbase_assert(new_top_size >= _base_idx, "zb::execution_stack::pop_to out of bounds");

    while (_top_idx > new_top_size) {
      vector_type::operator[](--_top_idx) = value_type{};
    }
  }

  /// @brief Returns a reference to the top element of the stack.
  /// @return Reference to the top element.
  ZB_CHECK ZB_INLINE reference top() noexcept {
    zbase_assert(!vector_type::empty() and _top_idx, "top out of bounds");
    return vector_type::operator[](_top_idx - 1);
  }

  ZB_CHECK ZB_INLINE const_reference top() const noexcept {
    zbase_assert(!vector_type::empty() and _top_idx, "top out of bounds");
    return vector_type::operator[](_top_idx - 1);
  }

  /// @brief Returns the size of the stack.
  /// @return The size of the stack.
  ZB_CK_INLINE size_type stack_size() const noexcept { return _top_idx - _base_idx; }

  ZB_CK_INLINE pointer stack_base_pointer() noexcept { return vector_type::data(_base_idx); }
  ZB_CK_INLINE const_pointer stack_base_pointer() const noexcept { return vector_type::data(_base_idx); }

  /// @brief Converts an index relative to the stack to an absolute index.
  /// @param n The relative index.
  /// @return The absolute index.
  ZB_CHECK ZB_INLINE size_type to_stack_index(difference_type n) const noexcept {
    if (n >= 0) {
      zbase_assert(n + _base_idx < _top_idx, "invalid stack index");
      return n + _base_idx;
    }

    difference_type next_idx = _top_idx + n;

    // n is negative here.
    zbase_assert(next_idx >= (difference_type)_base_idx and next_idx <= (difference_type)_top_idx,
        "invalid stack index");
    return (size_type)next_idx;
  }

  /// @brief Accesses an element by index relative to the stack base.
  /// @param n The relative index to access.
  /// @return A reference to the element at the given index.
  /// @{
  ZB_CHECK ZB_INLINE reference operator[](difference_type n) noexcept {
    n = to_stack_index(n);
    zbase_assert(n < (difference_type)vector_type::size());
    return vector_type::operator[](n);
  }

  ZB_CHECK ZB_INLINE const_reference operator[](difference_type n) const noexcept {
    n = to_stack_index(n);
    zbase_assert(n < (difference_type)vector_type::size());
    return vector_type::operator[](n);
  }
  /// @}

  /// @brief Accesses an element relative to the top of the stack.
  /// @param n The relative index to access.
  /// @return A reference to the element n positions from the top.
  ZB_CHECK ZB_INLINE reference get_up(difference_type n) noexcept {
    zbase_assert(_top_idx + n >= 0 and _top_idx + n < vector_type::size(), "out of bounds index");
    return vector_type::operator[](_top_idx + n);
  }

  /// @brief Accesses an element at an absolute index.
  /// @param n The absolute index to access.
  /// @return A reference to the element at the given index.
  ZB_CHECK ZB_INLINE reference get_at(size_type n) noexcept {
    zbase_assert(n < vector_type::size(), "out of bounds index");
    return vector_type::operator[](n);
  }

  //  Make sure that _top_idx is within bounds before assigning vector_type::operator[](_top_idx) =
  //  value_type{}. In other parts of your code, there are proper boundary checks with zbase_assert. It might
  //  be worth adding similar checks here to avoid out-of-bounds access.

  /// @brief Removes an element at a given index.
  /// @param n The index of the element to remove (relative to the stack base).
  inline void remove(difference_type n) noexcept {
    n = to_stack_index(n);

    for (difference_type i = n; i < (difference_type)_top_idx; i++) {
      vector_type::operator[](i) = vector_type::operator[](i + 1);
    }

    vector_type::operator[](_top_idx) = value_type{};
    _top_idx--;
  }

  /// @brief Swaps the values of two elements at given indices.
  /// @param n1 The first index.
  /// @param n2 The second index.
  inline void swap(difference_type n1, difference_type n2) noexcept {
    n1 = to_stack_index(n1);
    n2 = to_stack_index(n2);
    zbase_assert(n1 < (difference_type)vector_type::size());
    zbase_assert(n2 < (difference_type)vector_type::size());

    std::swap(vector_type::operator[](n1), vector_type::operator[](n2));
  }

  /// @brief Pushes elements from a range back onto the stack.
  /// @param base The base index from which to start repushing.
  /// @param count The number of elements to repush.
  inline void absolute_repush_n(size_type base, size_type count) {
    for (size_type i = base; i < base + count; i++) {
      push(vector_type::operator[](i));
    }
  }

  /// @brief Sets the base index of the stack.
  /// @param base_idx The new base index.
  ZB_INLINE void set_stack_base(size_type base_idx) noexcept { _base_idx = base_idx; }

  /// @brief Gets the base index of the stack.
  /// @return The current base index of the stack.
  ZB_CHECK ZB_INLINE size_type get_stack_base() const noexcept { return _base_idx; }

  /// @brief Gets the absolute index of the top of the stack.
  /// @return The absolute index of the top.
  ZB_CHECK ZB_INLINE size_type get_absolute_top() const noexcept { return _top_idx; }

  /// @brief Returns a view of the current stack.
  /// @return A stack_view object representing the current stack.
  ZB_CHECK ZB_INLINE __zb::stack_view<value_type> get_stack_view() noexcept;

  /// @brief Returns the current state of the stack (base and top indices).
  /// @return The current state of the stack.
  ZB_CHECK ZB_INLINE state get_state() const noexcept { return { _base_idx, _top_idx }; }

  /// @brief Returns a reference to the internal vector.
  /// @return A reference to the internal vector used by the stack.
  ZB_CHECK ZB_INLINE vector_type& get_internal_vector() noexcept { return (vector_type&)*this; }

  /// @brief Returns a constant reference to the internal vector.
  /// @return A constant reference to the internal vector used by the stack.
  ZB_CHECK ZB_INLINE const vector_type& get_internal_vector() const noexcept {
    return (const vector_type&)*this;
  }

private:
  size_type _base_idx; ///< The base index of the stack.
  size_type _top_idx; ///< The top index of the stack.

  template <class U>
  friend class stack_view;
};

template <class T>
class stack_view : public __zb::span<T> {
public:
  using value_type = T;
  using span_type = __zb::span<T>;
  using size_type = size_t;
  using difference_type = typename span_type::difference_type;
  using reference = typename span_type::reference;
  using const_reference = typename span_type::const_reference;

  inline explicit stack_view() {}

  template <class Allocator = std::allocator<T>, bool CanResize = true>
  inline stack_view(execution_stack<T, Allocator, CanResize>& fstack) noexcept
      : span_type(fstack.data() + fstack.get_stack_base(), fstack.stack_size()) {}

  ZB_CHECK inline reference top() noexcept {
    zbase_assert(!span_type::empty(), "top out of bounds");
    return span_type::operator[](span_type::size() - 1);
  }

  ZB_CHECK inline size_type stack_size() const noexcept { return span_type::size(); }

  ZB_CHECK inline size_type stack_index(difference_type n) const noexcept {
    if (n >= 0) {
      zbase_assert(n < (difference_type)span_type::size(), "invalid stack index");
      return n;
    }

    difference_type next_idx = span_type::size() + n;

    // n is negative here.
    zbase_assert(next_idx >= 0 and next_idx < (difference_type)span_type::size(), "invalid stack index");
    return (size_type)next_idx;
  }

  ZB_CHECK inline reference operator[](difference_type n) noexcept {
    n = stack_index(n);
    zbase_assert(n < (difference_type)span_type::size());
    return span_type::operator[](n);
  }

  ZB_CHECK inline const_reference operator[](difference_type n) const noexcept {
    n = stack_index(n);
    zbase_assert(n < (difference_type)span_type::size());
    return span_type::operator[](n);
  }

  ZB_CHECK inline reference get_at(size_type n) noexcept {
    zbase_assert(n < span_type::size(), "out of bounds index");
    return span_type::operator[](n);
  }
};

template <class T, class Allocator, bool CanResize>
inline __zb::stack_view<T> execution_stack<T, Allocator, CanResize>::get_stack_view() noexcept {
  return __zb::stack_view<T>(*this);
}

ZBASE_END_NAMESPACE
