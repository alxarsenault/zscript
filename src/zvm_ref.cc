
namespace zs {
vm_ref::vm_ref(zs::virtual_machine* v)
    : _vm(v) {}

int_t vm_ref::stack_size() const noexcept { return _vm->stack_size(); }

int_t vm_ref::push_root() {
  _vm->push_root();
  return 1;
}

int_t vm_ref::push_null() {
  _vm->push_null();
  return 1;
}

int_t vm_ref::push_bool(bool_t value) {
  _vm->push(value);
  return 1;
}
int_t vm_ref::push_integer(int_t value) {
  _vm->push(value);
  return 1;
}
int_t vm_ref::push_float(float_t value) {
  _vm->push(value);
  return 1;
}

int_t vm_ref::push_string(std::string_view s) {
  _vm->push(zs::_s(get_engine(), s));
  return 1;
}

int_t vm_ref::push_string_concat(std::string_view s1, std::string_view s2) {
  _vm->push(zs::object::create_concat_string(_vm->get_engine(), s1, s2));
  return 1;
}

int_t vm_ref::push(const object& obj) {
  _vm->push(obj);
  return 1;
}

int_t vm_ref::push(object&& obj) {
  _vm->push(std::move(obj));
  return 1;
}

zs::error_result vm_ref::new_closure(zs::native_closure* closure) {
  object obj = object::create_native_closure(_vm->get_engine(), closure);

  _vm->push(obj);
  return {};
}

zs::error_result vm_ref::get_integer(int_t idx, int_t& res) {
  zs::object& obj = _vm->stack_get(idx);
  return obj.get_integer(res);
}

zs::error_result vm_ref::get_float(int_t idx, float_t& res) {
  zs::object& obj = _vm->stack_get(idx);
  return obj.get_float(res);
}

zs::error_result vm_ref::get_string(int_t idx, std::string_view& res) {
  zs::object& obj = _vm->stack_get(idx);
  return obj.get_string(res);
}

zs::optional_result<float_t> vm_ref::get_float(int_t idx) {
  zs::object& obj = _vm->stack_get(idx);
  float_t res = 0;
  if (auto err = obj.get_float(res)) {
    return err.code;
  }

  return res;
}

zs::object_type vm_ref::get_type(int_t idx) const noexcept { return _vm->stack_get(idx).get_type(); }

zs::error_result vm_ref::call(zs::int_t n_params, bool returns, bool pop_callable) {
  return zs_call(_vm, n_params, returns, pop_callable);
}

zs::engine* vm_ref::get_engine() const noexcept { return _vm->get_engine(); }
std::ostream& vm_ref::get_stream() const noexcept { return _vm->get_engine()->get_stream(); }

void vm_ref::set_error(std::string_view msg) { _vm->set_error(msg); }
const zs::string& vm_ref::get_error() const noexcept { return _vm->get_error(); }

object& vm_ref::top() noexcept { return _vm->top(); }
const object& vm_ref::top() const noexcept { return _vm->top(); }

object& vm_ref::root() noexcept { return _vm->get_root(); }
const object& vm_ref::root() const noexcept { return _vm->get_root(); }

object& vm_ref::operator[](int_t idx) noexcept { return _vm->stack_get(idx); }
const object& vm_ref::operator[](int_t idx) const noexcept { return _vm->stack_get(idx); }

object* vm_ref::stack_base_pointer() noexcept { return _vm->stack().stack_base_pointer(); }

const object* vm_ref::stack_base_pointer() const noexcept { return _vm->stack().stack_base_pointer(); }

struct vm::helper {};

vm::vm(size_t stack_size, allocate_t alloc_cb, raw_pointer_t user_pointer,
    raw_pointer_release_hook_t user_release, stream_getter_t stream_getter,
    engine_initializer_t initializer) noexcept
    : vm_ref(create_virtual_machine(
          stack_size, alloc_cb, user_pointer, user_release, stream_getter, initializer)) {}

vm::vm(const config_t& config) noexcept
    : vm_ref(create_virtual_machine(config.stack_size, config.alloc_callback, config.user_pointer,
          config.user_release, config.stream_getter, config.initializer)) {}

vm::vm(zs::engine* eng, size_t stack_size) noexcept
    : vm_ref(create_virtual_machine(eng, stack_size)) {}

vm::vm(zs::virtual_machine* v) noexcept
    : vm_ref(v) {}

vm::vm(vm&& v) noexcept
    : vm_ref(v._vm) {
  v._vm = nullptr;
}

vm::~vm() noexcept {
  if (_vm) {
    close_virtual_machine(_vm);
    _vm = nullptr;
  }
}

vm& vm::operator=(vm&& v) noexcept {

  if (&v == this) {
    return *this;
  }

  if (_vm) {
    close_virtual_machine(_vm);
    _vm = nullptr;
  }

  _vm = v._vm;
  v._vm = nullptr;
  return *this;
}

} // namespace zs.
