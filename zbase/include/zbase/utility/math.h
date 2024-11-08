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
#include <array>
#include <cstdint>
#include <cmath>
#include <type_traits>

ZBASE_BEGIN_NAMESPACE

//
// Math constants.
//

// clang-format off
template <typename T> inline constexpr T zero = T(0);
template <typename T> inline constexpr T one = T(1);
template <typename T> inline constexpr T minus_one = T(-1);

template <typename T> inline constexpr T e = T(2.7182818284590452353602874713526624977572L);

template <typename T> inline constexpr T pi = T(3.1415926535897932384626433832795028841972L);
template <typename T> inline constexpr T pi_over_eight = pi<T> / T(8);
template <typename T> inline constexpr T pi_over_four = pi<T> / T(4);
template <typename T> inline constexpr T pi_over_two = pi<T> / T(2);
template <typename T> inline constexpr T two_pi = T(2) * pi<T>;
template <typename T> inline constexpr T four_pi = T(4) * pi<T>;
template <typename T> inline constexpr T eight_pi = T(8) * pi<T>;

template <typename T> inline constexpr T one_over_pi = T(1) / pi<T>;
template <typename T> inline constexpr T two_over_pi = T(2) / pi<T>;
template <typename T> inline constexpr T four_over_pi = T(4) / pi<T>;
template <typename T> inline constexpr T eight_over_pi = T(8) / pi<T>;

template <typename T> inline constexpr T pi_squared = pi<T> * pi<T>;
template <typename T> inline constexpr T two_pi_squared = two_pi<T> * two_pi<T>;

template <typename T> inline constexpr T one_over_pi_squared = T(1) / pi_squared<T>;
template <typename T> inline constexpr T two_over_pi_squared = T(2) / pi_squared<T>;
template <typename T> inline constexpr T four_over_pi_squared = T(4) / pi_squared<T>;
template <typename T> inline constexpr T eight_over_pi_squared = T(8) / pi_squared<T>;

template <typename T> inline constexpr T one_over_two_pi = T(1) / two_pi<T>;

template <typename T> inline constexpr T sqrt_pi = T(1.7724538509055160272981674833411451827975L);
template <typename T> inline constexpr T sqrt_half_pi = T(1.2533141373155002512078826424055226265035L);

template <typename T> inline constexpr T sqrt_2 = T(1.4142135623730950488016887242096980785697L);
template <typename T> inline constexpr T sqrt_2_over_2 = T(0.7071067811865476);
template <typename T> inline constexpr T log_2 = T(0.6931471805599453094172321214581765680755L);
template <typename T> inline constexpr T log_10 = T(2.3025850929940456840179914546843642076011L);
template <typename T> inline constexpr T log_pi = T(1.1447298858494001741434273513530587116473L);
template <typename T> inline constexpr T log_two_pi = T(1.8378770664093454835606594728112352797228L);
template <typename T> inline constexpr T log_sqrt_two_pi = T(0.9189385332046727417803297364056176398614L);
// clang-format on

/// 2^(k)
template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] inline constexpr uint64_t iexp2(T k) noexcept {
  return T(1) << T(k);
}

[[nodiscard]] inline constexpr bool is_power_of_two(uint64_t v) noexcept {
  return (bool)(v && !(v & (v - 1)));
}

/// Returns the next power of two (in 64-bits) that is strictly greater than the given value.
/// @note Returns zero on overflow.
[[nodiscard]] inline constexpr uint64_t next_power_of_two(uint64_t v) noexcept {
  v |= (v >> 1UL);
  v |= (v >> 2UL);
  v |= (v >> 4UL);
  v |= (v >> 8UL);
  v |= (v >> 16UL);
  v |= (v >> 32UL);
  return v + 1;
}

/// Returns the next power of two (in 64-bits) that is equal or greater to the given value.
/// @note Returns zero on overflow.
/// @note Same as `next_power_of_two` but will return the given value if it is already a power of two.
[[nodiscard]] inline constexpr uint64_t round_to_power_of_two(uint64_t v) noexcept {
  return v ? __zb::next_power_of_two(v - 1) : 1;
}

///
template <typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] inline constexpr bool will_next_power_of_two_overflow(T v) noexcept {
  if constexpr (std::is_same_v<std::remove_cvref_t<T>, uint64_t>) {
    return __zb::next_power_of_two((uint64_t)v) < v;
  }
  else {
    return __zb::next_power_of_two((uint64_t)v) > (uint64_t)(std::numeric_limits<T>::max)();
  }
}

// https://graphics.stanford.edu/~seander/bithacks.html#IntegerLog
[[nodiscard]] inline constexpr int32_t log2_of_power_of_two(uint32_t v) {
  constexpr std::array<int32_t, 32> multiply_debruijn_bit_position_2 = {
    0, 1, 28, 2, 29, 14, 24, 3, //
    30, 22, 20, 15, 25, 17, 4, 8, //
    31, 27, 13, 23, 21, 19, 16, 7, //
    26, 12, 18, 6, 11, 5, 10, 9 //
  };

  return multiply_debruijn_bit_position_2[(uint32_t)(v * 0x077CB531U) >> (uint32_t)27];
}

template <class _T, std::enable_if_t<std::is_integral_v<_T>, bool> = true>
[[nodiscard]] inline constexpr bool is_multiple_of_power_of_two(_T x, _T powTwoMultiple) {
  zbase_assert(__zb::is_power_of_two(powTwoMultiple) && "powTwoMultiple must be a power of two");
  return (x & (powTwoMultiple - 1)) == 0;
}

template <class T, class T2, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] inline constexpr T round_up_to_multiple_of_power_of_two(T numToRound, T2 multiple) noexcept {
  zbase_assert(__zb::is_power_of_two(T(multiple)) && "multiple must be a power of two");
  return (numToRound + T(multiple) - T(1)) & -T(multiple);
}

template <class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] inline constexpr bool is_multiple_of_4(T v) noexcept {
  return !(v & 3);
}

template <class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] inline constexpr bool is_multiple_of_8(T v) noexcept {
  return !(v & 7);
}

template <class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] inline constexpr bool is_multiple_of_16(T v) noexcept {
  return !(v & 15);
}

template <class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] inline constexpr bool is_multiple_of_32(T v) noexcept {
  return !(v & 31);
}

template <class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] inline constexpr bool is_multiple_of_64(T v) noexcept {
  return !(v & 63);
}

template <class T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
[[nodiscard]] inline constexpr T round_up(T numToRound, T multiple) noexcept {
  zbase_assert(multiple);
  T isPositive = (T)(numToRound >= 0);
  return ((numToRound + isPositive * (multiple - T(1))) / multiple) * multiple;
}

template <class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
[[nodiscard]] inline constexpr T sinc(T x) noexcept {
  return x == T(0) ? T(1) : std::sin(x) / x;
}

template <class T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
[[nodiscard]] inline constexpr T ssinc(T x) noexcept {
  return x == T(0) ? T(1) : std::sin(x * __zb::pi<T>) / (x * __zb::pi<T>);
}

template <class T>
[[nodiscard]] inline constexpr T square(T x) noexcept {
  return x * x;
}

ZBASE_END_NAMESPACE
