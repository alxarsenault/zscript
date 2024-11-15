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
#include <string_view>
#include <type_traits>
#include <array>
#include <concepts>
#include <functional>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

ZBASE_BEGIN_NAMESPACE

#define ZBASE_USING_DECLTYPE_1(x) \
  template <class T>              \
  using x = decltype(T::x)

#define ZBASE_USING_DECLTYPE_2(name, x) \
  template <class T>                    \
  using name = decltype(T::x)

#define ZBASE_USING_DECLTYPE_(N, ...) ZBASE_CONCAT(ZBASE_USING_DECLTYPE_, N)(__VA_ARGS__)

///
#define ZBASE_DECL_USING_DECLTYPE(...) ZBASE_USING_DECLTYPE_(ZBASE_NARG(__VA_ARGS__), __VA_ARGS__)

#define ZBASE_USING_TYPE_1(x) \
  template <class T>          \
  using x = T::x

#define ZBASE_USING_TYPE_2(name, x) \
  template <class T>                \
  using name = T::x

#define ZBASE_USING_TYPE_(N, ...) ZBASE_CONCAT(ZBASE_USING_TYPE_, N)(__VA_ARGS__)

///
#define ZBASE_DECL_USING_TYPE(...) ZBASE_USING_TYPE_(ZBASE_NARG(__VA_ARGS__), __VA_ARGS__)

#define ZBASE_HAS_MEMBER_1(x) \
  template <class T>          \
  using x = zb::has_members<T, x>;

#define ZBASE_HAS_MEMBER_2(name, x) \
  template <class T>                \
  using name = zb::has_members<T, x>;

#define ZBASE_HAS_MEMBER_(N, ...) ZBASE_CONCAT(ZBASE_HAS_MEMBER_, N)(__VA_ARGS__)

///
#define ZBASE_DECL_HAS_MEMBER(...) ZBASE_HAS_MEMBER_(ZBASE_NARG(__VA_ARGS__), __VA_ARGS__)

template <class...>
struct empty_t {};

using empty_struct = empty_t<>;

using true_t = std::true_type;
using false_t = std::false_type;

template <bool BValue>
using bool_t = std::bool_constant<BValue>;

// always_false
template <typename T>
inline constexpr bool always_false = false;

#define zb_static_error(msg) static_assert(zb::always_false<decltype([]() {})>, msg)

template <typename T>
concept arithmetic = std::is_arithmetic<T>::value;

//
//
//

template <class T, size_t N>
using carray = T[N];

//
// nonesuch
//

struct nonesuch {
  nonesuch(const nonesuch&) = delete;
  ~nonesuch() = delete;
  void operator=(const nonesuch&) = delete;
};

//
// detector_value
//

template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector_value : __zb::false_t {};

template <class Default, template <class...> class Op, class... Args>
struct detector_value<Default, std::void_t<Op<Args...>>, Op, Args...> : __zb::true_t {};

template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
  using value_t = __zb::false_t;
  using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
  using value_t = __zb::true_t;
  using type = Op<Args...>;
};

// is_detected
template <template <class...> class Op, class... Args>
using is_detected = typename __zb::detector<__zb::nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
inline constexpr bool is_detected_v = __zb::is_detected<Op, Args...>::value;

//
// has_members
//

/// @code
///   template <class T>
///   using has_my_function_impl = decltype(std::declval<T&>().has_my_function());
///
///   template <class T>
///   using has_my_function = zb::has_members<T, has_my_function_impl>;
/// @endcode
///
/// @code
///   template <class T>
///   using has_x_value = decltype(T::x);
///
///   template <class T>
///   using has_x = zb::has_members<T, has_x_value>;
///
///   // Same as:
///   ZBASE_DECL_USING_DECLTYPE(has_x_value, x);
///   ZBASE_DECL_HAS_MEMBER(has_x, has_x_value);
/// @endcode
///
/// @code
///   template <class T>
///   using has_value_type_def = T::value_type;
///
///   template <class T>
///   using has_value_type = zb::has_members<T, has_value_type_def>;
///
///   // Same as:
///   ZBASE_DECL_USING_TYPE(has_value_type_def, value_type);
///   ZBASE_DECL_HAS_MEMBER(has_value_type, has_value_type_def);
/// @endcode
///
/// @code
///   template <class T>
///   using has_static_size_impl = T::size();
///
///   template <class T>
///   using has_static_size = zb::has_members<T, has_static_size_impl>;
///
///   // Same as:
///   ZBASE_DECL_USING_DECLTYPE(has_static_size_impl, size());
///   ZBASE_DECL_HAS_MEMBER(has_static_size, has_static_size_impl);
/// @endcode
template <class T, template <class...> class... Ops>
using has_members = std::conjunction<__zb::detector_value<__zb::nonesuch, void, Ops, T>...>;

