#include "zvirtual_machine.h"
#include <zbase/container/enum_array.h>
#include <zbase/utility/print.h>
#include <zbase/utility/scoped.h>
#include <zbase/memory/ref_wrapper.h>
#include <zbase/strings/charconv.h>
#include <zbase/strings/stack_string.h>

#include "object/zfunction_prototype.h"

#include "jit/zjit_compiler.h"
#include "utility/json/zjson_lexer.h"
#include "utility/json/zjson_parser.h"

#include <zscript/std/zslib.h>
#include <zscript/std/zsys.h>
#include <zscript/std/zfs.h>
#include <zscript/std/zmath.h>
#include <zscript/std/zbase64.h>

#include "object/delegate/zglobal_table_delegate.h"
#include "object/delegate/znumber_delegate.h"
#include "object/delegate/zfunction_delegate.h"
#include "object/delegate/zarray_delegate.h"
#include "object/delegate/ztable_delegate.h"
#include "object/delegate/zstring_delegate.h"
#include "object/delegate/zstruct_delegate.h"

#include "utility/zvm_module.h"

#define ZS_VIRTUAL_MACHINE_CPP 1

namespace zs {

#define REF(...) zb::wref(__VA_ARGS__)
#define CREF(...) zb::wcref(__VA_ARGS__)

inline constexpr object k_enum_delegate_value_table_name = zs::_sv("__enum_delegate_value_table");
inline constexpr object k_enum_delegate_meta_get_name = zs::_sv("__enum_delegate_get");
inline constexpr object k_enum_delegate_meta_set_name = zs::_sv("__enum_delegate_set");
inline constexpr object k_enum_counter_name = zs::_sv("__enum_cntr");
inline constexpr object k_enum_array_name = zs::_sv("__enum_array");

static constexpr const uint32_t k_enum_table_value_mask
    = zs::create_type_mask(object_base::k_number_or_bool_mask, object_base::k_string_mask);

enum class runtime_code {
  invalid_set,
  delegate_set,
  weak_set,
  table_set,
  table_set_if_exists,
  array_set,
  struct_instance_set,
  user_data_set,
  atom_set,

  // Struct.
  struct_set,
  struct_new_slot,
  struct_new_default_constructor,
  struct_new_constructor,
  struct_new_method,
  struct_call_create,

  enter_function_call,
  leave_function_call,
  call_native_closure,
  call_native_function,
  call_closure,

  new_closure,
  rt_close_captures,

  handle_error,
  execute,
//  delegate_get_type_of
};

using enum runtime_code;
} // namespace zs.

#include "vm/zvm_common.h"
#include "vm/runtime/zvm_runtime.h"
#include "vm/zvm_op.h"

#include "fcts/zvm_get.h"
#include "fcts/zvm_raw_get.h"
#include "fcts/zvm_raw_contains.h"

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
  ++it;
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
ZS_VM_DECL_OP_NO_INST_PTR_INCR(op_if_null)
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

static inline object create_global_table(zs::engine* eng) {
  object global_table = object::create_table_with_delegate(eng, zs::create_global_table_delegate(eng), false);
  table_object& g = global_table.as_table();

  g.emplace(k_imported_modules_name, zs::_t(eng));
  g.emplace(k_module_loaders_name, zs::_t(eng));
  g.emplace(k_number_delegate_name, zs::create_number_default_delegate(eng));
  g.emplace(k_function_delegate_name, zs::create_function_default_delegate(eng));
  g.emplace(k_array_delegate_name, zs::create_array_default_delegate(eng));
  g.emplace(k_table_delegate_name, zs::create_table_default_delegate(eng));
  g.emplace(k_string_delegate_name, zs::create_string_default_delegate(eng));
  g.emplace(k_struct_delegate_name, zs::create_struct_default_delegate(eng));
  g.emplace(k_delegated_atom_delegates_table_name, zs::_t(eng));

  return global_table;
}

virtual_machine::virtual_machine(zs::engine* eng, size_t stack_size, bool owns_engine)
    : engine_holder(eng)
    , _global_table(create_global_table(eng))
    , _stack(eng, stack_size)
    , _call_stack({ { nullptr, 0, 0 } }, (zs::allocator<call_info>(eng, zs::memory_tag::nt_vm)))
    , _open_captures(zs::allocator<object>(eng))
    , _error_message(zs::allocator<char>(eng))
    , _errors(eng)
    , _owns_engine(owns_engine) {}

virtual_machine::virtual_machine(zs::virtual_machine* vm, size_t stack_size, bool same_global)
    : engine_holder(vm->_engine)
    , _global_table(same_global ? vm->_global_table : create_global_table(vm->_engine))
    , _stack(vm->_engine, stack_size)
    , _call_stack({ { nullptr, 0, 0 } }, (zs::allocator<call_info>(vm->_engine, zs::memory_tag::nt_vm)))
    , _open_captures(zs::allocator<object>(vm->_engine))
    , _error_message(zs::allocator<char>(vm->_engine))
    , _errors(vm->_engine)
    , _owns_engine(false) {}

