#include "zvirtual_machine.h"
#include <zscript/base/container/enum_array.h>
#include <zscript/base/utility/print.h>
#include <zscript/base/utility/scoped.h>
#include <zscript/base/memory/ref_wrapper.h>
#include <zscript/base/strings/charconv.h>
#include <zscript/base/strings/stack_string.h>
#include <zscript/base/strings/unicode.h>

#include "object/zfunction_prototype.h"

#include "jit/zjit_compiler.h"
#include "utility/json/zjson_lexer.h"
#include "utility/json/zjson_parser.h"

#include "std/zglobal.h"
#include <zscript/std/zslib.h>
#include <zscript/std/zio.h>
#include <zscript/std/zfs.h>
#include <zscript/std/zmath.h>
#include <zscript/std/zbase64.h>

#include "utility/zvm_module.h"

#define ZS_VIRTUAL_MACHINE_CPP 1

namespace zs {

#define REF(...) zb::wref(__VA_ARGS__)
#define CREF(...) zb::wcref(__VA_ARGS__)

enum class runtime_code {
  delegate_set,
  table_set,
  table_set_if_exists,

  user_data_set,

  handle_error
};

using enum runtime_code;
} // namespace zs.

#include "vm/zvm_common.h"
#include "vm/runtime/zvm_runtime.h"
#include "vm/zvm_op.h"

namespace zs {

zs::instruction_vector& virtual_machine::exec_op_data_t::instructions() const noexcept {
  return fct->_instructions;
}

int_t virtual_machine::exec_op_data_t::get_iterator_index(
    const zs::instruction_vector::iterator& it) const noexcept {
  return (int_t)fct->_instructions.get_iterator_index(it);
}

zs::instruction_vector::iterator virtual_machine::exec_op_data_t::get_instruction(
    size_t index) const noexcept {
  ZS_ASSERT(index < fct->_instructions._data.size());
  return fct->_instructions[index];
}

// static inline void print_stack(zs::vm_ref vm) {
//   for (zs::int_t i = 0; i < vm->stack_size(); i++) {
//     zb::print("value------------", i, vm->stack_get(i).to_debug_string());
//   }
// }

template <opcode Op>
zs::error_code virtual_machine::exec_op_wrapper(zs::instruction_iterator& it, exec_op_data_t& op_data) {
  zs::error_code err = exec_op<Op>(it, op_data);
  //  ++it;
  it.data_ptr_ref() += zs::get_instruction_size<Op>();
  return err;
}

#define ZS_VM_DECL_OP_NO_INST_PTR_INCR(OPCODE)                     \
  template <>                                                      \
  zs::error_code virtual_machine::exec_op_wrapper<opcode::OPCODE>( \
      zs::instruction_iterator & it, exec_op_data_t & op_data) {   \
    return exec_op<opcode::OPCODE>(it, op_data);                   \
  }

// These instructions takes care of incrementing the instruction pointer.
ZS_VM_DECL_OP_NO_INST_PTR_INCR(op_jz)
ZS_VM_DECL_OP_NO_INST_PTR_INCR(op_jmp)
ZS_VM_DECL_OP_NO_INST_PTR_INCR(op_if_not)
ZS_VM_DECL_OP_NO_INST_PTR_INCR(op_and)
ZS_VM_DECL_OP_NO_INST_PTR_INCR(op_or)
ZS_VM_DECL_OP_NO_INST_PTR_INCR(op_triple_or)

#undef ZS_VM_DECL_OP_NO_INST_PTR_INCR

struct virtual_machine::executor {
  using fct_type = zs::error_code (virtual_machine::*)(zs::instruction_iterator& it, exec_op_data_t& data);
  static constexpr zb::enum_array<fct_type, opcode> operations = {
#define ZS_DECL_OPCODE(name, INST_TYPES) &virtual_machine::exec_op_wrapper<ZS_OPCODE_ENUM_VALUE(name)>,
#include "bytecode/zopcode_def.h"
#undef ZS_DECL_OPCODE
  };

  ZB_CHECK ZB_INLINE static zs::error_code call_op(
      virtual_machine* v, opcode code, zs::instruction_iterator& it, exec_op_data_t& op_data) {

    //        zb::print("CALL_OP", *it);
    //    print_stack(vm_ref(v));

    return (v->*executor::operations[code])(it, op_data);
  }
};

//
// MARK: - virtual_machine
//

virtual_machine* virtual_machine::create(size_t stack_size, allocate_t alloc_cb, raw_pointer_t user_pointer,
    raw_pointer_release_hook_t user_release, stream_getter_t stream_getter,
    engine_initializer_t initializer) {
  zbase_warning(alloc_cb, "allocator callback can't be null");

  if (!alloc_cb) {
    alloc_cb = default_allocate;
  }

  // Create engine.
  // Can't use `zs_new` or anything special here since we are
  // allocating/creating the engine.
  zs::engine* eng = (zs::engine*)(*alloc_cb)(
      // Engine pointer.
      nullptr,
      // User pointer.
      user_pointer,
      // Last pointer.
      nullptr,
      // Alloc size.
      sizeof(zs::engine),
      // Old size.
      0,
      // Alloc info.
      (alloc_info_t)zs::memory_tag::nt_engine);

  eng = zb_placement_new(eng) zs::engine(alloc_cb, user_pointer, user_release, stream_getter, initializer);

  // Create the virtual machine.
  virtual_machine* v
      = internal::zs_new<memory_tag::nt_vm, virtual_machine>(eng, eng, stack_size, true // owns_engine.
      );

  // Init the virtual machine.
  if (auto err = v->init()) {
    ZS_ERROR("Could not initialize the virtual machine.");
    v->release();
    return nullptr;
  }

  return v;
}

virtual_machine* virtual_machine::create(zs::engine* eng, size_t stack_size) {
  // Create the virtual machine.
  virtual_machine* v = internal::zs_new<virtual_machine>(eng, eng, stack_size, false);

  // Init the virtual machine.
  if (auto err = v->init()) {
    ZS_ERROR("Could not initialize the virtual machine.");
    v->release();
    return nullptr;
  }

  return v;
}

virtual_machine* virtual_machine::create(zs::virtual_machine* vm, size_t stack_size, bool same_global) {
  // Create the virtual machine.
  virtual_machine* v = zs_new<virtual_machine>(vm->_engine, vm, stack_size, same_global);

  // Init the virtual machine.
  if (auto err = v->init()) {
    ZS_ERROR("Could not initialize the virtual machine.");
    v->release();
    return nullptr;
  }

  return v;
}

template <>
struct internal::proxy<virtual_machine> {
  inline static void reset_engine_user_pointer(zs::engine* eng) {
    eng->_user_pointer = nullptr;
    eng->_user_pointer_release = nullptr;
  }
};

virtual_machine::virtual_machine(zs::engine* eng, size_t stack_size, bool owns_engine)
    : engine_holder(eng)
    , _global_table(create_global_table(eng))
    , _stack(eng, stack_size)
    , _call_stack({ { nullptr, 0, 0 } }, (zs::allocator<call_info>(eng, zs::memory_tag::nt_vm)))
    , _open_captures(zs::allocator<object>(eng))
    , _errors(eng)
    , _owns_engine(owns_engine) {}

virtual_machine::virtual_machine(zs::virtual_machine* vm, size_t stack_size, bool same_global)
    : engine_holder(vm->_engine)
    , _global_table(same_global ? vm->_global_table : create_global_table(vm->_engine))
    , _stack(vm->_engine, stack_size)
    , _call_stack({ { nullptr, 0, 0 } }, (zs::allocator<call_info>(vm->_engine, zs::memory_tag::nt_vm)))
    , _open_captures(zs::allocator<object>(vm->_engine))
    , _errors(vm->_engine)
    , _owns_engine(false) {}

zs::error_result virtual_machine::init() {
  using namespace literals;

  table_object& g = _global_table.as_table();
  table_object& imported_modules = g[k_imported_modules_name].as_table();
  table_object& module_loaders = g[k_module_loaders_name].as_table();

  imported_modules.reserve(8);

  auto add_to_global_and_imported_modules = [&](const object& name, object&& lib) {
    imported_modules.emplace(name, g.emplace(name, std::move(lib)).first->second);
  };

  // zs lib.
  add_to_global_and_imported_modules("zs"_ss, zs::create_zs_lib(this));

  // io lib.
  add_to_global_and_imported_modules("io"_ss, zs::create_io_lib(this));

  // fs lib.
  add_to_global_and_imported_modules("fs"_ss, zs::create_fs_lib(this));

  // math lib.
  add_to_global_and_imported_modules("math"_ss, zs::create_math_lib(this));

  //
  // Extra libs only added to the module loaders table.
  //

  // base64 lib.
  module_loaders.emplace("base64"_ss, [](zs::vm_ref vm) -> int_t { return vm.push(create_base64_lib(vm)); });

  return {};
}

static void destroy_engine(zs::engine* eng) {
  allocate_t alloc_cb = eng->get_allocate_callback();
  raw_pointer_t user_ptr = eng->get_user_pointer();
  raw_pointer_release_hook_t user_release = eng->get_user_pointer_release_hook();

  // Prevent the engine from calling the user release callback in it's
  // destructor. It will be called after deleting the engine.
  internal::proxy<virtual_machine>::reset_engine_user_pointer(eng);

  // Delete the engine.
  // Can't use zs_delete or anything special here since we are
  // deleting/deallocating the engine.
  eng->~engine();
  (*alloc_cb)(eng, user_ptr, eng, 0, sizeof(zs::engine), constants::k_engine_deallocation);

  // User release is called after deleting the engine.
  if (user_release) {
    (*user_release)(alloc_cb, user_ptr);
  }
}

void virtual_machine::release() noexcept {

  zs::engine* eng = _engine;
  const bool owns_engine = _owns_engine;

  _global_table.reset();
  _stack.set_stack_base(0);
  _stack.pop_to(0);

  // Destroy itself (virtual_machine).
  internal::zs_delete(eng, this);

  // Can't access any virtual_machine members below here.
  // So let's implement the rest of this method in the static `destroy_engine`
  // function to prevent any confusion.

  if (owns_engine) {
    // Destroy the engine.
    destroy_engine(eng);
  }
}

const object& virtual_machine::get_default_table_delegate() const noexcept {
  return global_table()[k_table_delegate_name];
}

const object& virtual_machine::get_default_number_delegate() const {
  return global_table()[k_number_delegate_name];
}

const object& virtual_machine::get_default_array_delegate() const {
  return global_table()[k_array_delegate_name];
}

const object& virtual_machine::get_default_string_delegate() const {
  return global_table()[k_string_delegate_name];
}

const object& virtual_machine::get_delegated_atom_delegates_table() const {
  return global_table()[k_delegated_atom_delegates_table_name];
}

const object& virtual_machine::get_default_struct_delegate() const {
  return global_table()[k_struct_delegate_name];
}

const object& virtual_machine::get_default_function_delegate() const {
  return global_table()[k_function_delegate_name];
}

zs::string virtual_machine::get_error() const noexcept {
  zs::ostringstream stream(zs::create_string_stream(_engine));
  _errors.print(stream);
  return stream.str();
}

zs::error_result virtual_machine::handle_error(
    zs::error_code ec, const zs::line_info& linfo, std::string_view msg, const zb::source_location& loc) {
  std::string_view filename = "";
  std::string_view line_content = "";
  _errors.emplace_back(_engine, error_source::virtual_machine, ec, msg, filename, line_content, linfo, loc);
  return ec;
}

table_object& virtual_machine::get_imported_modules() noexcept {
  return global_table()[k_imported_modules_name].as_table();
}

table_object& virtual_machine::get_module_loaders() noexcept {
  return global_table()[k_module_loaders_name].as_table();
}

void virtual_machine::push(object&& obj) { _stack.push(std::move(obj)); }

void virtual_machine::push(const object& obj) { _stack.push(obj); }

void virtual_machine::push(std::span<const object> objs) { _stack.push(objs); }

void virtual_machine::push_null() { _stack.push(nullptr); }
void virtual_machine::push_nulls(size_t n) { _stack.push_n(n); }

void virtual_machine::push_global() { _stack.push(_global_table); }

void virtual_machine::remove(int_t n) { _stack.remove(n); }

void virtual_machine::swap(int_t n1, int_t n2) { _stack.swap(n1, n2); }

void virtual_machine::pop() noexcept { _stack.pop(); }

object virtual_machine::pop_get() noexcept { return _stack.pop_get(); }

void virtual_machine::pop(int_t n) noexcept { _stack.pop(n); }

const object& virtual_machine::top() const noexcept { return _stack.top(); }

object virtual_machine::start_delegate_chain(const object& obj) {
  return obj.is_meta_type() ? obj : get_delegate(obj);
}

object virtual_machine::get(const object& obj, const object& key) {
  object dest;