template <class T, template <class...> class... Ops>
using enable_if_has_members_t = std::enable_if_t<__zb::has_members<T, Ops...>::value, std::nullptr_t>;

//
// dependent_type
//

template <class _Tp, bool>
struct dependent_type : public _Tp {};

template <bool _Dummy, class D>
using dependent_type_condition = typename __zb::dependent_type<std::type_identity<D>, _Dummy>::type;

//
// is_different
//

template <class _T1, class _T2>
using is_different = std::negation<std::is_same<_T1, _T2>>;

template <class _T1, class _T2>
inline constexpr bool is_different_v = __zb::is_different<_T1, _T2>::value;

//
// enable_if_same
//

template <bool _Dummy, class _D>
using enable_if_same = typename std::enable_if<std::is_same<_D, __zb::true_t>::value>::type;

//
// enable_if_different
//

template <bool _Dummy, class _D>
using enable_if_different = typename std::enable_if<std::is_same<_D, __zb::false_t>::value>::type;

template <class _T, class _T2>
using is_same_rcv = std::is_same<std::remove_cv_t<_T>, std::remove_cv_t<_T2>>;

template <class _T, class _T2>
using is_same_rcvref = std::is_same<std::remove_cvref_t<_T>, std::remove_cvref_t<_T2>>;

template <class _T1, class _T2>
inline constexpr bool is_same_rcvref_v = __zb::is_same_rcvref<_T1, _T2>::value;

template <size_t, typename>
struct make_reverse_index_sequence_helper;

template <size_t N, size_t... NN>
struct make_reverse_index_sequence_helper<N, std::index_sequence<NN...>> : std::index_sequence<(N - NN)...> {
};

template <size_t N>
struct make_reverse_index_sequence
    : make_reverse_index_sequence_helper<N - 1, decltype(std::make_index_sequence<N>{})> {};

// template <class _Tp, class _Up>
// struct is_same_uncvref : std::is_same<std::remove_cvref_t<_Tp>, std::remove_cvref_t<_Up> > {};

ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wsign-compare")

///
template <typename _T, std::convertible_to<_T> _T1, std::convertible_to<_T>... _Ts>
ZB_CK_INLINE_CXPR bool is_one_of(const _T& t, const _T1& t1, const _Ts&... ts) noexcept {
  if constexpr (sizeof...(_Ts) == 0) {
    return t == t1;
  }
  else {
    return (t == t1) || __zb::is_one_of(t, ts...);
  }
}

template <typename _T, std::convertible_to<_T> _T1, std::convertible_to<_T>... _Ts>
ZB_CK_INLINE_CXPR bool all_equals(const _T& t, const _T1& t1, const _Ts&... ts) noexcept {
  if constexpr (sizeof...(_Ts) == 0) {
    return t == t1;
  }
  else {
    return (t == t1) && __zb::all_equals(t, ts...);
  }
}
ZBASE_PRAGMA_POP()

/// Get the maximum of n values.
/// @code
///   int a = zb::maximum(1, 2, 3, 4, 5);
/// @endcode
///
///
///
template <__zb::arithmetic _T0, __zb::arithmetic _T1, __zb::arithmetic... _Ts>
ZB_CK_INLINE_CXPR std::common_type_t<_T0, _T1, _Ts...> maximum(_T0 v1, _T1 v2, _Ts... vs) noexcept {
  using cm_type = std::common_type_t<_T0, _T1, _Ts...>;

  if constexpr (sizeof...(_Ts) == 0) {
    return (cm_type)v2 > (cm_type)v1 ? v2 : v1;
  }
  else {
    return (cm_type)v2 > (cm_type)v1 ? maximum(v2, vs...) : maximum(v1, vs...);
  }
}

template <__zb::arithmetic _T0, __zb::arithmetic _T1, __zb::arithmetic... _Ts>
ZB_CK_INLINE_CXPR std::common_type_t<_T0, _T1, _Ts...> minimum(_T0 v1, _T1 v2, _Ts... vs) noexcept {
  using cm_type = std::common_type_t<_T0, _T1, _Ts...>;

  if constexpr (sizeof...(_Ts) == 0) {
    return (cm_type)v2 < (cm_type)v1 ? v2 : v1;
  }
  else {
    return (cm_type)v2 < (cm_type)v1 ? minimum(v2, vs...) : minimum(v1, vs...);
  }
}

