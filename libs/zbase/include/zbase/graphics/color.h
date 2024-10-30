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

/* @WP-DOC (graphics/color.md)
# Color
*/

#include <zbase/zbase.h>
#include <zbase/sys/error_code.h>
#include <zbase/utility/traits.h>

#include <algorithm>
#include <iomanip>
#include <ostream>
#include <cmath>

ZBASE_BEGIN_NAMESPACE

namespace detail {
template <typename T>
static inline uint32_t color_float_component_to_uint32(T f) noexcept {
  return static_cast<uint32_t>(std::floor(f * 255.0));
}
} // namespace detail.

class color {
public:
  template <std::floating_point T>
  struct float_rgba {
    T r, g, b, a;

    [[nodiscard]] inline T* data() noexcept { return &r; }
    [[nodiscard]] inline const T* data() const noexcept { return &r; }
    [[nodiscard]] static inline size_t size() noexcept { return 4; }
  };

  template <std::floating_point T>
  struct float_rgb {
    T r, g, b;

    [[nodiscard]] inline T* data() noexcept { return &r; }
    [[nodiscard]] inline const T* data() const noexcept { return &r; }
    [[nodiscard]] static inline size_t size() noexcept { return 3; }
  };

  template <std::floating_point T>
  struct float_grey_alpha {
    T grey, alpha;

    [[nodiscard]] inline T* data() noexcept { return &grey; }
    [[nodiscard]] inline const T* data() const noexcept { return &grey; }
    [[nodiscard]] static inline size_t size() noexcept { return 2; }
  };

  static inline constexpr color black() noexcept { return color(0x000000ff); }
  static inline constexpr color white() noexcept { return color(0xffffffff); }

  inline constexpr color() noexcept = default;
  inline constexpr color(const color&) noexcept = default;
  inline constexpr color(color&&) noexcept = default;

  inline constexpr ~color() noexcept = default;

  inline constexpr color& operator=(const color&) noexcept = default;
  inline constexpr color& operator=(color&&) noexcept = default;

  inline constexpr color(uint32_t rgba) noexcept
      : _rgba(rgba) {}