  if (auto err = get(obj, key, dest)) {
    return zs::none();
  }

  return dest;
}

zs::error_result virtual_machine::set(object& obj, const object& key, const object& value) {

#define ZS_VM_RT_SET_FUNC_PTR(name) runtime_action<runtime_code::name>(REF(obj), CREF(key), CREF(value))

  switch (obj.get_type()) {
  case k_atom:
    return errc::unimplemented;

  case k_table:
    return ZS_VM_RT_SET_FUNC_PTR(table_set);
  case k_array: {
    array_object& arr = obj.as_array();

    if (!key.is_number()) {
      return zs::error_code::unimplemented;
    }

    const int_t sz = arr.size();

    int_t index = key.convert_to_integer_unchecked();

    if (index < 0) {
      index += sz;
    }

    return arr.set(index, value);
  }
  case k_struct: {
    struct_object& sobj = obj.as_struct();

    if (auto err = sobj.set_static(key, value)) {
      switch (err) {
      case zs::error_code::cant_modify_const_member:
        set_error("\nCan't modify a struct const member ", key, ".\n");
        return err;
      case zs::error_code::cant_modify_static_const:
        set_error("\nCan't modify a struct static const member ", key, ".\n");
        return err;
      case zs::error_code::invalid_type_assignment:
        set_error("\nInvalid value type assignment for struct field ", key, ".\n");
        return err;
      default:
        set_error("\nCan't add new value to a struct. Field ", key, " doesn't exists.\n");
        return err;
      }
    }

    return {};
  }
  case k_struct_instance:

  {
    zs::struct_instance_object& sobj = obj.as_struct_instance();

    if (auto err = sobj.set(key, value, _stack[0]._struct_instance == obj._struct_instance)) {
      switch (err) {
      case zs::error_code::inaccessible_private:
        set_error("Can't access or modify private struct member ", key, ".\n");
        return err;
      case zs::error_code::cant_modify_const_member:
        set_error("\nCan't modify a struct const member ", key, ".\n");
        return err;
      case zs::error_code::cant_modify_static_const:
        set_error("\nCan't modify a struct static const member ", key, ".\n");
        return err;
      case zs::error_code::invalid_type_assignment:
        set_error("\nInvalid value type assignment for struct field ", key, ".\n");
        return err;
      default:
        set_error("\nCan't add new value to a struct. Field ", key, " doesn't exists.\n");
        return err;
      }
    }

    return {};
  }
  case k_user_data:
    return ZS_VM_RT_SET_FUNC_PTR(user_data_set);
  case k_weak_ref: {
    object real_obj = obj.get_weak_ref_value();
    return this->set(real_obj, key, value);
  }
  default:
    set_error("Can't assign a value to '", zs::get_object_type_name(obj.get_type()), "'.\n");
    return zs::error_code::inaccessible;
  }
}

zs::error_result virtual_machine::set_if_exists(object& obj, const object& key, const object& value) {

#define ZS_VM_RT_SET_FUNC_PTR(name) runtime_action<runtime_code::name>(REF(obj), CREF(key), CREF(value))

  switch (obj.get_type()) {
  case k_atom:
    return errc::unimplemented;

  case k_table:
    return ZS_VM_RT_SET_FUNC_PTR(table_set_if_exists);
  case k_array: {
    array_object& arr = obj.as_array();

    if (!key.is_number()) {
      return zs::error_code::unimplemented;
    }

    const int_t sz = arr.size();

    int_t index = key.convert_to_integer_unchecked();

    if (index < 0) {
      index += sz;
    }

    return arr.set(index, value);
  }
  case k_struct: {
    struct_object& sobj = obj.as_struct();

    if (auto err = sobj.set_static(key, value)) {
      switch (err) {
      case zs::error_code::cant_modify_const_member:
        set_error("\nCan't modify a struct const member ", key, ".\n");
        return err;
      case zs::error_code::cant_modify_static_const:
        set_error("\nCan't modify a struct static const member ", key, ".\n");
        return err;
      case zs::error_code::invalid_type_assignment:
        set_error("\nInvalid value type assignment for struct field ", key, ".\n");
        return err;
      default:
        set_error("\nCan't add new value to a struct. Field ", key, " doesn't exists.\n");
        return err;
      }
    }

    return {};
  }
  case k_struct_instance: {
    zs::struct_instance_object& sobj = obj.as_struct_instance();

    if (auto err = sobj.set(key, value, _stack[0]._struct_instance == obj._struct_instance)) {
      switch (err) {
      case zs::error_code::inaccessible_private:
        set_error("Can't access or modify private struct member ", key, ".\n");
        return err;
      case zs::error_code::cant_modify_const_member:
        set_error("\nCan't modify a struct const member ", key, ".\n");
        return err;
      case zs::error_code::cant_modify_static_const:
        set_error("\nCan't modify a struct static const member ", key, ".\n");
        return err;
      case zs::error_code::invalid_type_assignment:
        set_error("\nInvalid value type assignment for struct field ", key, ".\n");
        return err;
      default:
        set_error("\nCan't add new value to a struct. Field ", key, " doesn't exists.\n");
        return err;
      }
    }

    return {};
  }
  case k_user_data:
    return ZS_VM_RT_SET_FUNC_PTR(user_data_set);
  case k_weak_ref: {
    object real_obj = obj.get_weak_ref_value();
    return this->set(real_obj, key, value);
  }
  default:
    set_error("Can't assign a value to '", zs::get_object_type_name(obj.get_type()), "'.\n");
    return zs::error_code::inaccessible;
  }
}

zs::error_result virtual_machine::close_captures(const object* stack_ptr) {
  ZS_TRACE("VM - rt_close_captures - CLOSE_CAPTURE", stack_ptr - _stack.get_internal_vector().data());

  for (auto it = _open_captures.begin(); it != _open_captures.end();) {
    if (capture::as_capture(*it).is_baked()) {
      it = _open_captures.erase(it);
      continue;
    }
    if (capture::as_capture(*it).get_value_ptr() >= stack_ptr) {
      capture::as_capture(*it).bake();
      it = _open_captures.erase(it);
    }
    else {
      ++it;
    }
  }
  return {};
}

zs::error_result virtual_machine::leave_function_call() {
  zbase_assert(!_call_stack.empty());
  ZS_TRACE("VM - leave_function_call");
  call_info cinfo = _call_stack.get_pop_back();
  _stack.set_stack_base(cinfo.previous_stack_base);

  if (cinfo.closure.is_closure()) {
    //    zs::closure_object& cobj = cinfo.closure.as_closure();
    //    zs::vector<zs::object>& closure_captured_values = cobj._captured_values;
    const object* stack_ptr = _stack.get_internal_vector().data() + cinfo.previous_top_index;

    if (auto err = close_captures(stack_ptr)) {
      return err;
    }
  }

  _stack.pop_to(cinfo.previous_top_index);

  // Just in case we somehow went below. Is this possible?
  if (_stack.get_absolute_top() < (size_t)cinfo.previous_top_index) {
    size_t diff = cinfo.previous_top_index - _stack.get_absolute_top();
    _stack.push_n(diff);
  }

  zbase_assert(cinfo.previous_top_index == (int_t)_stack.get_absolute_top());

  return {};
}

void virtual_machine::reset_protected_call_stack(size_t call_stack_index) {
  ZS_ASSERT(_call_stack.size() >= call_stack_index);
  call_info cinfo = _call_stack[call_stack_index];
  _stack.set_stack_base(cinfo.previous_stack_base);
  _stack.pop_to(cinfo.previous_top_index);
  _call_stack.resize(call_stack_index);
  ZS_ASSERT(cinfo.previous_top_index == (int_t)_stack.get_absolute_top());
}

zs::error_result virtual_machine::call_closure(
    const object& closure_obj, zs::parameter_list params, object& ret_value, bool is_protected_call) {

  zs::native_closure_object* nclosure = closure_obj._native_closure;
  zs::closure_object* closure = closure_obj._closure;
  zs::function_t nfct = closure_obj._nfct;

  const bool is_native_closure = closure_obj.is_native_closure();
  const size_t call_stack_index = _call_stack.size();

  // The call was made with `n_params` parameters.
  // This value will change during this function.
  int_t n_params = params.size();

  zs::function_parameter_interface cl_info;

  if (is_native_closure) {
    cl_info = nclosure->get_parameter_interface();
  }
  else if (closure_obj.is_closure()) {
    cl_info = closure->get_parameter_interface();
  }

  // The function expects `n_expected_params` parameters.
  const int_t n_expected_params = cl_info.get_parameters_count();

  // The closure has `n_default_params` default parameters values.
  const int_t n_default_params = cl_info.get_default_parameters_count();
  const int_t n_req_params = cl_info.get_minimum_required_parameters_count();
  const bool has_variadic_parameters = cl_info.has_variadic_parameters();
  std::span<const object> closure_default_params = cl_info.get_default_parameters();

  object base_obj = params[0];
  object this_obj = base_obj;

  // ?????????????????
  if (!_registers[0].is_null()) {
    this_obj = _registers[0];
    _registers[0] = nullptr;
  }
  else if (cl_info.get_this() and !cl_info.get_this()->is_null()) {
    this_obj = *cl_info.get_this();
  }

  if (closure_obj.is_native_function() or (is_native_closure and !n_expected_params)) {
    // Enter function call.
    // Get the stack state before pushing the parameters.
    // Set the new stack base (the current top of the stack will be the next stack base).
    _stack.set_stack_base(_call_stack.emplace_back(closure_obj, _stack.get_state()).previous_top_index);

    // Push `n_params` elements starting at `stack_base` on top of the stack.
    _stack.push(params);

    ZS_ASSERT(_call_stack.back().previous_top_index == int_t(_stack.get_absolute_top() - n_params),
        "invalid stack parameters");
    ZS_ASSERT(stack_size() == n_params, "invalid stack parameters");

    // This.
    _stack[0] = this_obj;

    const int_t native_call_result = is_native_closure ? nclosure->call(this) : (*nfct)(this);
    _stack[0] = base_obj;
    ret_value = nullptr;

    if (native_call_result < 0) {
      if (is_protected_call) {
        reset_protected_call_stack(call_stack_index);
      }

      // We leave the stack as is when a non-protected call error occurs.
      // In other words, we don't call `leave_function_call()`, this might help
      // to retrieve more accurate debug information.
      return errc::invalid_native_function_call;
    }

    if (native_call_result > 0) {
      ret_value = _stack.top();
    }

    return leave_function_call();
  }

  // A span of missing default parameters that will be pushed after the given `n_params` if needed.
  std::span<const object> extra_params;

  // Some parameters are missing?
  if (n_params < n_expected_params) {

    // We check if some of those have a default parameter value.
    // If `diff` is bigger than `n_default_params`, we're still gonna be short on parameters.

    if (const int_t diff = n_expected_params - n_params; diff <= n_default_params) {
      extra_params = closure_default_params.subspan(n_default_params - diff, diff);
    }
    else {
      // Let's leave this condition like this for now, we'll check
      // it again below. Maybe we'll add some other kind of default parameters
      // (e.g. named parameters) in the future.

      return ZS_VM_ERROR(errc::invalid_parameter_count, "Not enough parameters, got ", n_params - 1,
          ", but at least ", n_req_params - 1, " were expected (excluding 'this').");
    }
  }

  // We have `extra_params.size()` that we can add to `n_params`.
  // There's no way that we end up with some variadic parameters and some
  // missing defaults.

  object vargs;
  if (has_variadic_parameters and n_params >= n_expected_params) {
    // If `n_params == n_expected_params` and the last parameter is already an array,
    // we leave everything as is, otherwise, we still need to put the last parameter in an array.
    if (!(n_params == n_expected_params and params.back().is_array())) {
      // The function was expecting `n_expected_params`, we have to keep in mind
      // that the last expected parameter is actually the first variadic parameter.
      // We need to create an array with all the extra parameters starting at the
      // last expected one `(n_expected_params - 1)`.
      int_t first_vparam_index = n_expected_params - 1;
      vargs = zs::_a(_engine, params.subspan(first_vparam_index, n_params - first_vparam_index));
      extra_params = std::span<const object>(&vargs, 1);

      // We'll push the variadic array below.
      n_params = first_vparam_index;
    }
  }

  if (n_expected_params != n_params + (int_t)extra_params.size()) {
    return ZS_VM_ERROR(errc::invalid_parameter_count, "Too many parameters, got ", n_params - 1,
        ", but only ", n_expected_params - 1, " were expected (excluding 'this').");
  }

  // Enter function call.
  // Get the stack state before pushing the parameters.
  // Set the new stack base (the current top of the stack will be the next stack base).
  _stack.set_stack_base(_call_stack.emplace_back(closure_obj, _stack.get_state()).previous_top_index);

  // Push `n_params` elements starting at `stack_base` on top of the stack.
  _stack.push(params.subspan(0, n_params));

  ZS_ASSERT(_call_stack.back().previous_top_index == int_t(_stack.get_absolute_top() - n_params),
      "invalid stack parameters");
  ZS_ASSERT(stack_size() == n_params, "invalid stack parameters");

  // We push the extra default params.
  _stack.push(extra_params);
  n_params += extra_params.size();

  // This.
  _stack[0] = this_obj;

  zs::error_result call_error_result;

  if (is_native_closure) {
    const int_t native_call_result = nclosure->call(this);
    ret_value = nullptr;

    if (native_call_result < 0) {
      call_error_result = errc::invalid_native_function_call;
    }
    else if (native_call_result > 0) {
      ret_value = _stack.top();
    }
  }
  else {
    // We make room for the function stack by pushing some empty objects.
    zs::function_prototype_object* fpo = closure->get_function_prototype();
    _stack.push_n(fpo->_stack_size - n_params);

    // Execute.
    zs::instruction_vector& instructions = fpo->_instructions;
    exec_op_data_t op_data{ closure, fpo, ret_value };
    const instruction_iterator end_it = instructions.end();

    for (zs::instruction_iterator it = instructions.begin(); it != end_it;) {
      // Keeping the last instruction iterator in case of an error.
      const zs::instruction_iterator inst_it = it;

      call_error_result = executor::call_op(this, *it, it, op_data);

      if (call_error_result) {
        (void)runtime_action<runtime_code::handle_error>(fpo, inst_it, call_error_result.code);
        break;
      }

      else if (call_error_result == errc::returned) {
        // The `op_return` returns zs::error_code::returned on success.
        // We are done with this function.
        break;
      }
    }
  }

  // Reset the base object.
  ZS_ASSERT(&ret_value != &_stack[0]);
  _stack[0] = base_obj;

  if (call_error_result) {
    if (is_protected_call) {
      reset_protected_call_stack(call_stack_index);
    }

    // We leave the stack as is when a non-protected call error occurs.
    // In other words, we don't call `leave_function_call()`, this might help
    // to retrieve more accurate debug information.
    return call_error_result;
  }

  return leave_function_call();
}

zs::error_result virtual_machine::tail_call_closure(
    const object& closure_obj, zs::parameter_list params, object& ret_value, bool is_protected_call) {

  zs::native_closure_object* nclosure = closure_obj._native_closure;
  zs::closure_object* closure = closure_obj._closure;
  zs::function_t nfct = closure_obj._nfct;

  const bool is_native_closure = closure_obj.is_native_closure();
  //  const size_t call_stack_index = _call_stack.size();

  // The call was made with `n_params` parameters.
  // This value will change during this function.
  int_t n_params = params.size();

  zs::function_parameter_interface cl_info;

  if (is_native_closure) {
    cl_info = nclosure->get_parameter_interface();
  }
  else if (closure_obj.is_closure()) {
    cl_info = closure->get_parameter_interface();
  }

  // The function expects `n_expected_params` parameters.
  const int_t n_expected_params = cl_info.get_parameters_count();

  // The closure has `n_default_params` default parameters values.
  const int_t n_default_params = cl_info.get_default_parameters_count();

  //
  const int_t n_req_params = n_expected_params - n_default_params;

  //
  const bool has_variadic_parameters = cl_info.has_variadic_parameters();

  //
  std::span<const object> closure_default_params = cl_info.get_default_parameters();

  object base_obj = params[0];
  object this_obj = base_obj;

  if (cl_info.get_this() and !cl_info.get_this()->is_null()) {
    this_obj = *cl_info.get_this();
  }

  if (closure_obj.is_native_function() or (is_native_closure and !n_expected_params)) {
    // Enter function call.

    object* optr = &_stack[0];
    for (const auto& p : params) {
      *optr++ = p;
    }

    // This.
    _stack[0] = this_obj;

    const int_t native_call_result = is_native_closure ? nclosure->call(this) : (*nfct)(this);
    _stack[0] = base_obj;
    ret_value = nullptr;

    if (native_call_result < 0) {

      // We leave the stack as is when a non-protected call error occurs.
      // In other words, we don't call `leave_function_call()`, this might help
      // to retrieve more accurate debug information.
      return errc::invalid_native_function_call;
    }

    if (native_call_result > 0) {
      ret_value = _stack.top();
    }

    return {};
  }

  // A span of missing default parameters that will be pushed after the given `n_params` if needed.
  std::span<const object> extra_params;

  // Some parameters are missing?
  if (n_params < n_expected_params) {

    // We check if some of those have a default parameter value.
    // If `diff` is bigger than `n_default_params`, we're still gonna be short on parameters.

    if (const int_t diff = n_expected_params - n_params; diff <= n_default_params) {
      extra_params = closure_default_params.subspan(n_default_params - diff, diff);
    }
    else {
      // Let's leave this condition like this for now, we'll check
      // it again below. Maybe we'll add some other kind of default parameters
      // (e.g. named parameters) in the future.

      return ZS_VM_ERROR(errc::invalid_parameter_count, "Not enough parameters, got ", n_params - 1,
          ", but at least ", n_req_params - 1, " were expected (excluding 'this').");
    }
  }

  // We have `extra_params.size()` that we can add to `n_params`.
  // There's no way that we end up with some variadic parameters and some
  // missing defaults.

  object vargs;
  if (has_variadic_parameters and n_params >= n_expected_params) {
    // If `n_params == n_expected_params` and the last parameter is already an array,
    // we leave everything as is, otherwise, we still need to put the last parameter in an array.
    if (!(n_params == n_expected_params and params.back().is_array())) {
      // The function was expecting `n_expected_params`, we have to keep in mind
      // that the last expected parameter is actually the first variadic parameter.
      // We need to create an array with all the extra parameters starting at the
      // last expected one `(n_expected_params - 1)`.
      int_t first_vparam_index = n_expected_params - 1;
      vargs = zs::_a(_engine, params.subspan(first_vparam_index, n_params - first_vparam_index));
      extra_params = std::span<const object>(&vargs, 1);

      // We'll push the variadic array below.
      n_params = first_vparam_index;
    }
  }

  if (n_expected_params != n_params + (int_t)extra_params.size()) {
    return ZS_VM_ERROR(errc::invalid_parameter_count, "Too many parameters, got ", n_params - 1,
        ", but only ", n_expected_params - 1, " were expected (excluding 'this').");
  }

  // Enter function call.
  // Get the stack state before pushing the parameters.
  // Set the new stack base (the current top of the stack will be the next stack base).
  //  _stack.set_stack_base(_call_stack.emplace_back(closure_obj, _stack.get_state()).previous_top_index);
  //
  //  // Push `n_params` elements starting at `stack_base` on top of the stack.
  //  _stack.push(params.subspan(0, n_params));

  //  ZS_ASSERT(_call_stack.back().previous_top_index == (_stack.get_absolute_top() - n_params),
  //      "invalid stack parameters");
  //  ZS_ASSERT(stack_size() == n_params, "invalid stack parameters");

  object* optr = &_stack[0];
  for (const auto& p : params.subspan(0, n_params)) {
    *optr++ = p;
  }

  // We push the extra default params.
  for (const auto& p : extra_params) {
    *optr++ = p;
  }

  //  _stack.push(extra_params);
  n_params += extra_params.size();

  // This.
  _stack[0] = this_obj;

  zs::error_result call_error_result;

  if (is_native_closure) {
    const int_t native_call_result = nclosure->call(this);
    ret_value = nullptr;

    if (native_call_result < 0) {
      call_error_result = errc::invalid_native_function_call;
    }
    else if (native_call_result > 0) {
      ret_value = _stack.top();
    }
  }
  else {
    // We make room for the function stack by pushing some empty objects.
    zs::function_prototype_object* fpo = closure->get_function_prototype();
    //    _stack.push_n(fpo->_stack_size - n_params);

    // Execute.
    zs::instruction_vector& instructions = fpo->_instructions;
    exec_op_data_t op_data{ closure, fpo, ret_value };
    const instruction_iterator end_it = instructions.end();

    for (zs::instruction_iterator it = instructions.begin(); it != end_it;) {
      // Keeping the last instruction iterator in case of an error.
      const zs::instruction_iterator inst_it = it;

      call_error_result = executor::call_op(this, *it, it, op_data);

      if (call_error_result) {
        (void)runtime_action<runtime_code::handle_error>(fpo, inst_it, call_error_result.code);
        break;
      }

      else if (call_error_result == errc::returned) {
        // The `op_return` returns zs::error_code::returned on success.
        // We are done with this function.
        break;
      }
    }
  }

  // Reset the base object.
  ZS_ASSERT(&ret_value != &_stack[0]);
  _stack[0] = base_obj;

  if (call_error_result) {
    if (is_protected_call) {
      //      reset_protected_call_stack(call_stack_index);
    }

    // We leave the stack as is when a non-protected call error occurs.
    // In other words, we don't call `leave_function_call()`, this might help
    // to retrieve more accurate debug information.
    return call_error_result;
  }

  return {};
}

zs::error_result virtual_machine::call(
    const object& closure, zs::parameter_list params, object& ret_value, bool is_protected_call) noexcept {

  const object_type otype = closure.get_type();

  ZS_ASSERT(
      zb::is_one_of(otype, k_closure, k_native_closure, k_native_function, k_table, k_user_data, k_struct),
      get_object_type_name(otype));

  switch (otype) {
  case k_closure:
  case k_native_function:
  case k_native_closure:
    return call_closure(closure, params, ret_value);

  case zs::object_type::k_user_data: {
    object fct;

    object key = constants::get<meta_method::mt_call>();

    if (auto err = proxy::get(this, closure, key, closure.as_udata().get_delegate(), fct, false)) {
      return err;
    }

    if (fct.is_function()) {
      ZS_TODO("Add comments and make sure that this works");

      _registers[0] = closure;
      if (auto err = call(fct, params, ret_value)) {
        _registers[0] = nullptr;
        return err;
      }
      _registers[0] = nullptr;

      return {};
    }
    return zs::error_code::inaccessible;
  }

  case zs::object_type::k_table: {
    object fct;
    object key = constants::get<meta_method::mt_call>();

    if (auto err = proxy::get(this, closure, key, closure, fct, false)) {
      return err;
    }

    if (fct.is_function()) {
      ZS_TODO("Add comments and make sure that this works");

      _registers[0] = closure;
      if (auto err = call(fct, params, ret_value)) {
        _registers[0] = nullptr;
        return err;
      }

      _registers[0] = nullptr;
      return {};
    }

    return zs::error_code::inaccessible;
  }

  case k_struct: {
    zs::struct_object& strct = closure.as_struct();

    struct_instance_object* new_strct_obj = strct.create_instance();
    new_strct_obj->set_initialized(false);

    object new_obj;
    new_obj._type = k_struct_instance;
    new_obj._struct_instance = new_strct_obj;
    ret_value = new_obj;

    zs::object constructor;
    if (auto err = strct.resolve_constructor(this, params, constructor)) {
      return ZS_VM_ERROR(err, "Could not resolve constructor for struct.");
    }

    if (constructor.is_function()) {

      _registers[0] = new_obj;
      if (auto err = call(constructor, params, ret_value)) {
        _registers[0] = nullptr;
        new_strct_obj->set_initialized(true);
        return err;
      }
      _registers[0] = nullptr;
    }

    new_strct_obj->set_initialized(true);
    return {};
  }

  default:
    return errc::invalid;
  }

  return zs::error_code::invalid;
}

ZBASE_PRAGMA_PUSH_NO_MISSING_SWITCH_WARNING()

zs::error_result virtual_machine::get(const object& obj, const object& key, object& dest) {

  switch (obj.get_type()) {
  case k_integer:
  case k_float:
    return proxy::get(this, obj, key, get_default_number_delegate(), dest, true);

  case k_small_string:
  case k_string_view:
  case k_long_string: {
    if (key.is_integer()) {
      return proxy::string_raw_get_internal(obj.get_string_unchecked(), key._int, dest);
    }

    return proxy::get(this, obj, key, get_default_string_delegate(), dest, true);
  }

  case k_atom: {
    zs::object delegate;
    if (auto err
        = get_delegated_atom_delegates_table().as_table().get((int_t)obj._ex2_delegate_id, delegate)) {
      // We don't want to return a 'not_found' error to prevent any get propagation.
      return errc::inaccessible;
    }

    return proxy::get(this, obj, key, delegate, dest);
  }

  case k_table:
    return proxy::get(this, obj, key, obj, dest);

  case k_array: {
    // Get 'key' from array.
    array_object& aobj = obj.as_array();

    // If the array has a delegate, we look in the delegate first.
    if (aobj.has_delegate()) {
      auto err = proxy::get(this, obj, key, aobj.get_delegate(), dest, true);

      // No error, the key was found in the delegate.
      if (!err) {
        return {};
      }

      // The key wasn't found in the delegate and 'not_found' was returned.
      // We can now look directly in the array only if the key is an integer.
      if (err == errc::not_found) {
        if (key.is_integer()) {
          return aobj.get(key._int, dest);
        }

        // We don't want to return a 'not_found' error to prevent any get propagation.
        return errc::inaccessible;
      }

      return err;
    }

    // The array doesn't have a delegate.
    // We look directly in the array only if the key is an integer.
    if (key.is_integer()) {
      // array_object::get() returns either a 'success' or 'out_of_bounds' error code.
      return aobj.get(key._int, dest);
    }

    // At this point, the array has no delegate and the key is not an integer.
    // We can look in the default array delegate if 'delegable_object::use_default_delegate()'
    // returns true.
    if (aobj.is_use_default()) {
      return proxy::get(this, obj, key, get_default_array_delegate(), dest, true);
    }

    // We get to this point when the key isn't an integer and 'delegable_object::use_default_delegate()'
    // returns false.
    return errc::inaccessible;
  }

  case k_struct:
    return obj.as_struct().get(key, dest);

  case k_struct_instance: {
    struct_instance_object& sobj = obj.as_struct_instance();

    if (auto err = sobj.get(key, dest, _stack[0]._struct_instance == &sobj)) {
      if (err == zs::errc::inaccessible_private) {
        return ZS_VM_ERROR(err, "Could not access private struct member ", key, ".\n");
      }

      return ZS_VM_ERROR(err, "Struct get. Field ", key, " doesn't exists.\n", err.message());
    }

    return {};
  }

  case k_user_data: {
    user_data_object& udata = obj.as_udata();

    if (ZBASE_UNLIKELY(!udata.has_delegate())) {
      return errc::inaccessible;
    }

    return proxy::get(this, obj, key, udata.get_delegate(), dest, true);
  }

  case k_weak_ref:
    return this->get(obj.get_weak_ref_value(), key, dest);

  case k_closure:
  case k_native_closure:
  case k_native_function:
    return proxy::get(this, obj, key, get_default_function_delegate(), dest, true);

  default:
    set_error("Can't get a value from '", zs::get_object_type_name(obj.get_type()), "'.\n");
    dest.reset();
    return errc::inaccessible;
  }
}

zs::error_result virtual_machine::raw_get(const object& obj, const object& key, object& dest) {
  object_type t = obj.get_type();

  switch (t) {
  case k_small_string:
  case k_string_view:
  case k_long_string:
    return key.is_integer() ? proxy::string_raw_get_internal(obj.get_string_unchecked(), key._int, dest)
                            : error_result(errc::inaccessible);

  case k_table:
    return obj.as_table().get(key, dest);

  case k_array:
    return obj.as_array().get(key, dest);

  case k_struct:
    return obj.as_struct().get(key, dest);

  case k_struct_instance:
    // Private members and methods are also accessible when calling raw_get.
    return obj.as_struct_instance().get(key, dest, true);

  case k_weak_ref:
    return raw_get(obj.get_weak_ref_value(), key, dest);

  default:
    dest = nullptr;
    return ZS_VM_ERROR(errc::inaccessible, "Can't raw_get a value from '", zs::get_object_type_name(t), "'.");
  }
}

zs::error_result virtual_machine::raw_has(const object& obj, const object& key, object& dest) {

  switch (obj.get_type()) {

  case k_atom:
    return errc::unimplemented;

  case k_long_string:
  case k_small_string:
  case k_string_view: {
    if (key.is_integer()) {
      std::string_view s = obj.get_string_unchecked();
      const int_t sz = (int_t)s.size();
      int_t index = key._int;

      if (index < 0) {
        index += sz;
      }

      if (index >= 0 && index < sz) {
        dest = (int_t)s[index];
        return {};
      }

      return zs::errc::out_of_bounds;
    }

    return errc::inaccessible;
  }

  case k_table:
    return obj.as_table().get(key, dest);

  case k_array:
    return obj.as_array().get(key, dest);

  case k_struct:
    return obj.as_struct().get(key, dest);

  case k_struct_instance: {
    zs::struct_instance_object& sobj = obj.as_struct_instance();

    if (auto err = sobj.get(key, dest, _stack[0]._struct_instance == &sobj)) {

      if (err == zs::errc::inaccessible_private) {
        set_error("Could not access private struct member ", key, ".\n");
      }
      else {
        set_error("Struct get. Field ", key, " doesn't exists.\n", err.message());
      }

      return err;
    }

    return {};
  }

  case k_user_data:
    return errc::inaccessible;

  case k_weak_ref:
    return raw_get(obj.get_weak_ref_value(), key, dest);

  default:
    set_error("Can't get a value from '", zs::get_object_type_name(obj.get_type()), "'.\n");
    dest.reset();
    return zs::error_code::inaccessible;
  }
}

zs::error_result virtual_machine::has(const object& obj, const object& key, object& dest) {

  switch (obj.get_type()) {
  case k_small_string:
    return proxy::get(this, obj, key, nullptr, dest);

  case k_string_view:
    return proxy::get(this, obj, key, nullptr, dest);

  case k_atom:
    return zs::error_code::unimplemented;

  case k_long_string:
    return proxy::get(this, obj, key, nullptr, dest);

  case k_table:
    return proxy::get(this, obj, key, obj, dest);

  case k_array:
    return proxy::get(this, obj, key, obj.as_array().get_delegate(), dest);

  case k_struct:
    return proxy::get(this, obj, key, obj, dest);

  case k_struct_instance: {
    zs::struct_instance_object& sobj = obj.as_struct_instance();

    if (auto err = sobj.get(key, dest, _stack[0]._struct_instance == &sobj)) {

      if (err == zs::errc::inaccessible_private) {
        set_error("Could not access private struct member ", key, ".\n");
      }
      else {
        set_error("Struct get. Field ", key, " doesn't exists.\n", err.message());
      }

      return err;
    }

    return {};
  }
  case k_user_data:
    return proxy::get(this, obj, key, obj.as_udata().get_delegate(), dest);

  case k_weak_ref:
    return this->get(obj.get_weak_ref_value(), key, dest);

  default:
    set_error("Can't get a value from '", zs::get_object_type_name(obj.get_type()), "'.\n");
    dest.reset();
    return zs::error_code::inaccessible;
  }
}

zs::error_result virtual_machine::to_string(const object& obj, object& dest) {

  ZS_ASSERT(&obj != &dest);

  dest = nullptr;

  switch (obj._type) {
  case k_weak_ref:
    return zs::error_code::unimplemented;

  case object_type::k_null:
    dest = zs::_ss("null");
    return {};

  case object_type::k_none:
    dest = zs::_ss("none");
    return {};

  case object_type::k_bool:
    dest = (bool)obj._int ? zs::_ss("true") : zs::_ss("false");
    return {};

  case object_type::k_integer: {
    if (obj.has_flags(object_flags_t::f_char)) {
      zb::stack_string<4> buffer(zb::unicode::code_point_size_u8((char32_t)obj._int), 0);
      zb::unicode::append_u32_to_u8((char32_t)obj._int, buffer.data());
      dest = zs::_s(_engine, buffer);
      return {};
    }

    zb::stack_string<128> buffer(128, '\0');

    if (zb::optional_result<size_t> res = zb::to_chars(buffer.data(), buffer.capacity(), obj._int)) {
      buffer.resize(res.value());
      dest = zs::_s(_engine, buffer);
      return {};
    }

    return zs::error_code::conversion_error;
  }

  case object_type::k_float: {
    zb::stack_string<128> buffer(128, '\0');

    if (zb::optional_result<size_t> res = zb::to_chars(buffer.data(), buffer.capacity(), obj._float)) {
      buffer.resize(res.value());
      dest = zs::_s(_engine, buffer);
      return {};
    }

    return zs::error_code::conversion_error;
  }

  case object_type::k_long_string:
  case object_type::k_string_view:
  case object_type::k_small_string:
    dest = obj;
    return {};

  case k_user_data:
  case k_array:
  case k_table:
  case k_atom:
  case k_struct_instance:
    if (auto err = unary_delegate_operation(
            meta_method::mt_tostring, obj, dest, constants::k_string_mask, errc::success)) {
      return err;
    }

    if (!dest.is_string() and obj.is_user_data()) {

      zs::ostringstream stream(zs::create_string_stream(_engine));
      if (auto err = obj.as_udata().convert_to_string(stream)) {
        return err;
      }

      dest = zs::_s(_engine, stream.view());
    }

    break;

  default:
    break;
  }

  if (!dest.is_string()) {
    zs::ostringstream stream(zs::create_string_stream(_engine));
    stream << zs::get_exposed_object_type_name(obj.get_type()) << ": 0x" << std::setfill('0')
           << std::setw(sizeof(int_t) * 2) << std::hex << (int_t)obj._lvalue;
    dest = zs::_s(_engine, stream.view());
  }

  return {};
}

zs::error_result virtual_machine::copy(const object& obj, object& dest) {

  ZS_ASSERT(&obj != &dest);

  switch (obj._type) {
  case k_weak_ref:
    return zs::error_code::unimplemented;

  case object_type::k_long_string:
    dest = zs::_s(_engine, obj.get_long_string_unchecked());
    return {};

  case k_closure:
    dest = obj.as_closure().clone();
    return {};

  case k_native_closure:
    dest = obj.as_native_closure().clone();
    return {};

  case k_user_data:
    if (auto err = unary_delegate_operation(meta_method::mt_copy, obj, dest, 0, errc::success)) {
      return err;
    }

    if (dest.is_null()) {
      dest = obj.as_udata().clone();
    }
    break;

  case k_array:
    if (auto err = unary_delegate_operation(meta_method::mt_copy, obj, dest, 0, errc::success)) {
      return err;
    }

    if (dest.is_null()) {
      dest = obj.as_array().clone();
    }
    break;

  case k_table:
    if (auto err = unary_delegate_operation(meta_method::mt_copy, obj, dest, 0, errc::success)) {
      return err;
    }

    if (dest.is_null()) {
      dest = obj.as_table().clone();
    }
    break;

  case k_atom:
    if (auto err = unary_delegate_operation(meta_method::mt_copy, obj, dest, 0, errc::success)) {
      return err;
    }

    break;

  case k_struct_instance:
    if (auto err = unary_delegate_operation(meta_method::mt_copy, obj, dest, 0, errc::success)) {
      return err;
    }

    if (dest.is_null()) {
      dest = obj.as_struct_instance().clone();
    }
    break;

  default:
    break;
  }

  if (dest.is_null()) {
    dest = object(obj, true);
  }

  return {};
}

zs::object virtual_machine::get_delegate(const object& obj, bool with_defaults) {
  if (with_defaults) {
    switch (obj.get_type()) {
    case k_bool:
    case k_null:
    case k_none:
      return nullptr;

    case k_integer:
    case k_float:
      return get_default_number_delegate();

    case k_string_view:
    case k_small_string:
    case k_long_string:
      return get_default_string_delegate();

    case k_atom: {
      zs::object delegate;
      if (auto err
          = get_delegated_atom_delegates_table().as_table().get((int_t)obj._ex2_delegate_id, delegate)) {
        return nullptr;
      }

      return delegate;
    }

    case k_struct:
      return get_default_struct_delegate();

    case k_closure:
    case k_native_closure:
    case k_native_function:
      return get_default_function_delegate();

    case k_table: {
      const table_object& tobj = obj.as_table();
      return tobj.has_delegate()  ? tobj.get_delegate()
          : tobj.is_use_default() ? get_default_table_delegate()
                                  : nullptr;
    }
    case k_array: {
      const array_object& aobj = obj.as_array();
      return aobj.has_delegate()  ? aobj.get_delegate()
          : aobj.is_use_default() ? get_default_array_delegate()
                                  : nullptr;
    }

    case k_user_data: {
      const user_data_object& uobj = obj.as_udata();
      return uobj.has_delegate() ? uobj.get_delegate() : nullptr;
    }

    default:
      break;
    }

    return obj.is_delegable() ? obj.as_delegable().get_delegate() : nullptr;
  }

  if (obj.is_atom()) {
    zs::object delegate;
    if (auto err
        = get_delegated_atom_delegates_table().as_table().get((int_t)obj._ex2_delegate_id, delegate)) {
      return nullptr;
    }

    return delegate;
  }

  return obj.is_delegable() ? obj.as_delegable().get_delegate() : nullptr;
}

zs::error_result virtual_machine::type_of(const object obj, object& dest) {

  dest = nullptr;

  switch (obj._type) {
  case k_user_data:
  case k_array:
  case k_table:
  case k_atom:
    if (auto err = unary_delegate_operation(
            meta_method::mt_typeof, obj, dest, constants::k_string_mask, errc::success)) {
      return err;
    }

    if (!dest.is_string() and obj.is_user_data()) {
      dest = obj.as_udata().get_type_id();
    }

    break;

  case k_struct_instance:
    dest = obj.as_struct_instance().get_name();
    break;

  case k_struct:
    dest = obj.as_struct().get_name();
    break;

  default:
    break;
  }

  if (!dest.is_string()) {
    dest = obj.get_exposed_type_name();
  }

  return {};
}

zs::error_result virtual_machine::arithmetic_operation(
    arithmetic_op op, object& target, const object lhs, const object rhs) {

  if (lhs.is_null() or rhs.is_null()) {
    if (op == aop_compare) {
      if (lhs.is_null() and rhs.is_null()) {
        target = 0;
        return {};
      }

      target = lhs.get_type() < rhs.get_type() ? -1 : 1;
      return {};
    }

    target = nullptr;
    return ZS_VM_ERROR(errc::invalid_operation, "Invalid arithmetic operation on a null type.");
  }

  if (lhs.is_number_or_bool() and rhs.is_number_or_bool()) {
    if (lhs.is_float() or rhs.is_float()) {
      float_t a = lhs.convert_to_float_unchecked();
      float_t b = rhs.convert_to_float_unchecked();
      switch (op) {
      case aop_add:
      case aop_add_eq:
        target = a + b;
        return {};
      case aop_sub:
      case aop_sub_eq:
        target = a - b;
        return {};
      case aop_mul:
      case aop_mul_eq:
        target = a * b;
        return {};
      case aop_div:
      case aop_div_eq:
        target = a / b;
        return {};
      case aop_mod:
      case aop_mod_eq:
        target = std::fmod(a, b);
        return {};
      case aop_exp:
      case aop_exp_eq:
        target = std::pow(a, b);
        return {};
      case aop_compare:
        target = a == b ? 0 : a < b ? -1 : 1;
        return {};
      }

      target = nullptr;
      return ZS_VM_ERROR(errc::invalid_operation, "Invalid arithmetic operation '",
          zs::arithmetic_op_to_string(op), "' on a 'float' type.");
    }

    int_t a = lhs._int;
    int_t b = rhs._int;

    switch (op) {
    case aop_add:
    case aop_add_eq:
      target = a + b;
      return {};
    case aop_sub:
    case aop_sub_eq:
      target = a - b;
      return {};
    case aop_mul:
    case aop_mul_eq:
      target = a * b;
      return {};
    case aop_div:
    case aop_div_eq:
      target = a / b;
      return {};
    case aop_mod:
    case aop_mod_eq:
      target = a % b;
      return {};
    case aop_exp:
    case aop_exp_eq:
      target = (int_t)std::pow(a, b);
      return {};
    case aop_bitwise_or:
    case aop_bitwise_or_eq:
      target = a | b;
      return {};
    case aop_bitwise_and:
    case aop_bitwise_and_eq:
      target = a & b;
      return {};
    case aop_bitwise_xor:
    case aop_bitwise_xor_eq:
      target = a ^ b;
      return {};
    case aop_lshift:
    case aop_lshift_eq:
      target = a << b;
      return {};
    case aop_rshift:
    case aop_rshift_eq:
      target = a >> b;
      return {};
    case aop_compare:
      target = a == b ? 0 : a < b ? -1 : 1;
      return {};
    }

    target = nullptr;
    return ZS_VM_ERROR(errc::invalid_operation, "Invalid arithmetic operation '",
        zs::arithmetic_op_to_string(op), "' on a 'integer' type.");
  }

  if (lhs.is_string() and rhs.is_string()) {

    switch (op) {
    case aop_add:
    case aop_add_eq:
      target = object::create_concat_string(_engine, lhs.get_string_unchecked(), rhs.get_string_unchecked());
      return {};

    case aop_compare: {
      std::string_view a = lhs.get_string_unchecked();
      std::string_view b = rhs.get_string_unchecked();
      target = a == b ? 0 : a < b ? -1 : 1;
      return {};
    }
    }

    target = nullptr;
    return ZS_VM_ERROR(errc::invalid_operation, "Invalid arithmetic operation '",
        zs::arithmetic_op_to_string(op), "' on a 'string' type.");
  }

  //  object delegate = start_delegate_chain(lhs);
  return binary_delegate_operation(zs::arithmetic_op_to_meta_method(op), lhs, rhs, target);
}
ZBASE_PRAGMA_POP()

template <class T>
static constexpr inline error_result number_arith(arithmetic_uop uop, T& value, object& target) noexcept {
  switch (uop) {
  case arithmetic_uop::uop_minus:
    target = -value;
    return {};

  case arithmetic_uop::uop_bitwise_not:
    if constexpr (std::is_integral_v<T>) {
      target = ~value;
      return {};
    }
    else {
      target = nullptr;
      return errc::invalid_operation;
    }

  case arithmetic_uop::uop_incr:
    target = value++;
    return {};

  case arithmetic_uop::uop_decr:
    target = value--;
    return {};

  case arithmetic_uop::uop_pre_incr:
    target = ++value;
    return {};

  case arithmetic_uop::uop_pre_decr:
    target = --value;
    return {};
  }

  target = nullptr;
  return errc::invalid_operation;
};

zs::error_result virtual_machine::unary_arithmetic_operation(
    arithmetic_uop uop, object& target, object& src) {

  if (!src.has_type_mask(constants::k_unary_arithmetic_type_mask)) {
    target = nullptr;
    return ZS_VM_ERROR(errc::invalid_operation, "Invalid arithmetic operation on a '",
        zs::get_object_type_name(src.get_type()), "' type.");
  }

  if (src.is_bool()) {
    if (uop == uop_minus) {
      target = -src._int;
      return {};
    }
    else if (uop == uop_bitwise_not) {
      target = ~src._int;
      return {};
    }

    return ZS_VM_ERROR(errc::invalid_operation, "Invalid arithmetic operation on a bool type.");
  }

  if (src.is_float()) {
    return number_arith(uop, src._float, target);
  }

  if (src.is_integer()) {
    return number_arith(uop, src._int, target);
  }

  if (!src.is_ref_counted() and zs::is_postfix_arithmetic_uop(uop)) {
    const meta_method mt = zs::arithmetic_uop_to_meta_method(arithmetic_uop_postfix_to_prefix(uop));

    // Swap target and src.
    target = src;
    return unary_delegate_operation(mt, target, src);
  }

  const meta_method mt = zs::arithmetic_uop_to_meta_method(uop);
  return unary_delegate_operation(mt, src, target);
}

//--------------------------------------------------------------------------

zs::error_result virtual_machine::compare(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(aop_compare, target, lhs, rhs);
}

zs::error_result virtual_machine::lt(object& target, const object& lhs, const object& rhs) {

  if (auto err = arithmetic_operation(aop_compare, target, lhs, rhs)) {
    return err;
  }

  target = target._int == -1;
  return {};
}

zs::error_result virtual_machine::eq(object& target, const object& lhs, const object& rhs) {
  if (memcmp(&lhs, &rhs, sizeof(object)) == 0) {
    target = true;
    return {};
  }

  if (auto err = arithmetic_operation(aop_compare, target, lhs, rhs)) {
    return err;
  }

  target = target._int == 0;
  return {};
}

//--------------------------------------------------------------------------

error_result virtual_machine::binary_delegate_operation(
    meta_method mt, const object lhs, const object rhs, object& dest) {

  zs::object func;
  zs::object delegate;

  if (auto err = find_meta_method(mt, lhs, delegate, func)) {

    if (err != errc::not_found) {
      dest = nullptr;
      return ZS_VM_ERROR(err, "Invalid meta method type '", get_exposed_object_type_name(func.get_type()),
          "' in '", meta_method_name(mt), "' between '", get_exposed_object_type_name(lhs.get_type()),
          "' and '", get_exposed_object_type_name(rhs.get_type()), "'.");
    }
  }

  object l = lhs;
  object r = rhs;

  if (meta_method rmt; !func.is_function() and (rmt = zs::get_rhs_meta_method(mt)) != meta_method::mt_none) {
    if (auto err = find_meta_method(rmt, rhs, delegate, func)) {
      dest = nullptr;

      return ZS_VM_ERROR(err, "Invalid meta method type '", get_exposed_object_type_name(func.get_type()),
          "' in '", meta_method_name(mt), "' between '", get_exposed_object_type_name(lhs.get_type()),
          "' and '", get_exposed_object_type_name(rhs.get_type()), "'.");
    }

    std::swap(l, r);
    mt = rmt;
  }

  if (!func.is_function()) {
    dest = nullptr;
    return ZS_VM_ERROR(errc::invalid_operation, "Invalid meta method type '",
        get_exposed_object_type_name(func.get_type()), "' in '", meta_method_name(mt), "' between '",
        get_exposed_object_type_name(lhs.get_type()), "' and '", get_exposed_object_type_name(rhs.get_type()),
        "'.");
  }

  // We're getting ready to call the function with 3 parameters:
  // 0 - The left hand side object.
  // 1 - The right hand side value.
  // 2 - The delegate table containing the meta method.
  const int_t top_idx = stack_size();

  push(l);
  push(r);

  // This prevents the user from having to always declare the 3rd delegate parameter.
  // If the meta method is a closure and has only 2 parameters, we'll call
  // it with 2 instead.
  if (!(func.is_closure() and func.as_closure().get_parameters_count() == 2)) {
    push(delegate);
  }

  int_t n_params = stack_size() - top_idx;

  // We now have to call the function and remove the parameters from the stack.
  auto err = call(func, n_params, top_idx, dest);
  pop(n_params);

  if (err) {

    return ZS_VM_ERROR(err, "Invalid meta method call in '", meta_method_name(mt), "' between '",
        get_exposed_object_type_name(lhs.get_type()), "' and '", get_exposed_object_type_name(rhs.get_type()),
        "'.");
  }

  return err;
}

error_result virtual_machine::find_meta_method(
    meta_method mt, const object& obj, object& delegate_out, object& dest, meta_method alt) {

  static constexpr auto _get_meta_method
      = [](const object& delegate, const object& mt_name) -> const object* {
    if (delegate.is_table()) {
      return delegate.as_table().get(mt_name);
    }
    else {
      return delegate.as_struct_instance().get_meta_method(mt_name);
    }
  };

  object delegate = start_delegate_chain(obj);

  const object mt_name = zs::get_meta_method_name_object(mt);
  const object mt_alt_name = zs::get_meta_method_name_object(alt);

  while (delegate.is_meta_type()) {

    // We look directly in the delegate table for the meta function.
    // We never call '__get' when looking for a meta method.

    if (const zs::object* meta_func = _get_meta_method(delegate, mt_name)) {
      dest = *meta_func;
      delegate_out = delegate;
      return {};
    }

    if (const zs::object* meta_func = nullptr;
        alt != meta_method::mt_none and (meta_func = _get_meta_method(delegate, mt_alt_name))) {
      dest = *meta_func;
      delegate_out = delegate;
      return {};
    }

    // We couldn't find the function in the delegate.
    // Let's keep looking deeper for a parent delegate.
    if (delegate.is_table()) {
      delegate = delegate.as_table().get_delegate();

      continue;
    }

    // No delegate for struct instances.
    break;
  }

  dest = nullptr;
  delegate_out = nullptr;
  return errc::not_found;
}

error_result virtual_machine::unary_delegate_operation(meta_method mt, const object src, object& dest,
    uint32_t valid_objects_mask, error_code error_code_if_not_found) {

  zs::object func, delegate;
  if (auto err = find_meta_method(mt, src, delegate, func)) {
    dest = nullptr;
    return error_code_if_not_found;
  }

  // An arithmetic meta method should always be a function.
  if (!func.is_function()) {

    // Other types could be valid??
    if (func.has_type_mask(valid_objects_mask)) {
      dest = func;
      return {};
    }

    dest = nullptr;
    return ZS_VM_ERROR(errc::invalid_operation, "Invalid operator in user data.");
  }

  // We're getting ready to call the function with 2 parameters:
  // 0 - The left hand side object.
  // 1 - The delegate table containing the meta method.
  const int_t top_idx = stack_size();

  push(src);

  // This prevents the user from having to always declare the 2nd delegate parameter.
  // If the meta method is a closure and has only 1 parameters, we'll call
  // it with 1 instead.
  if (!(func.is_closure() and func.as_closure().get_parameters_count() == 1)) {
    push(delegate);
  }

  int_t n_params = stack_size() - top_idx;

  // We now have to call the function and remove the parameters from the stack.
  error_result err = call(func, n_params, top_idx, dest);
  pop(n_params);

  return err ? ZS_VM_ERROR(err, "Invalid operator call in user data.") : err;
  //
  //  while (delegate.is_meta_type()) {
  //
  //    // We look directly in the delegate table for the meta function.
  //    // We never call '__get' when looking for a meta method.
  //
  //    if (delegate.is_table()) {
  //      meta_func = delegate.as_table().get(lhs_mt_name);
  //    }
  //    else {
  //      meta_func = delegate.as_struct_instance().get_meta_method(lhs_mt_name);
  //    }
  //
  //    // We couldn't find the function in the delegate.
  //    // Let's keep looking deeper for a parent delegate.
  //    if (!meta_func) {
  //      if (delegate.is_table()) {
  //        delegate = delegate.as_table().get_delegate();
  //        continue;
  //      }
  //
  //      // No delegate for struct instances.
  //      break;
  //    }
  //
  //    // An arithmetic meta method should always be a function.
  //    if (!meta_func->is_function()) {
  //
  //      // Other types could be valid??
  //      if (meta_func->has_type_mask(valid_objects_mask)) {
  //        dest = *meta_func;
  //        return {};
  //      }
  //
  //      dest = nullptr;
  //      return ZS_VM_ERROR(errc::invalid_operation, "Invalid operator in user data.");
  //    }
  //
  //    // We're getting ready to call the function with 2 parameters:
  //    // 0 - The left hand side object.
  //    // 1 - The delegate table containing the meta method.
  //    const int_t top_idx = stack_size();
  //
  //    push(src);
  //
  //    // This prevents the user from having to always declare the 2nd delegate parameter.
  //    // If the meta method is a closure and has only 1 parameters, we'll call
  //    // it with 1 instead.
  //    if (!(meta_func->is_closure() and meta_func->as_closure().get_parameters_count() == 1)) {
  //      push(delegate);
  //    }
  //
  //    int_t n_params = stack_size() - top_idx;
  //
  //    // We now have to call the function and remove the parameters from the stack.
  //    error_result err = call(*meta_func, n_params, top_idx, dest);
  //    pop(n_params);
  //
  //    if (err) {
  //      return ZS_VM_ERROR(err, "Invalid operator call in user data.");
  //    }
  //
  //    return err;
  //  }
  //
  //  dest = nullptr;
  //  return error_code_if_not_found;
  //  //  return ZS_VM_ERROR(errc::invalid_operation, "Invalid operator in user data.");
}

zs::error_result virtual_machine::compile_buffer(
    std::string_view content, object&& source_name, zs::object& closure_result, bool with_vargs) {
  zs::jit_compiler compiler(_engine);
  zs::object fct_state;

  if (auto err = compiler.compile(content, std::move(source_name), fct_state, this, nullptr, with_vargs)) {
    _errors.append(compiler.get_errors());
    return err;
  }

  closure_result = zs::_c(_engine, std::move(fct_state), _global_table);

  if (with_vargs) {
    closure_result.as_closure()._default_params.push_back(zs::_a(_engine, 0));
  }

  return {};
}

zs::error_result virtual_machine::compile_buffer(
    std::string_view content, const object& source_name, zs::object& closure_result, bool with_vargs) {
  zs::jit_compiler compiler(_engine);
  zs::object fct_state;

  if (auto err = compiler.compile(content, source_name, fct_state, this, nullptr, with_vargs)) {
    _errors.append(compiler.get_errors());
    return err;
  }

  closure_result = zs::_c(_engine, std::move(fct_state), _global_table);

  if (with_vargs) {
    closure_result.as_closure()._default_params.push_back(zs::_a(_engine, 0));
  }

  return {};
}

zs::error_result virtual_machine::compile_file(
    const char* filepath, std::string_view source_name, zs::object& closure_result, bool with_vargs) {
  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return compile_buffer(loader.content(), source_name, closure_result, with_vargs);
}

zs::error_result virtual_machine::compile_file(
    const zs::object& filepath, std::string_view source_name, zs::object& closure_result, bool with_vargs) {
  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return compile_buffer(loader.content(), source_name, closure_result, with_vargs);
}

zs::error_result virtual_machine::call_buffer(
    std::string_view content, std::string_view source_name, zs::object& ret_value, bool with_vargs) {

  zs::object closure_result;
  if (auto err = compile_buffer(content, source_name, closure_result, with_vargs)) {
    return err;
  }

  if (!closure_result.is_closure()) {
    return zs::error_code::invalid_type;
  }

  //  zs::object this_table = zs::_t(_engine);
  //
  //  this_table.as_table().set_delegate(_root_table);

  if (auto err = call(closure_result, _global_table, ret_value)) {
    return err;
  }

  return {};
}

zs::error_result virtual_machine::call_buffer(std::string_view content, std::string_view source_name,
    zs::object& this_table, zs::object& ret_value, std::span<const object> args, bool with_vargs) {

  if (!this_table.is_table()) {
    //    this_table = create_this_table_from_root();
    this_table = _global_table;
  }

  zs::object closure_result;

  {
    zs::jit_compiler compiler(_engine);
    zs::object fct_state;

    if (auto err = compiler.compile(content, source_name, fct_state, this, nullptr, with_vargs)) {
      _errors.append(compiler.get_errors());
      return err;
    }

    closure_result = zs::object::create_closure(_engine, fct_state, this_table);

    if (with_vargs) {
      closure_result.as_closure()._default_params.push_back(zs::_a(_engine, 0));
    }
  }

  std::span<const object> params(&this_table, 1);
  if (auto err = call(closure_result, params, ret_value)) {
    return err;
  }

  return {};

  //  if(!closure_result.is_closure()) {
  //    return zs::error_code::invalid_type;
  //  }
  //
  //  zs::object this_table = zs::_t(_engine);
  //  this_table.as_table().set_delegate(_root_table);
  //
  //
  //    zs::object set_method = zs::_nf( [](zs::vm_ref vm) -> int_t {
  ////      vm.set_error("Cannot set value in enum\n");
  //
  //
  //
  ////      zs::string* path = ZS_GET_PATH();
  //      const object& this_table = vm[0];
  //      const object& key = vm[1];
  //      const object& value = vm[2];
  //
  //      if(auto err =       this_table.as_table().set(key, value)) {
  //
  //        return -1;
  //
  //      }
  //
  ////      vm.push(zs::object::create_none());
  //      return 0;
  //    });
  //
  //
  //
  //  this_table.as_table()[zs::constants::get<meta_method::mt_set>()] = set_method;
  //
  //  if (auto err = call(closure_result, {this_table} , ret_value)) {
  //    return err;
  //  }
  //
}

zs::error_result virtual_machine::call_file(const std::filesystem::path& filepath,
    std::string_view source_name, zs::object& ret_value, bool with_vargs) {

  zs::object closure_result;
  if (auto err = compile_file(filepath.c_str(), source_name, closure_result, with_vargs)) {
    return err;
  }

  if (!closure_result.is_closure()) {
    return zs::error_code::invalid_type;
  }

  //  zs::object this_table = zs::_t(_engine);
  //
  //  this_table.as_table().set_delegate(_root_table);

  if (auto err = call(closure_result, _global_table, ret_value)) {
    return err;
  }

  return {};
}

zs::error_result virtual_machine::add(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_add, target, lhs, rhs);
}