zs::error_result virtual_machine::init() {
  table_object& g = _global_table.as_table();

  table_object& imported_modules_tbl = g[k_imported_modules_name].as_table();
  imported_modules_tbl.reserve(8);

  vm_ref vref(this);

  object zs_lib = zs::create_zs_lib(vref);
  object sys_lib = zs::create_sys_lib(vref);
  object fs_lib = zs::create_fs_lib(vref);
  object math_lib = zs::create_math_lib(vref);

  const object zs_lib_name = zs::_ss("zs");
  const object sys_lib_name = zs::_ss("sys");
  const object fs_lib_name = zs::_ss("fs");
  const object math_lib_name = zs::_ss("math");

  g.emplace(zs_lib_name, zs_lib);
  g.emplace(sys_lib_name, sys_lib);
  g.emplace(fs_lib_name, fs_lib);
  g.emplace(math_lib_name, math_lib);

  imported_modules_tbl.emplace(zs_lib_name, zs_lib);
  imported_modules_tbl.emplace(sys_lib_name, sys_lib);
  imported_modules_tbl.emplace(fs_lib_name, fs_lib);
  imported_modules_tbl.emplace(math_lib_name, math_lib);

  table_object& module_loaders = g[k_module_loaders_name].as_table();

  // Base64.
  module_loaders.emplace(
      _ss("base64"), [](zs::vm_ref vm) -> int_t { return vm.push(zs::create_base64_lib(vm)); });

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
  return globals()[k_table_delegate_name];
}

const object& virtual_machine::get_default_number_delegate() const {
  return globals()[k_number_delegate_name];
}

const object& virtual_machine::get_default_array_delegate() const { return globals()[k_array_delegate_name]; }

const object& virtual_machine::get_default_string_delegate() const {
  return globals()[k_string_delegate_name];
}

const object& virtual_machine::get_delegated_atom_delegates_table() const {
  return globals()[k_delegated_atom_delegates_table_name];
}

const object& virtual_machine::get_default_struct_delegate() const {
  return globals()[k_struct_delegate_name];
}

const object& virtual_machine::get_default_function_delegate() const {
  return globals()[k_function_delegate_name];
}

zs::string virtual_machine::get_error() const noexcept {
  zs::ostringstream stream(zs::create_string_stream(_engine));
  _errors.print(stream);
  return stream.str();
}

zs::error_result virtual_machine::handle_error(zs::error_code ec, const zs::line_info& linfo,
    std::string_view msg, const zs::developer_source_location& loc) {
  std::string_view filename = "";
  std::string_view line_content = "";
  _errors.emplace_back(_engine, error_source::virtual_machine, ec, msg, filename, line_content, linfo, loc);
  return ec;
}

object& virtual_machine::get_imported_modules() noexcept { return globals()[k_imported_modules_name]; }

object& virtual_machine::get_module_loaders() noexcept { return globals()[k_module_loaders_name]; }

void virtual_machine::remove(int_t n) { _stack.remove(n); }

void virtual_machine::swap(int_t n1, int_t n2) { _stack.swap(n1, n2); }

void virtual_machine::pop() { _stack.pop(); }

void virtual_machine::pop(int_t n) { _stack.pop(n); }

void virtual_machine::push_null() { _stack.push(object()); }

void virtual_machine::push_root() { _stack.push(_global_table); }

void virtual_machine::push(const object& obj) { _stack.push(obj); }
void virtual_machine::push(object&& obj) { _stack.push(std::move(obj)); }

 const object& virtual_machine::top() const noexcept { return _stack.top(); }
 
object virtual_machine::start_delegate_chain(const object& obj) {
  return obj.is_meta_type() ? obj :  get_delegate(obj);
}

object virtual_machine::get(const object& obj, const object& key) {
  object dest;

  if (auto err = get(obj, key, dest)) {
    return object::create_none();
  }

  return dest;
}

zs::error_result virtual_machine::get(const object& obj, const object& key, object& dest) {

  switch (obj.get_type()) {
  case k_integer:
    return proxy::get<k_integer>(this, obj, key, dest);

  case k_float:
    return proxy::get<k_float>(this, obj, key, dest);

  case k_small_string:
    return proxy::get<k_small_string>(this, obj, key, dest);

  case k_string_view:
    return proxy::get<k_string_view>(this, obj, key, dest);

  case k_long_string:
    return proxy::get<k_long_string>(this, obj, key, dest);

  case k_atom:
    return proxy::get<k_atom>(this, obj, key, dest);

  case k_table:
    return proxy::get<k_table>(this, obj, key, dest);

  case k_array:
    return proxy::get<k_array>(this, obj, key, dest);

  case k_struct:
    return proxy::get(this, obj, key, obj, dest);

  case k_struct_instance:
    return proxy::get<k_struct_instance>(this, obj, key, dest);

  case k_user_data:
    return proxy::get<k_user_data>(this, obj, key, dest);

  case k_weak_ref:
    return proxy::get<k_weak_ref>(this, obj, key, dest);

  case k_closure:
    return proxy::get<k_closure>(this, obj, key, dest);

  case k_native_closure:
    return proxy::get<k_native_closure>(this, obj, key, dest);

  case k_native_function:
    return proxy::get<k_native_function>(this, obj, key, dest);

  default:
    set_error("Can't get a value from '", zs::get_object_type_name(obj.get_type()), "'.\n");
    dest.reset();
    return zs::error_code::inaccessible;
  }
}

