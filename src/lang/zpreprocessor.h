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
#include "lang/preprocessor/zmacro.h"

namespace zs {

///
class preprocessor : public zs::engine_holder {
public:
  preprocessor(zs::engine* eng);

  zs::error_result preprocess(
      std::string_view content, std::string_view filename, object& output, zs::virtual_machine* vm = nullptr);

  ZS_CK_INLINE const zs::string& get_error() const noexcept { return _error_message; }

  struct helper;

  // private:
  zs::vector<zs::macro> _macros;
  zs::object_unordered_set _imported_files_set;
  zs::object _uuid_map;
  zs::string _error_message;
  int_t _counter = 0;

  zs::error_result handle_error(
      const zs::lexer_ref& lx, zs::error_code ec, std::string_view msg, const zb::source_location& loc);

  zs::error_code expect(zs::lexer_ref& lx, token_type tok, const zb::source_location& loc) noexcept;
  zs::error_code expect(
      zs::lexer_ref& lx, token_type tok, zs::error_code err, const zb::source_location& loc) noexcept;

  template <class ParserType>
  zs::error_result parse(zs::string& input_code, zs::string& output_code, bool& found_any) {
    ParserType parser(this);
    if (auto err = parser.parse(input_code, output_code, found_any)) {
      return err;
    }

    input_code = output_code;
    return {};
  }
};

} // namespace zs.
