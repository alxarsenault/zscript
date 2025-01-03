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

#pragma once

#include <zscript/base/zbase.h>
#include <zscript/base/sys/assert.h>
#include <algorithm>
#include <span>
#include <vector>

ZBASE_BEGIN_NAMESPACE

template <class Container>
class container_adapter : public Container {
public:
  using container_type = Container;
  using value_type = typename container_type::value_type;
  using size_type = typename container_type::size_type;
  using difference_type = typename container_type::difference_type;
  using pointer = typename container_type::pointer;
  using const_pointer = typename container_type::const_pointer;
  using reference = typename container_type::reference;
  using const_reference = typename container_type::const_reference;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  static constexpr const size_type npos = -1;

  using container_type::data;
  using container_type::size;
  using container_type::container_type;
  using container_type::operator[];

  ZB_CHECK inline std::span<value_type> subspan(
      size_type offset, size_type count = std::dynamic_extent) noexcept {
    zbase_assert(offset <= size(), "zb::vector<T>::subspan(offset, count): offset out of range");

    if (count == std::dynamic_extent) {
      return std::span<value_type>{ data() + offset, size() - offset };
    }

    zbase_assert(
        count <= size() - offset, "zb::vector<T>::subspan(offset, count): offset + count out of range");
    return std::span<value_type>(data() + offset, count);
  }

  ZB_CHECK inline std::span<const value_type> subspan(
      size_type offset, size_type count = std::dynamic_extent) const noexcept {
    zbase_assert(offset <= size(), "zb::vector<T>::subspan(offset, count): offset out of range");

    if (count == std::dynamic_extent) {
      return std::span<const value_type>{ data() + offset, size() - offset };
    }

    zbase_assert(
        count <= size() - offset, "zb::vector<T>::subspan(offset, count): offset + count out of range");
    return std::span<const value_type>(data() + offset, count);
  }

  ZB_CHECK ZB_INLINE pointer data(size_type index) noexcept { return data() + index; }
  ZB_CHECK ZB_INLINE const_pointer data(size_type index) const noexcept { return data() + index; }

  ZB_CK_INLINE bool is_index_in_range(int64_t idx) const noexcept {
    return idx >= 0 and idx < (int64_t)this->size();
  }

  //
  // Find iterator.
  //

  ZB_CHECK ZB_INLINE bool contains(const value_type& v) const noexcept {
    return std::find(this->begin(), this->end(), v) != this->end();
  }

  template <class Val>
  ZB_CHECK ZB_INLINE bool contains(const Val& v) const noexcept {
    return std::find_if(this->begin(), this->end(), [&](const value_type& item) { return item == v; })
        != this->end();
  }

  ZB_CHECK ZB_INLINE iterator find(const value_type& v) noexcept {
    return std::find(this->begin(), this->end(), v);
  }

