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

namespace zs {
#define __ZS_TOK_IMPL_LAST(name) zs::token_type::tok_##name
#define __ZS_TOK_IMPL(name) zs::token_type::tok_##name,
#define ZS_TOK(...) ZBASE_FOR_EACH_WITH_LAST(__ZS_TOK_IMPL, __ZS_TOK_IMPL_LAST, __VA_ARGS__)

enum class token_type : uint8_t {
#define ZS_DECL_TOKEN(name) tok_##name,
#include "ztoken_def.h"
#undef ZS_DECL_TOKEN
};

inline constexpr const char* token_to_string(token_type token) noexcept {
  switch (token) {
#define ZS_DECL_TOKEN(name)    \
  case token_type::tok_##name: \
    return #name;
#include "ztoken_def.h"
#undef ZS_DECL_TOKEN
  }

  return "unknown";
}
} // namespace zs.
