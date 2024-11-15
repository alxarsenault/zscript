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
#include "lang/jit/zclosure_compile_state.h"
#include "lang/preprocessor/zmacro.h"

namespace zs {
class preprocessor;

///
class macro_parser : public zs::engine_holder, zs::lexer_ref {
public:
  macro_parser(zs::preprocessor* pp);

  zs::error_result parse(std::string_view input_code, zs::string& output_code, bool& found_macro,
      zs::virtual_machine* vm = nullptr);

private:
  zs::preprocessor* _preprocessor;

  zs::error_result parse_macro_call(zs::string& output, const char*& end_ptr);
  zs::error_result replace_pass(zs::string& output_code, bool& keep_going);
  zs::error_result replace_macro_content(
      zs::string& output_code, const macro& m, zs::array_object& in_params);

  zs::error_result look_and_replace_one_macro_call(zs::macro& m, zs::string& output_code, bool& found_one);
};

} // namespace zs.