/// Boolean only one true.
// template <class... _Ts>
//   requires std::is_convertible_v<std::common_type_t<_Ts...>, bool>
// ZB_CK_INLINE_CXPR bool is_only_one_true(_Ts... ts) noexcept {
//   return (ts ^ ...);
// }
//
// template <bool... Bs>
// struct is_only_one_true_t {
//   static constexpr bool value = (Bs ^ ...);
// };

/// Boolean only one false.
// template <class... _Ts>
//   requires std::is_convertible_v<std::common_type_t<_Ts...>, bool>
// ZB_CK_INLINE_CXPR bool is_only_one_false(_Ts... ts) noexcept {
//   return (!ts ^ ...);
// }
//
// template <bool... Bs>
// struct is_only_one_false_t {
//   static constexpr bool value = (!Bs ^ ...);
// };

// template <class... _Ts>
//   requires std::is_convertible_v<std::common_type_t<_Ts...>, bool>
// ZB_CK_INLINE_CXPR bool is_more_than_one_true(_Ts... ts) noexcept {
//   return (int(ts) + ...) > 1;
// }
//
// template <bool... Bs>
// struct is_more_than_one_true_t {
//   static constexpr bool value = __zb::is_more_than_one_true(Bs...);
// };

// template <class T>
// ZB_CK_INLINE_CXPR bool is_in_range(T x, T left, T right) noexcept {
//   return x >= left && x <= right;
// }
//
// template <class T>
// ZB_CK_INLINE_CXPR bool is_out_of_range(T x, T left, T right) noexcept {
//   return !__zb::is_in_range(x, left, right);
// }

// template <class T, class S>
// ZB_CK_INLINE_CXPR bool assign(T& dst, S&& src) noexcept {
//   if (dst == src) {
//     return false;
//   }
//
//   dst = std::forward<S>(src);
//   return true;
// }
//
//  function_arguments
//

namespace detail {
template <class Ret, class... Args>
std::tuple<Args...> function_arguments_helper(Ret (*)(Args...));

template <class Ret, class F, class... Args>
std::tuple<Args...> function_arguments_helper(Ret (F::*)(Args...));

template <class Ret, class F, class... Args>
std::tuple<Args...> function_arguments_helper(Ret (F::*)(Args...) const);

template <typename F>
decltype(function_arguments_helper(&F::operator())) function_arguments_helper(F);
} // namespace detail.

template <typename T>
using function_arguments = decltype(detail::function_arguments_helper(std::declval<T>()));

template <class T1, class T2>
struct type_pair {
  using first = T1;
  using second = T2;
};

//
//
//

template <class T>
struct is_vector : __zb::false_t {};

template <class T, class Allocator>
struct is_vector<std::vector<T, Allocator>> : __zb::true_t {};

template <class T>
inline constexpr bool is_vector_v = __zb::is_vector<T>::value;

template <class T>
struct is_array : __zb::false_t {};

template <class T, size_t N>
struct is_array<std::array<T, N>> : __zb::true_t {};

template <class T>
inline constexpr bool is_array_v = __zb::is_array<T>::value;

template <class T>
struct array_size;

template <class T, size_t N>
struct array_size<std::array<T, N>> {
  static constexpr size_t value = N;
};

template <class T>
inline constexpr size_t array_size_v = __zb::array_size<T>::value;

//
//
//

template <class Stream, class T, class = void>
struct is_streamable : std::false_type {};

template <class Stream, class T>
struct is_streamable<Stream, T, std::void_t<decltype(std::declval<Stream&>() << std::declval<T>())>>
    : std::true_type {};

// template <typename S, typename T>
// struct is_streamable<S, T, std::void_t<decltype(operator<<(std::declval<S&>(), std::declval<T>()))>> :
// std::true_type {
// };

//
//
//

template <class T>
struct is_pair : __zb::false_t {};

template <class T1, class T2>
struct is_pair<std::pair<T1, T2>> : __zb::true_t {};

template <class T>
inline constexpr bool is_pair_v = __zb::is_pair<T>::value;

//
//
//

template <class T>
struct is_tuple : __zb::false_t {};