zs::error_result virtual_machine::raw_get(const object& obj, const object& key, object& dest) {

#define ZS_VM_RT_GET_FUNC_PTR(name) runtime_action<runtime_code::name>(CREF(obj), CREF(key), REF(dest))

  switch (obj.get_type()) {

  case k_atom:
    return proxy::raw_get<k_atom>(this, obj, key, dest);

  case k_long_string:
    return proxy::raw_get<k_long_string>(this, obj, key, dest);

  case k_small_string:
    return proxy::raw_get<k_small_string>(this, obj, key, dest);

  case k_string_view:
    return proxy::raw_get<k_string_view>(this, obj, key, dest);

  case k_table:
    return proxy::raw_get<k_table>(this, obj, key, dest);

  case k_array:
    return proxy::raw_get<k_array>(this, obj, key, dest);

  case k_struct:
    return proxy::raw_get<k_struct>(this, obj, key, dest);

  case k_struct_instance:
    return proxy::raw_get<k_struct_instance>(this, obj, key, dest);

  case k_user_data:
    return proxy::raw_get<k_user_data>(this, obj, key, dest);

  case k_weak_ref:
    return proxy::raw_get<k_weak_ref>(this, obj, key, dest);

  default:
    set_error("Can't get a value from '", zs::get_object_type_name(obj.get_type()), "'.\n");
    dest.reset();
    return zs::error_code::inaccessible;
  }
}

