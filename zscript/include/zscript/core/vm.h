#pragma once

#include <zscript/core/common.h>
#include <zscript/core/object.h>

namespace zs {

//
// MARK: - vm_ref
//

class vm_ref {
public:
  explicit vm_ref(zs::virtual_machine* v);

  vm_ref(std::nullptr_t) = delete;

  ZS_CK_INLINE bool is_valid_vm() const noexcept { return _vm != nullptr; }

  int_t stack_size() const noexcept;

  int_t push_root();
  int_t push_null();
  int_t push_bool(bool_t value);
  int_t push_integer(int_t value);
  int_t push_float(float_t value);
  int_t push_string(std::string_view s);
  int_t push_string_concat(std::string_view s1, std::string_view s2);

  int_t push(const object& obj);
  int_t push(object&& obj);

  //
  //
  //

  zs::error_result new_closure(zs::native_closure* closure);

  template <class Fct>
  inline zs::error_result new_closure(Fct&& fct);

  //
  //
  //

  zs::error_result get_integer(int_t idx, int_t& res);
  zs::error_result get_float(int_t idx, float_t& res);
  zs::error_result get_string(int_t idx, std::string_view& res);

  zs::optional_result<float_t> get_float(int_t idx);

  //
  //
  //

  ZS_CHECK zs::object_type get_type(int_t idx) const noexcept;

  ZS_CK_INLINE bool is_type(int_t idx, zs::object_type t) const noexcept { return get_type(idx) == t; }

  ZS_CK_INLINE bool is_null(int_t idx) const noexcept { return get_type(idx) == zs::object_type::k_null; }

  ZS_CK_INLINE bool is_integer(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_integer;
  }

  ZS_CK_INLINE bool is_float(int_t idx) const noexcept { return get_type(idx) == zs::object_type::k_float; }

  ZS_CK_INLINE bool is_bool(int_t idx) const noexcept { return get_type(idx) == zs::object_type::k_bool; }

  ZS_CK_INLINE bool is_array(int_t idx) const noexcept { return get_type(idx) == zs::object_type::k_array; }
  ZS_CK_INLINE bool is_struct(int_t idx) const noexcept { return get_type(idx) == zs::object_type::k_struct; }
  ZS_CK_INLINE bool is_struct_instance(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_struct_instance;
  }

  ZS_CK_INLINE bool is_table(int_t idx) const noexcept { return get_type(idx) == zs::object_type::k_table; }

  ZS_CK_INLINE bool is_user_data(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_user_data;
  }

  ZS_CK_INLINE bool is_closure(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_closure;
  }

  ZS_CK_INLINE bool is_class(int_t idx) const noexcept { return get_type(idx) == zs::object_type::k_class; }

  ZS_CK_INLINE bool is_instance(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_instance;
  }

  ZS_CK_INLINE bool is_weak_ref(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_weak_ref;
  }

  ZS_CK_INLINE bool is_long_string(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_long_string;
  }

  ZS_CK_INLINE bool is_small_string(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_small_string;
  }

  ZS_CK_INLINE bool is_string_view(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_string_view;
  }

  ZS_CK_INLINE bool is_mutable_string(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_mutable_string;
  }

  ZS_CK_INLINE bool is_native_closure(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_native_closure;
  }

  ZS_CK_INLINE bool is_native_function(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_native_function;
  }

  ZS_CK_INLINE bool is_raw_pointer(int_t idx) const noexcept {
    return get_type(idx) == zs::object_type::k_raw_pointer;
  }

  ZS_CK_INLINE bool is_string(int_t idx) const noexcept {
    return zs::get_object_type_mask(get_type(idx)) & zs::object_base::k_string_mask;
  }

  ZS_CK_INLINE bool is_number(int_t idx) const noexcept {
    return zs::get_object_type_mask(get_type(idx)) & zs::object_base::k_number_mask;
  }

  ZS_CK_INLINE bool is_function(int_t idx) const noexcept {
    return zs::get_object_type_mask(get_type(idx)) & zs::object_base::k_function_mask;
  }

  //
  //
  //

  zs::error_result call(int_t n_params, bool returns, bool pop_callable);

  //
  // Error.
  //

  void set_error(std::string_view msg);

  template <class... Args>
  inline void set_error(Args&&... args) {
    set_error(std::string_view(zs::strprint<"">(get_engine(), std::forward<Args>(args)...)));
  }

  template <class... Args>
  inline zs::error_code set_error(zs::error_code err, Args&&... args) {
    set_error(std::string_view(zs::strprint<"">(get_engine(), std::forward<Args>(args)...)));
    return err;
  }

  template <class... Args>
  inline zs::error_code set_error(zs::error_result err, Args&&... args) {
    set_error(std::string_view(zs::strprint<"">(get_engine(), std::forward<Args>(args)...)));
    return err.code;
  }

  ZS_CHECK const zs::string& get_error() const noexcept;

  //
  // Virtual machine
  //

  ZS_CHECK object& top() noexcept;
  ZS_CHECK const object& top() const noexcept;

  ZS_CHECK object& root() noexcept;
  ZS_CHECK const object& root() const noexcept;

  ZS_CHECK object& operator[](int_t idx) noexcept;
  ZS_CHECK const object& operator[](int_t idx) const noexcept;

  ZS_CHECK object* stack_base_pointer() noexcept;
  ZS_CHECK const object* stack_base_pointer() const noexcept;

  ZS_CK_INLINE zs::virtual_machine* get_virtual_machine() const noexcept { return _vm; }
  ZS_CK_INLINE operator zs::virtual_machine*() const noexcept { return _vm; }
  ZS_CK_INLINE operator zs::virtual_machine&() const noexcept { return *_vm; }

  ZS_CK_INLINE zs::virtual_machine* operator->() noexcept { return _vm; }
  ZS_CK_INLINE zs::virtual_machine* operator->() const noexcept { return _vm; }

  zs::engine* get_engine() const noexcept;
  std::ostream& get_stream() const noexcept;

  operator std::ostream&() const noexcept { return get_stream(); }

protected:
  zs::virtual_machine* _vm;
};

//
// MARK: - vm
//

class vm : public vm_ref {
public:
  vm(size_t stack_size = ZS_DEFAULT_STACK_SIZE, allocate_t alloc_cb = ZS_DEFAULT_ALLOCATE,
      raw_pointer_t user_pointer = nullptr, raw_pointer_release_hook_t user_release = nullptr,
      stream_getter_t stream_getter = ZS_DEFAULT_STREAM_GETTER,
      engine_initializer_t initializer = ZS_DEFAULT_ENGINE_INITIALIZER) noexcept;

  vm(zs::engine* eng, size_t stack_size = ZS_DEFAULT_STACK_SIZE) noexcept;

  explicit vm(zs::virtual_machine* v) noexcept;
  vm(const vm&) = delete;
  vm(vm&&) noexcept;

  ~vm() noexcept;

  vm& operator=(const vm&) = delete;
  vm& operator=(vm&&) noexcept;

private:
  struct helper;
};

} // namespace zs.