template <class... Args>
struct is_tuple<std::tuple<Args...>> : __zb::true_t {};

template <class T>
inline constexpr bool is_tuple_v = __zb::is_tuple<T>::value;

//
//
//

template <class T>
using has_key_type_def = typename T::key_type;

template <class T>
using has_key_type = __zb::has_members<T, has_key_type_def>;

template <class T>
using has_mapped_type_def = typename T::mapped_type;

template <class T>
using has_mapped_type = __zb::has_members<T, has_mapped_type_def>;

template <class T>
struct is_map_type {
  static constexpr bool value = []() {
    if constexpr (__zb::has_key_type<T>::value && __zb::has_mapped_type<T>::value) {
      return std::is_same_v<typename T::value_type,
          std::pair<const typename T::key_type, typename T::mapped_type>>;
    }
    { return false; }
  }();
};

template <class T>
inline constexpr bool is_map_type_v = __zb::is_map_type<T>::value;

//
//
//
template <class _T>
struct is_trivial_cref
    : std::bool_constant<std::is_trivially_copyable_v<_T> && sizeof(_T) <= sizeof(size_t)> {};

template <class _T>
inline constexpr bool is_trivial_cref_v = __zb::is_trivial_cref<_T>::value;

template <class _T>
struct cref : std::conditional<__zb::is_trivial_cref_v<std::remove_cvref_t<_T>>, _T, const _T&> {};

template <class _T>
using cref_t = typename __zb::cref<_T>::type;

//
// MARK: Strings
//

namespace detail {
template <class _CharT, class _T>
struct is_basic_string_view_convertible_impl : __zb::false_t {};

template <class _CharT>
struct is_basic_string_view_convertible_impl<_CharT, std::basic_string_view<_CharT>> : __zb::true_t {};

template <class T, class _CharT>
  requires std::is_convertible_v<T, std::string_view> || std::is_base_of_v<std::string_view, T>
struct is_basic_string_view_convertible_impl<_CharT, T> : __zb::true_t {};

template <class _CharT, class _Allocator>
struct is_basic_string_view_convertible_impl<_CharT,
    std::basic_string<_CharT, std::char_traits<_CharT>, _Allocator>> : __zb::true_t {};

template <class _CharT>
struct is_basic_string_view_convertible_impl<_CharT, _CharT*> : __zb::true_t {};

template <class _CharT>
struct is_basic_string_view_convertible_impl<_CharT, const _CharT*> : __zb::true_t {};

template <class _CharT, size_t N>
struct is_basic_string_view_convertible_impl<_CharT, _CharT (&)[N]> : __zb::true_t {};

template <class _CharT, size_t N>
struct is_basic_string_view_convertible_impl<_CharT, const _CharT (&)[N]> : __zb::true_t {};
} // namespace detail.

template <class _CharT, class _T>
using is_basic_string_view_convertible
    = __zb::detail::is_basic_string_view_convertible_impl<_CharT, std::decay_t<_T>>;

template <class _CharT, class _T>
inline constexpr bool is_basic_string_view_convertible_v
    = __zb::is_basic_string_view_convertible<_CharT, _T>::value;

template <class _T>
using is_string_view_convertible = __zb::is_basic_string_view_convertible<char, std::decay_t<_T>>;

template <class _T>
inline constexpr bool is_string_view_convertible_v = __zb::is_string_view_convertible<_T>::value;

template <class _T>
using is_wstring_view_convertible = __zb::is_basic_string_view_convertible<wchar_t, std::decay_t<_T>>;

template <class _T>
inline constexpr bool is_wstring_view_convertible_v = __zb::is_wstring_view_convertible<_T>::value;

template <class _T>
struct is_u16string_view_convertible : std::is_convertible<const _T&, std::basic_string_view<char16_t>> {};

template <class _T>
struct is_u32string_view_convertible : std::is_convertible<const _T&, std::basic_string_view<char32_t>> {};

template <class _CharT, class _T>
struct is_basic_string_view_constructible : std::is_constructible<std::basic_string_view<_CharT>, _T> {};

template <class SType, class _CharT>
using enable_if_string_view_ctor_t
    = std::enable_if_t<__zb::is_basic_string_view_constructible<_CharT, std::remove_cvref_t<SType>>::value>;

///
template <class SType, typename = void>
struct string_char_type;

template <class SType, typename>
struct string_char_type {
  using type = void;
};