zs::error_result virtual_machine::sub(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_sub, target, lhs, rhs);
}

zs::error_result virtual_machine::mul(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_mul, target, lhs, rhs);
}

zs::error_result virtual_machine::div(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_div, target, lhs, rhs);
}

zs::error_result virtual_machine::exp(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_exp, target, lhs, rhs);
}

zs::error_result virtual_machine::mod(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_mod, target, lhs, rhs);
}

zs::error_result virtual_machine::bitwise_or(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_bitwise_or, target, lhs, rhs);
}

zs::error_result virtual_machine::bitwise_and(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_bitwise_and, target, lhs, rhs);
}

zs::error_result virtual_machine::bitwise_xor(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_bitwise_xor, target, lhs, rhs);
}

zs::error_result virtual_machine::lshift(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_lshift, target, lhs, rhs);
}

zs::error_result virtual_machine::rshift(object& target, const object& lhs, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_rshift, target, lhs, rhs);
}

zs::error_result virtual_machine::add_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_add_eq, target, target, rhs);
}

zs::error_result virtual_machine::sub_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_sub_eq, target, target, rhs);
}

zs::error_result virtual_machine::mul_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_mul_eq, target, target, rhs);
}

zs::error_result virtual_machine::div_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_div_eq, target, target, rhs);
}

