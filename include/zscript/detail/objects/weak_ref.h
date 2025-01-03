#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {
class weak_ref_object final : public reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  static weak_ref_object* create(zs::engine* eng, const object_base& obj);

  object get_object() const noexcept;

private:
  weak_ref_object() = delete;
  weak_ref_object(zs::engine* eng, const object_base& obj) noexcept;
  ~weak_ref_object() noexcept = default;

  /// Holds a copy of a reference counted object.
  /// This object doesn't affect the ref count.
  zs::object_base _obj;

  friend class reference_counted_object;

  static void destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept;
  static object clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept;
};
} // namespace zs.
