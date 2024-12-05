#include <zscript/objects/object_include_guard.h>

namespace zs {
class capture_object final : public reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  ZS_CHECK static capture_object* create(zs::engine* eng, object* ptr) noexcept;

  virtual ~capture_object() override = default;

  ZS_CHECK capture_object* clone() noexcept;

  ZS_CK_INLINE bool is_baked() const noexcept { return _is_baked; }

  ZS_INLINE void bake() {
    if (_is_baked) {
      return;
    }
    _value = *_ptr;
    _ptr = &_value;
    _is_baked = true;
  }

  ZS_CK_INLINE object* get_value_ptr() const noexcept { return _ptr; }

  ZS_CK_INLINE const object& get_value() const noexcept { return *_ptr; }

private:
  capture_object(zs::engine* eng, object* ptr);

  object* _ptr = nullptr;
  object _value;
  bool _is_baked = false;
};
} // namespace zs.
