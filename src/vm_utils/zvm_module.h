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

namespace zs {
zs::error_result import_module(zs::vm_ref vm, const zs::object& name, zs::object& output_module);

zs::error_result try_import_module_from_cache(
    zs::vm_ref vm, const zs::object& name, zs::object& output_module);

zs::error_result compile_or_load_buffer(
    zs::vm_ref vm, zb::byte_view content, const object& filename, object& output_closure);

zs::error_result compile_or_load_buffer(
    zs::vm_ref vm, zb::byte_view content, std::string_view filename, object& output_closure);

zs::error_result compile_or_load_buffer(
    zs::vm_ref vm, zb::byte_view content, const char* filename, object& output_closure);

zs::error_result compile_or_load_file(zs::vm_ref vm, const char* filename, object& output_closure);
zs::error_result compile_or_load_file(zs::vm_ref vm, const object& filename, object& output_closure);
zs::error_result compile_or_load_file(zs::vm_ref vm, std::string_view filename, object& output_closure);

} // namespace zs.
