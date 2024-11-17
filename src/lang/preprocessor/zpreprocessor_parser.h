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
#include "lang/zpreprocessor.h"

#define ZS_PREPROCESSOR_EXPECT(...) \
  ZBASE_DEFER(ZBASE_CONCAT(__ZS_PREPROCESSOR_EXPECT_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)

#define ZS_PREPROCESSOR_ERROR(err, ...) \
  ZBASE_DEFER(ZBASE_CONCAT(ZS_PREPROCESSOR_ERROR_, ZBASE_NARG_BINARY(__VA_ARGS__)), err, __VA_ARGS__)

#define __ZS_PREPROCESSOR_EXPECT_1(tok)                                                         \
  if (zs::error_result err = _preprocessor->expect(*this, tok, ZB_CURRENT_SOURCE_LOCATION())) { \
    return err;                                                                                 \
  }

#define __ZS_PREPROCESSOR_EXPECT_2(tok, ec)                                                         \
  if (zs::error_result err = _preprocessor->expect(*this, tok, ec, ZB_CURRENT_SOURCE_LOCATION())) { \
    return err;                                                                                     \
  }

#define ZS_PREPROCESSOR_ERROR_1(err, msg) \
  _preprocessor->handle_error(*this, err, msg, ZB_CURRENT_SOURCE_LOCATION())

#define ZS_PREPROCESSOR_ERROR_MULTIPLE(err, ...) \
  _preprocessor->handle_error(*this, err, zs::sstrprint(_engine, __VA_ARGS__), ZB_CURRENT_SOURCE_LOCATION())

namespace zs {} // namespace zs.
