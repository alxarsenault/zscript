#include "zclosure_compile_state.h"
#include "object/zfunction_prototype.h"

namespace zs {
closure_compile_state::closure_compile_state(
    private_constructor_tag, zs::engine* eng, jit::shared_state_data& sdata, closure_compile_state* parent)
    : jit::shared_state_data_ref(sdata)
    , _parent(parent)
    , _parameter_names(zs::allocator<zs::object>(eng))
    , _instructions(eng)
    , _target_stack(zs::allocator<target_type_info_t>(eng))
    , _literals(zs::unordered_map_allocator<object, int_t>(eng))
    , _vlocals(zs::allocator<zs::scoped_local_var_info_t>(eng))
    , _local_var_infos(zs::allocator<zs::local_var_info_t>(eng))
    , _line_info(zs::allocator<zs::line_info_op_t>(eng))
    , _functions(zs::allocator<object>(eng))
    , _default_params(zs::allocator<int_t>(eng))
    , _breaks(zs::allocator<size_t>(eng))
    , _unresolved_breaks(zs::allocator<size_t>(eng))
    , _continues(zs::allocator<size_t>(eng))
    , _unresolved_continues(zs::allocator<size_t>(eng))
    , _captures(zs::allocator<captured_variable>(eng))
#if ZS_DEBUG
    , _debug_line_info(zs::allocator<zs::line_info_op_t>(eng))
#endif
{
}

closure_compile_state::closure_compile_state(zs::engine* eng, jit::shared_state_data& sdata)
    : closure_compile_state(private_constructor_tag{}, eng, sdata, nullptr) {}

closure_compile_state::closure_compile_state(zs::engine* eng, closure_compile_state& parent)
    : closure_compile_state(private_constructor_tag{}, eng, parent._sdata, &parent) {}

closure_compile_state::closure_compile_state(
    zs::engine* eng, closure_compile_state& parent, const object& sname)
    : closure_compile_state(eng, parent) {
  name = sname;
}

closure_compile_state::~closure_compile_state() {}

zs::error_result closure_compile_state::add_parameter(const object& param_name) {
  _parameter_names.push_back(param_name);
  return push_local_variable(param_name, 0);
}

zs::error_result closure_compile_state::add_parameter(
    const object& param_name, uint32_t mask, uint64_t custom_mask, bool is_const) {
  _parameter_names.push_back(param_name);
  return push_local_variable(param_name, 0, nullptr, mask, custom_mask, is_const);
}

zs::error_result closure_compile_state::push_local_variable(
    const object& name, int_t scope_id, int_t* ret_pos, uint32_t mask, uint64_t custom_mask, bool is_const) {

  //  if(zs::optional_result<int_t> pos = find_local_variable(name)) {
  //    return zs::errc::duplicated_local_variable_name;
  //  }

  int_t pos = _vlocals.size();
  _vlocals.emplace_back(name, scope_id, get_instruction_index(), 0, pos, mask, custom_mask, is_const);

  if (ret_pos) {
    *ret_pos = pos;
  }

  return update_total_stack_size();
}

int_t closure_compile_state::alloc_stack_pos() {
  _vlocals.push_back(scoped_local_var_info_t());

  if (auto err = update_total_stack_size()) {
    zs::throw_error(err);
  }

  return _vlocals.size() - 1;
}

// target_t closure_compile_state::push_target(target_t n) {
//   const zs::local_var_info_t& v = _vlocals[n];
//
//   ZS_ASSERT(v._pos == n, "Invalid local index.");
//
//   _target_stack.push_back({ n, v._mask, v._custom_mask, v._is_const });
//
//   zbase_assert(n <= k_maximum_target_index, "too many targets");
//   return (target_t)n;
// }

target_t closure_compile_state::push_var_target(target_t n) {
  ZS_ASSERT(n <= k_maximum_target_index, "too many targets.");
  ZS_ASSERT(n < _vlocals.size(), "invalid target stack index.");
  const zs::scoped_local_var_info_t& v = _vlocals[n];

  //  _target_stack.push_back({ n, v._mask, v._custom_mask, v._is_const });
  ZS_ASSERT(n == _vlocals[n].pos);
  target_t idx = _target_stack.emplace_back(_vlocals[n]).index;
  ZS_ASSERT(n == idx);

  return idx;
}

// int_t closure_compile_state::push_export_target() {
//   ZS_ASSERT(has_export(), "no export table");
//   //  const zs::local_var_info_t& v = _vlocals[_export_table_target];
//   //  _target_stack.push_back({ _export_table_target, v.mask, v.custom_mask, v.is_const });
//
//   _target_stack.emplace_back(_vlocals[_export_table_target]);
//   return _export_table_target;
// }

target_t closure_compile_state::new_target() {
  size_t n = alloc_stack_pos();
  _target_stack.emplace_back((int_t)n);

  zbase_assert(n <= k_maximum_target_index, "too many targets");
  return (target_t)n;
}

target_t closure_compile_state::new_target(uint32_t mask, uint64_t custom_mask, bool is_const) {
  size_t n = alloc_stack_pos();
  _target_stack.emplace_back(n, mask, custom_mask, is_const);

  zbase_assert(n <= k_maximum_target_index, "too many targets");
  return (target_t)n;
}

target_t closure_compile_state::pop_target() {
  zbase_assert(!_target_stack.empty(), "trying to pop an empty target stack");

  int_t npos = _target_stack.back().index;
  zbase_assert(npos < (int_t)_vlocals.size());

  scoped_local_var_info_t& t = _vlocals[npos];

  if (!t.is_named()) {
    _vlocals.pop_back();
  }

  _target_stack.pop_back();
  return (target_t)npos;
}

int_t closure_compile_state::get_literal(const object& name) {
  // Get the constant from the literals table.
  if (auto it = _literals.find(name); it != _literals.end()) {
    return it->second;
  }

  return _literals.emplace(name, (int_t)_literals.size()).first->second;
}

// zs::optional_result<int_t> closure_compile_state::find_local_variable(const object& name) const {
//   int_t locals = (int_t)_vlocals.size();
//
//   while (locals) {
//     const local_var_info_t& lvi = _vlocals[locals - 1];
//
//     if(lvi.is_named(name)) {
//       return locals - 1;
//     }
//
//     locals--;
//   }
//
//   return zs::errc::not_found;
// }

zs::optional_result<int_t> closure_compile_state::find_local_variable(const object& name) const {

  if (is_top_level() and name.get_string_unchecked() == "this") {
    return find_local_variable(zs::_ss("__this__"));
  }

  int_t locals = (int_t)_vlocals.size();

  while (locals--) {
    if (const scoped_local_var_info_t& lvi = _vlocals[locals]; lvi.is_named(name)) {
      return locals;
    }
  }

  return zs::errc::not_found;
}

const zs::scoped_local_var_info_t* closure_compile_state::find_local_variable_ptr(
    const object& name) const noexcept {

  if (is_top_level() and name.get_string_unchecked() == "this") {
    return find_local_variable_ptr(zs::_ss("__this__"));
  }

  int_t locals = (int_t)_vlocals.size();

  while (locals--) {
    if (const scoped_local_var_info_t& lvi = _vlocals[locals]; lvi.is_named(name)) {
      return &lvi;
    }
  }

  return nullptr;
}

zs::error_result closure_compile_state::find_local_variable(const object& name, int_t& pos) const {

  if (is_top_level() and name.get_string_unchecked() == "this") {
    return find_local_variable(zs::_ss("__this__"), pos);
  }

  int_t locals = (int_t)_vlocals.size();

  while (locals--) {
    if (const scoped_local_var_info_t& lvi = _vlocals[locals]; lvi.is_named(name)) {
      pos = locals;
      return {};
    }
  }

  return zs::errc::not_found;
}

void closure_compile_state::mark_local_as_capture(int_t pos) {
  scoped_local_var_info_t& lvi = _vlocals[pos];
  lvi.end_op = k_captured_end_op;
  _n_capture++;
}

zs::optional_result<int_t> closure_compile_state::get_capture(const object& name) {
  size_t count = _captures.size();
  for (size_t i = 0; i < count; i++) {
    if (_captures[i].name.get_string_unchecked() == name.get_string_unchecked()) {
      return (int_t)i;
    }
  }

  if (!_parent) {
    return zs::errc::not_found;
  }

  //  const bool is_export = name == "__exports__";

  if (zs::optional_result<int_t> pos = _parent->find_local_variable(name)) {
    _parent->mark_local_as_capture(pos);
    _captures.push_back(captured_variable(name, pos, captured_variable::local, false));
    return _captures.size() - 1;
  }

  if (zs::optional_result<int_t> pos = _parent->get_capture(name)) {
    _captures.push_back(captured_variable(name, pos, captured_variable::outer, false));
    return _captures.size() - 1;
  }

  return zs::errc::not_found;
}

bool closure_compile_state::is_local(size_t pos) const {
  return (pos < _vlocals.size()) and _vlocals[pos].is_named();

  //  if (pos >= _vlocals.size()) {
  //    return false;
  //  }
  //
  //  return !_vlocals[pos].name.is_null();
}

// void closure_compile_state::set_stack_size(int_t n) {
//   int_t size = (int_t)_vlocals.size();
//
//   while (size-- > n) {
////    size--;
//
//    local_var_info_t lvi = _vlocals.back();
//    if (lvi.is_named()) {
//
//      // This means is a capture.
//      if (lvi.end_op == k_captured_end_op) {
//        _n_capture--;
//      }
//
//      lvi.end_op = get_next_instruction_index() - 1;
//      _local_var_infos.push_back(lvi);
//    }
//
//    _vlocals.pop_back();
//  }
//}

void closure_compile_state::set_stack_size(int_t n) {
  int_t size = (int_t)_vlocals.size();

  while (size-- > n) {
    if (scoped_local_var_info_t lvi = _vlocals.get_pop_back(); lvi.is_named()) {
      // This means is a capture.
      _n_capture -= lvi.end_op == k_captured_end_op;

      lvi.end_op = get_next_instruction_index() - 1;
      _local_var_infos.push_back(std::move(lvi));
    }
  }
}

zs::error_result closure_compile_state::get_restricted_type_index(const object& name, int_t& index) {
  const size_t sz = _sdata._restricted_types.size();

  std::string_view name_str = name.get_string_unchecked();

  for (size_t i = 0; i < sz; i++) {
    if (_sdata._restricted_types[i].get_string_unchecked() == name_str) {
      index = (int_t)i;
      return {};
    }
  }

  if (sz == k_max_restricted_type_check) {
    index = -1;
    return zs::error_code::inaccessible;
  }

  _sdata._restricted_types.push_back(name);
  index = (int_t)sz;
  return {};
}

zs::error_result closure_compile_state::get_restricted_type_mask(
    const object& name, int_t& mask) const noexcept {
  const size_t sz = _sdata._restricted_types.size();

  std::string_view name_str = name.get_string_unchecked();

  for (size_t i = 0; i < sz; i++) {
    const object& obj = _sdata._restricted_types[i];
    if (obj.is_string() and obj.get_string_unchecked() == name_str) {
      mask = (int_t)(1 << i);
      return {};
    }
  }

  return zs::error_code::not_found;
}

// zs::error_result closure_compile_state::create_export_table() {
//   if (has_export()) {
//     return {};
//   }
//
//   add_instruction<opcode::op_new_obj>(new_target(), object_type::k_table);
//   pop_target();
//   return push_local_variable(zs::_ss("__exports__"), 0, &_export_table_target);
// }

void closure_compile_state::add_line_infos(const zs::line_info& linfo) {

  //  if(_last_line != linfo.line) {
  line_info_op_t li;
  li.line = linfo.line;
  li.column = linfo.column;

  if (!_instructions._data.empty()) {
    li.op = zs::instruction_iterator(&_instructions._data[_last_instruction_index]).get_opcode();
  }
  else {
    li.op = opcode::op_line;
  }

  li.op_index = _last_instruction_index;

  _line_info.push_back(li);
  add_instruction<opcode::op_line>(li.line);
  _last_line = li.line;
  //  }

  //  if (_lastline != line || force) {
  //    SQLineInfo li;
  //    li._line = line;
  //    li._op = (GetCurrentPos() + 1);
  //    if (lineop)
  //      AddInstruction(_OP_LINE, 0, line);
  //
  //    if (_lastline != line) {
  //      _lineinfos.push_back(li);
  //    }
  //
  //    _lastline = line;
  //  }
}

#if ZS_DEBUG

void closure_compile_state::add_debug_line_info(const zs::line_info& linfo) {

  line_info_op_t li;
  li.line = linfo.line;
  li.column = linfo.column;

  if (!_instructions._data.empty()) {
    li.op = zs::instruction_iterator(&_instructions._data[_last_instruction_index]).get_opcode();
  }
  else {
    li.op = opcode::op_line;
  }

  li.op_index = _last_instruction_index;

  _debug_line_info.push_back(li);
  //    _last_line = li.line;
  //  }

  //  if (_lastline != line || force) {
  //    SQLineInfo li;
  //    li._line = line;
  //    li._op = (GetCurrentPos() + 1);
  //    if (lineop)
  //      AddInstruction(_OP_LINE, 0, line);
  //
  //    if (_lastline != line) {
  //      _lineinfos.push_back(li);
  //    }
  //
  //    _lastline = line;
  //  }
}
#endif

object closure_compile_state::build_function_prototype() {

