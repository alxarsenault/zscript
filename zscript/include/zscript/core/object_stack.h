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

#include <zscript/zscript.h>
#include <zbase/container/stack.h>

namespace zs {

namespace detail {
  using object_stack_base
      = zb::execution_stack<object, zs::allocator<object>, constants::k_is_object_stack_resizable>;
} // namespace detail.

class object_stack : public detail::object_stack_base {
public:
  using stack_type = detail::object_stack_base;
  using value_type = typename stack_type::difference_type;
  using size_type = typename stack_type::size_type;
  using difference_type = typename stack_type::difference_type;
  using reference = typename stack_type::reference;
  using const_reference = typename stack_type::const_reference;
  using allocator_type = typename stack_type::allocator_type;
  using state = typename stack_type::state;

  ZB_INLINE object_stack(zs::engine* eng, size_type init_stack_size = ZS_DEFAULT_STACK_SIZE)
      : stack_type(init_stack_size, zs::allocator<object>(eng, memory_tag::nt_vm)) {}
};

} // namespace zs.
