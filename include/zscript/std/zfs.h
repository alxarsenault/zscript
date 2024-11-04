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

#include <zscript/zscript.h>
#include <zscript/std/zmutable_string.h>

namespace zs {
ZS_CHECK zs::object create_fs_lib(zs::vm_ref vm);

/// Creates a filesystem path by moving the zs::string 's'.
ZS_CHECK zs::object create_path(zs::vm_ref vm, zs::string&& s) noexcept;

/// Creates a filesystem path by moving the zs::mutable_string 's'.
ZS_CK_INLINE object create_path(zs::vm_ref vm, zs::mutable_string&& s) noexcept {
  return create_path(vm, (zs::string&&)std::move(s));
}

/// Creates a filesystem path from the string_view 's'.
ZS_CHECK zs::object create_path(zs::vm_ref vm, std::string_view s) noexcept;

/// Creates a filesystem path from the string 's'.
/// Same as creating a filesystem path from a string_view.
ZS_CK_INLINE object create_path(zs::vm_ref vm, const char* s) noexcept {
  return create_path(vm, std::string_view(s));
}

/// Creates a filesystem path from the string 's'.
/// Same as creating a filesystem path from a string_view.
ZS_CK_INLINE object create_path(zs::vm_ref vm, const std::string& s) noexcept {
  return create_path(vm, std::string_view(s));
}

/// Creates a filesystem path from the string 's'.
/// Same as creating a filesystem path from a string_view.
ZS_CK_INLINE object create_path(zs::vm_ref vm, const zs::string& s) noexcept {
  return create_path(vm, std::string_view(s));
}

/// Creates a filesystem path from the string 's'.
/// Same as creating a filesystem path from a string_view.
ZS_CK_INLINE object create_path(zs::vm_ref vm, const zs::mutable_string& s) noexcept {
  return create_path(vm, std::string_view(s));
}

/// Creates a filesystem path from a vm function call.
ZS_CHECK int_t vm_create_path(zs::vm_ref vm) noexcept;

/// Returns true if 'obj' is a filesystem path.
ZS_CHECK bool is_path(const object& obj) noexcept;

mutable_string& get_path(const object& obj) noexcept;

} // namespace zs.
