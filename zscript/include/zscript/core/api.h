#pragma once

#include <zscript/core/common.h>
#include <zscript/core/types.h>

namespace zs {

// Allocation interface.
// Calls engine.

/// Allocate memory.
void* allocate(zs::engine* eng, size_t sz, alloc_info_t ainfo = alloc_info_t{});

/// Reallocate memory.
void* reallocate(
    zs::engine* eng, void* ptr, size_t size, size_t old_size, alloc_info_t ainfo = alloc_info_t{});

/// Deallocate memory.
void deallocate(zs::engine* eng, void* ptr, alloc_info_t ainfo = alloc_info_t{});

} // namespace zs.
