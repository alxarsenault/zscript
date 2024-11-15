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
#include "lex/zlexer.h"

namespace zs {
class preprocessor;

///
class stringify_parser : public zs::engine_holder, zs::lexer_ref {
public:
  stringify_parser(zs::preprocessor* pp);

  zs::error_result parse(std::string_view content, std::string_view filename, object& output,
      bool& did_include, zs::virtual_machine* vm = nullptr);

  ZS_CK_INLINE const zs::string& get_error() const noexcept { return _error_message; }

private:
  zs::preprocessor* _preprocessor;
  zs::string _error_message;
  zs::error_result handle_error(zs::error_code ec, std::string_view msg, const zb::source_location& loc);

  ZS_CHECK zs::error_code expect(token_type tok) noexcept;
  ZS_CHECK zs::error_code expect_get(token_type tok, object& ret);
};

} // namespace zs.