zs::error_result virtual_machine::mod_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_mod_eq, target, target, rhs);
}

zs::error_result virtual_machine::exp_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_exp_eq, target, target, rhs);
}

zs::error_result virtual_machine::lshift_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_lshift_eq, target, target, rhs);
}

zs::error_result virtual_machine::rshift_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_rshift_eq, target, target, rhs);
}

zs::error_result virtual_machine::bitwise_or_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_bitwise_or_eq, target, target, rhs);
}

zs::error_result virtual_machine::bitwise_and_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_bitwise_and_eq, target, target, rhs);
}

zs::error_result virtual_machine::bitwise_xor_eq(object& target, const object& rhs) {
  return arithmetic_operation(arithmetic_op::aop_bitwise_xor_eq, target, target, rhs);
}

/////////////////////////////////////////////////
object virtual_machine::add(const object& lhs, const object& rhs) {
  object obj;
  if (add(obj, lhs, rhs)) {
    return nullptr;
  }
  return obj;
}

object virtual_machine::sub(const object& lhs, const object& rhs) {
  object obj;
  if (sub(obj, lhs, rhs)) {
    return obj;
  }
  return obj;
}

object virtual_machine::mul(const object& lhs, const object& rhs) {
  object obj;
  if (mul(obj, lhs, rhs)) {
    return nullptr;
  }
  return obj;
}

