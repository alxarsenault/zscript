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

#include <zbase/zbase.h>
#include <zbase/sys/assert.h>
#include <zbase/utility/traits.h>
#include <algorithm>
#include <span>

ZBASE_BEGIN_NAMESPACE

template <class T>
class span : public std::span<T> {
public:
  using base_span_type = std::span<T>;
  using element_type = typename base_span_type::element_type;
  using value_type = typename base_span_type::value_type;
  using size_type = typename base_span_type::size_type;
  using difference_type = typename base_span_type::difference_type;
  using pointer = typename base_span_type::pointer;
  using const_pointer = typename base_span_type::const_pointer;
  using reference = typename base_span_type::reference;
  using const_reference = typename base_span_type::const_reference;
  using iterator = typename base_span_type::iterator;

  static constexpr const size_type npos = -1;

  using base_span_type::base_span_type;
  using base_span_type::operator=;
  using base_span_type::data;
  using base_span_type::size;
  using base_span_type::operator[];

  ZB_INLINE_CXPR span(const base_span_type& s)
      : base_span_type(s) {}

  template <class U = T>
    requires(std::is_same_v<std::remove_const_t<T>, std::remove_const_t<U>> and std::is_const_v<T>
        and !std::is_const_v<U>)
  ZB_INLINE_CXPR span(const std::span<const U>& s)
      : base_span_type(s) {}

  ZB_INLINE_CXPR span(const span&) = default;
  ZB_INLINE_CXPR span(span&&) = default;

  ZB_INLINE_CXPR span& operator=(const span&) = default;
  ZB_INLINE_CXPR span& operator=(span&&) = default;

  ZB_CK_INLINE_CXPR reference operator()(difference_type n) const noexcept {
    const size_t sz = size();
    zbase_assert(sz, "call operator[] in an empty vector");
    return base_span_type::operator[]((n + sz) % sz);
  }

  ZB_CK_INLINE_CXPR __zb::span<element_type> subspan(
      size_type offset, size_type count = std::dynamic_extent) const noexcept {
    zbase_assert(offset <= size(), "zb::span<T>::subspan(offset, count): offset out of range");

    if (count == std::dynamic_extent) {
      return __zb::span<element_type>{ data() + offset, size() - offset };
    }

    zbase_assert(
        count <= size() - offset, "zb::span<T>::subspan(offset, count): offset + count out of range");
    return __zb::span<element_type>(data() + offset, count);
  }

  ZB_CK_INLINE_CXPR pointer data(size_type index) const noexcept { return data() + index; }

  //
  // Find iterator.
  //

  ZB_CK_INLINE iterator find(const value_type& v) const noexcept {
    return std::find(this->begin(), this->end(), v);
  }

  template <class UnaryPred>
  ZB_CK_INLINE iterator find_if(UnaryPred&& fct) const noexcept {
    return std::find_if(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  template <class UnaryPred>
  ZB_CK_INLINE iterator find_if_not(UnaryPred&& fct) const noexcept {
    return std::find_if_not(this->begin(), this->end(), std::forward<UnaryPred>(fct));
  }

  //
  // Find pointer.
  //

  ZB_CK_INLINE pointer pfind(const value_type& v) const noexcept {
    iterator it = this->find(v);
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  ZB_CK_INLINE pointer pfind_if(UnaryPred&& fct) const noexcept {
    iterator it = this->find_if(std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  template <class UnaryPred>
  ZB_CK_INLINE pointer pfind_if_not(UnaryPred&& fct) const noexcept {
    iterator it = this->find_if_not(std::forward<UnaryPred>(fct));
    return it == this->end() ? nullptr : &(*it);
  }

  //
  // Find index.
  //

  ZB_CK_INLINE size_type ifind(const value_type& v) const noexcept {
    iterator it = this->find(v);
    return it == this->end() ? npos : std::distance(this->begin(), it);
  }

  template <class UnaryPred>
  ZB_CK_INLINE size_type ifind_if(UnaryPred&& fct) const noexcept {
    iterator it = this->find_if(std::forward<UnaryPred>(fct));
    return it == this->end() ? npos : std::distance(this->begin(), it);
  }

  template <class UnaryPred>
  ZB_CK_INLINE size_type ifind_if_not(UnaryPred&& fct) const noexcept {
    iterator it = this->find_if_not(std::forward<UnaryPred>(fct));
    return it == this->end() ? npos : std::distance(this->begin(), it);
  }

  //
  // Visitor.
  //

  template <class Operator>
  inline void visit(Operator&& oper) {
    const size_type sz = size();
    for (size_type i = 0; i < sz; i++) {
      oper(base_span_type::operator[](i));
    }
  }

  template <class Operator, class Predicate>
  inline void visit_if(Operator&& oper, Predicate&& predicate) {
    const size_type sz = size();
    for (size_type i = 0; i < sz; i++) {
      if (predicate(base_span_type::operator[](i))) {
        oper(base_span_type::operator[](i));
      }
    }
  }

  template <class Operator, class Predicate>
  inline void visit_first_if(Operator&& oper, Predicate&& predicate) {
    const size_type sz = size();

    for (size_type i = 0; i < sz; i++) {
      if (predicate(base_span_type::operator[](i))) {
        oper(base_span_type::operator[](i));
        return;
      }
    }
  }

  ZB_CHECK inline bool operator==(const span& rhs) const {
    if (rhs.size() != this->size()) {
      return false;
    }

    return std::equal(this->begin(), this->end(), rhs.begin());
  }

  ZB_CHECK inline bool equals(const span& rhs) const {
    if (rhs.size() != this->size()) {
      return false;
    }

    return std::equal(this->begin(), this->end(), rhs.begin());
  }

  template <class U>
  ZB_CHECK inline bool equals(std::initializer_list<U> rhs) const {
    if (rhs.size() != this->size()) {
      return false;
    }

    return std::equal(this->begin(), this->end(), rhs.begin());
  }

  template <auto, class... Ts>
  auto ext(Ts...);
};
ZBASE_END_NAMESPACE