template <class SType>
struct string_char_type<SType, __zb::enable_if_string_view_ctor_t<SType, char>> {
  using type = char;
};

template <class SType>
struct string_char_type<SType, __zb::enable_if_string_view_ctor_t<SType, char16_t>> {
  using type = char16_t;
};

template <class SType>
struct string_char_type<SType, __zb::enable_if_string_view_ctor_t<SType, char32_t>> {
  using type = char32_t;
};

template <class SType>
struct string_char_type<SType, __zb::enable_if_string_view_ctor_t<SType, wchar_t>> {
  using type = wchar_t;
};

template <class SType>
struct string_char_type<SType, __zb::enable_if_string_view_ctor_t<SType, char8_t>> {
  using type = char8_t;
};

///
template <class SType>
using string_char_type_t = typename __zb::string_char_type<SType>::type;

//
//
//

template <class T>
inline constexpr auto has_static_allocate(
    int) -> decltype(T::allocate(std::declval<size_t>()), void(), __zb::true_t{});

template <class T>
inline constexpr __zb::false_t has_static_allocate(...);

template <class T, class U>
inline constexpr auto has_static_deallocate(
    int) -> decltype(T::deallocate(std::declval<U*>(), std::declval<size_t>()), void(), __zb::true_t{});

template <class T, class U>
inline constexpr __zb::false_t has_static_deallocate(...);

namespace detail {
template <typename T>
auto is_contiguous_container(
    int) -> decltype(std::data(std::declval<T&>()), std::size(std::declval<T&>()), void(), __zb::true_t{});
template <typename T>
__zb::false_t is_contiguous_container(...);

template <typename T>
auto is_iterable_container(
    int) -> decltype(std::size(std::declval<T&>()), std::begin(std::declval<T&>()), void(), __zb::true_t{});
template <typename T>
__zb::false_t is_iterable_container(...);

template <typename T>
auto has_static_size(int) -> decltype(T::size(), void(), __zb::true_t{});
template <typename T>
__zb::false_t has_static_size(...);

template <typename T>
auto has_static_capacity(int) -> decltype(T::capacity(), void(), __zb::true_t{});
template <typename T>
__zb::false_t has_static_capacity(...);

template <typename T>
auto has_static_max_size(int) -> decltype(T::max_size(), void(), __zb::true_t{});
template <typename T>
__zb::false_t has_static_max_size(...);

template <typename T>
auto is_iterable_impl(int) -> decltype(std::begin(std::declval<T&>()) != std::end(std::declval<T&>()), void(),
                               ++std::declval<decltype(std::begin(std::declval<T&>()))&>(),
                               void(*std::begin(std::declval<T&>())), __zb::true_t{});

template <typename T>
__zb::false_t is_iterable_impl(...);

template <typename T>
auto is_const_iterable_impl(
    int) -> decltype(std::cbegin(std::declval<const T&>()) != std::cend(std::declval<const T&>()), void(),
             ++std::declval<const decltype(std::cbegin(std::declval<const T&>()))&>(),
             void(*std::cbegin(std::declval<const T&>())), __zb::true_t{});

template <typename T>
__zb::false_t is_const_iterable_impl(...);
} // namespace detail

// is_container
template <class T>
using is_contiguous_container = decltype(detail::is_contiguous_container<T>(0));

template <class _T>
inline constexpr bool is_contiguous_container_v = __zb::is_contiguous_container<_T>::value;

template <class T>
using is_contiguous_container_not_string
    = std::conjunction<__zb::is_contiguous_container<T>, std::negation<__zb::is_string_view_convertible<T>>>;

template <class _T>
inline constexpr bool is_contiguous_container_not_string_v
    = __zb::is_contiguous_container_not_string<_T>::value;

// is_iterable_container
template <class T>
using is_iterable_container = decltype(detail::is_iterable_container<T>(0));

template <class _T>
inline constexpr bool is_iterable_container_v = __zb::is_iterable_container<_T>::value;

//
template <class T>
using is_iterable_container_not_string
    = std::conjunction<__zb::is_iterable_container<T>, std::negation<__zb::is_string_view_convertible<T>>>;

template <class _T>
inline constexpr bool is_iterable_container_not_string_v = __zb::is_iterable_container_not_string<_T>::value;

template <class T>
using output_iterator_value_type_t = typename std::iterator_traits<T>::value_type;

template <class _T>
using container_value_type_t = std::remove_pointer_t<decltype(std::data(std::declval<_T&>()))>;

