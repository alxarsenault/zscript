#include <zscript/zscript.h>

namespace zs {

namespace {
  static table_object* s_none_table_object_unique_pointer = nullptr;
  static table_object* s_none_table_object = []() {
    ZS_ASSERT((ptrdiff_t)&s_none_table_object_unique_pointer > 8);
    return (table_object*)&s_none_table_object_unique_pointer;
  }();
} // namespace

static_assert(!std::is_polymorphic_v<delegable_object>, "delegable_object should not be polymorphic.");

delegable_object::~delegable_object() noexcept { reset(); }

void delegable_object::reset() noexcept {
  table_object* dtbl = _delegate.get_pointer<table_object*>();

  if (dtbl and dtbl != s_none_table_object) {
    dtbl->release();
    _delegate.set_pointer(nullptr);
  }
}

void delegable_object::set_no_default_none() noexcept {
  reset();
  _delegate.set_pointer(s_none_table_object);
  _delegate.set_int<delegate_flags_t>(df_none);
}

zs::object delegable_object::get_delegate() const noexcept {
  table_object* dtbl = _delegate.get_pointer<table_object*>();

  if (dtbl == s_none_table_object) {
    return zs::none();
  }

  if (dtbl) {
    return object(dtbl, true);
  }

  return nullptr;
}

bool delegable_object::has_delegate() const noexcept {
  table_object* dtbl = _delegate.get_pointer<table_object*>();
  return dtbl and dtbl != s_none_table_object;
}

bool delegable_object::is_null() const noexcept { return !(bool)_delegate.get_pointer<table_object*>(); }

bool delegable_object::is_none() const noexcept {
  return _delegate.get_pointer<table_object*>() == s_none_table_object;
}

zs::error_result delegable_object::set_delegate(const zs::object& delegate) noexcept {
  ZS_ASSERT(delegate.has_type_mask(zs::constants::k_delegate_mask));

  if (!delegate.has_type_mask(zs::constants::k_delegate_mask)) {
    return errc::invalid_type;
  }

  if (delegate.is_table() and delegate._pointer == this) {
    return errc::invalid_argument;
  }

  reset();

  table_object* tptr = nullptr;

  if (delegate.is_table()) {
    delegate._table->retain();
    tptr = delegate._table;
  }
  else if (delegate.is_none()) {
    tptr = s_none_table_object;
  }

  _delegate.set_pointer(tptr);
  return {};
}

bool delegable_object::is_locked() const noexcept { return zb::has_flag(get_delegate_flags(), df_locked); }

bool delegable_object::is_use_default() const noexcept {
  return zb::has_flag(get_delegate_flags(), df_use_default);
}

delegate_flags_t delegable_object::get_delegate_flags() const noexcept {
  return _delegate.get_int<delegate_flags_t>();
}

void delegable_object::set_delegate_flags(delegate_flags_t flags) noexcept {
  _delegate.set_int<delegate_flags_t>(flags);
}

void delegable_object::set_delegate_flag(delegate_flags_t flag, bool value) noexcept {
  delegate_flags_t flags = get_delegate_flags();
  zb::set_flag(flags, flag, value);
  _delegate.set_int<delegate_flags_t>(flags);
}

void delegable_object::set_use_default(bool use_default) noexcept {
  set_delegate_flag(df_use_default, use_default);
}

void delegable_object::set_locked(bool locked) noexcept { set_delegate_flag(df_locked, locked); }

} // namespace zs.
