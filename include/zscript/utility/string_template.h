#pragma once

#include <zscript.h>
namespace zs {

zs::string render_template_string(zs::vm_ref vm, const zs::object& tbl, std::string_view content,
    std::string_view l_quote = "≤≤", std::string_view r_quote = "≥≥");
} // namespace zs.
