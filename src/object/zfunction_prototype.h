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

#include <zscript/zscript.h>
#include "jit/zclosure_compile_state.h"
#include <zbase/container/byte.h>

namespace zs {

///
class function_prototype_object final : public engine_holder {
public:
  ZS_CLASS_COMMON;

  ZS_CHECK static function_prototype_object& as_proto(const object_base& obj);

  ZS_CHECK static bool is_proto(const object_base& obj) noexcept;

  static constexpr std::array<uint8_t, 4> k_compiled_header = { 'Z', 'S', 'C', 'C' };

   ZB_CHECK static object create(zs::engine* eng);

  ZS_CHECK static bool is_compiled_data(zb::byte_view content) noexcept {
    return content.subspan(0, 4) == k_compiled_header;
  }

  ~function_prototype_object() = default;

  ZS_CHECK zs::error_result save(zb::byte_vector& buffer);
  ZS_CHECK zs::error_result save(zs::write_function_t write_func, void* udata);

  ZS_CHECK zs::error_result load(zb::byte_view buffer);

  zs::object find_function(std::string_view name) const;
  const zs::local_var_info_t* find_local(std::string_view name) const;
  const zs::local_var_info_t* find_local(const zs::object& name) const;

  void debug_print(std::ostream& stream = std::cout) const;

  int_t get_parameters_count() const noexcept;
  int_t get_default_parameters_count() const noexcept;

  int_t get_minimum_required_parameters_count() const noexcept;
  bool is_possible_parameter_count(size_t sz) const noexcept;
  bool is_valid_parameters(zs::vm_ref vm, zb::span<const object> params, int_t& n_type_match) const noexcept;

private: 
  function_prototype_object(zs::engine* eng);

public:
  zs::object _source_name;
  zs::object _name;
  zs::object _module_info;
  zs::int_t _stack_size;
  bool _has_vargs_params = false;

  zs::vector<zs::local_var_info_t> _vlocals;
  zs::vector<zs::object> _literals;
  zs::vector<int_t> _default_params;
  zs::small_vector<zs::object, 8> _parameter_names;
  zs::small_vector<zs::object, 8> _restricted_types;
  zs::vector<zs::object> _functions;
  zs::vector<zs::captured_variable> _captures;
  size_t _n_capture = 0;

  // Line infos.
  zs::vector<zs::line_info_op_t> _line_info;

#if ZS_DEBUG
  zs::vector<zs::line_info_op_t> _debug_line_info;
#endif

  zs::instruction_vector _instructions;
};

} // namespace zs.
