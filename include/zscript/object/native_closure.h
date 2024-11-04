#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {

struct named_variable_type_info : variable_type_info {
  object name;
};

class native_closure_object final : public reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  enum class closure_type : uint8_t { fct, obj };

  ZS_CHECK static native_closure_object* create(zs::engine* eng, zs::native_closure* closure);
  ZS_CHECK static native_closure_object* create(zs::engine* eng, zs::function_t fct);

  virtual ~native_closure_object() override;

  ZS_CHECK int_t call(vm_ref v);

  ZS_INLINE void set_user_pointer(zs::raw_pointer_t ptr) noexcept { _user_pointer = ptr; }
  ZS_CK_INLINE zs::raw_pointer_t get_user_pointer() const noexcept { return _user_pointer; }

  template <class T>
  ZS_CK_INLINE T* get_user_pointer() noexcept {
    return reinterpret_cast<T*>(_user_pointer);
  }

  ZS_INLINE void set_release_hook(native_closure_release_hook_t hook) noexcept { _release_hook = hook; }

  ZS_CHECK object clone() const noexcept override;

  ZS_CK_INLINE closure_type get_closure_type() const noexcept { return _callback.ctype; }

  template <class Object>
  ZS_INLINE void set_bounded_this(Object&& obj) {
    _this = std::forward<Object>(obj);
  }

  ZS_CK_INLINE bool has_parameter_info() const noexcept { return !_parameter_names.empty(); }
  ZS_CK_INLINE bool has_default_parameters() const noexcept { return !_default_params.empty(); }

  template <class Object>
  inline zs::error_result add_parameter(Object&& param_name, uint32_t mask = 0, uint64_t custom_mask = 0,
      variable_attribute_t flgs = variable_attribute_t::va_none) {

    if (!param_name.is_string()) {
      return errc::not_a_string;
    }

    if (_parameter_names.contains([&](const named_variable_type_info& n) { return n.name == param_name; })) {
      return errc::duplicated_parameter_name;
    }

    _parameter_names.push_back({ custom_mask, mask, flgs, std::forward<Object>(param_name) });
    return {};
  }

  template <class Object>
  ZS_INLINE void add_default_parameter(Object&& obj) {
    _default_params.push_back(std::forward<Object>(obj));
  }

  ZS_CK_INLINE int_t get_parameters_count() const noexcept { return _parameter_names.size(); }

  ZS_CK_INLINE int_t get_default_parameters_count() const noexcept { return _default_params.size(); }

  ZS_CK_INLINE int_t get_min_parameters_count() const noexcept {
    return _parameter_names.size() - _default_params.size();
  }

  ZS_CK_INLINE bool is_possible_parameter_count(size_t sz) const noexcept {
    return has_parameter_info() ? sz >= get_min_parameters_count() and sz <= get_parameters_count() : true;
  }

  ZS_CHECK bool is_valid_parameters(
      zs::vm_ref vm, zb::span<const object> params, int_t& n_type_match) const noexcept;

private:
  struct callback_type {

    inline callback_type(native_closure* c)
        : closure(c)
        , ctype(closure_type::obj) {}

    inline callback_type(function_t f)
        : fct(f)
        , ctype(closure_type::fct) {}

    union {
      zs::native_closure* closure;
      zs::function_t fct;
    };

    closure_type ctype;
  };

  native_closure_object() = delete;
  native_closure_object(zs::engine* eng, callback_type cb) noexcept;

  zs::object _this;
  callback_type _callback;
  native_closure_release_hook_t _release_hook = nullptr;
  zs::raw_pointer_t _user_pointer = nullptr;

  zs::small_vector<zs::object, 8> _default_params;
  zs::small_vector<zs::named_variable_type_info, 8> _parameter_names;
  zs::small_vector<zs::object, 8> _restricted_types;
  bool _has_vargs_params = false;
};
} // namespace zs.
