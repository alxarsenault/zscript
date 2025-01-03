#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {
class string_object final : public reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  ZS_CHECK static string_object* create(zs::engine* eng, std::string_view s);
  ZS_CHECK static string_object* create(zs::engine* eng, size_t n);

  ZS_CK_INLINE std::string_view get_string() const noexcept { return std::string_view(_str, _size); }
  ZS_CK_INLINE const char* get_cstring() const noexcept { return _str; }

  ZS_CK_INLINE char* data() noexcept { return _str; }
  ZS_CK_INLINE const char* data() const noexcept { return _str; }

  ZS_CK_INLINE uint64_t hash() const noexcept {
    if (_hash == 0) {
      update_hash();
    }

    return _hash;
  }

  void update_hash() const noexcept;

private:
  inline string_object(zs::engine* eng) noexcept
      : reference_counted_object(eng, object_type::k_long_string) {}

  ~string_object() noexcept = default;

  size_t _size;
  mutable uint64_t _hash = 0;
  char _str[1];

  static void destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept;
  static object clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept;
};
} // namespace zs.
