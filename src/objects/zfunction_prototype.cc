#include "zinclude_guard.h"

namespace zs {

function_prototype_object::function_prototype_object(zs::engine* eng)
    : reference_counted_object(eng, zs::object_type::k_function_prototype)
    , _vlocals(zs::allocator<zs::local_var_info_t>(eng))
    , _literals(zs::allocator<zs::object>(eng))
    , _default_params(zs::allocator<zs::int_t>(eng))
    , _parameter_names(zs::allocator<zs::object>(eng))
    , _restricted_types(zs::allocator<zs::object>(eng))
    , _functions(zs::allocator<zs::object>(eng))
    , _captures(zs::allocator<zs::captured_variable>(eng))
    , _line_info(zs::allocator<zs::line_info_op_t>(eng))

#if ZS_DEBUG
    , _debug_line_info(zs::allocator<zs::line_info_op_t>(eng))
#endif

    , _instructions(eng) {
}

function_prototype_object* function_prototype_object::create(zs::engine* eng) {
  function_prototype_object* fpo = internal::zs_new<function_prototype_object>(eng, eng);
  return fpo;
}

zs::object function_prototype_object::find_function(std::string_view name) const {

  for (const auto& f : _functions) {
    if (f._fproto->_name == name) {
      return f;
    }
  }

  return nullptr;
}

void function_prototype_object::debug_print(std::ostream& stream) const {
  zb::stream_print(stream, "Name: ", _name, "\n");
  zb::stream_print(stream, "SourceName: ", _source_name, "\n");
  zb::stream_print(stream, "Stack Size : ", _stack_size, "\n");
  zb::stream_print(stream, "Locals : ", _vlocals, "\n");
  zb::stream_print(stream, "Literals : ", _literals, "\n");
  zb::stream_print(stream, "_n_capture : ", _n_capture, "\n");

  zb::stream_print(stream, "Instructions:\n");
  _instructions.debug_print(stream);
  //  _instructions.serialize(stream);
}

zs::error_result function_prototype_object::serialize(zs::vector<uint8_t>& buffer) {
  zs::engine* eng = _engine;
  zs::object stable = zs::object::create_table(eng);
  zs::object_unordered_map<zs::object>& sbl = stable._table->get_map();

  sbl[zs::_ss("source_name")] = _source_name;
  sbl[zs::_ss("name")] = _name;
  sbl[zs::_ss("stack_size")] = _stack_size;
  sbl[zs::_ss("literals")] = zs::object::create_array(eng, _literals);
  sbl[zs::_s(eng, "default_params")] = zs::object::create_array(eng, _default_params);
  sbl[zs::_s(eng, "parameter_names")] = zs::object::create_array(eng, _parameter_names);
  sbl[zs::_s(eng, "restricted_types")] = zs::object::create_array(eng, _restricted_types);

  sbl[zs::_ss("n_capture")] = _n_capture;

  size_t sz = 0;
  if (auto err = stable.to_binary(buffer, sz, 0)) {
    return err;
  }

  return {};
}
} // namespace zs.