namespace detail {
template <typename T>
using has_push_back_t
    = decltype(std::declval<T>().push_back(std::declval<__zb::container_value_type_t<T>>()));
} // namespace detail.

// has_push_back
template <typename T>
using has_push_back = __zb::is_detected<detail::has_push_back_t, T>;

template <class _T>
inline constexpr bool has_push_back_v = __zb::has_push_back<_T>::value;

// clang-format off

///
enum class char_encoding {
  utf8 = sizeof(char),
  utf16 = sizeof(char16_t),
  utf32 = sizeof(char32_t)
};

///
template <class CharT>
struct is_utf_char_type : std::disjunction<
  __zb::is_same_rcv<char, CharT>,
  __zb::is_same_rcv<char16_t, CharT>,
  __zb::is_same_rcv<char32_t, CharT>,
  __zb::is_same_rcv<wchar_t, CharT>,
  __zb::is_same_rcv<char8_t, CharT>> {};

///
template <size_t _CSize>
struct is_utf_char_size : std::bool_constant<
  __zb::is_one_of(_CSize, sizeof(char), sizeof(char16_t), sizeof(char32_t))> {};

template <class SType>
struct is_utf_string_type : std::bool_constant<
  std::is_constructible_v<std::basic_string_view<char>, SType>     ||
  std::is_constructible_v<std::basic_string_view<char16_t>, SType> ||
  std::is_constructible_v<std::basic_string_view<char32_t>, SType> ||
  std::is_constructible_v<std::basic_string_view<wchar_t>, SType>  ||
  std::is_constructible_v<std::basic_string_view<char8_t>, SType>> {};

// clang-format on

template <class SType>
using enable_if_utf_string_type_t = std::enable_if_t<__zb::is_utf_string_type<SType>::value, std::nullptr_t>;

template <class SType>
struct is_utf_basic_string_type
    : std::bool_constant<__zb::is_utf_string_type<SType>::value && __zb::has_push_back_v<SType>> {};

template <class SType, std::enable_if_t<__zb::is_utf_string_type<SType>::value, std::nullptr_t> = nullptr>
using string_view_type = std::basic_string_view<__zb::string_char_type_t<SType>>;

template <typename CharT, std::enable_if_t<__zb::is_utf_char_type<CharT>::value, std::nullptr_t> = nullptr>
struct utf_encoding_of {
  static constexpr __zb::char_encoding value = []() {
    if constexpr (sizeof(CharT) == sizeof(char)) {
      return __zb::char_encoding::utf8;
    }
    else if constexpr (sizeof(CharT) == sizeof(char16_t)) {
      return __zb::char_encoding::utf16;
    }
    else if constexpr (sizeof(CharT) == sizeof(char32_t)) {
      return __zb::char_encoding::utf32;
    }
  }();
};

template <__zb::char_encoding Encoding>
struct utf_encoding_size {
  static constexpr size_t value = static_cast<size_t>(Encoding);
};

template <__zb::char_encoding Encoding>
struct utf_encoding_to_max_char_count {
  static constexpr size_t value = []() {
    if constexpr (Encoding == __zb::char_encoding::utf8) {
      return 4;
    }
    else if constexpr (Encoding == __zb::char_encoding::utf16) {
      return 2;
    }
    else if constexpr (Encoding == __zb::char_encoding::utf32) {
      return 1;
    }
  }();
};

template <class T>
ZB_CK_INLINE size_t hash(const T& t) noexcept {
  return std::hash<T>()(t);
}

template <class... Ts>
class type_list {

  template <size_t Index>
  struct __type_at_index;

  template <class T>
  struct __type_index;

  template <class T>
  struct __type_counter;

  template <size_t... Idx>
  struct __sub_type_list;

  struct __reversed;

  template <size_t N>
  struct __n_first;

  template <size_t N>
  struct __n_last;

public:
  ///
  static constexpr size_t size() noexcept { return sizeof...(Ts); }

  static constexpr size_t k_size = size();

  using tuple_type = std::tuple<Ts...>;

  using tuple_type_no_ref = std::tuple<std::remove_cvref_t<Ts>...>;

  ///
  template <class T>
  static constexpr size_t type_index() noexcept {
    return __type_index<T>::value;
  }

  template <class T>
  static constexpr size_t k_index = type_index<T>();

  template <class T>
  static constexpr size_t type_count() noexcept {
    return __type_counter<T>::value;
  }

