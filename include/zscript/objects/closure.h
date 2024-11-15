#include <zscript/objects/object_include_guard.h>

namespace zs {
class closure_object final : public reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  ZS_CHECK static closure_object* create(zs::engine* eng, const zs::object& fpo, const zs::object& root);

  virtual ~closure_object() override = default;

  ZS_CHECK zs::function_prototype_object* get_function_prototype() const noexcept;
  ZS_CHECK zs::function_prototype_object& get_proto() const noexcept;

  void clear() {
    _default_params.clear();
    _capture_values.clear();
    _function.reset();
    _root.reset();
    _base.reset();
    _module.reset();
    _env.reset();
  }

private:
  closure_object(zs::engine* eng, const zs::object& fpo);

public:
  zs::object _function;
  zs::object _root;
  zs::object _base;
  zs::object _module;
  zs::object _env;
  zs::vector<zs::object> _default_params;
  zs::vector<zs::object> _capture_values;
};
} // namespace zs.
