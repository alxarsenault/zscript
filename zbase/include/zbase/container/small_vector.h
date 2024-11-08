// Copyright(c) 2024, Meta-Sonic.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.  See the file COPYING included with
// this distribution for more information.
//
// Alternatively, if you have a valid commercial licence for aulib obtained
// by agreement with the copyright holders, you may redistribute and/or modify
// it under the terms described in that licence.
//
// If you wish to distribute code using aulib under terms other than those of
// the GNU General Public License, you must obtain a valid commercial licence
// before doing so.

#pragma once

#include <zbase/zbase.h>
#include <zbase/sys/assert.h>
#include <zbase/utility/compressed_pair.h>
#include <zbase/memory/memory.h>
#include <zbase/utility/math.h>
#include <algorithm>
#include <iterator>
#include <type_traits>

ZBASE_BEGIN_NAMESPACE

namespace detail {

/// Returns the next power of two (in 64-bits) that is strictly greater than
/// A. Returns zero on overflow.
// inline constexpr uint64_t next_power_of_two(uint64_t v) noexcept {
//   v |= (v >> 1);
//   v |= (v >> 2);
//   v |= (v >> 4);
//   v |= (v >> 8);
//   v |= (v >> 16);
//   v |= (v >> 32);
//   return v + 1;
// }

///

template <class _T>
inline void copy_element(_T& dst, cref_t<_T> src) noexcept {
  if constexpr (__zb::is_trivial_cref_v<_T>) {
    dst = src;
  }
  else {
    __zb::mem_copy(&dst, &src, 1);
  }
}

template <class _T>
inline void move_element(_T& dst, _T&& src) noexcept {
  if constexpr (__zb::is_trivial_cref_v<_T>) {
    dst = src;
  }
  else {
    __zb::mem_copy(&dst, &src, 1);
  }
}

template <class _T, class _Allocator>
class small_vector_base_content {
public:
  using value_type = _T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using reference = value_type&;
  using const_reference = __zb::cref_t<value_type>;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using allocator_type = _Allocator;

  [[nodiscard]] inline constexpr allocator_type get_allocator() const noexcept { return _p_data.second(); }

  [[nodiscard]] inline constexpr size_t size() const noexcept { return _size; }

  [[nodiscard]] inline static constexpr size_t max_size() noexcept {
    return (std::numeric_limits<size_type>::max)() / sizeof(value_type);
  }

  [[nodiscard]] inline constexpr size_t capacity() const noexcept { return _capacity; }

  [[nodiscard]] inline constexpr bool empty() const noexcept { return _size == 0; }

  [[nodiscard]] inline constexpr bool is_full() const noexcept { return _size == capacity(); }

  [[nodiscard]] inline static constexpr bool is_resizable() noexcept { return true; }

  [[nodiscard]] inline constexpr pointer data() noexcept { return (pointer)_p_data.first(); }

  [[nodiscard]] inline constexpr const_pointer data() const noexcept {
    return (const_pointer)_p_data.first();
  }

  [[nodiscard]] inline constexpr pointer data(size_type offset) noexcept {
    zbase_assert(data() && "access nullptr");
    zbase_assert(offset < this->size() && "offset out of bounds");
    return data() + offset;
  }

  [[nodiscard]] inline constexpr const_pointer data(size_type offset) const noexcept {
    zbase_assert(data() && "access nullptr");
    zbase_assert(offset < this->size() && "offset out of bounds");
    return data() + offset;
  }

  template <size_t I>
  [[nodiscard]] inline constexpr pointer data() noexcept {
    zbase_assert(data() && "access nullptr");
    zbase_assert(I < this->size() && "offset out of bounds");
    return data() + I;
  }

  template <size_t I>
  [[nodiscard]] inline constexpr const_pointer data() const noexcept {
    zbase_assert(data() && "access nullptr");
    zbase_assert(I < this->size() && "offset out of bounds");
    return data() + I;
  }

  [[nodiscard]] inline constexpr iterator begin() noexcept { return iterator(data()); }

  [[nodiscard]] inline constexpr const_iterator begin() const noexcept { return const_iterator(data()); }