object virtual_machine::div(const object& lhs, const object& rhs) {
  object obj;
  if (div(obj, lhs, rhs)) {
    return nullptr;
  }
  return obj;
}

object virtual_machine::mod(const object& lhs, const object& rhs) {
  object obj;
  if (mod(obj, lhs, rhs)) {
    return nullptr;
  }
  return obj;
}

object virtual_machine::exp(const object& lhs, const object& rhs) {
  object obj;
  if (exp(obj, lhs, rhs)) {
    return nullptr;
  }
  return obj;
}

object virtual_machine::get_default_delegate_for_type(object_type t) const {

  switch (t) {
  case k_integer:
  case k_float:
    return get_default_number_delegate();

  case k_string_view:
  case k_small_string:
  case k_long_string:
    return get_default_string_delegate();

  case k_table:
    return get_default_table_delegate();

  case k_array:
    return get_default_array_delegate();

  case k_struct:
    return get_default_struct_delegate();

  case k_closure:
  case k_native_closure:
  case k_native_function:
    return get_default_function_delegate();

  default:
    return nullptr;
  }
}

zs::error_result virtual_machine::new_closure(uint32_t fct_idx, uint8_t bounded_target, object& dest) {

  // Current function call info.
  const call_info& cinfo = _call_stack.back();

  // Get the current closure.
  const zs::object& obj_current_closure = cinfo.closure;
  if (!obj_current_closure.is_closure()) {
    return ZS_VM_ERROR(zs::errc::invalid_type, "Current closure is not a closure (",
        zs::get_object_type_name(obj_current_closure.get_type()), ").");
  }

  zs::closure_object& current_closure = obj_current_closure.as_closure();

  // Get the current closure function prototype.
  const zs::object& current_closure_fct_proto = current_closure._function;
  if (!function_prototype_object::is_proto(current_closure_fct_proto)) {
    return ZS_VM_ERROR(zs::errc::invalid_type, "Could not retrieve function prototype from closure object.");
  }

  // We want to create a closure object with the function prototype at index
  // `inst.fct_idx` in the current closure function prototype.
  const zs::object& new_closure_fct_proto
      = function_prototype_object::as_proto(current_closure_fct_proto)._functions[fct_idx];

  // TODO: Use closure's root.

  // Create the new closure object.
  object new_closure_obj = zs::_c(_engine, new_closure_fct_proto, current_closure._root);
  //  object new_closure_obj = zs::_c(_engine, new_closure_fct_proto, _global_table);
  zs::closure_object& new_closure = new_closure_obj.as_closure();

  if (bounded_target != k_invalid_target) {
    new_closure._this = _stack[bounded_target];
  }

  // In the new closure function prototype, we might have some captures to fetch.
  const zs::vector<captured_variable>& captures
      = function_prototype_object::as_proto(new_closure_fct_proto)._captures;
  if (const size_t capture_sz = captures.size()) {

    // Ref to the vector or capture values in the new closure object.
    // We will push all the required capture values in here.
    zs::vector<zs::object>& new_closure_captured_values = new_closure._captured_values;

    for (size_t i = 0; i < capture_sz; i++) {
      const captured_variable& captured_var = captures[i];

      switch (captured_var.type) {
      case captured_variable::local: {
        // The capture value is on the stack. `captured_var.src` is the index of
        // the value on the stack. We push that value in the new closure capture
        // values vector.
        const int_t cap_idx = captured_var.src;

        if (cap_idx >= (int_t)_stack.stack_size()) {
          return ZS_VM_ERROR(errc::out_of_bounds, "op_new_closure could not find local capture");
        }

        //        object obj = _stack[cap_idx];
        //        if (captured_var.is_weak) {
        //          obj = obj.get_weak_ref();
        //        }

        object* ptr = _stack.stack_base_pointer() + cap_idx;
        object* found_cap = nullptr;

        for (object& p : _open_captures) {
          if (capture::as_capture(p).get_value_ptr() == ptr) {
            found_cap = &p;
            break;
            //            zb::print("dsdsdsdsds");
          }
        }

        if (found_cap) {
          new_closure_captured_values.emplace_back(*found_cap);
        }
        else {
          new_closure_captured_values.push_back(zs::capture::create(_engine, ptr));
          //        new_closure_capture_values.push_back(std::move(obj));
          _open_captures.push_back(new_closure_captured_values.back());
        }
        //        if (captured_var.name == "__exports__") {
        //          new_closure_obj.as_closure()._module = _stack[cap_idx];
        //
        //          //          new_closure_obj.as_closure()._module = new_closure_capture_values.back();
        //        }
        break;
      }

      case captured_variable::outer: {
        // When the capture type is outer, the capture value is in the
        // `capture_values` vector of the current closure object.
        const zs::vector<zs::object>& current_closure_captured_values = current_closure._captured_values;
        //
        const int_t cap_idx = captured_var.src;
        //
        if (cap_idx >= (int_t)current_closure_captured_values.size()) {
          return ZS_VM_ERROR(errc::out_of_bounds, "op_new_closure could not find parent capture\n");
        }

        ZS_ASSERT(capture::is_capture(current_closure_captured_values[cap_idx]));
        new_closure_captured_values.push_back(current_closure_captured_values[cap_idx]);

        //_open_captures.push_back(new_closure_capture_values.back());
        //        //        new_closure_capture_values.push_back(current_closure_capture_values[cap_idx]);
        //        object obj = current_closure_capture_values[cap_idx];
        //        if (captured_var.is_weak) {
        //          obj = obj.get_weak_ref();
        //        }
        //
        //        new_closure_capture_values.push_back(std::move(obj));
        //
        //        if (captured_var.name == "__exports__") {
        //          new_closure._module = *new_closure_captured_values.back().as_capture().get_value_ptr();
        //        }
        //        _error_message += zs::strprint(_engine, "unimplemented", std::source_location::current());
        //        return zs::errc::unimplemented;
        break;
      }
      }
    }
  }

  // In the new closure function prototype, we might have some default
  // parameters to fetch.
  const zs::vector<zs::int_t>& default_params
      = function_prototype_object::as_proto(new_closure_fct_proto)._default_params;
  if (const size_t default_params_sz = default_params.size()) {
    zs::vector<zs::object>& new_closure_default_param_values = new_closure_obj._closure->_default_params;

    const int_t stack_sz = _stack.stack_size();

    for (size_t i = 0; i < default_params_sz; i++) {
      int_t default_param_idx = default_params[i];

      if (default_param_idx >= stack_sz) {
        return ZS_VM_ERROR(errc::out_of_bounds, "op_new_closure could not find default param.");
      }

      new_closure_default_param_values.push_back(_stack[default_param_idx]);
    }
  }

  dest = std::move(new_closure_obj);
  return zs::error_code::success;
}