  object obj = zs::function_prototype_object::create(get_engine());
  zs::function_prototype_object* fpo = &function_prototype_object::as_proto(obj);
  fpo->_source_name = _sdata._source_name;
  fpo->_name = name;
  fpo->_stack_size = _total_stack_size;
  fpo->_vlocals = std::move(_local_var_infos);

  // Copy the literal map to a vector.
  fpo->_literals.resize(_literals.size());

  for (auto it : _literals) {
    // TODO: Bounds checking?
    fpo->_literals[it.second] = it.first;
  }

  fpo->_parameter_names = std::move(_parameter_names);
  fpo->_restricted_types = _sdata._restricted_types;
  fpo->_instructions = std::move(_instructions);
  fpo->_functions = std::move(_functions);
  fpo->_captures = std::move(_captures);
  fpo->_line_info = std::move(_line_info);
  fpo->_default_params = _default_params;
  fpo->_n_capture = _n_capture;
  //  fpo->_export_table_target = _export_table_target;
  fpo->_module_info = _sdata._module_info;
  fpo->_has_vargs_params = _has_vargs_params;

#if ZS_DEBUG
  fpo->_debug_line_info = std::move(_debug_line_info);
#endif

  return obj;
}

namespace jit {

  zs::function_prototype_object& closure_compile_state_ref::get_function_proto(size_t index) noexcept {
    return function_prototype_object::as_proto(_ccs->_functions[index]);
  }

  const zs::function_prototype_object& closure_compile_state_ref::get_function_proto(
      size_t index) const noexcept {
    return function_prototype_object::as_proto(_ccs->_functions[index]);
  }

  zs::function_prototype_object& closure_compile_state_ref::get_last_function_proto() noexcept {
    return function_prototype_object::as_proto(_ccs->_functions.back());
  }

  const zs::function_prototype_object& closure_compile_state_ref::get_last_function_proto() const noexcept {
    return function_prototype_object::as_proto(_ccs->_functions.back());
  }

} // namespace jit.

} // namespace zs.