  [[nodiscard]] inline constexpr iterator end() noexcept { return iterator(data() + this->size()); }

  [[nodiscard]] inline constexpr const_iterator end() const noexcept {
    return const_iterator(data() + this->size());
  }

  [[nodiscard]] inline constexpr const_iterator cbegin() const noexcept { return begin(); }

  [[nodiscard]] inline constexpr const_iterator cend() const noexcept { return end(); }

  inline constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  inline constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
  inline constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  inline constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

  [[nodiscard]] inline constexpr reference front() noexcept { return data()[0]; }

  [[nodiscard]] inline constexpr const_reference front() const noexcept { return data()[0]; }

  [[nodiscard]] inline constexpr reference back() noexcept { return data()[this->size() - 1]; }

  [[nodiscard]] inline constexpr const_reference back() const noexcept { return data()[this->size() - 1]; }

  [[nodiscard]] inline constexpr reference operator[](size_type index) noexcept { return data()[index]; }

  [[nodiscard]] inline constexpr const_reference operator[](size_type index) const noexcept {
    return data()[index];
  }

  [[nodiscard]] inline constexpr reference reversed(size_type index) noexcept {
    zbase_assert(index < this->size() && "index out of bounds");
    return data()[this->size() - index - 1];
  }

  [[nodiscard]] inline constexpr const_reference reversed(size_type index) const noexcept {
    zbase_assert(index < this->size() && "index out of bounds");
    return data()[this->size() - index - 1];
  }

  inline iterator find(const value_type& v) noexcept { return std::find(this->begin(), this->end(), v); }

  inline const_iterator find(const value_type& v) const noexcept {
    return std::find(this->begin(), this->end(), v);
  }

