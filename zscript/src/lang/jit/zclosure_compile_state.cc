// #include "lang/zclosure_compile_state.h"
//
// #include "object/zfunction_prototype.h"
// #include "lang/zopcode.h"

namespace zs {
closure_compile_state::closure_compile_state(zs::engine* eng, closure_compile_state* parent)
    : engine_holder(eng)
    , _parent(parent)
    , _parameter_names(zs::allocator<zs::object>(eng))
    , _children(zs::allocator<zs::unique_ptr<closure_compile_state>>(eng))
    , _instructions(eng)
    , _restricted_types(zs::allocator<object>(eng))
    , _target_stack(zs::allocator<variable_type_info_t>(eng))
    , _literals(zs::unordered_map_allocator<object, int_t>(eng))
    , _vlocals(zs::allocator<zs::local_var_info_t>(eng))
    , _local_var_infos(zs::allocator<zs::local_var_info_t>(eng))
    , _line_info(zs::allocator<zs::line_info_op_t>(eng))
    , _functions(zs::allocator<object>(eng))
    , _default_params(zs::allocator<int_t>(eng))
    , _captures(zs::allocator<captured_variable>(eng))
    , _imported_files_set(zs::allocator<object>(eng))

#if ZS_DEBUG
    , _debug_line_info(zs::allocator<zs::line_info_op_t>(eng))
#endif

{
}

closure_compile_state::~closure_compile_state() {}

zs::error_result closure_compile_state::add_parameter(const object& param_name) {
  _parameter_names.push_back(param_name);
  return push_local_variable(param_name);
}

zs::error_result closure_compile_state::add_parameter(
    const object& param_name, uint32_t mask, uint64_t custom_mask, bool is_const) {
  _parameter_names.push_back(param_name);
  return push_local_variable(param_name, nullptr, mask, custom_mask, is_const);
}

zs::error_result closure_compile_state::push_local_variable(
    const object& name, int_t* ret_pos, uint32_t mask, uint64_t custom_mask, bool is_const) {

  size_t sz = _vlocals.size();

  int_t pos = (int_t)sz;
  local_var_info_t lvi;
  lvi._name = name;
  lvi._start_op = get_instruction_index();
  lvi._pos = pos;
  lvi._mask = mask;
  lvi._custom_mask = custom_mask;
  lvi._is_const = is_const;
  _vlocals.push_back(std::move(lvi));

  ++sz;

  if (sz > ((uint_t)_total_stack_size)) {
    if (sz > k_max_func_stack_size) {
      return zs::error_code::too_many_locals;
    }

    _total_stack_size = (uint32_t)sz;
  }

  if (ret_pos) {
    *ret_pos = pos;
  }

  return {};
}

int_t closure_compile_state::alloc_stack_pos() {

  size_t sz = _vlocals.size();
  int_t npos = sz;
  _vlocals.push_back(local_var_info_t());
  ++sz;

  if (sz > ((uint_t)_total_stack_size)) {
    if (sz > k_max_func_stack_size) {
      zs::throw_exception(zs::error_type::compiler, "internal compiler error: too many locals");
    }

    _total_stack_size = (uint32_t)sz;
  }

  return npos;
}

int_t closure_compile_state::push_target(int_t n) {
  if (n == -1) {
    n = alloc_stack_pos();
  }

  const zs::local_var_info_t& v = _vlocals[n];
  _target_stack.push_back({ n, v._mask, v._custom_mask, v._is_const });

  zbase_assert(n <= (std::numeric_limits<uint8_t>::max)(), "too many targets");
  return n;
}

uint8_t closure_compile_state::new_target() {
  size_t n = alloc_stack_pos();
  _target_stack.emplace_back((int_t)n);

  zbase_assert(n <= (std::numeric_limits<uint8_t>::max)(), "too many targets");
  return (uint8_t)n;
}

uint8_t closure_compile_state::new_target(uint32_t mask, uint64_t custom_mask, bool is_const) {
  size_t n = alloc_stack_pos();
  _target_stack.emplace_back(n, mask, custom_mask, is_const);

  zbase_assert(n <= (std::numeric_limits<uint8_t>::max)(), "too many targets");
  return (uint8_t)n;
}

int_t closure_compile_state::pop_target() {
  int_t npos = _target_stack.back().index;
  zbase_assert(npos < (int_t)_vlocals.size());

  local_var_info_t& t = _vlocals[npos];

  if (t._name.is_null()) {
    _vlocals.pop_back();
  }

  zbase_assert(!_target_stack.empty(), "trying to pop an empty target stack");
  _target_stack.pop_back();
  return npos;
}

int_t closure_compile_state::get_literal(const object& name) {
  // Get the constant from the literals table.
  if (auto it = _literals.find(name); it != _literals.end()) {
    return it->second;
  }

  return _literals.emplace(name, (int_t)_literals.size()).first->second;
}

int_t closure_compile_state::find_local_variable(const object& name) const {
  int_t locals = (int_t)_vlocals.size();

  while (locals) {
    const local_var_info_t& lvi = _vlocals[locals - 1];

    if (lvi._name.is_string() && lvi._name.get_string_unchecked() == name.get_string_unchecked()) {
      return locals - 1;
    }

    locals--;
  }

  return -1;
}

void closure_compile_state::mark_local_as_capture(int_t pos) {
  local_var_info_t& lvi = _vlocals[pos];
  lvi._end_op = std::numeric_limits<uint_t>::max();
  _n_capture++;
}

int_t closure_compile_state::get_capture(const object& name) {
  size_t count = _captures.size();
  for (size_t i = 0; i < count; i++) {
    if (_captures[i].name.get_string_unchecked() == name.get_string_unchecked()) {
      return (int_t)i;
    }
  }

  if (!_parent) {
    return -1;
  }

  if (int_t pos = _parent->find_local_variable(name); pos != -1) {
    _parent->mark_local_as_capture(pos);
    _captures.push_back(captured_variable(name, pos, captured_variable::local));
    return _captures.size() - 1;
  }
  else if (pos = _parent->get_capture(name); pos != -1) {
    _captures.push_back(captured_variable(name, pos, captured_variable::outer));
    return _captures.size() - 1;
  }

  return -1;
}

bool closure_compile_state::is_local(size_t pos) const {
  if (pos >= _vlocals.size()) {
    return false;
  }

  return !_vlocals[pos]._name.is_null();
}

void closure_compile_state::set_stack_size(int_t n) {
  int_t size = (int_t)_vlocals.size();

  while (size > n) {
    size--;

    local_var_info_t lvi = _vlocals.back();
    if (!lvi._name.is_null()) {

      if (lvi._end_op == std::numeric_limits<uint_t>::max()) { // this means is an outer
        _n_capture--;
      }

      lvi._end_op = get_next_instruction_index() - 1;
      _local_var_infos.push_back(lvi);
    }

    _vlocals.pop_back();
  }
}

zs::error_result closure_compile_state::get_restricted_type_index(const object& name, int_t& index) {
  const size_t sz = _restricted_types.size();

  std::string_view name_str = name.get_string_unchecked();

  for (size_t i = 0; i < sz; i++) {
    if (_restricted_types[i].get_string_unchecked() == name_str) {
      index = (int_t)i;
      return {};
    }
  }

  if (sz == k_max_restricted_type_check) {
    index = -1;
    return zs::error_code::inaccessible;
  }

  _restricted_types.push_back(name);
  index = (int_t)sz;
  return {};
}

zs::error_result closure_compile_state::get_restricted_type_mask(
    const object& name, int_t& mask) const noexcept {
  const size_t sz = _restricted_types.size();

  std::string_view name_str = name.get_string_unchecked();

  for (size_t i = 0; i < sz; i++) {
    const object& obj = _restricted_types[i];
    if (obj.is_string() and obj.get_string_unchecked() == name_str) {
      mask = (int_t)(1 << i);
      return {};
    }
  }

  return zs::error_code::not_found;
}

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

zs::function_prototype_object* closure_compile_state::build_function_prototype() {

  zs::function_prototype_object* fpo = zs::function_prototype_object::create(_engine);

  fpo->_source_name = std::move(source_name);
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
  fpo->_restricted_types = std::move(_restricted_types);
  fpo->_instructions = std::move(_instructions);
  fpo->_functions = std::move(_functions);
  fpo->_captures = std::move(_captures);
  fpo->_line_info = std::move(_line_info);
  fpo->_default_params = _default_params;
  fpo->_n_capture = _n_capture;

#if ZS_DEBUG
  fpo->_debug_line_info = std::move(_debug_line_info);
#endif
  return fpo;
}

zs::closure_compile_state* closure_compile_state::push_child_state() {
  closure_compile_state* cs = internal::zs_new<closure_compile_state>(_engine, _engine, this);
  _children.push_back(zs::unique_ptr<closure_compile_state>(cs, { _engine }));
  return cs;
}

void closure_compile_state::pop_child_state() {
  zbase_assert(!_children.empty(),
      "Trying to call zs::closure_compile_state::pop_child_state() when "
      "empty.");
  _children.pop_back();
}
} // namespace zs.