//
// MARK: - API
//

// zs::error_result zs_call(zs::virtual_machine* v, zs::int_t n_params, bool returns, bool pop_callable) {
//   zbase_assert(v->stack_size() >= n_params + 1, "invalid stack size");
//
//   // Get the closure.
//   zs::object& closure = v->stack_get(-n_params - 1);
//   zs::object ret_value;
//
//   if (auto err = v->call(closure, n_params, v->stack_size() - n_params, ret_value)) {
//     return err;
//   }
//
//   // Function call doesn't pop the params from the stack.
//   v->pop(pop_callable ? n_params + 1 : n_params);
//
//   if (returns) {
//     v->push(ret_value);
//   }
//
//   return {};
// }

std::ostream& operator<<(std::ostream& s, const call_info& ci) {
  s << "{\n";

  zb::stream_print(s, "    type: ", ci.closure.get_type(), "\n    value: ", ci.closure, "\n");

  if (ci.closure.is_closure()) {
    zb::stream_print(s, "    name: ", ci.closure._closure->get_function_prototype()->_name,
        "\n    source_name: ", ci.closure._closure->get_function_prototype()->_source_name, "\n");
  }
  return s << "    previous_stack_base: " << ci.previous_stack_base
           << "\n    previous_top_index: " << ci.previous_top_index << "\n  }\n";
}

} // namespace zs.

#undef REF
#undef CREF

#undef ZS_VIRTUAL_MACHINE_CPP
