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

#include <zscript.h>
#include <zbase/graphics/color.h>

namespace zs {
zs::object create_graphics_lib(zs::virtual_machine* vm);

inline constexpr zs::object create_color(zb::color c) noexcept {
  return object(object_base{ { ._color = c.rgba() },
                    { { { 0 }, { ._ext_type = extension_type::kext_color }, object_type::k_extension,
                        object_flags_t::f_none } } },
      false);
}

template <class... Args>
inline constexpr zs::object create_color(Args&&... args) noexcept {
  return create_color(zb::color(args...));
}

inline constexpr zs::object create_hsv_color(float_t h, float_t s, float_t v) noexcept {
  return object(object_base{ { ._color = zb::color::from_hsv(h, s, v).rgba() },
                    { { { 0 }, { ._ext_type = extension_type::kext_color }, object_type::k_extension,
                        object_flags_t::f_none } } },
      false);
}

inline constexpr zs::object create_hsv_color(float_t h, float_t s, float_t v, float_t a) noexcept {
  return object(object_base{ { ._color = zb::color::from_hsv(h, s, v).alpha(a).rgba() },
                    { { { 0 }, { ._ext_type = extension_type::kext_color }, object_type::k_extension,
                        object_flags_t::f_none } } },
      false);
}

} // namespace zs.
