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
#include "lang/jit/zclosure_compile_state.h"

namespace zs {

///
class function_prototype_object final : public reference_counted_object {
public:
  ZS_CLASS_COMMON;

  ZB_CHECK static function_prototype_object* create(zs::engine* eng);

  virtual ~function_prototype_object() override = default;

  zs::error_result serialize(zs::vector<uint8_t>& buffer);

  zs::object find_function(std::string_view name) const;

  void debug_print(std::ostream& stream = std::cout) const;

private:
  function_prototype_object(zs::engine* eng);

public:
  zs::object _source_name;
  zs::object _name;
  zs::object _module_name;
  //  zs::object _export_table;
  zs::int_t _stack_size;

  zs::vector<zs::local_var_info_t> _vlocals;
  zs::vector<zs::object> _literals;
  zs::vector<int_t> _default_params;
  zs::small_vector<zs::object, 8> _parameter_names;
  zs::small_vector<zs::object, 8> _restricted_types;
  zs::vector<zs::object> _functions;
  zs::vector<zs::captured_variable> _captures;
  size_t _n_capture = 0;
  int_t _export_table_target = -1;

  // Line infos.
  zs::vector<zs::line_info_op_t> _line_info;

#if ZS_DEBUG
  zs::vector<zs::line_info_op_t> _debug_line_info;
#endif

  zs::instruction_vector _instructions;
};

} // namespace zs.
