#include <zscript/objects/object_include_guard.h>

namespace zs {
class mutable_string_object final : public reference_counted_object, public zs::string {
public:
  ZS_OBJECT_CLASS_COMMON;

  using string_type = zs::string;

  ZS_CHECK static mutable_string_object* create(zs::engine* eng, std::string_view s);
  ZS_CHECK static mutable_string_object* create(zs::engine* eng, size_t n);

  virtual ~mutable_string_object() override = default;

  ZS_CK_INLINE std::string_view get_string() const noexcept { return std::string_view(*this); }

  ZS_CHECK mutable_string_object* clone() const noexcept;

private:
  mutable_string_object(zs::engine* eng) noexcept;
};
} // namespace zs.
