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
#include <zscript/base/utility/traits.h>

ZBASE_BEGIN_NAMESPACE

struct default_construct_t {};

// struct variadic_separator_t {};

// struct variadic_first_separator_t {};

namespace detail {
template <class T1, class T2, class = void>
struct compressed_pair_content;

template <class T1, class T2>
struct compressed_pair_content<T1, T2, std::enable_if_t<!std::is_empty_v<T1> && !std::is_empty_v<T2>>> {
  using first_value_type = T1;
  using first_reference = first_value_type&;
  using first_const_reference = const first_value_type&;

  using second_value_type = T2;
  using second_reference = second_value_type&;
  using second_const_reference = const second_value_type&;

  // clang-format off
  inline constexpr compressed_pair_content() noexcept = default;
  inline constexpr compressed_pair_content(__zb::default_construct_t, __zb::default_construct_t) noexcept {}
  inline constexpr compressed_pair_content(__zb::default_construct_t, const second_value_type& s) noexcept : second(s) {}
  inline constexpr compressed_pair_content(__zb::default_construct_t, second_value_type&& s) noexcept : second(std::move(s)) {}
  inline constexpr compressed_pair_content(const first_value_type& f, __zb::default_construct_t) noexcept : first(f) {}
  inline constexpr compressed_pair_content(first_value_type&& f, __zb::default_construct_t) noexcept : first(std::move(f)) {}
  inline constexpr compressed_pair_content(const first_value_type& f, const second_value_type& s) noexcept : first(f), second(s) {}
  inline constexpr compressed_pair_content(first_value_type&& f, const second_value_type& s) noexcept : first(std::move(f)), second(s) {}
  inline constexpr compressed_pair_content(const first_value_type& f, second_value_type&& s) noexcept : first(f) , second(std::move(s)) {}
  inline constexpr compressed_pair_content(first_value_type&& f, second_value_type&& s) noexcept : first(std::move(f)) , second(std::move(s)) {}

  template <class... _Args>
  inline constexpr compressed_pair_content(_Args&&... args, __zb::default_construct_t) noexcept : first(std::forward<_Args>(args)...) {}

