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
} // namespace zs.

#include "vm/zvm_runtime.h"
#include "vm/zvm_arithmetic.h"
#include "vm/zvm_op.h"

namespace zs {

zs::instruction_vector& virtual_machine::exec_op_data_t::instructions() const noexcept {
  return fct->_instructions;
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

    //    zb::print("CALL_OP", *it);
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
  virtual_machine* v = internal::zs_new<memory_tag::nt_vm, virtual_machine>(eng, eng, stack_size,
      true // owns_engine.
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
  virtual_machine* v = internal::zs_new<virtual_machine>(eng, eng, stack_size,
      // owns_engine.
      false);

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
    , _stack(eng, stack_size)
    , _call_stack((zs::allocator<call_info>(eng, zs::memory_tag::nt_vm)))
    , _error_message(zs::allocator<char>(eng))
    , _constexpr_variables(zs::allocator<object>(eng))
    , _owns_engine(owns_engine) {

  _default_array_delegate = zs::create_array_default_delegate(_engine);
  _default_native_array_delegate = zs::create_native_array_default_delegate(_engine);
  _default_table_delegate = zs::create_table_default_delegate(_engine);
  _default_string_delegate = zs::create_string_default_delegate(_engine);
  _default_struct_delegate = zs::create_struct_default_delegate(_engine);
  _default_color_delegate = zs::create_color_default_delegate(_engine);
  _default_array_iterator_delegate = create_array_iterator_default_delegate(_engine);
  _default_table_iterator_delegate = create_table_iterator_default_delegate(_engine);
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

  _root_table.reset();
  _imported_module_cache.reset();

  _stack.set_stack_base(0);
  _stack.pop_to(0);
  _constexpr_variables.clear();

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

zs::error_result virtual_machine::init() {
  _call_stack.push_back(call_info{ nullptr, 0, 0 });
  _root_table = object::create_table(_engine);
  _imported_module_cache = zs::_t(_engine);

  table_object& tbl = _imported_module_cache.as_table();
  tbl.reserve(8);

  if (auto err = zs::include_lang_lib(this)) {
    return err;
  }

  tbl.set(zs::_ss("zs"), zs::create_zs_lib(this));
  tbl.set(zs::_ss("base64"), zs::create_base64_lib(this));
  tbl.set(zs::_ss("graphics"), zs::create_graphics_lib(this));
  tbl.set(zs::_ss("fs"), zs::create_fs_lib(this));
  tbl.set(zs::_ss("math"), zs::create_math_lib(this));

  return {};
}

object& virtual_machine::get_imported_module_cache() noexcept { return _imported_module_cache; }

void virtual_machine::remove(int_t n) { _stack.remove(n); }

void virtual_machine::swap(int_t n1, int_t n2) { _stack.swap(n1, n2); }

void virtual_machine::pop() { _stack.pop(); }

void virtual_machine::pop(int_t n) { _stack.pop(n); }

void virtual_machine::push_null() { _stack.push(object()); }

void virtual_machine::push_root() { _stack.push(_root_table); }

void virtual_machine::push(const object& obj) { _stack.push(obj); }
void virtual_machine::push(object&& obj) { _stack.push(std::move(obj)); }

object& virtual_machine::top() noexcept { return _stack.top(); }
const object& virtual_machine::top() const noexcept { return _stack.top(); }
object virtual_machine::pop_get() { return _stack.pop_get(); }

object& virtual_machine::get_up(int_t n) { return _stack.get_up(n); }
object& virtual_machine::get_at(int_t n) { return _stack.get_at(n); }

zs::error_result virtual_machine::type_of(const object& obj, object& dest) {
  using enum object_type;

  switch (obj._type) {
  case k_table: {
    // Table.
    table_object* tbl = obj._table;

    object meta_typeof;

    // Look for typeof operator in table.
    if (!tbl->get(zs::constants::get<meta_method::mt_typeof>(), meta_typeof)) {

      // The meta typeof was found in the table.

      // If it is a function, let's call it.
      if (meta_typeof.is_function()) {

        push(obj);
        if (auto err = call(meta_typeof, 1, stack_size() - 1, dest)) {
          pop();
          return err;
        }

        pop();

        if (!dest.is_string()) {
          zb::print("Invalid typeof operator return type (should be a string)");
          return zs::error_code::invalid_type;
        }

        return {};
      }

      // If it's a string, we return that string.
      else if (meta_typeof.is_string()) {
        dest = meta_typeof;
        return {};
      }

      else {
        zb::print("Invalid typeof operator");
        return zs::error_code::invalid_type;
      }
    }

    if (tbl->has_delegate()) {
      object& delegate_obj = tbl->get_delegate();
      zs::error_result err
          = runtime_action<runtime_code::delegate_get_type_of>(CREF(obj), CREF(delegate_obj), REF(dest));

      if (!err) {
        return {};
      }

      if (err == zs::error_code::not_found) {
        dest = object::create_small_string(zs::get_exposed_object_type_name(obj.get_type()));
        return {};
      }

      return err;
    }

    dest = zs::_ss(zs::get_exposed_object_type_name(obj.get_type()));
    return {};
  }

  case k_array: {

    array_object* tbl = obj._array;
    if (tbl->has_delegate()) {
      object& delegate_obj = tbl->get_delegate();

      if (!runtime_action<runtime_code::delegate_get_type_of>(CREF(obj), CREF(delegate_obj), REF(dest))) {
        return {};
      }
    }

    dest = object::create_small_string(zs::get_exposed_object_type_name(obj.get_type()));

    return {};
  }

  case k_user_data: {
    user_data_object* tbl = obj._udata;
    if (tbl->has_delegate()) {
      object& delegate_obj = tbl->get_delegate();
      if (!runtime_action<runtime_code::delegate_get_type_of>(CREF(obj), CREF(delegate_obj), REF(dest))) {
        return {};
      }
    }

    dest = object::create_small_string(zs::get_exposed_object_type_name(obj.get_type()));

    return {};
  }
  case k_class:
    return zs::error_code::unimplemented;

  case k_instance:
    return zs::error_code::unimplemented;

  case k_weak_ref:
    return zs::error_code::unimplemented;

  default:
    dest = object::create_small_string(zs::get_exposed_object_type_name(obj.get_type()));

    return {};
  }

  dest = object::create_small_string(zs::get_exposed_object_type_name(obj.get_type()));

  return {};
}

object virtual_machine::get(const object& obj, const object& key) {
  object dest;

  if (auto err = get(obj, key, dest)) {
    return object::create_none();
  }

  return dest;
}

zs::error_result virtual_machine::get(const object& obj, const object& key, object& dest) {
  using enum object_type;
  using enum runtime_code;

#define ZS_VM_RT_GET_FUNC_PTR(name) runtime_action<runtime_code::name>(CREF(obj), CREF(key), REF(dest))

  switch (obj.get_type()) {
  case k_small_string:
    return ZS_VM_RT_GET_FUNC_PTR(string_get);
  case k_string_view:
    return ZS_VM_RT_GET_FUNC_PTR(string_get);
  case k_extension:
    return ZS_VM_RT_GET_FUNC_PTR(extension_get);
  case k_long_string:
    return ZS_VM_RT_GET_FUNC_PTR(string_get);
  case k_mutable_string:
    return ZS_VM_RT_GET_FUNC_PTR(string_get);
  case k_table:
    return ZS_VM_RT_GET_FUNC_PTR(table_get);
  case k_array:
    return ZS_VM_RT_GET_FUNC_PTR(array_get);
  case k_struct:
    return ZS_VM_RT_GET_FUNC_PTR(struct_get);
  case k_struct_instance:
    return ZS_VM_RT_GET_FUNC_PTR(struct_instance_get);
  case k_native_array:
    return ZS_VM_RT_GET_FUNC_PTR(native_array_get);
  case k_user_data:
    return ZS_VM_RT_GET_FUNC_PTR(user_data_get);
  case k_instance:
    return ZS_VM_RT_GET_FUNC_PTR(instance_get);
  case k_weak_ref:
    return ZS_VM_RT_GET_FUNC_PTR(weak_get);

  default:
    return ZS_VM_RT_GET_FUNC_PTR(invalid_get);
  }
}

zs::error_result virtual_machine::set(object& obj, const object& key, const object& value) {
  using enum object_type;
#define ZS_VM_RT_SET_FUNC_PTR(name) runtime_action<runtime_code::name>(REF(obj), CREF(key), CREF(value))

  switch (obj.get_type()) {
  case k_extension:
    return ZS_VM_RT_SET_FUNC_PTR(extension_set);
  case k_mutable_string:
    return ZS_VM_RT_SET_FUNC_PTR(string_set);
  case k_table:
    return ZS_VM_RT_SET_FUNC_PTR(table_set);
  case k_array:
    return ZS_VM_RT_SET_FUNC_PTR(array_set);
  case k_native_array:
    return ZS_VM_RT_SET_FUNC_PTR(native_array_set);
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
  using enum object_type;

  const int_t stack_base = local_stack_base + (stack_base_relative ? _stack.get_stack_base() : 0);

  const object_type otype = closure.get_type();

  zbase_assert(zb::is_one_of(otype, k_closure, k_native_closure, k_native_function, k_table, k_class,
                   k_instance, k_struct, k_user_data, k_native_function2),
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

  case zs::object_type::k_native_function2:
    return runtime_action<runtime_code::call_native_function2>(
        CREF(closure), n_params, stack_base, REF(ret_value));

  case zs::object_type::k_user_data: {
    object fct;

    object key = constants::get<meta_method::mt_call>();
    if (auto err = runtime_action<runtime_code::user_data_get>(CREF(closure), CREF(key), REF(fct))) {
      return err;
    }

    if (fct.is_function()) {
      ZS_TODO("Add comments and make sure that this works");

      object sb = std::exchange(_stack.get_at(stack_base), closure);
      if (auto err = call(fct, n_params, stack_base, ret_value, false)) {
        return err;
      }
      _stack.get_at(stack_base) = sb;

      return {};
    }
    return zs::error_code::inaccessible;
  }

  case zs::object_type::k_class: {
    zs::class_object& cls = closure.as_class();

    zs::var inst = cls.create_instance();
    ret_value = inst;

    ZS_TODO("Really not sure about this.");
    if (zs::object* constructor = cls.get(zs::_ss("constructor"))) {
      zs::object tmp_ret;
      return this->call(*constructor, n_params, local_stack_base, tmp_ret, stack_base_relative);
    }

    return zs::error_code::inaccessible;
  }

  case zs::object_type::k_struct: {
    return runtime_action<runtime_code::struct_call_create>(
        CREF(closure), n_params, stack_base, REF(ret_value));
  }

  case zs::object_type::k_instance:
    return zs::error_code::unimplemented;

  case zs::object_type::k_table:
    return zs::error_code::unimplemented;

  default:
    return zs::error_code::invalid;
  }

  return {};
}

zs::error_result virtual_machine::call(
    const object& closure, std::span<const object> params, object& ret_value) {
  using enum object_type;

  const object_type otype = closure.get_type();

  zbase_assert(
      zb::is_one_of(otype, k_closure, k_native_closure, k_native_function, k_table, k_instance, k_user_data),
      get_object_type_name(otype));

  switch (otype) {
  case k_closure:
    return runtime_action<runtime_code::call_closure>(CREF(closure), params, REF(ret_value));

  case zs::object_type::k_native_closure:
    return runtime_action<runtime_code::call_native_closure>(CREF(closure), params, REF(ret_value));

  case zs::object_type::k_native_function:
    return runtime_action<runtime_code::call_native_function>(CREF(closure), params, REF(ret_value));

  case zs::object_type::k_native_function2:
    return runtime_action<runtime_code::call_native_function2>(CREF(closure), params, REF(ret_value));

  case zs::object_type::k_user_data:
    return zs::error_code::unimplemented;

  case zs::object_type::k_instance:
    return zs::error_code::unimplemented;

  case zs::object_type::k_table:
    return zs::error_code::unimplemented;

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

zs::error_result virtual_machine::runtime_arith_operation(
    arithmetic_op op, object& target, const object& lhs, const object& rhs) {
  using enum object_type;
  using enum exposed_object_type;

#define _FIRST_LEVEL_TYPE(lhs_name, str) \
  case lhs_name:                         \
    return second_level_func.operator()<lhs_name>(this, op, target, lhs, rhs);

#define _SECOND_LEVEL_TYPE(name, str) \
  case name:                          \
    return self->arithmetic_operation<LhsType, name>(op, target, lhs, rhs);

  static constexpr auto second_level_func
      = []<exposed_object_type LhsType>(virtual_machine* self, arithmetic_op op, object& target,
            const object& lhs, const object& rhs) -> zs::error_result {
    switch (to_exposed_object_type(rhs.get_type())) {
      ZS_EXPOSED_TYPE_ENUM(_SECOND_LEVEL_TYPE)
    default:
      target = nullptr;
      return zs::error_code::invalid_operation;
    }
  };

  switch (to_exposed_object_type(lhs.get_type())) {
    ZS_EXPOSED_TYPE_ENUM(_FIRST_LEVEL_TYPE)
  default:
    target = nullptr;
    return zs::error_code::invalid_operation;
  }

#undef _FIRST_LEVEL_TYPE
#undef _SECOND_LEVEL_TYPE
}

zs::error_result virtual_machine::runtime_arith_operation(arithmetic_uop op, object& target, object& src) {
  using enum object_type;
  using enum exposed_object_type;

  switch (to_exposed_object_type(src.get_type())) {
  case ke_bool:
    return arithmetic_operation<ke_bool>(op, target, src);

  case ke_integer:
    return arithmetic_operation<ke_integer>(op, target, src);

  case ke_float:
    return arithmetic_operation<ke_float>(op, target, src);

  case ke_string:
    return arithmetic_operation<ke_string>(op, target, src);

  case ke_table:
    return arithmetic_operation<ke_table>(op, target, src);

  case ke_extension:
    return arithmetic_operation<ke_extension>(op, target, src);

  default:
    break;
  }

  target = nullptr;
  return zs::error_code::invalid_operation;
}

zs::error_result virtual_machine::compile_buffer(
    std::string_view content, std::string_view source_name, zs::object& closure_result, bool with_vargs) {
  zs::jit_compiler compiler(_engine);
  zs::object fct_state;

  if (auto err = compiler.compile(content, source_name, fct_state, nullptr, nullptr, with_vargs)) {
    _error_message = compiler.get_error();
    return err;
  }

  closure_result = zs::object::create_closure(_engine, fct_state, _root_table);

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

zs::error_result virtual_machine::call_buffer(
    std::string_view content, std::string_view source_name, zs::object& ret_value, bool with_vargs) {

  zs::object closure_result;
  if (auto err = compile_buffer(content, source_name, closure_result, with_vargs)) {
    return err;
  }

  if (!closure_result.is_closure()) {
    return zs::error_code::invalid_type;
  }

  zs::object this_table = zs::_t(_engine);

  this_table.as_table().set_delegate(_root_table);

  if (auto err = call(closure_result, { this_table }, ret_value)) {
    return err;
  }

  return {};
}

zs::error_result virtual_machine::call_buffer(std::string_view content, std::string_view source_name,
    zs::object& this_table, zs::object& ret_value, std::span<const object> args, bool with_vargs) {

  if (!this_table.is_table()) {
    this_table = create_this_table_from_root();
  }

  zs::object closure_result;

  {
    zs::jit_compiler compiler(_engine);
    zs::object fct_state;

    if (auto err = compiler.compile(content, source_name, fct_state, nullptr, nullptr, with_vargs)) {
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

  zs::object this_table = zs::_t(_engine);

  this_table.as_table().set_delegate(_root_table);

  if (auto err = call(closure_result, { this_table }, ret_value)) {
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
  if (auto err = compiler.compile(content, source_name, fct_state, nullptr, &prepended_token)) {
    _error_message = compiler.get_error();
    return err;
  }

  zs::object closure_result = zs::object::create_closure(_engine, fct_state, _root_table);

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
  return runtime_arith_operation(arithmetic_op::add, target, lhs, rhs);
}

zs::error_result virtual_machine::sub(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::sub, target, lhs, rhs);
}

zs::error_result virtual_machine::mul(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::mul, target, lhs, rhs);
}

zs::error_result virtual_machine::div(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::div, target, lhs, rhs);
}

zs::error_result virtual_machine::exp(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::exp, target, lhs, rhs);
}

zs::error_result virtual_machine::mod(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::mod, target, lhs, rhs);
}

zs::error_result virtual_machine::bitwise_or(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::bitwise_or, target, lhs, rhs);
}

zs::error_result virtual_machine::bitwise_and(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::bitwise_and, target, lhs, rhs);
}

zs::error_result virtual_machine::bitwise_xor(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::bitwise_xor, target, lhs, rhs);
}

zs::error_result virtual_machine::lshift(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::lshift, target, lhs, rhs);
}

zs::error_result virtual_machine::rshift(object& target, const object& lhs, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::rshift, target, lhs, rhs);
}

zs::error_result virtual_machine::add_eq(object& target, const object& rhs) {
  using enum object_type;

  return runtime_arith_operation(arithmetic_op::add, target, target, rhs);
}

zs::error_result virtual_machine::sub_eq(object& target, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::sub, target, target, rhs);
}

zs::error_result virtual_machine::mul_eq(object& target, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::mul, target, target, rhs);
}

zs::error_result virtual_machine::div_eq(object& target, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::div, target, target, rhs);
}

zs::error_result virtual_machine::mod_eq(object& target, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::mod, target, target, rhs);
}

zs::error_result virtual_machine::exp_eq(object& target, const object& rhs) {
  return runtime_arith_operation(arithmetic_op::exp, target, target, rhs);
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
  using enum opcode;

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
