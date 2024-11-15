#pragma once

#include <zscript.h>

namespace zs {
zs::object create_ztest_lib(zs::virtual_machine* vm);

zs::error_result import_ztest_lib(zs::vm_ref vm);

std::string rewrite_code(std::string_view code);
} // namespace zs.