  template <class T>
  static constexpr size_t k_count = type_count<T>();

  ///
  template <size_t Index>
  using type_at_index = typename __type_at_index<Index>::type;

  ///
  template <size_t... Idx>
  using sub_list = typename __sub_type_list<Idx...>::type;

  ///
  using reversed_list = typename __reversed::type;

  template <size_t N>
  using n_first_list = typename __n_first<N>::type;

  template <size_t N>
  using n_last_list = typename __n_last<N>::type;

private:
  template <class T>
  struct __type_index {
    template <size_t N, class T1, class... Tss>
    static constexpr size_t type_index_impl() noexcept {
      if constexpr (std::is_same_v<T, T1>) {
        return N;
      }
      else if constexpr (sizeof...(Tss) == 0) {
        return -1;
      }
      else {
        return type_index_impl<N + 1, Tss...>();
      }
    }

    static constexpr size_t value = type_index_impl<0, Ts...>();
  };

  template <class T>
  struct __type_counter {
    template <size_t Count, size_t N, class T1, class... Tss>
    static constexpr size_t type_counter_impl() noexcept {
      if constexpr (std::is_same_v<T, T1>) {
        if constexpr (sizeof...(Tss) == 0) {
          return Count + 1;
        }
        else {
          return type_counter_impl<Count + 1, N + 1, Tss...>();
        }
      }
      else if constexpr (sizeof...(Tss) == 0) {
        return Count;
      }
      else {
        return type_counter_impl<Count, N + 1, Tss...>();
      }
    }

    static constexpr size_t value = type_counter_impl<0, 0, Ts...>();
  };

  template <size_t Index>
  struct __type_at_index {
    template <size_t N, class T, class... Tss>
    static constexpr auto type_at_index_impl2() noexcept {
      if constexpr (N == Index) {
        return std::type_identity<T>{};
      }
      else if constexpr (sizeof...(Tss) == 0) {
        return std::type_identity<void>{};
      }
      else {
        return type_at_index_impl2<N + 1, Tss...>();
      }
    }

    using type = typename decltype(type_at_index_impl2<0, Ts...>())::type;
  };

  template <size_t... Idx>
  struct __sub_type_list {
    using type = type_list<type_at_index<Idx>...>;
  };

  template <size_t N>
  struct __n_first {
    template <size_t... Idx>
    static constexpr auto n_first(std::integer_sequence<size_t, Idx...>) noexcept {
      return std::type_identity<sub_list<Idx...>>{};
    }

    using type = typename decltype(n_first(std::make_integer_sequence<size_t, N>{}))::type;
  };

  template <size_t N>
  struct __n_last {
    template <size_t... Idx>
    static constexpr auto n_last(std::integer_sequence<size_t, Idx...>) noexcept {
      return std::type_identity<sub_list<(k_size - N) + Idx...>>{};
    }

    using type = typename decltype(n_last(std::make_integer_sequence<size_t, N>{}))::type;
  };

  struct __reversed {
    template <size_t... Idx>
    static constexpr auto rev(std::integer_sequence<size_t, Idx...>) noexcept {
      return std::type_identity<sub_list<Idx...>>{};
    }

    using type = typename decltype(rev(__zb::make_reverse_index_sequence<k_size>{}))::type;
  };
};

template <auto... Ts>
class value_list {

  template <size_t Index>
  struct __value_at_index;
  //
  template <auto T>
  struct __value_index;

public:
  ///
  static constexpr size_t size() noexcept { return sizeof...(Ts); }

  static constexpr size_t k_size = size();
  //
  ///
  template <auto T>
  static constexpr size_t value_index() noexcept {
    return __value_index<T>::value;
  }

  ///
  template <size_t Index>
  static constexpr auto value_at_index() noexcept {
    return __value_at_index<Index>::value;
  }

private:
  template <auto T>
  struct __value_index {

    using T_TYPE = decltype(T);

    template <size_t N, auto T1, auto... Tss>
    static constexpr size_t value_index_impl() noexcept {

      using T1_TYPE = decltype(T1);

      if constexpr (std::is_same_v<T_TYPE, T1_TYPE>) {

        if (T == T1) {
          return N;
        }

        if constexpr (sizeof...(Tss) == 0) {
          return -1;
        }
        else {
          return value_index_impl<N + 1, Tss...>();
        }
      }
      else if constexpr (sizeof...(Tss) == 0) {
        return -1;
      }
      else {
        return value_index_impl<N + 1, Tss...>();
      }
    }

