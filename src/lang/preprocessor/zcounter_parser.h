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

class counter_parser : public zs::engine_holder, zs::lexer_ref {
public:
  counter_parser(zs::preprocessor* pp);

  zs::error_result parse(std::string_view input_code, zs::string& output_code, bool& did_include,
      zs::virtual_machine* vm = nullptr);

private:
  zs::preprocessor* _preprocessor;
};

} // namespace zs.
