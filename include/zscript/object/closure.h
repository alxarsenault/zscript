#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {
class closure_object final : public reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  ZS_CHECK static closure_object* create(zs::engine* eng, const zs::object& fpo, const zs::object& root);
  ZS_CHECK static closure_object* create(zs::engine* eng, zs::object&& fpo, const zs::object& root);

  virtual ~closure_object() override = default;

  ZS_CHECK zs::function_prototype_object* get_function_prototype() const noexcept;
  ZS_CHECK zs::function_prototype_object& get_proto() const noexcept;

  void clear() {
    _default_params.clear();
    _captured_values.clear();
    _function.reset();
    _root.reset();
    _base.reset();
    _module.reset();
    _this.reset();
  }

  template <class Object>
  inline void set_bounded_this(Object&& obj) {
    _this = std::forward<Object>(obj);
  }

  int_t get_parameters_count() const noexcept;
  int_t get_default_parameters_count() const noexcept;

  int_t get_minimum_required_parameters_count() const noexcept;
  bool is_possible_parameter_count(size_t sz) const noexcept;
  bool is_valid_parameters(zs::vm_ref vm, zb::span<const object> params, int_t& n_type_match) const noexcept;

  object copy_with_binded_this(const zs::object& env) const noexcept;

  ZS_CHECK object clone() const noexcept override;

private:
  closure_object(zs::engine* eng, const zs::object& fpo);
  closure_object(zs::engine* eng, zs::object&& fpo);

public:
  zs::object _function;
  zs::object _root;
  zs::object _base;
  zs::object _module;
  zs::object _this;
  zs::vector<zs::object> _default_params;
  zs::vector<zs::object> _captured_values;
};
} // namespace zs.