  template <class... _Args2>
  inline constexpr compressed_pair_content(__zb::default_construct_t, _Args2&&... args2) noexcept : second(std::forward<_Args2>(args2)...) {}
  // clang-format on

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_reference get_first() noexcept { return first; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_const_reference get_first() const noexcept { return first; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_reference get_second() noexcept { return second; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_const_reference get_second() const noexcept {
    return second;
  }

  first_value_type first;
  second_value_type second;
};

template <class T1, class T2>
struct compressed_pair_content<T1, T2, std::enable_if_t<!std::is_empty_v<T1> && std::is_empty_v<T2>>> {
  using first_value_type = T1;
  using first_reference = first_value_type&;
  using first_const_reference = const first_value_type&;

  using second_value_type = T2;
  using second_reference = second_value_type;
  using second_const_reference = second_value_type;

  // clang-format off
  inline constexpr compressed_pair_content() noexcept = default;
  inline constexpr compressed_pair_content(__zb::default_construct_t, __zb::default_construct_t) noexcept {}
  inline constexpr compressed_pair_content(__zb::default_construct_t, const second_value_type&) noexcept {}
  inline constexpr compressed_pair_content(__zb::default_construct_t, second_value_type&&) noexcept {}
  inline constexpr compressed_pair_content(const first_value_type& f, __zb::default_construct_t) noexcept : first(f) {}
  inline constexpr compressed_pair_content(first_value_type&& f, __zb::default_construct_t) noexcept : first(std::move(f)) {}
  inline constexpr compressed_pair_content(const first_value_type& f, const second_value_type&) noexcept : first(f) {}
  inline constexpr compressed_pair_content(first_value_type&& f, const second_value_type&) noexcept : first(std::move(f)) {}
  inline constexpr compressed_pair_content(const first_value_type& f, second_value_type&&) noexcept : first(f) {}
  inline constexpr compressed_pair_content(first_value_type&& f, second_value_type&&) noexcept : first(std::move(f)) {}

  template <class... _Args>
  inline constexpr compressed_pair_content(_Args&&... args, __zb::default_construct_t) noexcept : first(std::forward<_Args>(args)...) {}

  template <class... _Args2>
  inline constexpr compressed_pair_content(__zb::default_construct_t, _Args2&&...) noexcept {}
  // clang-format on

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_reference get_first() noexcept { return first; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_const_reference get_first() const noexcept { return first; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_reference get_second() noexcept {
    return second_value_type{};
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_const_reference get_second() const noexcept {
    return second_value_type{};
  }

  first_value_type first;
};

template <class T1, class T2>
struct compressed_pair_content<T1, T2, std::enable_if_t<std::is_empty_v<T1> && !std::is_empty_v<T2>>> {
  using first_value_type = T1;
  using first_reference = first_value_type;
  using first_const_reference = first_value_type;

  using second_value_type = T2;
  using second_reference = second_value_type&;
  using second_const_reference = const second_value_type&;

  // clang-format off
  inline constexpr compressed_pair_content() noexcept = default;
  inline constexpr compressed_pair_content(__zb::default_construct_t, __zb::default_construct_t) noexcept {}
  inline constexpr compressed_pair_content(__zb::default_construct_t, const second_value_type& s) noexcept : second(s) {}
  inline constexpr compressed_pair_content(__zb::default_construct_t, second_value_type&& s) noexcept  : second(std::move(s)) {}
  inline constexpr compressed_pair_content(const first_value_type&, __zb::default_construct_t) noexcept {}
  inline constexpr compressed_pair_content(first_value_type&&, __zb::default_construct_t) noexcept {}
  inline constexpr compressed_pair_content(const first_value_type&, const second_value_type& s) noexcept : second(s) {}
  inline constexpr compressed_pair_content(first_value_type&&, const second_value_type& s) noexcept : second(s) {}
  inline constexpr compressed_pair_content(const first_value_type&, second_value_type&& s) noexcept : second(std::move(s)) {}
  inline constexpr compressed_pair_content(first_value_type&&, second_value_type&& s) noexcept : second(std::move(s)) {}

  template <class... _Args>
  inline constexpr compressed_pair_content(_Args&&..., __zb::default_construct_t) noexcept {}

  template <class... _Args2>
  inline constexpr compressed_pair_content(__zb::default_construct_t, _Args2&&... args2) noexcept : second(std::forward<_Args2>(args2)...) {}
  // clang-format on

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_reference get_first() noexcept { return first_value_type{}; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_const_reference get_first() const noexcept {
    return first_value_type{};
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_reference get_second() noexcept { return second; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_const_reference get_second() const noexcept {
    return second;
  }

  second_value_type second;
};

template <class T1, class T2>
struct compressed_pair_content<T1, T2, std::enable_if_t<std::is_empty_v<T1> && std::is_empty_v<T2>>> {
  using first_value_type = T1;
  using first_reference = first_value_type;
  using first_const_reference = first_value_type;

  using second_value_type = T2;
  using second_reference = second_value_type;
  using second_const_reference = second_value_type;

  inline constexpr compressed_pair_content() noexcept = default;
  inline constexpr compressed_pair_content(__zb::default_construct_t, __zb::default_construct_t) noexcept {}
  inline constexpr compressed_pair_content(__zb::default_construct_t, const second_value_type&) noexcept {}
  inline constexpr compressed_pair_content(__zb::default_construct_t, second_value_type&&) noexcept {}
  inline constexpr compressed_pair_content(const first_value_type&, __zb::default_construct_t) noexcept {}
  inline constexpr compressed_pair_content(first_value_type&&, __zb::default_construct_t) noexcept {}
  inline constexpr compressed_pair_content(const first_value_type&, const second_value_type&) noexcept {}
  inline constexpr compressed_pair_content(first_value_type&&, const second_value_type&) noexcept {}
  inline constexpr compressed_pair_content(const first_value_type&, second_value_type&&) noexcept {}
  inline constexpr compressed_pair_content(first_value_type&&, second_value_type&&) noexcept {}

  template <class... _Args>
  inline constexpr compressed_pair_content(_Args&&..., __zb::default_construct_t) noexcept {}

  template <class... _Args2>
  inline constexpr compressed_pair_content(__zb::default_construct_t, _Args2&&...) noexcept {}

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_reference get_first() noexcept { return first_value_type{}; }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_const_reference get_first() const noexcept {
    return first_value_type{};
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_reference get_second() noexcept {
    return second_value_type{};
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_const_reference get_second() const noexcept {
    return second_value_type{};
  }
};
} // namespace detail.

template <class T1, class T2>
struct compressed_pair : private detail::compressed_pair_content<T1, T2> {
  using base = detail::compressed_pair_content<T1, T2>;
  using first_value_type = typename base::first_value_type;
  using first_reference = typename base::first_reference;
  using first_const_reference = typename base::first_const_reference;

  using second_value_type = typename base::second_value_type;
  using second_reference = typename base::second_reference;
  using second_const_reference = typename base::second_const_reference;

  using base::base;

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_reference first() noexcept { return this->get_first(); }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr first_const_reference first() const noexcept {
    return this->get_first();
  }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_reference second() noexcept { return this->get_second(); }

  [[nodiscard]] ZB_ALWAYS_INLINE constexpr second_const_reference second() const noexcept {
    return this->get_second();
  }
};

// is_pair
template <class>
struct is_compressed_pair : std::false_type {};

template <class T1, class T2>
struct is_compressed_pair<__zb::compressed_pair<T1, T2>> : std::true_type {};
ZBASE_END_NAMESPACE
