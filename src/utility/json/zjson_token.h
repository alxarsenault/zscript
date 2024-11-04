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

#include <zbase/zbase.h>
#include <stdint.h>

namespace zs {

enum class json_token_type : uint8_t {
#define ZS_DECL_TOKEN(name) tok_##name,
#include "zjson_token_def.h"
#undef ZS_DECL_TOKEN
};

inline constexpr const char* json_token_to_string(json_token_type token) noexcept {
  switch (token) {
#define ZS_DECL_TOKEN(name)         \
  case json_token_type::tok_##name: \
    return #name;
#include "zjson_token_def.h"
#undef ZS_DECL_TOKEN
  }

  return "unknown";
}
} // namespace zs.
