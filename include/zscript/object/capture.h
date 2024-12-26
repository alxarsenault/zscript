#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {

class capture {
public:
  ZS_OBJECT_CLASS_COMMON;

  static object create(zs::engine* eng, object* ptr);

  ZS_CHECK static capture& as_capture(const object_base& obj);

  ZS_CHECK static bool is_capture(const object_base& obj) noexcept;

  ZS_CK_INLINE bool is_baked() const noexcept { return _is_baked; }

  void bake();

  ZS_CK_INLINE object* get_value_ptr() const noexcept { return _ptr; }
  ZS_CK_INLINE const object& get_value() const noexcept { return *_ptr; }

private:
  capture(object* ptr) noexcept;

  object* _ptr = nullptr;
  object _value;
  bool _is_baked = false;
};
} // namespace zs.
