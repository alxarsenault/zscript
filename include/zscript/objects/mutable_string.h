#include <zscript/objects/object_include_guard.h>

namespace zs {
class mutable_string_object final : public delegate_object, public zs::string {
public:
  ZS_OBJECT_CLASS_COMMON;

  using string_type = zs::string;

  ZS_CHECK static mutable_string_object* create(zs::engine* eng, std::string_view s);
  ZS_CHECK static mutable_string_object* create(zs::engine* eng, zs::string&& s);
  ZS_CHECK static mutable_string_object* create(zs::engine* eng, size_t n);

  virtual ~mutable_string_object() override = default;

  ZS_CK_INLINE zs::string& get_string() noexcept { return *this; }
  ZS_CK_INLINE const zs::string& get_string() const noexcept { return *this; }

  ZS_CHECK mutable_string_object* clone() const noexcept;

private:
  mutable_string_object(zs::engine* eng) noexcept;
  mutable_string_object(zs::engine* eng, zs::string&& s) noexcept;
};
} // namespace zs.
