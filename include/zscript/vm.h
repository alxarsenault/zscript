#pragma once

#include <zscript/object.h>

namespace zs {

//
// MARK: - vm_ref
//

class vm_ref {
public:
  vm_ref(std::nullptr_t) = delete;
  vm_ref(zs::virtual_machine* vm) noexcept;

  ZS_CHECK int_t stack_size() const noexcept;

  int_t push_global();
  int_t push_null();
  int_t push_none();
  int_t push_true();
  int_t push_false();
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

  zs::error_result get_integer(int_t idx, int_t& res);
  zs::error_result get_float(int_t idx, float_t& res);
  zs::error_result get_string(int_t idx, std::string_view& res);

  zs::optional_result<float_t> get_float(int_t idx);

  //
  // Error.
  //

  int_t set_error(std::string_view msg);

  template <class... Args>
  ZS_INLINE int_t set_error(const Args&... args) {
    return set_error(std::string_view(zs::strprint(get_engine(), args...)));
  }

  ZS_CHECK zs::string get_error() const noexcept;

  //
  // Virtual machine
  //

  ZS_CHECK const object& top() const noexcept;
  ZS_CHECK const object& global() const noexcept;

  ZS_CHECK object& operator[](int_t idx) noexcept;
  ZS_CHECK const object& operator[](int_t idx) const noexcept;

  ZS_CHECK object* stack_base_pointer() noexcept;
  ZS_CHECK const object* stack_base_pointer() const noexcept;

  ZS_CK_INLINE zs::virtual_machine* get_virtual_machine() const noexcept { return _vm; }

  ZS_CK_INLINE zs::virtual_machine* operator->() noexcept { return _vm; }
  ZS_CK_INLINE zs::virtual_machine* operator->() const noexcept { return _vm; }

  ZS_CHECK zs::engine* get_engine() const noexcept;

  ZS_CHECK std::ostream& get_stream() const noexcept;

  operator std::ostream&() const noexcept { return get_stream(); }

  ZS_CHECK table_object& get_registry_table_object() const noexcept;

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

  vm(const config_t& config) noexcept;

  vm(zs::engine* eng, size_t stack_size = ZS_DEFAULT_STACK_SIZE) noexcept;

  explicit vm(zs::virtual_machine* v) noexcept;
  vm(const vm&) = delete;
  vm(vm&&) noexcept;

  ~vm() noexcept;

  vm& operator=(const vm&) = delete;
  vm& operator=(vm&&) noexcept;
};

} // namespace zs.