    static constexpr size_t value = value_index_impl<0, Ts...>();
  };

  template <size_t Index>
  struct __value_at_index {
    template <size_t N, auto T, auto... Tss>
    static constexpr auto value_at_index_impl2() noexcept {
      if constexpr (N == Index) {
        return T;
      }
      else if constexpr (sizeof...(Tss) == 0) {
        using t_type = decltype(T);

        return t_type{ 0 };
      }
      else {
        return value_at_index_impl2<N + 1, Tss...>();
      }
    }

    static constexpr auto value = value_at_index_impl2<0, Ts...>();
  };
};

template <class T, auto Value>
struct type_value_pair {
  using type = T;
  static constexpr auto value = Value;
};

template <class... TypePairs>
class type_map {
public:
  using map = __zb::type_list<TypePairs...>;

  using types_part = __zb::type_list<typename TypePairs::type...>;

  using value_part = __zb::value_list<TypePairs::value...>;

  static constexpr size_t size() noexcept { return sizeof...(TypePairs); }

  struct __create_type_list {
    static constexpr auto create() { return std::type_identity<int>{}; }
  };
};

template <typename T>
struct function_traits : public function_traits<decltype(&T::operator())> {};

// For generic types, directly use the result of the signature of its 'operator()'.
// we specialize for pointers to member function.
template <typename ClassType, typename ReturnType, typename... Args>
struct function_traits<ReturnType (ClassType::*)(Args...) const> {
  using class_type = ClassType;

  // arity is the number of arguments.
  enum { arity = sizeof...(Args) };

  using args_list = __zb::type_list<Args...>;

  typedef ReturnType result_type;

  // the i-th argument is equivalent to the i-th tuple element of a tuple
  // composed of those arguments.
  template <size_t i>
  struct arg {
    typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
  };
};

//
//
//

template <class FctType, FctType fct>
struct static_functor {
  ZB_CK_INLINE_CXPR operator FctType() const noexcept { return fct; }
};

template <auto Fct>
  requires std::is_function_v<std::remove_pointer_t<decltype(Fct)>>
using static_functor_t = static_functor<decltype(Fct), Fct>;

///
template <class R, class... Args>
using function_pointer = R (*)(Args...);

template <class Class, class R, class... Args>
using member_function_pointer = R (Class::*)(Args...);

template <class Class, class R, class... Args>
using const_member_function_pointer = R (Class::*)(Args...) const;

template <class Fct, class Class, class R, class... Args>
struct member_function_pointer_wrapper {
  static constexpr bool is_const
      = std::is_same_v<__zb::const_member_function_pointer<Class, R, Args...>, Fct>;
  Fct fct;
};

namespace detail {
template <class>
struct function_pointer_type {};

template <class R, class... Args>
struct function_pointer_type<std::function<R(Args...)>> {
  using type = __zb::function_pointer<R, Args...>;
};
} // namespace detail.

template <class Fct>
struct function_pointer_type {
  using type = typename detail::function_pointer_type<decltype(std::function(std::declval<Fct>()))>::type;
};

template <class Fct>
using function_pointer_type_t = typename function_pointer_type<Fct>::type;

template <class Fct>
struct is_function_pointer {
  static constexpr bool value
      = sizeof(Fct) == 1 && (std::is_convertible_v<Fct, __zb::function_pointer_type_t<Fct>>);
};

template <class Fct>
inline constexpr bool is_function_pointer_v = is_function_pointer<Fct>::value;

template <class Fct>
struct member_function_pointer_object_type {};

template <class Class, class R, class... Args>
struct member_function_pointer_object_type<__zb::member_function_pointer<Class, R, Args...>> {
  using type = Class;
};

template <class Class, class R, class... Args>
struct member_function_pointer_object_type<__zb::const_member_function_pointer<Class, R, Args...>> {
  using type = Class;
};

template <class Fct>
using member_function_pointer_object_type_t = typename member_function_pointer_object_type<Fct>::type;

template <class Fct>
struct is_const_member_function_pointer : __zb::false_t {};

template <class Class, class R, class... Args>
struct is_const_member_function_pointer<__zb::const_member_function_pointer<Class, R, Args...>>
    : __zb::true_t {};

template <class Fct>
inline constexpr bool is_const_member_function_pointer_v = __zb::is_const_member_function_pointer<Fct>::value;

ZBASE_END_NAMESPACE