zs::error_result virtual_machine::raw_contains(const object& obj, const object& key, object& dest) {

  switch (obj.get_type()) {

  case k_atom:
    return errc::unimplemented;

  case k_long_string:
  case k_small_string:
  case k_string_view:

  {
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
    //    return ZS_VM_RT_GET_FUNC_PTR(invalid_get);
    set_error("Can't get a value from '", zs::get_object_type_name(obj.get_type()), "'.\n");
    dest.reset();
    return zs::error_code::inaccessible;
  }
}

zs::error_result virtual_machine::contains(const object& obj, const object& key, object& dest) {

  switch (obj.get_type()) {
  case k_small_string:
    //    return ZS_VM_RT_GET_FUNC_PTR(string_get);
    return proxy::get(this, obj, key, nullptr, dest);

  case k_string_view:
    //    return ZS_VM_RT_GET_FUNC_PTR(string_get);
    return proxy::get(this, obj, key, nullptr, dest);

  case k_atom:
    return zs::error_code::unimplemented;

  case k_long_string:
    return proxy::get(this, obj, key, nullptr, dest);

  case k_table:
    //    return ZS_VM_RT_GET_FUNC_PTR(table_get);
    return proxy::get(this, obj, key, obj, dest);

  case k_array:
    return proxy::get(this, obj, key, obj.as_array().get_delegate(), dest);

    //    return ZS_VM_RT_GET_FUNC_PTR(array_get);
  case k_struct:
    return proxy::get(this, obj, key, obj, dest);

    //    return ZS_VM_RT_GET_FUNC_PTR(struct_get);
  case k_struct_instance:
    //      return  this->delegate_table_get(obj , key, nullptr, dest);

    //    return ZS_VM_RT_GET_FUNC_PTR(struct_instance_get);
    {
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
    //    return ZS_VM_RT_GET_FUNC_PTR(user_data_get);

    return proxy::get(this, obj, key, obj.as_udata().get_delegate(), dest);

  case k_weak_ref:
    return this->get(obj.get_weak_ref_value(), key, dest);
    //    return ZS_VM_RT_GET_FUNC_PTR(weak_get);

  default:
    //    return ZS_VM_RT_GET_FUNC_PTR(invalid_get);
    set_error("Can't get a value from '", zs::get_object_type_name(obj.get_type()), "'.\n");
    dest.reset();
    return zs::error_code::inaccessible;
  }
}

zs::error_result virtual_machine::set(object& obj, const object& key, const object& value) {

#define ZS_VM_RT_SET_FUNC_PTR(name) runtime_action<runtime_code::name>(REF(obj), CREF(key), CREF(value))

  switch (obj.get_type()) {
  case k_atom:
    return ZS_VM_RT_SET_FUNC_PTR(atom_set);

  case k_table:
    return ZS_VM_RT_SET_FUNC_PTR(table_set);
  case k_array:
    return ZS_VM_RT_SET_FUNC_PTR(array_set);
  case k_struct:
    return ZS_VM_RT_SET_FUNC_PTR(struct_set);
  case k_struct_instance:
    return ZS_VM_RT_SET_FUNC_PTR(struct_instance_set);
  case k_user_data:
    return ZS_VM_RT_SET_FUNC_PTR(user_data_set);
  case k_weak_ref:
    return ZS_VM_RT_SET_FUNC_PTR(weak_set);
  default:
    return ZS_VM_RT_SET_FUNC_PTR(invalid_set);
  }
}

zs::error_result virtual_machine::set_if_exists(object& obj, const object& key, const object& value) {

#define ZS_VM_RT_SET_FUNC_PTR(name) runtime_action<runtime_code::name>(REF(obj), CREF(key), CREF(value))

  switch (obj.get_type()) {
  case k_atom:
    return ZS_VM_RT_SET_FUNC_PTR(atom_set);

  case k_table:
    return ZS_VM_RT_SET_FUNC_PTR(table_set_if_exists);
  case k_array:
    return ZS_VM_RT_SET_FUNC_PTR(array_set);
  case k_struct:
    return ZS_VM_RT_SET_FUNC_PTR(struct_set);
  case k_struct_instance:
    return ZS_VM_RT_SET_FUNC_PTR(struct_instance_set);
  case k_user_data:
    return ZS_VM_RT_SET_FUNC_PTR(user_data_set);
  case k_weak_ref:
    return ZS_VM_RT_SET_FUNC_PTR(weak_set);
  default:
    return ZS_VM_RT_SET_FUNC_PTR(invalid_set);
  }
}

zs::error_result virtual_machine::call(const object& closure, int_t n_params, int_t local_stack_base,
    object& ret_value, bool stack_base_relative) {

  const int_t stack_base = local_stack_base + (stack_base_relative ? _stack.get_stack_base() : 0);

  const object_type otype = closure.get_type();

  zbase_assert(
      zb::is_one_of(otype, k_closure, k_native_closure, k_native_function, k_table, k_struct, k_user_data),
      get_object_type_name(otype));

  switch (otype) {
  case k_closure:
    return call_closure(closure, n_params, stack_base, ret_value);

  case zs::object_type::k_native_closure:
    return runtime_action<runtime_code::call_native_closure>(
        CREF(closure), n_params, stack_base, REF(ret_value));

  case zs::object_type::k_native_function:
    return runtime_action<runtime_code::call_native_function>(
        CREF(closure), n_params, stack_base, REF(ret_value));

  case zs::object_type::k_user_data: {
    object fct;

    object key = constants::get<meta_method::mt_call>();

    if (auto err = proxy::get(this, closure, key, closure.as_udata().get_delegate(), fct, false)) {
      return err;
    }
    else if (fct.is_function()) {
      ZS_TODO("Add comments and make sure that this works");

      object sb = std::exchange(_stack.get_at(stack_base), closure);
      if (auto err = call(fct, n_params, stack_base, ret_value, false)) {
        _stack.get_at(stack_base) = sb;
        return err;
      }
      _stack.get_at(stack_base) = sb;

      return {};
    }
    return zs::error_code::inaccessible;
  }

  case zs::object_type::k_struct: {
    return runtime_action<runtime_code::struct_call_create>(
        CREF(closure), n_params, stack_base, REF(ret_value));
  }

  case zs::object_type::k_table: {
    object fct;

    object key = constants::get<meta_method::mt_call>();

    if (auto err = proxy::get(this, closure, key, closure.as_table().get_delegate(), fct, false)) {
      return err;
    }

    if (fct.is_function()) {
      ZS_TODO("Add comments and make sure that this works");

      object sb = std::exchange(_stack.get_at(stack_base), closure);
      if (auto err = call(fct, n_params, stack_base, ret_value, false)) {
        _stack.get_at(stack_base) = sb;
        return err;
      }
      _stack.get_at(stack_base) = sb;

      return {};
    }
    return zs::error_code::inaccessible;
  }
  default:
    return zs::error_code::invalid;
  }

  return {};
}

zs::error_result virtual_machine::call_closure(
    const object& cobj, int_t n_params, int_t stack_base, object& ret_value, bool is_pcall) {

  zs::closure_object* closure = cobj._closure;
  zs::function_prototype_object* fpo = closure->get_function_prototype();

  zs::object_stack& stack = _stack;

  // The call was made with `n_params` parameters.
  // The function expects `n_expected_params` parameters.
  const int_t n_expected_params = fpo->_parameter_names.size();

  // The closure has `n_default_params` default parameters values.
  const int_t n_default_params = closure->_default_params.size();

  // A span of parameters that will be pushed after the `n_params` given params
  // if needed.
  std::span<object> default_params;

  // Some parameters are missing?
  if (n_default_params && n_params < n_expected_params) {

    // We check if some of those have a default parameter value.
    const int_t diff = n_expected_params - n_params;

    // If `diff` is bigger than `n_default_params`, we're still gonna be short
    // on parameters. Let's leave this condition like this for now, we'll check
    // it again below. Maybe we'll add some other kind of default parameters
    // (e.g. named parameters) in the future.
    if (diff <= n_default_params) {
      const int_t start_index = n_default_params - diff;
      default_params = std::span<object>(&closure->_default_params[start_index], diff);
    }
  }

  // We have `n_extra_default` that we can add to `n_params`.
  const int_t n_extra_default = default_params.size();

  object vargs;
  if (fpo->_has_vargs_params and n_params >= n_expected_params) {
    int_t nvargs = n_params - (n_expected_params - 1);
    vargs = zs::_a(_engine, nvargs);

    for (int_t i = 0; i < nvargs; i++) {
      vargs.as_array()[i] = _stack.get_at(stack_base + n_expected_params - 1 + i);
    }

    default_params = std::span<object>();
    n_params = n_expected_params - 1;
  }
  else if (n_expected_params != n_params + n_extra_default) {
    zb::print("WRONG NUMBER OF PARAMETERS");
    return zs::error_code::invalid_parameter_count;
  }

  // This is a modified version of `enter_function_call`.
  //  zb::execution_stack_state stack_state = stack.get_state();
  size_t call_stack_index = _call_stack.size();

  ///////////////////////////////
  ZS_RETURN_IF_ERROR(runtime_action<runtime_code::enter_function_call>(CREF(cobj), n_params, stack_base));
  if (vargs.is_array()) {
    _stack.push(vargs);
    n_params = n_expected_params;
  }
  else {
    // This section differs from `enter_function_call`.

    // We push the extra default params.
    for (const auto& param : default_params) {
      _stack.push(param);
    }

    n_params += n_extra_default;
  }
  ///////////////////////

  //  {
  //    //    zb::execution_stack_state stack_state = stack.get_state();
  //    //    _call_stack.emplace_back(cobj, stack_state);
  //    _call_stack.emplace_back(cobj, stack_state);
  //
  //    // The current top of the stack will be the next stack base.
  //    const size_t next_base = stack.get_absolute_top();
  //
  //    // Push `n_params` elements starting at `stack_base` on top of the stack.
  //    stack.absolute_repush_n(stack_base, n_params);
  //
  //    if (vargs.is_array()) {
  //      stack.push(vargs);
  //      n_params = n_expected_params;
  //    }
  //    else {
  //      // This section differs from `enter_function_call`.
  //
  //      // We push the extra default params.
  //      for (const auto& param : default_params) {
  //        stack.push(param);
  //      }
  //
  //      n_params += n_extra_default;
  //    }
  //
  //    // Set the new stack base.
  //    stack.set_stack_base(next_base);
  //
  //    zbase_assert(next_base == (stack.get_absolute_top() - n_params), "invalid stack parameters");
  //    zbase_assert((int_t)stack.stack_size() == n_params, "invalid stack parameters");
  //  }

  // We make room for the function stack by pushing some empty objects.
  stack.push_n(fpo->_stack_size - n_params);

  if (!closure->_this.is_null()) {
    stack[0] = closure->_this;
  }

  //  auto err = runtime_action<runtime_code::execute>(closure, zb::wref(ret_value));
  //  runtime_action<runtime_code::leave_function_call>();
  //  return err;
  if (auto err = runtime_action<runtime_code::execute>(closure, zb::wref(ret_value))) {

    if (is_pcall) {
      zbase_assert(!_call_stack.empty());

      call_info cinfo = _call_stack[call_stack_index];
      _stack.set_stack_base(cinfo.previous_stack_base);
      _stack.pop_to(cinfo.previous_top_index);
      _call_stack.resize(call_stack_index);
      //      // Just in case we somehow went below. Is this possible?
      //      if (stack().get_absolute_top() < (size_t)cinfo.previous_top_index) {
      //        size_t diff = cinfo.previous_top_index - stack().get_absolute_top();
      //        stack().push_n(diff);
      //      }

      zbase_assert(cinfo.previous_top_index == (int_t)stack.get_absolute_top());
    }
    return err;
  }

  return runtime_action<runtime_code::leave_function_call>();
}

zs::error_result virtual_machine::pcall(const object& closure, int_t n_params, int_t local_stack_base,
    object& ret_value, bool stack_base_relative) {

  const int_t stack_base = local_stack_base + (stack_base_relative ? _stack.get_stack_base() : 0);

  const object_type otype = closure.get_type();

  zbase_assert(
      zb::is_one_of(otype, k_closure, k_native_closure, k_native_function, k_table, k_struct, k_user_data),
      get_object_type_name(otype));

  switch (otype) {
  case k_closure:
    return runtime_action<runtime_code::call_closure>(CREF(closure), n_params, stack_base, REF(ret_value));

  case zs::object_type::k_native_closure:
    return runtime_action<runtime_code::call_native_closure>(
        CREF(closure), n_params, stack_base, REF(ret_value));

  case zs::object_type::k_native_function:
    return runtime_action<runtime_code::call_native_function>(
        CREF(closure), n_params, stack_base, REF(ret_value));

  case zs::object_type::k_user_data: {
    object fct;

    object key = constants::get<meta_method::mt_call>();
    //    if (auto err = runtime_action<runtime_code::user_data_get>(CREF(closure), CREF(key), REF(fct))) {
    //      return err;
    //    }

    if (auto err = proxy::get(this, closure, key, closure.as_udata().get_delegate(), fct, false)) {
      return err;
    }
    else if (fct.is_function()) {
      ZS_TODO("Add comments and make sure that this works");

      object sb = std::exchange(_stack.get_at(stack_base), closure);
      if (auto err = call(fct, n_params, stack_base, ret_value, false)) {
        _stack.get_at(stack_base) = sb;
        return err;
      }
      _stack.get_at(stack_base) = sb;

      return {};
    }
    return zs::error_code::inaccessible;
  }

  case zs::object_type::k_struct: {
    return runtime_action<runtime_code::struct_call_create>(
        CREF(closure), n_params, stack_base, REF(ret_value));
  }

  case zs::object_type::k_table: {
    object fct;

    object key = constants::get<meta_method::mt_call>();

    //    if (auto err = runtime_action<runtime_code::table_get>(CREF(closure), CREF(key), REF(fct))) {
    //      return err;
    //    }

    if (auto err = proxy::get(this, closure, key, closure.as_udata().get_delegate(), fct, false)) {
      return err;
    }

    if (fct.is_function()) {
      ZS_TODO("Add comments and make sure that this works");

      object sb = std::exchange(_stack.get_at(stack_base), closure);
      if (auto err = call(fct, n_params, stack_base, ret_value, false)) {
        _stack.get_at(stack_base) = sb;
        return err;
      }
      _stack.get_at(stack_base) = sb;

      return {};
    }
    return zs::error_code::inaccessible;
  }
  default:
    return zs::error_code::invalid;
  }

  return {};
}

zs::error_result virtual_machine::call(const object& closure, zs::parameter_list params, object& ret_value) {

  const object_type otype = closure.get_type();

  ZS_ASSERT(zb::is_one_of(otype, k_closure, k_native_closure, k_native_function, k_table, k_user_data),
      get_object_type_name(otype));

  switch (otype) {
  case k_closure:
    return runtime_action<runtime_code::call_closure>(CREF(closure), params, REF(ret_value));

  case zs::object_type::k_native_closure:
    return runtime_action<runtime_code::call_native_closure>(CREF(closure), params, REF(ret_value));

  case zs::object_type::k_native_function:
    return runtime_action<runtime_code::call_native_function>(CREF(closure), params, REF(ret_value));

  case zs::object_type::k_user_data:
    return zs::error_code::unimplemented;

  case zs::object_type::k_table: {
    object fct;
    object key = constants::get<meta_method::mt_call>();

    if (auto err = proxy::get(this, closure, key, closure, fct, false)) {
      return err;
    }

    if (fct.is_function()) {
      ZS_TODO("Add comments and make sure that this works");

      //        object sb = std::exchange(_stack.get_at(stack_base), closure);
      if (auto err = call(fct, params, ret_value)) {
        return err;
      }
      //        _stack.get_at(stack_base) = sb;

      return {};
    }
    return zs::error_code::inaccessible;
  }
  default:
    return zs::error_code::invalid;
  }

  return zs::error_code::invalid;
}

zs::optional_result<zs::object> virtual_machine::call_from_top_opt(const object& closure, int_t n_params) {

  if (!n_params) {
    push_root();
    n_params = 1;
  }

  object ret_value;
  if (auto err = call_from_top(closure, n_params, ret_value)) {
    return err.code;
  }

  return ret_value;
}

zs::error_result virtual_machine::call_from_top(const object& closure, int_t n_params) {

  if (!n_params) {
    push_root();
    n_params = 1;
  }

  object ret_value;
  if (auto err = call_from_top(closure, n_params, ret_value)) {
    return err.code;
  }

  return {};
}

ZBASE_PRAGMA_PUSH()
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wswitch")
ZBASE_PRAGMA_DISABLE_WARNING_CLANG("-Wlanguage-extension-token")

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
           << std::setw(sizeof(int_t) * 2) << std::hex << (int_t)obj._value;
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
      dest =  obj.as_table().clone();
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


zs::object virtual_machine::get_delegate(const object& obj ) {
 
 
    if (obj.is_atom()) {
      zs::object delegate;
      if(auto err = get_delegated_atom_delegates_table().as_table().get((int_t)obj._ex2_delegated_atom_delegate_id, delegate)) {
        return nullptr;
      }
      
     return delegate;
    }

  return obj.is_delegable() ? obj.as_delegable().get_delegate() : object::create_null();
 
}


zs::error_result virtual_machine::type_of(const object& obj, object& dest) {
  ZS_ASSERT(&obj != &dest);

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
//
// error_result virtual_machine::binary_delegate_operation(
//    meta_method mt, const object& lhs, const object& rhs, object delegate, object& dest) {
//
//  const object lhs_mt_name = zs::get_meta_method_name_object(mt);
//
//  const zs::object* meta_func = nullptr;
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
//      dest = nullptr;
//      return ZS_VM_ERROR(errc::invalid_operation, "Invalid operator in user data.");
//    }
//
//    // We're getting ready to call the function with 3 parameters:
//    // 0 - The left hand side object.
//    // 1 - The right hand side value.
//    // 2 - The delegate table containing the meta method.
//    const int_t top_idx = stack_size();
//
//    push(lhs);
//    push(rhs);
//
//    // This prevents the user from having to always declare the 3rd delegate parameter.
//    // If the meta method is a closure and has only 2 parameters, we'll call
//    // it with 2 instead.
//    if (!(meta_func->is_closure() and meta_func->as_closure().get_parameters_count() == 2)) {
//      push(delegate);
//    }
//
//    int_t n_params = stack_size() - top_idx;
//
//    // We now have to call the function and remove the parameters from the stack.
//    auto err = call(*meta_func, n_params, top_idx, dest);
//    pop(n_params);
//
//    if (err) {
//      return ZS_VM_ERROR(err, "Invalid operator call in user data.");
//    }
//
//    return err;
//  }
//
//  // If we got here, it means that the left hand side user data doesn't have
//  // the meta method in any of it's delegate.
//  //
//  // The plan is now to look in the right hand side parameter for a rhs meta method.
//
//  delegate = start_delegate_chain(rhs);
//
//  meta_method rhs_mt_method = zs::get_rhs_meta_method(mt);
//
//  if (rhs_mt_method == meta_method::mt_none) {
//    dest = nullptr;
//    return ZS_VM_ERROR(errc::invalid_operation, "Invalid operator in user data.");
//  }
//
//  const object rhs_mt_name = zs::get_meta_method_name_object(rhs_mt_method);
//
//  while (delegate.is_meta_type()) {
//    // We look directly in the delegate table for the meta function.
//    // We never call '__get' when looking for a meta method.
//    meta_func = nullptr;
//
//    if (delegate.is_table()) {
//      meta_func = delegate.as_table().get(rhs_mt_name);
//    }
//    else {
//      meta_func = delegate.as_struct_instance().get_meta_method(rhs_mt_name);
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
//      dest = nullptr;
//      return ZS_VM_ERROR(errc::invalid_operation, "Invalid operator in user data.");
//    }
//
//    // We're getting ready to call the function with 3 parameters:
//    // 0 - The right hand side value.
//    // 1 - The user data.
//    // 2 - The delegate table containing the meta method.
//    const int_t top_idx = stack_size();
//
//    push(rhs);
//    push(lhs);
//
//    // This prevents the user from having to always declare the 3rd delegate parameter.
//    // If the meta method is a closure and has only 2 parameters, we'll call
//    // it with 2 instead.
//    if (!(meta_func->is_closure() and meta_func->as_closure().get_parameters_count() == 2)) {
//      push(delegate);
//    }
//
//    int_t n_params = stack_size() - top_idx;
//
//    // We now have to call the function and remove the parameters from the stack.
//    auto err = call(*meta_func, n_params, top_idx, dest);
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
//  return ZS_VM_ERROR(errc::invalid_operation, "Invalid operator in user data.");
//}

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
    _error_message = compiler.get_error();
    _errors.insert(_errors.end(), compiler.get_errors().begin(), compiler.get_errors().end());
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
    _error_message = compiler.get_error();
    _errors.insert(_errors.end(), compiler.get_errors().begin(), compiler.get_errors().end());
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
      _error_message = compiler.get_error();
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

zs::error_result virtual_machine::load_buffer_as_array(
    std::string_view content, std::string_view source_name, zs::object& value, std::string_view sep) {
  return zs::error_code::unimplemented;
}

zs::error_result virtual_machine::load_buffer_as_value(
    std::string_view content, std::string_view source_name, zs::object& value) {
  zs::jit_compiler compiler(_engine);
  zs::object fct_state;

  zs::token_type prepended_token = token_type::tok_return;
  if (auto err = compiler.compile(content, source_name, fct_state, this, &prepended_token)) {
    _error_message = compiler.get_error();
    return err;
  }

  zs::object closure_result = zs::object::create_closure(_engine, fct_state, _global_table);

  if (!closure_result.is_closure()) {
    return zs::error_code::invalid;
  }

  zs::int_t n_params = 1;
  push_root();

  // Top index = 3
  // N param = 1

  if (auto err = call(closure_result, n_params, stack_size() - n_params, value)) {
    return err;
  }

  return {};
}

zs::error_result virtual_machine::load_file_as_value(
    const char* filepath, std::string_view source_name, zs::object& value) {

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return load_buffer_as_value(loader.content(), source_name, value);
}

zs::error_result virtual_machine::load_file_as_array(
    const char* filepath, std::string_view source_name, zs::object& value, std::string_view sep) {

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return load_buffer_as_array(loader.content(), source_name, value, sep);
}

zs::error_result virtual_machine::load_json_table(
    std::string_view content, const object& table, object& output) {
  zs::json_parser parser(_engine);

  if (auto err = parser.parse(this, content, table, output)) {
    return err;
  }
  return {};
}

zs::error_result virtual_machine::load_json_file(const char* filepath, const object& table, object& output) {

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return load_json_table(loader.content(), table, output);
}

zs::error_result virtual_machine::load_json_file(
    std::string_view filepath, const object& table, object& output) {

  zs::file_loader loader(_engine);

  if (auto err = loader.open(filepath)) {
    return err;
  }

  return load_json_table(loader.content(), table, output);
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
  object j = target;
  zs::print(j.get_type());
  return arithmetic_operation(arithmetic_op::aop_add_eq, target, j, rhs);
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
 
//
// MARK: Execute
//

template <>
zs::error_result virtual_machine::runtime_action<runtime_code::execute>(
    closure_object* closure, zb::ref_wrapper<object> ret_value) {

  zs::function_prototype_object* fpo = closure->get_function_prototype();
  zs::instruction_vector& instructions = fpo->_instructions;
  exec_op_data_t op_data{ closure, fpo, ret_value };
  zs::error_result err;
  const instruction_iterator end_it = instructions.end();

  for (zs::instruction_iterator it = instructions.begin(); it != end_it;) {
    // Keeping the last instruction iterator in case of an error.
    const zs::instruction_iterator inst_it = it;

    if ((err = executor::call_op(this, *it, it, op_data))) {
      return runtime_action<runtime_code::handle_error>(fpo, inst_it, err.code);
    }
    else if (err == zs::error_code::returned) {
      // The `op_return` returns zs::error_code::returned on success.
      // We are done with this function.
      return {};
    }
  }

  return err;
}

static inline object get_enum_delegate_meta_set(virtual_machine* vm) {
  zs::engine* eng = vm->get_engine();
  zs::table_object& registry_table = eng->get_registry_table().as_table();

  // Look for method in `registry_table`.
  if (object* set_method = registry_table.get(k_enum_delegate_meta_set_name)) {
    return *set_method;
  }

  zs::object set_method = zs::object::create_native_closure(eng, [](zs::vm_ref vm) -> int_t {
    vm.set_error("Cannot set value in enum\n");
    return -1;
  });

  // Add method to `registry_table`.
  return registry_table.emplace(k_enum_delegate_meta_set_name, std::move(set_method)).first->second;
}

static inline object get_enum_delegate_meta_get(virtual_machine* vm) {
  zs::engine* eng = vm->get_engine();
  zs::table_object& registry_table = eng->get_registry_table().as_table();

  // Look for method in `registry_table`.
  if (object* get_method = registry_table.get(k_enum_delegate_meta_get_name)) {
    return *get_method;
  }

  zs::object get_method = zs::object::create_native_closure(eng, [](zs::vm_ref vm) -> int_t {
    // The `obj` should be an empty table with a delegate.
    const object& key = vm[1];

    // The delegate, contains the `__table` field.
    const object& delegate_obj = vm[2];

    if (!delegate_obj.is_table()) {
      vm.set_error("Wrong enum type\n");
      return -1;
    }

    // Try to get the `__table` field from the obj table.
    zs::object* value_table_obj = delegate_obj.as_table().get(k_enum_delegate_value_table_name);

    if (!value_table_obj) {
      vm.set_error("Cannot get value table in enum\n");
      return -1;
    }

    object dst;
    if (auto err = value_table_obj->as_table().get(key, dst)) {
      vm.set_error("Cannot get value in enum\n");
      return -1;
    }

    vm.push(dst);
    return 1;
  });

  // Add method to `registry_table`.
  return registry_table.emplace(k_enum_delegate_meta_get_name, std::move(get_method)).first->second;
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

zs::error_result virtual_machine::make_enum_table(object& obj) {
  if (!obj.is_table()) {
    set_error("Enum is not a table\n");
    return zs::error_code::invalid_type;
  }

  // Create a delegate.
  zs::object delegate_obj = zs::object::create_table(_engine);
  zs::table_object& dtable = delegate_obj.as_table();
  dtable.reserve(8);

  // Add the enum table to the delegate in the `__table` field.
  //  delegate_obj.as_table()[zs::_sv(k_enum_delegate_value_table_name)] = obj;
  dtable[k_enum_delegate_value_table_name] = obj;

  // Set the `__operator_set` meta method to the delegate.
  dtable[zs::constants::get<meta_method::mt_set>()] = get_enum_delegate_meta_set(this);

  // Set the `__operator_get` meta method to the delegate.
  dtable[zs::constants::get<meta_method::mt_get>()] = get_enum_delegate_meta_get(this);

  dtable[k_enum_array_name] = obj.as_table()[k_enum_array_name];

  dtable["name_list"] = zs::object::create_native_closure(_engine, [](vm_ref vm) -> int_t {
    vm.push(vm[0].as_table().get_delegate().as_table()[k_enum_array_name]);
    return 1;
  });

  obj.as_table().erase(k_enum_counter_name);
  obj.as_table().erase(k_enum_array_name);

  // Replace the obj with the new enum table.
  obj = zs::object::create_table_with_delegate(_engine, std::move(delegate_obj));
  obj._flags = object_flags_t::f_enum_table;
  return {};
}

//
// MARK: - API
//

zs::error_result zs_call(zs::virtual_machine* v, zs::int_t n_params, bool returns, bool pop_callable) {
  zbase_assert(v->stack_size() >= n_params + 1, "invalid stack size");

  // Get the closure.
  zs::object& closure = v->stack_get(-n_params - 1);
  zs::object ret_value;

  if (auto err = v->call(closure, n_params, v->stack_size() - n_params, ret_value)) {
    return err;
  }

  // Function call doesn't pop the params from the stack.
  v->pop(pop_callable ? n_params + 1 : n_params);

  if (returns) {
    v->push(ret_value);
  }

  return {};
}

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
