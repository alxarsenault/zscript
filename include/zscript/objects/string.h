#include <zscript/objects/object_include_guard.h>

namespace zs {
class string_object final : public reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  ZS_CHECK static string_object* create(zs::engine* eng, std::string_view s);
  ZS_CHECK static string_object* create(zs::engine* eng, size_t n);

  virtual ~string_object() override = default;

  ZS_CK_INLINE std::string_view get_string() const noexcept { return std::string_view(_str, _size); }

  ZS_CK_INLINE char* data() noexcept { return _str; }
  ZS_CK_INLINE const char* data() const noexcept { return _str; }

  ZS_CHECK string_object* clone() const noexcept;

private:
  inline string_object(zs::engine* eng) noexcept
      : reference_counted_object(eng, zs::object_type::k_long_string) {}

  size_t _size;
  char _str[1];
};
} // namespace zs.
