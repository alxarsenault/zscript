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

/// @file zinclude.h
/// @author Alexandre Arsenault
/// @date October 6 2024
/// @brief Contains all the `.cc` files to include in `zscript.cpp`.
/// @warning This file is auto-generated.
///
/// File naming convention:
/// * Any `*.cc` files will be compiled as part of `zscript.cpp`.
/// * Any `*.cpp` files are compiled normally.
///
/// @note Even if auto-generated, any changes to this file should be commited.

#include "bytecode/zinstruction_vector.cc"
#include "json/zjson_lexer.cc"
#include "json/zjson_parser.cc"
#include "lang/jit/zclosure_compile_state.cc"
#include "lang/jit/zjit_compiler.cc"
#include "lang/zcompiler.cc"
#include "lex/zlexer.cc"
#include "lib/a0/zgraphics.cc"
#include "lib/delegate/zarray_delegate.cc"
#include "lib/delegate/zcolor_delegate.cc"
#include "lib/delegate/znative_array_delegate.cc"
#include "lib/delegate/zstring_delegate.cc"
#include "lib/delegate/zstruct_delegate.cc"
#include "lib/delegate/ztable_delegate.cc"
#include "lib/zfs/zfile.cc"
#include "lib/zfs/zfs.cc"
#include "lib/zlang.cc"
#include "lib/zmath.cc"
#include "lib/zsbase64.cc"
#include "lib/zslib.cc"
#include "objects/zfunction_prototype.cc"
#include "xml/zxml_lexer.cc"
#include "xml/zxml_parser.cc"
#include "zvirtual_machine.cc"
#include "zvm_ref.cc"
