#include <zscript/core/objects/object_include_guard.h>

namespace zs {
class delegate_object : public zs::reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  inline delegate_object(zs::engine* eng, zs::object_type objtype)
      : zs::reference_counted_object(eng, objtype) {}

  virtual ~delegate_object() override = default;

  inline void set_delegate(const zs::object& delegate) noexcept {
    ZS_ASSERT(!(delegate.is_delegable() && delegate._pointer == this), "Setting itself as a delegate.");

    if (delegate.is_delegable() && delegate._pointer == this) {
      _delegate = delegate.get_weak_ref();
    }
    else {
      _delegate = delegate;
    }
  }

  inline void set_delegate(zs::object&& delegate) noexcept {
    ZS_ASSERT(!(delegate.is_table() and this == delegate._odelegate), "Setting itself as a delegate.");
    _delegate = std::move(delegate);
  }

  ZS_CK_INLINE bool has_delegate() const noexcept { return _delegate.is_table(); }

  ZS_CK_INLINE zs::object& get_delegate() noexcept { return _delegate; }

  ZS_CK_INLINE const zs::object& get_delegate() const noexcept { return _delegate; }

protected:
  zs::object _delegate;
};
} // namespace zs.