  template <class UnaryPred>
  inline iterator find_if(UnaryPred&& fct) noexcept {
    return std::find_if(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  template <class UnaryPred>
  inline const_iterator find_if(UnaryPred&& fct) const noexcept {
    return std::find_if(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  template <class UnaryPred>
  inline iterator find_if_not(UnaryPred&& fct) noexcept {
    return std::find_if_not(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  template <class UnaryPred>
  inline const_iterator find_if_not(UnaryPred&& fct) const noexcept {
    return std::find_if_not(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  //
  //
  //

  inline iterator pfind(const value_type& v) noexcept {
    iterator it = std::find(this->begin(), this->end(), v);
    return it == this->end() ? nullptr : &(*it);
  }

  inline const_iterator pfind(const value_type& v) const noexcept {
    const_iterator it = std::find(this->begin(), this->end(), v);
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  inline iterator pfind_if(UnaryPred&& fct) noexcept {
    iterator it = std::find_if(this->begin(), this->end(), std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  inline const_iterator pfind_if(UnaryPred&& fct) const noexcept {
    iterator it = std::find_if(this->begin(), this->end(), std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  inline iterator pfind_if_not(UnaryPred&& fct) noexcept {
    iterator it = std::find_if_not(this->begin(), this->end(), std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  inline const_iterator pfind_if_not(UnaryPred&& fct) const noexcept {
    iterator it = std::find_if_not(this->begin(), this->end(), std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  inline bool contains(const value_type& v) const noexcept {
    return std::find(this->begin(), this->end(), v) != this->end();
  }

protected:
  small_vector_base_content(pointer dat, size_t cap, const allocator_type& alloc)
      : _p_data({ (void*)dat, alloc })
      , _size(0)
      , _capacity(cap) {}

  __zb::compressed_pair<void*, _Allocator> _p_data;
  size_t _size;
  size_type _capacity;
};

template <class _Allocator>
class dummy_small_vector_base_content {
public:
  inline dummy_small_vector_base_content(const _Allocator& alloc)
      : _p_data(nullptr, alloc) {}

  __zb::compressed_pair<void*, _Allocator> _p_data;
  size_t _size;
  size_t _capacity;
};

template <typename _T, class _Allocator>
class dummy_small_vector_base : public dummy_small_vector_base_content<_Allocator> {
public:
  using base_content = dummy_small_vector_base_content<_Allocator>;

  inline dummy_small_vector_base(const _Allocator& alloc)
      : base_content(alloc) {}
};

template <typename _T, class _Allocator>
class dummy_small_vector {
public:
  inline dummy_small_vector(const _Allocator& alloc)
      : content(alloc) {}

  dummy_small_vector_base_content<_Allocator> content;
  alignas(alignof(_T)) __zb::aligned_type_storage<_T> _sdata[1];
};

template <typename _T, class _Allocator>
inline constexpr size_t small_vector_stack_buffer_offset
    = offsetof(decltype(dummy_small_vector<_T, _Allocator>{ std::declval<_Allocator>() }), _sdata);
} // namespace detail

// template <class _T>
// struct allocator {
//   static inline void* allocate(size_t size) noexcept { return ::malloc(size); }
//   static inline void deallocate(void* ptr) noexcept { ::free(ptr); }
// };

template <typename _T, class _Allocator = std::allocator<_T>>
class small_vector_base : public detail::small_vector_base_content<_T, _Allocator> {
  using base_content = detail::small_vector_base_content<_T, _Allocator>;

public:
  using value_type = _T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using reference = value_type&;
  using const_reference = __zb::cref_t<value_type>;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using allocator_type = _Allocator;

  using base_content::back;
  using base_content::begin;
  using base_content::capacity;
  using base_content::cbegin;
  using base_content::cend;
  using base_content::data;
  using base_content::empty;
  using base_content::end;
  using base_content::front;
  using base_content::size;
  using base_content::operator[];

protected:
  inline small_vector_base(pointer dat, size_t cap, const allocator_type& alloc) noexcept
      : base_content(dat, cap, alloc) {}

  inline small_vector_base(const small_vector_base&) noexcept = default;
  inline small_vector_base(small_vector_base&&) noexcept = default;

  inline small_vector_base& operator=(const small_vector_base&) noexcept = default;
  inline small_vector_base& operator=(small_vector_base&&) noexcept = default;

  //  ZB_CHECK inline pointer get_stack_pointer() noexcept {
  //    uint8_t* s = reinterpret_cast<uint8_t*>(this);
  //    s += sizeof(small_vector_base);
  //    static_assert(sizeof(small_vector_base) == detail::small_vector_stack_buffer_offset<value_type,
  //    _Allocator>, "error"); return pointer(s);
  //  }

  ZB_CHECK inline const_pointer get_stack_pointer() const noexcept {
    const uint8_t* s = reinterpret_cast<const uint8_t*>(this);
    s += sizeof(small_vector_base);
    static_assert(
        sizeof(small_vector_base) == detail::small_vector_stack_buffer_offset<value_type, _Allocator>,
        "error");
    return const_pointer(s);
  }

public:
  ZB_CHECK inline constexpr bool has_stack_data() const noexcept {
    return _p_data.first() == get_stack_pointer();
  }

  ZB_CHECK inline constexpr bool has_allocated_data() const noexcept {
    return _p_data.first() != get_stack_pointer();
  }

  inline ~small_vector_base() noexcept {
    if (_p_data.first()) {

      __zb::destruct_range(data(), size());

      if (_p_data.first() != get_stack_pointer()) {
        _p_data.second().deallocate((_T*)_p_data.first(), size());
      }
    }
  }

  void reserve(size_type rsize) noexcept {

    if (rsize < capacity()) {
      return;
    }

    if (grow(__zb::next_power_of_two(rsize))) {
      return;
    }

    zbase_error("LLL");
  }

  inline constexpr void clear() noexcept {

    if (_p_data.first()) {

      if constexpr (!std::is_trivially_destructible_v<_T>) {
        for (size_t i = 0; i < size(); i++) {
          ((_T*)_p_data.first())[i].~_T();
        }
      }
    }

    _size = 0;
  }

  inline constexpr void push_back(const_reference value) noexcept {
    if (size() < capacity()) {
      if constexpr (std::is_trivially_copyable_v<_T>) {
        detail::copy_element((*this)[_size++], value);
      }
      else {
        zb_placement_new(this->data() + _size++) value_type(value);
      }

      return;
    }

    if (grow(__zb::next_power_of_two(capacity()))) {
      if constexpr (std::is_trivially_copyable_v<_T>) {
        detail::copy_element((*this)[_size++], value);
      }
      else {
        zb_placement_new(this->data() + _size++) value_type(value);
      }

      return;
    }
  }

  template <class U = value_type,
      std::enable_if_t<!__zb::is_trivial_cref_v<U> && std::is_same_v<U, value_type>, int> = 0>
  inline constexpr void push_back(U&& value) noexcept {
    if (size() < capacity()) {
      __zb::move_construct_element(*(this->data() + _size++), std::forward<U>(value));
      return;
    }

    if (grow(__zb::next_power_of_two(capacity()))) {
      if constexpr (std::is_trivially_copyable_v<_T>) {
        detail::move_element((*this)[_size++], std::forward<U>(value));
      }
      else {
        zb_placement_new(this->data() + _size++) value_type(std::forward<U>(value));
      }

      return;
    }
  }

  template <typename... _Args>
  inline constexpr reference emplace_back(_Args&&... args) noexcept {
    if (size() < capacity()) {
      if constexpr (std::is_trivially_copyable_v<_T>) {
        detail::move_element((*this)[_size++], std::forward<_Args>(args)...);
      }
      else {
        zb_placement_new(this->data() + _size++) value_type(std::forward<_Args>(args)...);
      }

      return this->back();
    }

    if (grow(__zb::next_power_of_two(capacity()))) {
      if constexpr (std::is_trivially_copyable_v<_T>) {
        detail::move_element((*this)[_size++], std::forward<_Args>(args)...);
      }
      else {
        zb_placement_new(this->data() + _size++) value_type(std::forward<_Args>(args)...);
      }

      return this->back();
    }

    return this->back();
  }

  inline constexpr iterator insert(const_iterator pos, const_reference value) noexcept {
    if (!_size || pos == cend()) {
      push_back(value);
      return end() - 1;
    }

    if (size() < capacity()) {
      const size_type index = std::distance(cbegin(), pos);

      if constexpr (std::is_trivially_copyable_v<_T>) {
        ::memmove(begin() + index + 1, begin() + index, (_size - index) * sizeof(value_type));
        detail::copy_element((*this)[index], value);
      }
      else {
        zb_placement_new(end()) value_type(std::move(back()));

        for (size_type i = size() - 1; i > index; i--) {
          detail::move_element((*this)[i], std::move((*this)[i - 1]));
        }

        detail::copy_element((*this)[index], value);
      }

      _size++;
      return begin() + index;
    }

    const size_type index = std::distance(cbegin(), pos);

    if (!grow(__zb::next_power_of_two(capacity()),
            [&](pointer ndata, pointer data, size_type size, size_type new_capacity) {
              __zb::relocate(ndata, data, index);
              __zb::relocate(ndata + index + 1, data + index, size - index);
              zb_placement_new(ndata + index) value_type(value);
            })) {

      zbase_error("DSDS");
      return end();
    }

    _size++;

    return begin() + index;
  }

  template <class U = value_type, std::enable_if_t<!__zb::is_trivial_cref_v<U>, int> = 0>
  inline constexpr iterator insert(const_iterator pos, U&& value) noexcept {
    if (!_size || pos == cend()) {
      push_back(std::move(value));
      return end() - 1;
    }

    if (size() < capacity()) {
      const size_type index = std::distance(cbegin(), pos);

      if constexpr (std::is_trivially_copyable_v<_T>) {
        ::memmove(begin() + index + 1, begin() + index, (_size - index) * sizeof(value_type));
        detail::copy_element((*this)[index], value);
      }
      else {
        zb_placement_new(end()) value_type(std::move(back()));

        for (size_type i = size() - 1; i > index; i--) {
          detail::move_element((*this)[i], std::move((*this)[i - 1]));
        }

        detail::move_element((*this)[index], std::move(value));
      }

      _size++;
      return begin() + index;
    }

    const size_type index = std::distance(cbegin(), pos);

    if (!grow(__zb::next_power_of_two(capacity()),
            [&](pointer ndata, pointer data, size_type size, size_type new_capacity) {
              __zb::relocate(ndata, data, index);
              __zb::relocate(ndata + index + 1, data + index, size - index);
              zb_placement_new(ndata + index) value_type(std::move(value));
            })) {

      zbase_error("DSDS");
      return end();
    }

    _size++;
    return begin() + index;
  }

  inline constexpr void pop_back() noexcept {
    zbase_assert(_size > 0 && "Can't pop_back an empty fixed_vector.");
    if constexpr (!std::is_trivially_destructible_v<value_type>) {
      (*this)[_size].~value_type();
    }
    --_size;
  }

  inline constexpr void erase_at(size_type index) noexcept {
    zbase_assert(index < _size && "Try to erase out of bounds index.");

    if (index == _size - 1) {
      pop_back();
      return;
    }

    if constexpr (std::is_trivially_copyable_v<value_type>) {
      ::memmove(begin() + index, begin() + index + 1, (_size - index) * sizeof(value_type));
    }
    else {
      for (size_type i = index; i < size() - 1; i++) {
        (*this)[i] = std::move((*this)[i + 1]);
      }

      if constexpr (!std::is_trivially_destructible_v<value_type>) {
        (*this)[_size].~value_type();
      }
    }

    _size--;
  }

  inline constexpr void erase(const_iterator it) noexcept {
    erase_at((size_type)std::distance(cbegin(), it));
  }

  inline constexpr void unordered_erase(size_type index) noexcept {
    zbase_assert(index < _size && "Try to erase out of bounds index.");

    if (index == _size - 1) {
      pop_back();
      return;
    }

    detail::move_element((*this)[index], std::move(this->back()));

    if constexpr (!std::is_trivially_destructible_v<value_type>) {
      (*this)[_size].~value_type();
    }
    _size--;
  }

  inline constexpr void resize(size_type count) noexcept {
    if (count <= size()) {
      if constexpr (!std::is_trivially_destructible_v<value_type>) {
        for (size_type i = count; i < size(); i++) {
          (*this)[i].~value_type();
        }
      }
      _size = count;
      return;
    }

    if (count <= capacity()) {
      if constexpr (!std::is_trivially_default_constructible_v<value_type>) {
        for (size_type i = size(); i < count; i++) {
          zb_placement_new(data() + i) value_type();
        }
      }

      _size = count;

      return;
    }

    // TODO: What if, the current capacity is small and the count if a power of two?
    // (It will pick the next power of two which is most likely not wanted).

    if (!grow(__zb::next_power_of_two(count))) {
      zbase_assert(count <= capacity());
      return;
    }

    if constexpr (!std::is_trivially_default_constructible_v<value_type>) {
      for (size_type i = size(); i < count; i++) {
        zb_placement_new(data() + i) value_type();
      }
    }

    _size = count;
  }

  inline constexpr void resize(size_type count, const_reference value) noexcept {
    if (count <= size()) {
      __zb::destruct_range(data(), size());

      _size = count;
      return;
    }

    if (count <= capacity()) {
      if constexpr (std::is_trivially_copyable_v<value_type>) {
        for (size_type i = size(); i < count; i++) {
          detail::copy_element((*this)[i], value);
        }
      }
      else {
        for (size_type i = size(); i < count; i++) {
          zb_placement_new(data() + i) value_type(value);
        }
      }

      _size = count;

      return;
    }

    if (!grow(__zb::next_power_of_two(count))) {
      zbase_assert(count <= capacity());
      return;
    }

    if constexpr (std::is_trivially_copyable_v<value_type>) {
      for (size_type i = size(); i < count; i++) {
        detail::copy_element((*this)[i], value);
      }
    }
    else {
      for (size_type i = size(); i < count; i++) {
        zb_placement_new(data() + i) value_type(value);
      }
    }

    _size = count;
  }

protected:
  ZB_CHECK inline bool grow(size_t new_capacity) noexcept {
    return grow(new_capacity, [](pointer ndata, pointer data, size_type size, size_type new_capacity) {
      __zb::relocate(ndata, data, size);
    });
  }

  template <class _Mod>
  ZB_CHECK inline bool grow(size_t new_capacity, _Mod mod) noexcept {
    _T* tmp_data = (_T*)_p_data.second().allocate(new_capacity);

    if (!tmp_data) {
      return false;
    }

    _capacity = new_capacity;

    if (_p_data.first()) {
      mod(tmp_data, (pointer)_p_data.first(), _size, new_capacity);

      if (has_allocated_data()) {

        _p_data.second().deallocate((_T*)_p_data.first(), _size);
      }
    }

    _p_data.first() = tmp_data;

    return true;
  }

  using base_content::_capacity;
  using base_content::_p_data;
  using base_content::_size;
};

///////////////////////////////
///
///
///
///
////////////////////////////////

template <typename _T, size_t _Size, class _Allocator = std::allocator<_T>>
class small_vector : public small_vector_base<_T, _Allocator> {

  using base = small_vector_base<_T, _Allocator>;

  template <class, size_t, class>
  friend class small_vector;

public:
  using value_type = _T;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using reference = value_type&;
  using const_reference = __zb::cref_t<value_type>;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using allocator_type = _Allocator;

  using base::back;
  using base::begin;
  using base::capacity;
  using base::cbegin;
  using base::cend;
  using base::data;
  using base::empty;
  using base::end;
  using base::front;
  using base::size;
  using base::operator[];

  inline small_vector() noexcept
      : base((_T*)&_sdata[0], _Size, _Allocator{}) {}

  inline small_vector(size_t sz) noexcept
      : base((_T*)&_sdata[0], _Size, _Allocator{}) {
    this->resize(sz);
  }

  explicit inline small_vector(const allocator_type& alloc) noexcept
      : base((_T*)&_sdata[0], _Size, alloc) {}

  explicit inline small_vector(size_t sz, const allocator_type& alloc) noexcept
      : base((_T*)&_sdata[0], _Size, alloc) {
    this->resize(sz);
  }

protected:
  struct copy_tag {};

  template <size_t _RhsSize>
  inline small_vector(const small_vector<_T, _RhsSize, _Allocator>& vec, copy_tag) noexcept
      : base((_T*)&_sdata[0], _Size, vec.get_allocator()) {
    if (vec.size() <= _Size) {
      __zb::copy_construct_range(data(), vec.data(), vec.size());
      _size = vec.size();
    }
    else {
      if (base::grow(vec.size())) {
        __zb::copy_construct_range(data(), vec.data(), vec.size());
        _size = vec.size();
      }
      else {
        zbase_error("CALADLKLADKA");
      }
    }
  }

public:
  inline small_vector(const small_vector& vec) noexcept
      : small_vector(vec, copy_tag{}) {}

  template <size_t _RhsSize>
  inline small_vector(const small_vector<_T, _RhsSize, _Allocator>& vec) noexcept
      : small_vector(vec, copy_tag{}) {}

  inline small_vector(small_vector&& vec) noexcept
      : base((_T*)&_sdata[0], _Size, vec.get_allocator()) {

    if (vec.has_stack_data()) {
      //      detail::copy_construct_range(data(), vec.data(), vec.size());
      __zb::relocate(data(), vec.data(), vec.size());
      _size = vec.size();
      vec._size = 0;
    }
    else {
      _p_data = vec._p_data;
      _capacity = vec.capacity();
      _size = vec._size;

      vec._p_data.first() = (pointer)&vec._sdata[0];
      vec._capacity = _Size;
      vec._size = 0;
    }
  }

  template <size_t _RhsSize>
  inline small_vector(small_vector<_T, _RhsSize, _Allocator>&& vec) noexcept
      : base((_T*)&_sdata[0], _Size, vec.get_allocator()) {

    if (vec.size() <= capacity()) {
      //      detail::move_construct_range(data(), vec.data(), vec.size());
      //      _size = vec.size();
      //      vec.reset();

      __zb::relocate(data(), vec.data(), vec.size());
      _size = vec.size();
      vec._size = 0;

      return;
    }

    if (vec.has_stack_data()) {
      if (base::grow(vec.size())) {
        //        detail::move_construct_range(data(), vec.data(), vec.size());
        //        _size = vec.size();
        //        vec.reset();

        __zb::relocate(data(), vec.data(), vec.size());
        _size = vec.size();
        vec._size = 0;

        return;
      }

      zbase_error("CALADLKLADKA");

      return;
    }

    _p_data = vec._p_data;
    _capacity = vec.capacity();
    _size = vec._size;

    vec._p_data.first() = (pointer)&vec._sdata[0];
    vec._capacity = _RhsSize;
    vec._size = 0;
  }

  inline ~small_vector() noexcept = default;

  template <size_t _RhsSize>
  small_vector& copy(const small_vector<_T, _RhsSize, _Allocator>& vec) noexcept {
    if constexpr (_RhsSize == _Size) {
      if (&vec == this) {
        return *this;
      }
    }

    // If other vector is empty we clear the memory.
    if (vec.empty()) {
      reset();
      return *this;
    }

    // If we have enough capacity we don't delete the memory.
    if (vec.size() <= capacity()) {
      if (!empty()) {
        __zb::destruct_range(data(), size());
      }

      __zb::copy_construct_range(data(), vec.data(), vec.size());
      _size = vec.size();

      return *this;
    }

    reset();

    if (base::grow(vec.size())) {
      __zb::copy_construct_range(data(), vec.data(), vec.size());
      _size = vec.size();
      return *this;
    }

    zbase_error("CALADLKLADKA");

    return *this;
  }

  inline small_vector& operator=(const small_vector& vec) noexcept { return copy(vec); }

  template <size_t _RhsSize>
  inline small_vector& operator=(const small_vector<_T, _RhsSize, _Allocator>& vec) noexcept {
    return copy(vec);
  }

  inline small_vector& operator=(small_vector&& vec) noexcept {
    if (&vec == this) {
      return *this;
    }

    reset();

    if (vec.has_stack_data()) {
      __zb::move_construct_range(data(), vec.data(), vec.size());
      _size = vec.size();
    }
    else {
      _p_data = vec._p_data;
      _capacity = vec.capacity();

      _size = vec._size;

      vec._p_data.first() = (pointer)&vec._sdata[0];
      vec._capacity = _Size;

      vec._size = 0;
    }
    return *this;
  }

  template <size_t _RhsSize>
  inline small_vector& operator=(small_vector<_T, _RhsSize, _Allocator>&& vec) noexcept {

    reset();

    if (vec.empty()) {
      return *this;
    }

    if (vec.size() <= capacity()) {
      __zb::move_construct_range(data(), vec.data(), vec.size());
      _size = vec.size();
      vec.reset();
      return *this;
    }

    if (vec.has_stack_data()) {
      if (base::grow(vec.size())) {
        __zb::move_construct_range(data(), vec.data(), vec.size());
        _size = vec.size();
        vec.reset();
        return *this;
      }

      zbase_error("CALADLKLADKA");

      return *this;
    }

    _p_data = vec._p_data;
    _capacity = vec.capacity();
    _size = vec._size;

    vec._p_data.first() = (pointer)&vec._sdata[0];
    vec._capacity = _RhsSize;
    vec._size = 0;

    return *this;
  }

  inline void reset() {
    if (_p_data.first()) {
      __zb::destruct_range(data(), size());

      if (base::has_allocated_data()) {

        _p_data.second().deallocate((_T*)_p_data.first(), size());
      }
    }

    _p_data.first() = (pointer)&_sdata[0];
    _size = 0;
    _capacity = _Size;
  }

private:
  using base::_capacity;
  using base::_p_data;
  using base::_size;

  ZBASE_PRAGMA_PUSH()
  ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wzero-length-array")
  alignas(alignof(_T)) __zb::aligned_type_storage<_T> _sdata[_Size];
  ZBASE_PRAGMA_POP()
};

// is_small_vector
template <class _T>
struct is_small_vector : std::false_type {};

template <class _T, size_t _Size, class _Allocator>
struct is_small_vector<small_vector<_T, _Size, _Allocator>> : std::true_type {};

ZBASE_END_NAMESPACE
