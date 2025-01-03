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

namespace zs {

///
ZS_CHECK zs::error_result load_buffer_as_value(
    zs::vm_ref vm, std::string_view content, std::string_view source_name, zs::object& value);

///
ZS_CHECK zs::error_result load_buffer_as_array(zs::vm_ref vm, std::string_view content,
    std::string_view source_name, zs::object& value, std::string_view sep = ",");

///
ZS_CHECK zs::error_result load_file_as_value(
    zs::vm_ref vm, const char* filepath, std::string_view source_name, zs::object& result_value);

ZS_CK_INLINE zs::error_result load_file_as_value(zs::vm_ref vm, const std::filesystem::path& filepath,
    std::string_view source_name, zs::object& result_value) {
  return load_file_as_value(vm, filepath.c_str(), source_name, result_value);
}

///
ZS_CHECK zs::error_result load_file_as_array(zs::vm_ref vm, const char* filepath,
    std::string_view source_name, zs::object& result_value, std::string_view sep = ",");

ZS_CK_INLINE zs::error_result load_file_as_array(zs::vm_ref vm, const std::filesystem::path& filepath,
    std::string_view source_name, zs::object& result_value, std::string_view sep = ",") {
  return load_file_as_array(vm, filepath.c_str(), source_name, result_value, sep);
}

ZS_CHECK zs::error_result load_json_table(
    zs::vm_ref vm, std::string_view content, const object& table, object& output);

ZS_CHECK zs::error_result load_json_file(
    zs::vm_ref vm, const char* filepath, const object& table, object& output);

ZS_CHECK zs::error_result load_json_file(
    zs::vm_ref vm, std::string_view filepath, const object& table, object& output);

ZS_CK_INLINE zs::error_result load_json_file(zs::vm_ref vm, const char* filepath, object& output) {
  return zs::load_json_file(vm, filepath, nullptr, output);
}

ZS_CK_INLINE zs::error_result load_json_file(zs::vm_ref vm, std::string_view filepath, object& output) {
  return zs::load_json_file(vm, filepath, nullptr, output);
}
} // namespace zs.