  ZB_CHECK ZB_INLINE const_iterator find(const value_type& v) const noexcept {
    return std::find(this->begin(), this->end(), v);
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE iterator find_if(UnaryPred&& fct) noexcept {
    return std::find_if(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE const_iterator find_if(UnaryPred&& fct) const noexcept {
    return std::find_if(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE iterator find_if_not(UnaryPred&& fct) noexcept {
    return std::find_if_not(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE const_iterator find_if_not(UnaryPred&& fct) const noexcept {
    return std::find_if_not(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  //
  // Find pointer.
  //

  ZB_CHECK ZB_INLINE pointer pfind(const value_type& v) noexcept {
    iterator it = this->find(v);
    return it == this->end() ? nullptr : &(*it);
  }

  ZB_CHECK ZB_INLINE const_pointer pfind(const value_type& v) const noexcept {
    const_iterator it = this->find(v);
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE pointer pfind_if(UnaryPred&& fct) noexcept {
    iterator it = this->find_if(std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE const_pointer pfind_if(UnaryPred&& fct) const noexcept {
    const_iterator it = this->find_if(std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE pointer pfind_if_not(UnaryPred&& fct) noexcept {
    iterator it = this->find_if_not(std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE const_pointer pfind_if_not(UnaryPred&& fct) const noexcept {
    const_iterator it = this->find_if_not(std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  //
  // Find index.
  //

  ZB_CHECK ZB_INLINE size_type ifind(const value_type& v) const noexcept {
    const_iterator it = this->find(v);
    return it == this->end() ? npos : std::distance(this->begin(), it);
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE size_type ifind_if(UnaryPred&& fct) const noexcept {
    const_iterator it = this->find_if(std::forward<UnaryPred>(fct));
    return it == this->end() ? npos : std::distance(this->begin(), it);
  }

  template <class UnaryPred>
  ZB_CHECK ZB_INLINE size_type ifind_if_not(UnaryPred&& fct) const noexcept {
    const_iterator it = this->find_if_not(std::forward<UnaryPred>(fct));
    return it == this->end() ? npos : std::distance(this->begin(), it);
  }

  //
  // Visitor.
  //

  template <class Operator>
  inline void visit(Operator&& oper) {
    const size_type sz = size();
    for (size_type i = 0; i < sz; i++) {
      oper(container_type::operator[](i));
    }
  }

  template <class Operator, class Predicate>
  inline void visit_if(Operator&& oper, Predicate&& predicate) {
    const size_type sz = size();
    for (size_type i = 0; i < sz; i++) {
      if (predicate(container_type::operator[](i))) {
        oper(container_type::operator[](i));
      }
    }
  }

  template <class Operator, class Predicate>
  inline void visit_first_if(Operator&& oper, Predicate&& predicate) {
    const size_type sz = size();

    for (size_type i = 0; i < sz; i++) {
      if (predicate(container_type::operator[](i))) {
        oper(container_type::operator[](i));
        return;
      }
    }
  }

  ZB_CHECK ZB_INLINE value_type get_pop_back() noexcept {
    value_type v = std::move(container_type::back());
    container_type::pop_back();
    return v;
  }

  inline void erase_last_n(size_t n) noexcept {
    if (container_type::size() >= n) {
      container_type::resize(container_type::size() - n);
    }
  }

  inline void unordered_erase(size_type index) {
    const size_type sz = size();
    zbase_assert(index < sz, "Try to erase out of bounds index.");

    if (sz == 0) {
      return;
    }

    if (index == sz - 1) {
      this->pop_back();
      return;
    }

    this->operator[](index) = std::move(this->back());
    this->pop_back();
  }

  template <class Predicate>
  inline void unordered_erase_if(Predicate&& predicate) {
    for (size_type i = 0; i < size();) {
      if (predicate(container_type::operator[](i))) {
        unordered_erase(i);
        continue;
      }

      i++;
    }
  }

  template <class Predicate>
  inline void unordered_erase_first_if(Predicate&& predicate) {
    for (size_type i = 0; i < size(); i++) {
      if (predicate(container_type::operator[](i))) {
        unordered_erase(i);
        return;
      }
    }
  }
};

template <class T, class _Allocator = std::allocator<T>>
using vector = zb::container_adapter<std::vector<T, _Allocator>>;

// template <class T, class _Allocator = std::allocator<T>>
// using vector =  std::vector<T, _Allocator> ;

// template <class T, class _Allocator = std::allocator<T>>
// class vector : public std::vector<T, _Allocator> {
// public:
//   using vector_type = std::vector<T, _Allocator>;
//   using value_type = typename vector_type::value_type;
//   using size_type = typename vector_type::size_type;
//   using difference_type = typename vector_type::difference_type;
//   using pointer = typename vector_type::pointer;
//   using const_pointer = typename vector_type::const_pointer;
//   using reference = typename vector_type::reference;
//   using const_reference = typename vector_type::const_reference;
//   using iterator = typename vector_type::iterator;
//   using const_iterator = typename vector_type::const_iterator;
//   using allocator_type = typename vector_type::allocator_type;
//
//   static constexpr const size_type npos = -1;
//
//   using vector_type::data;
//   using vector_type::size;
//   using vector_type::vector_type;
//   using vector_type::operator[];
//
//   ZB_INLINE vector(const allocator_type& alloc)
//       : vector_type(alloc) {}
//
//   ZB_INLINE vector(const vector&) = default;
//   ZB_INLINE vector(vector&&) = default;
//
//   template <class Alloc, std::enable_if_t<!std::is_same_v<_Allocator, Alloc>, int> = 0>
//   ZB_INLINE vector(const std::vector<value_type, Alloc>& vec)
//       : vector(vec.begin(), vec.end()) {}
//
//   template <class Alloc, std::enable_if_t<!std::is_same_v<_Allocator, Alloc>, int> = 0>
//   ZB_INLINE vector(const __zb::vector<value_type, Alloc>& vec)
//       : vector(vec.begin(), vec.end()) {}
//
//   template <class Alloc, std::enable_if_t<std::is_same_v<_Allocator, Alloc>, int> = 0>
//   ZB_INLINE vector(const std::vector<value_type, Alloc>& vec)
//       : vector(vec.begin(), vec.end(), vec.get_allocator()) {}
//
//   template <class Alloc, std::enable_if_t<std::is_same_v<_Allocator, Alloc>, int> = 0>
//   ZB_INLINE vector(const __zb::vector<value_type, Alloc>& vec)
//       : vector(vec.begin(), vec.end(), vec.get_allocator()) {}
//
//   ZB_INLINE vector& operator=(const vector&) = default;
//   ZB_INLINE vector& operator=(vector&&) = default;
//
////  template <class Alloc, std::enable_if_t<!std::is_same_v<_Allocator, Alloc>, int> = 0>
////  ZB_INLINE vector& operator=(const std::vector<value_type, Alloc>& vec) {
////    this->clear();
////    this->insert(this->end(), vec.begin(), vec.end());
////    return *this;
////  }
////
////  template <class Alloc, std::enable_if_t<!std::is_same_v<_Allocator, Alloc>, int> = 0>
////  ZB_INLINE vector& operator=(const __zb::vector<value_type, Alloc>& vec) {
////    this->clear();
////    this->insert(this->end(), vec.begin(), vec.end());
////    return *this;
////  }
////
////  template <class Alloc, std::enable_if_t<std::is_same_v<_Allocator, Alloc>, int> = 0>
////  ZB_INLINE vector& operator=(const std::vector<value_type, Alloc>& vec) {
////    this->clear();
////    this->insert(this->end(), vec.begin(), vec.end());
////    return *this;
////  }
////
////  template <class Alloc, std::enable_if_t<std::is_same_v<_Allocator, Alloc>, int> = 0>
////  ZB_INLINE vector& operator=(const __zb::vector<value_type, Alloc>& vec) {
////    this->clear();
////    this->insert(this->end(), vec.begin(), vec.end());
////    return *this;
////  }
//
//  //  ZB_CHECK ZB_INLINE operator vector_type&() noexcept { return *this; }
//  //  ZB_CHECK ZB_INLINE operator const vector_type&() const noexcept { return *this; }
//
//  ZB_CHECK ZB_INLINE reference operator()(difference_type n) noexcept {
//    const size_t sz = size();
//    zbase_assert(sz, "call operator[] in an empty vector");
//    return vector_type::operator[]((n + sz) % sz);
//  }
//
//  ZB_CHECK ZB_INLINE const_reference operator()(difference_type n) const noexcept {
//    const size_t sz = size();
//    zbase_assert(sz, "call operator[] in an empty vector");
//    return vector_type::operator[]((n + sz) % sz);
//  }
//
//  ZB_CHECK inline std::span<value_type> subspan(
//      size_type offset, size_type count = std::dynamic_extent) noexcept {
//    zbase_assert(offset <= size(), "zb::vector<T>::subspan(offset, count): offset out of range");
//
//    if (count == std::dynamic_extent) {
//      return std::span<value_type>{ data() + offset, size() - offset };
//    }
//
//    zbase_assert(count <= size() - offset, "zb::vector<T>::subspan(offset, count): offset + count out of
//    range"); return std::span<value_type>(data() + offset, count);
//  }
//
//  ZB_CHECK inline std::span<const value_type> subspan(
//      size_type offset, size_type count = std::dynamic_extent) const noexcept {
//    zbase_assert(offset <= size(), "zb::vector<T>::subspan(offset, count): offset out of range");
//
//    if (count == std::dynamic_extent) {
//      return std::span<const value_type>{ data() + offset, size() - offset };
//    }
//
//    zbase_assert(count <= size() - offset, "zb::vector<T>::subspan(offset, count): offset + count out of
//    range"); return std::span<const value_type>(data() + offset, count);
//  }
//
//  ZB_CHECK ZB_INLINE value_type get_pop_back() noexcept {
//    value_type v = std::move(vector_type::back());
//    vector_type::pop_back();
//    return v;
//  }
//
//  ZB_CHECK ZB_INLINE pointer data(size_type index) noexcept { return data() + index; }
//
//  ZB_CHECK ZB_INLINE const_pointer data(size_type index) const noexcept { return data() + index; }
//
//  inline void unordered_erase(size_type index) {
//    const size_type sz = size();
//    zbase_assert(index < sz, "Try to erase out of bounds index.");
//
//    if (sz == 0) {
//      return;
//    }
//
//    if (index == sz - 1) {
//      this->pop_back();
//      return;
//    }
//
//    this->operator[](index) = std::move(this->back());
//    this->pop_back();
//  }
//
//  template <class Predicate>
//  inline void unordered_erase_if(Predicate&& predicate) {
//    for (size_type i = 0; i < size();) {
//      if (predicate(vector_type::operator[](i))) {
//        unordered_erase(i);
//        continue;
//      }
//
//      i++;
//    }
//  }
//
//  template <class Predicate>
//  inline void unordered_erase_first_if(Predicate&& predicate) {
//    for (size_type i = 0; i < size(); i++) {
//      if (predicate(vector_type::operator[](i))) {
//        unordered_erase(i);
//        return;
//      }
//    }
//  }
//
//  //
//  // Find iterator.
//  //
//
//  ZB_CHECK ZB_INLINE iterator find(const value_type& v) noexcept {
//    return std::find(this->begin(), this->end(), v);
//  }
//
//  ZB_CHECK ZB_INLINE const_iterator find(const value_type& v) const noexcept {
//    return std::find(this->begin(), this->end(), v);
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE iterator find_if(UnaryPred&& fct) noexcept {
//    return std::find_if(this->begin(), this->end(), std::forward<UnaryPred>(fct));
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE const_iterator find_if(UnaryPred&& fct) const noexcept {
//    return std::find_if(this->begin(), this->end(), std::forward<UnaryPred>(fct));
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE iterator find_if_not(UnaryPred&& fct) noexcept {
//    return std::find_if_not(this->begin(), this->end(), std::forward<UnaryPred>(fct));
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE const_iterator find_if_not(UnaryPred&& fct) const noexcept {
//    return std::find_if_not(this->begin(), this->end(), std::forward<UnaryPred>(fct));
//  }
//
//  //
//  // Find pointer.
//  //
//
//  ZB_CHECK ZB_INLINE pointer pfind(const value_type& v) noexcept {
//    iterator it = this->find(v);
//    return it == this->end() ? nullptr : &(*it);
//  }
//
//  ZB_CHECK ZB_INLINE const_pointer pfind(const value_type& v) const noexcept {
//    const_iterator it = this->find(v);
//    return it == this->end() ? nullptr : &(*it);
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE pointer pfind_if(UnaryPred&& fct) noexcept {
//    iterator it = this->find_if(std::forward<UnaryPred>(fct));
//    return it == this->end() ? nullptr : &(*it);
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE const_pointer pfind_if(UnaryPred&& fct) const noexcept {
//    const_iterator it = this->find_if(std::forward<UnaryPred>(fct));
//    return it == this->end() ? nullptr : &(*it);
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE pointer pfind_if_not(UnaryPred&& fct) noexcept {
//    iterator it = this->find_if_not(std::forward<UnaryPred>(fct));
//    return it == this->end() ? nullptr : &(*it);
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE const_pointer pfind_if_not(UnaryPred&& fct) const noexcept {
//    const_iterator it = this->find_if_not(std::forward<UnaryPred>(fct));
//    return it == this->end() ? nullptr : &(*it);
//  }
//
//  //
//  // Find index.
//  //
//
//  ZB_CHECK ZB_INLINE size_type ifind(const value_type& v) const noexcept {
//    const_iterator it = this->find(v);
//    return it == this->end() ? npos : std::distance(this->begin(), it);
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE size_type ifind_if(UnaryPred&& fct) const noexcept {
//    iterator it = this->find_if(std::forward<UnaryPred>(fct));
//    return it == this->end() ? npos : std::distance(this->begin(), it);
//  }
//
//  template <class UnaryPred>
//  ZB_CHECK ZB_INLINE size_type ifind_if_not(UnaryPred&& fct) const noexcept {
//    const_iterator it = this->find_if_not(std::forward<UnaryPred>(fct));
//    return it == this->end() ? npos : std::distance(this->begin(), it);
//  }
//
//  //
//  // Visitor.
//  //
//
//  template <class Operator>
//  inline void visit(Operator&& oper) {
//    const size_type sz = size();
//    for (size_type i = 0; i < sz; i++) {
//      oper(vector_type::operator[](i));
//    }
//  }
//
//  template <class Operator, class Predicate>
//  inline void visit_if(Operator&& oper, Predicate&& predicate) {
//    const size_type sz = size();
//    for (size_type i = 0; i < sz; i++) {
//      if (predicate(vector_type::operator[](i))) {
//        oper(vector_type::operator[](i));
//      }
//    }
//  }
//
//  template <class Operator, class Predicate>
//  inline void visit_first_if(Operator&& oper, Predicate&& predicate) {
//    const size_type sz = size();
//
//    for (size_type i = 0; i < sz; i++) {
//      if (predicate(vector_type::operator[](i))) {
//        oper(vector_type::operator[](i));
//        return;
//      }
//    }
//  }
//
//  template <auto, class... Ts>
//  auto ext(Ts...);
//};

ZBASE_END_NAMESPACE
