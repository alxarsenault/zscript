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

#if __ZBASE_APPLE__

#include <zbase/memory/callable_object.h>
#include <zbase/sys/apple/cf/type.h>
#include <zbase/sys/apple/cf/pointer.h>
#include <zbase/sys/apple/cf/string_ref.h>

ZBASE_BEGIN_SUB_NAMESPACE(apple, cf)
class run_loop {
public:
  run_loop();
  ~run_loop();

  void run();

  void stop();

  void perform(std::unique_ptr<__zb::task> task);

  inline CFRunLoopRef native() const { return _native_run_loop; }

private:
  CFRunLoopRef _native_run_loop;
};
ZBASE_END_SUB_NAMESPACE(apple, cf)
#endif // __ZBASE_APPLE__