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
#include <zbase/utils.h>
#include <zbase/geometry/concepts.h>
#include <ostream>

ZBASE_BEGIN_NAMESPACE

//
template <class T>
struct border {
  using value_type = T;
  value_type top;
  value_type left;
  value_type bottom;
  value_type right;

  border() noexcept = default;
  border(const border&) noexcept = default;
  border(border&&) noexcept = default;

  ~border() noexcept = default;

  border& operator=(const border&) noexcept = default;
  border& operator=(border&&) noexcept = default;

  inline constexpr border(value_type t, value_type l, value_type b, value_type r) noexcept
      : top(t)
      , left(l)
      , bottom(b)
      , right(r) {}

  inline constexpr border(value_type p) noexcept
      : top(p)
      , left(p)
      , bottom(p)
      , right(p) {}

  inline bool is_empty() const noexcept { return __zb::all_equals(top, left, bottom, right) && top == 0; }
};

template <typename T>
inline std::ostream& operator<<(std::ostream& s, const __zb::border<T>& b) {
  return s << "{" << b.top << ", " << b.left << ", " << b.bottom << ", " << b.right << "}";
}

ZBASE_END_NAMESPACE
