#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {
class delegable_object : public zs::reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  using reference_counted_object::reference_counted_object;

  /// @brief Get the object delegate.
  ZS_CHECK zs::object get_delegate() const noexcept;

  /// @brief Set the object delegate.
  /// A delegate can only be k_null, k_none or k_table.
  zs::error_result set_delegate(const zs::object& delegate) noexcept;

  ZS_INLINE zs::error_result set_delegate(const zs::object& delegate, delegate_flags_t flags) noexcept {
    set_delegate_flags(flags);
    return set_delegate(delegate);
  }

  void set_no_default_none() noexcept;

  ZS_CHECK bool has_delegate() const noexcept;

  ZS_CHECK bool is_null() const noexcept;
  ZS_CHECK bool is_none() const noexcept;
  ZS_CHECK bool is_locked() const noexcept;

  ZS_CHECK bool is_use_default() const noexcept;

  ZS_CHECK delegate_flags_t get_delegate_flags() const noexcept;

  void set_delegate_flags(delegate_flags_t flags) noexcept;

  void set_delegate_flag(delegate_flags_t flag, bool value) noexcept;

  void set_use_default(bool use_default) noexcept;

  void set_locked(bool locked) noexcept;

protected:
  ~delegable_object() noexcept;

private:
  using enum delegate_flags_t;
  zb::packed_pointer _delegate{ nullptr, df_use_default };

  void reset() noexcept;
};
} // namespace zs.
