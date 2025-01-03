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

// #include <zscript/zscript.h>
#include <zscript/object.h>
#include <zscript/base/container/stack.h>

namespace zs {
class object_stack : public zb::execution_stack<object, zs::allocator<object>> {
public:
  using stack_type = zb::execution_stack<object, zs::allocator<object>>;
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