  inline constexpr color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) noexcept {
    using u32 = uint32_t;
    _rgba
        = (u32(a) << shift_alpha) | (u32(b) << shift_blue) | (u32(g) << shift_green) | (u32(r) << shift_red);
  }

  static inline constexpr color from_argb(uint32_t argb) noexcept {
    const color c(argb);
    return color(c.green(), c.blue(), c.alpha(), c.red());
  }

  template <typename T>
  inline constexpr color(const float_rgba<T>& rgba) noexcept
      : color(rgba.data(), rgba.size()) {}

  template <typename T>
  inline constexpr color(const float_rgb<T>& rgb) noexcept
      : color(rgb.data(), rgb.size()) {}

  template <typename T>
  inline constexpr color(const float_grey_alpha<T>& ga) noexcept
      : color(ga.data(), ga.size()) {}

  template <std::floating_point T, size_t Size>
  inline constexpr color(const T (&data)[Size]) noexcept
      : color(&data[0], Size) {}

  template <std::floating_point T>
  inline constexpr color(const T* data, size_t size) noexcept {
    switch (size) {
    case 2: {
      uint32_t u = detail::color_float_component_to_uint32(data[0]);
      uint32_t a = detail::color_float_component_to_uint32(data[1]);
      _rgba = (a << shift_alpha) | (u << shift_blue) | (u << shift_green) | (u << shift_red);
    }
      return;
    case 3: {
      uint32_t ur = detail::color_float_component_to_uint32(data[0]);
      uint32_t ug = detail::color_float_component_to_uint32(data[1]);
      uint32_t ub = detail::color_float_component_to_uint32(data[2]);
      _rgba = (255 << shift_alpha) | (ub << shift_blue) | (ug << shift_green) | (ur << shift_red);
    }
      return;
    case 4: {
      uint32_t ur = detail::color_float_component_to_uint32(data[0]);
      uint32_t ug = detail::color_float_component_to_uint32(data[1]);
      uint32_t ub = detail::color_float_component_to_uint32(data[2]);
      uint32_t ua = detail::color_float_component_to_uint32(data[3]);
      _rgba = (ua << shift_alpha) | (ub << shift_blue) | (ug << shift_green) | (ur << shift_red);
    }
      return;
    }
  }

  static inline __zb::error_result from_string(std::string_view s, color& col) noexcept {
    if (s[0] == '#') {
      s = s.substr(1);
    }

    if (s.size() == 6) {
      std::array<char, 3> buffer = {};
      __zb::error_result err;

      auto get_component = [&](size_t n) -> uint8_t {
        buffer[0] = s[n * 2];
        buffer[1] = s[n * 2 + 1];

        unsigned value = 0;
        if (sscanf(buffer.data(), "%02x", &value) != 1) {
          err = __zb::error_code::conversion_error;
          return 0;
        }

        return (uint8_t)value;
      };

      uint8_t r = get_component(0);
      uint8_t g = get_component(1);
      uint8_t b = get_component(2);

      if (err) {
        return err;
      }

      col = color(r, g, b);
      return {};
    }
    else if (s.size() == 8) {
      std::array<char, 3> buffer = {};

      __zb::error_result err;
      auto get_component = [&](size_t n) -> uint8_t {
        buffer[0] = s[n * 2];
        buffer[1] = s[n * 2 + 1];

        unsigned value = 0;
        if (sscanf(buffer.data(), "%02x", &value) != 1) {
          err = __zb::error_code::conversion_error;
          return 0;
        }

        return (uint8_t)value;
      };

      uint8_t r = get_component(0);
      uint8_t g = get_component(1);
      uint8_t b = get_component(2);
      uint8_t a = get_component(3);

      if (err) {
        return err;
      }

      col = color(r, g, b, a);
      return {};
    }

    return __zb::error_code::conversion_error;
  }
  //

  inline static color from_hsv(float h, float s, float v) {
    h /= 360.0f;
    s /= 100.0f;
    v *= 2.55f;

    int i = std::floor(h * 6.0f);
    float f = h * 6 - i;

    switch (i % 6) {
    case 0:
      return color(v, v * (1.0f - (1.0f - f) * s), v * (1.0f - s));
    case 1:
      return color(v * (1.0f - f * s), v, v * (1.0f - s));
    case 2:
      return color(v * (1.0f - s), v, v * (1.0f - (1.0f - f) * s));
    case 3:
      return color(v * (1.0f - s), v * (1.0f - f * s), v);
    case 4:
      return color(v * (1.0f - (1.0f - f) * s), v * (1.0f - s), v);
    case 5:
      return color(v, v * (1.0f - s), v * (1.0f - f * s));
    default:
      return 0;
    }
  }

  //  inline static color from_hsv(float h, float s, float v) {
  //    using frgb = float_rgb<float>;
  //
  //    h /= 360.0f;
  //    s /= 100.0f;
  //    v /= 100.0f;
  //
  //    int i = std::floor(h * 6.0f);
  //    float f = h * 6 - i;
  //
  //    switch (i % 6) {
  //    case 0:
  //      return frgb{ v, v * (1.0f - (1.0f - f) * s), v * (1.0f - s) };
  //    case 1:
  //      return frgb{ v * (1.0f - f * s), v, v * (1.0f - s) };
  //    case 2:
  //      return frgb{ v * (1.0f - s), v, v * (1.0f - (1.0f - f) * s) };
  //    case 3:
  //      return frgb{ v * (1.0f - s), v * (1.0f - f * s), v };
  //    case 4:
  //      return frgb{ v * (1.0f - (1.0f - f) * s), v * (1.0f - s), v };
  //    case 5:
  //      return frgb{ v, v * (1.0f - s), v * (1.0f - f * s) };
  //    default:
  //      return 0;
  //    }
  //  }

  [[nodiscard]] inline constexpr uint32_t& rgba() noexcept { return _rgba; }

  [[nodiscard]] inline constexpr const uint32_t& rgba() const noexcept { return _rgba; }

  [[nodiscard]] inline constexpr uint32_t argb() const noexcept {
    using u32 = uint32_t;
    return (u32(blue()) << shift_alpha) | (u32(green()) << shift_blue) | (u32(red()) << shift_green)
        | (u32(alpha()) << shift_red);
  }

  [[nodiscard]] inline constexpr float_rgba<float> f_rgba() const noexcept {
    return { f_red(), f_green(), f_blue(), f_alpha() };
  }

  template <std::floating_point T>
  [[nodiscard]] inline constexpr float_rgba<T> f_rgba() const noexcept {
    return { static_cast<T>(f_red()), static_cast<T>(f_green()), static_cast<T>(f_blue()),
      static_cast<T>(f_alpha()) };
  }

  template <__zb::arithmetic T>
  [[nodiscard]] inline constexpr T red() const noexcept {
    if constexpr (std::is_floating_point_v<T>) {
      return static_cast<T>(f_red());
    }
    else {
      return static_cast<T>(red());
    }
  }

  template <__zb::arithmetic T>
  [[nodiscard]] inline constexpr T green() const noexcept {
    if constexpr (std::is_floating_point_v<T>) {
      return static_cast<T>(f_green());
    }
    else {
      return static_cast<T>(green());
    }
  }

  template <__zb::arithmetic T>
  [[nodiscard]] inline constexpr T blue() const noexcept {
    if constexpr (std::is_floating_point_v<T>) {
      return static_cast<T>(f_blue());
    }
    else {
      return static_cast<T>(blue());
    }
  }

  template <__zb::arithmetic T>
  [[nodiscard]] inline constexpr T alpha() const noexcept {
    if constexpr (std::is_floating_point_v<T>) {
      return static_cast<T>(f_alpha());
    }
    else {
      return static_cast<T>(alpha());
    }
  }

  inline constexpr color& red(uint8_t r) noexcept {
    _rgba = (_rgba & ~red_mask) | (uint32_t(r) << shift_red);
    return *this;
  }

  inline constexpr color& green(uint8_t g) noexcept {
    _rgba = (_rgba & ~green_mask) | (uint32_t(g) << shift_green);
    return *this;
  }

  inline constexpr color& blue(uint8_t b) noexcept {
    _rgba = (_rgba & ~blue_mask) | (uint32_t(b) << shift_blue);
    return *this;
  }

  inline constexpr color& alpha(uint8_t a) noexcept {
    _rgba = (_rgba & ~alpha_mask) | (uint32_t(a) << shift_alpha);
    return *this;
  }

  [[nodiscard]] inline constexpr uint8_t red() const noexcept {
    return static_cast<uint8_t>((_rgba & red_mask) >> shift_red);
  }

  [[nodiscard]] inline constexpr uint8_t green() const noexcept {
    return static_cast<uint8_t>((_rgba & green_mask) >> shift_green);
  }

  [[nodiscard]] inline constexpr uint8_t blue() const noexcept {
    return static_cast<uint8_t>((_rgba & blue_mask) >> shift_blue);
  }

  [[nodiscard]] inline constexpr uint8_t alpha() const noexcept {
    return static_cast<uint8_t>((_rgba & alpha_mask) >> shift_alpha);
  }

  [[nodiscard]] inline constexpr float f_red() const noexcept { return red() / 255.0f; }
  [[nodiscard]] inline constexpr float f_green() const noexcept { return green() / 255.0f; }
  [[nodiscard]] inline constexpr float f_blue() const noexcept { return blue() / 255.0f; }
  [[nodiscard]] inline constexpr float f_alpha() const noexcept { return alpha() / 255.0f; }

  [[nodiscard]] inline constexpr bool is_opaque() const noexcept { return alpha() == 255; }

  [[nodiscard]] inline constexpr bool is_transparent() const noexcept { return alpha() == 0; }

  [[nodiscard]] inline constexpr color darker(float amount) const noexcept {
    amount = 1.0f - std::clamp<float>(amount, 0.0f, 1.0f);
    return color(uint8_t(red() * amount), uint8_t(green() * amount), uint8_t(blue() * amount), alpha());
  }

  [[nodiscard]] inline constexpr color brighter(float amount) const noexcept {
    const float ratio = 1.0f / (1.0f + std::abs(amount));
    const float mu = 255 * (1.0f - ratio);

    return color(static_cast<uint8_t>(mu + ratio * red()), // r
        static_cast<uint8_t>(mu + ratio * green()), // g
        static_cast<uint8_t>(mu + ratio * blue()), // b
        alpha() // a
    );
  }

  /// mu should be between [0, 1]
  [[nodiscard]] inline constexpr color operator*(float mu) const noexcept {
    return color(static_cast<uint8_t>(red() * mu), static_cast<uint8_t>(green() * mu),
        static_cast<uint8_t>(blue() * mu), static_cast<uint8_t>(alpha() * mu));
  }

  [[nodiscard]] inline constexpr bool operator==(const color& c) const noexcept { return _rgba == c._rgba; }

  [[nodiscard]] inline constexpr bool operator!=(const color& c) const noexcept { return !operator==(c); }

  template <class CharT, class TraitsT>
  friend inline std::basic_ostream<CharT, TraitsT>& operator<<(
      std::basic_ostream<CharT, TraitsT>& s, const color& c);

private:
  uint32_t _rgba = 0;

  static constexpr uint32_t shift_red = 24;
  static constexpr uint32_t shift_green = 16;
  static constexpr uint32_t shift_blue = 8;
  static constexpr uint32_t shift_alpha = 0;

  static constexpr uint32_t red_mask = 0xff000000;
  static constexpr uint32_t green_mask = 0x00ff0000;
  static constexpr uint32_t blue_mask = 0x0000ff00;
  static constexpr uint32_t alpha_mask = 0x000000ff;
};
//

template <class CharT, class TraitsT>
inline std::basic_ostream<CharT, TraitsT>& operator<<(std::basic_ostream<CharT, TraitsT>& s, const color& c) {
  std::ios_base::fmtflags flags(s.flags());
  s << CharT('#') << std::uppercase << std::hex << std::setfill(CharT('0')) << std::setw(8) << c.rgba();
  s.flags(flags);
  return s;
}

ZBASE_END_NAMESPACE
