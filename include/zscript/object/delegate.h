#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {
class delegable_object : public zs::reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  inline delegable_object(zs::engine* eng, zs::object_type objtype)
      : zs::reference_counted_object(eng, objtype) {}

  virtual ~delegable_object() override = default;

  inline void set_delegate(const zs::object& delegate) noexcept {
    ZS_ASSERT(delegate.has_type_mask(zs::constants::k_delegate_mask));

    if (delegate.is_table() && delegate._pointer == this) {
      _delegate = delegate.get_weak_ref();
    }
    else {
      _delegate = delegate;
    }
  }

  inline void set_delegate(const zs::object& delegate, bool use_default) noexcept {
    ZS_ASSERT(delegate.has_type_mask(zs::constants::k_delegate_mask));

    if (delegate.is_table() && delegate._pointer == this) {
      _delegate = delegate.get_weak_ref();
    }
    else {
      _delegate = delegate;
    }

    set_use_default_delegate(use_default);
  }

  inline void set_delegate(zs::object&& delegate) noexcept {
    ZS_ASSERT(delegate.has_type_mask(zs::constants::k_delegate_mask));
    ZS_ASSERT(!(delegate.is_table() and this == delegate._delegable), "Setting itself as a delegate.");
    _delegate = std::move(delegate);
  }

  inline void set_delegate(zs::object&& delegate, bool use_default) noexcept {
    ZS_ASSERT(delegate.has_type_mask(zs::constants::k_delegate_mask));
    ZS_ASSERT(!(delegate.is_table() and this == delegate._delegable), "Setting itself as a delegate.");
    _delegate = std::move(delegate);
    set_use_default_delegate(use_default);
  }

  ZS_CK_INLINE bool has_delegate() const noexcept { return _delegate.is_table(); }
  ZS_CK_INLINE bool is_locked() const noexcept { return zb::has_flag( _delegate_flags, delegate_flags_t::df_locked); }

  ZS_CK_INLINE zs::object& get_delegate() noexcept { return _delegate; }
  ZS_CK_INLINE const zs::object& get_delegate() const noexcept { return _delegate; }

  ZS_INLINE void set_use_default_delegate(bool use_default) noexcept {
    zb::set_flag(_delegate_flags, delegate_flags_t::df_use_default, use_default);
  }
  ZS_INLINE void set_locked(bool locked) noexcept {
    zb::set_flag(_delegate_flags, delegate_flags_t::df_locked, locked);
  }

  ZS_CK_INLINE bool get_use_default_delegate() const noexcept { return zb::has_flag(_delegate_flags, delegate_flags_t::df_use_default); }

protected:
  zs::object _delegate;
  delegate_flags_t _delegate_flags = delegate_flags_t::df_use_default;
  
    void copy_delegate(object& obj) const noexcept ;
};
} // namespace zs.
