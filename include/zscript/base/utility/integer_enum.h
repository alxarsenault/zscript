//
// MIT License
//
// Copyright (c) 2024 Alexandre Arsenault
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#include <zscript/base/zbase.h>
#include <zscript/base/utility/traits.h>

ZBASE_BEGIN_NAMESPACE

template <class _FlagType>
inline constexpr bool has_flag(_FlagType flags, _FlagType flag_to_verity) noexcept {
  using type = zb::underlying_type_t<_FlagType>;
  return ((type(flags) & (type)flag_to_verity) != (type)0);
}

template <auto FlagToVerity, class _FlagType = decltype(FlagToVerity)>
inline constexpr bool has_flag(_FlagType flags) noexcept {
  using type = zb::underlying_type_t<_FlagType>;
  return ((type(flags) & (type)FlagToVerity) != (type)0);
}

template <class _FlagType>
inline constexpr _FlagType& set_flag(_FlagType& flags, _FlagType flag_to_set, bool value) noexcept {
  using type = zb::underlying_type_t<_FlagType>;

  if (value) {
    flags = (_FlagType)((type)flags | (type)flag_to_set);
    return flags;
  }

  flags = (_FlagType)((type)flags & ~(type)flag_to_set);
  return flags;
}

template <class _FlagType>
inline constexpr _FlagType& set_flag(_FlagType& flags, _FlagType flag_to_set) noexcept {
  using type = zb::underlying_type_t<_FlagType>;
  flags = (_FlagType)((type)flags | (type)flag_to_set);
  return flags;
}

template <class _FlagType>
inline constexpr _FlagType& remove_flag(_FlagType& flags, _FlagType flag_to_remove) noexcept {
  using type = zb::underlying_type_t<_FlagType>;
  flags = (_FlagType)((type)flags & ~(type)flag_to_remove);
  return flags;
}

template <auto FlagToSet, class _FlagType = decltype(FlagToSet)>
inline constexpr _FlagType& set_flag(_FlagType& flags, bool value) noexcept {
  using type = zb::underlying_type_t<_FlagType>;

  if (value) {
    flags = (_FlagType)((type)flags | (type)FlagToSet);
    return flags;
  }

  flags = (_FlagType)((type)flags & ~(type)FlagToSet);
  return flags;
}

template <auto FlagToSet, class _FlagType = decltype(FlagToSet)>
inline constexpr _FlagType& set_flag(_FlagType& flags) noexcept {
  using type = zb::underlying_type_t<_FlagType>;
  flags = (_FlagType)((type)flags | (type)FlagToSet);
  return flags;
}

///
template <class EnumType, class = void>
struct integer_enum_type : __zb::false_t {};

///
template <class EnumType>
using is_integer_enum = __zb::bool_t<std::is_enum_v<EnumType> && __zb::integer_enum_type<EnumType>::value>;

template <class EnumType>
concept integer_enum = __zb::is_integer_enum<EnumType>::value;

template <class EnumType>
ZB_CK_INLINE_CXPR std::underlying_type_t<EnumType> enum_cast(EnumType e) noexcept {
  return static_cast<std::underlying_type_t<EnumType>>(e);
}

ZBASE_END_NAMESPACE

template <zb::integer_enum EnumType>
ZB_CK_INLINE_CXPR EnumType operator|(EnumType lhs, EnumType rhs) noexcept {
  return static_cast<EnumType>(zb::enum_cast(lhs) | zb::enum_cast(rhs));
}

template <zb::integer_enum EnumType>
ZB_CK_INLINE_CXPR EnumType operator&(EnumType lhs, EnumType rhs) noexcept {
  return static_cast<EnumType>(zb::enum_cast(lhs) & zb::enum_cast(rhs));
}

template <zb::integer_enum EnumType>
ZB_INLINE_CXPR EnumType& operator|=(EnumType& lhs, EnumType rhs) noexcept {
  return (lhs = (lhs | rhs));
}

template <zb::integer_enum EnumType>
ZB_INLINE_CXPR EnumType& operator&=(EnumType& lhs, EnumType rhs) noexcept {
  return (lhs = (lhs & rhs));
}

template <zb::integer_enum EnumType>
ZB_CK_INLINE_CXPR EnumType operator~(EnumType lhs) noexcept {
  return static_cast<EnumType>(~zb::enum_cast(lhs));
}

template <zb::integer_enum EnumType>
ZB_CK_INLINE_CXPR bool operator==(EnumType lhs, std::underlying_type_t<EnumType> value) noexcept {
  return zb::enum_cast(lhs) == value;
}

template <zb::integer_enum EnumType>
ZB_CK_INLINE_CXPR bool operator==(std::underlying_type_t<EnumType> value, EnumType lhs) noexcept {
  return zb::enum_cast(lhs) == value;
}

template <zb::integer_enum EnumType>
ZB_CK_INLINE_CXPR bool operator!=(EnumType lhs, std::underlying_type_t<EnumType> value) noexcept {
  return zb::enum_cast(lhs) != value;
}

template <zb::integer_enum EnumType>
ZB_CK_INLINE_CXPR bool operator!=(std::underlying_type_t<EnumType> value, EnumType lhs) noexcept {
  return zb::enum_cast(lhs) != value;
}

// NOLINTBEGIN
#define ZBASE_ENUM_CLASS_FLAGS(enum_class)                                                               \
  ZB_CK_INLINE_CXPR enum_class operator|(enum_class lhs, enum_class rhs) noexcept {                      \
    using type = std::underlying_type_t<enum_class>;                                                     \
    return static_cast<enum_class>(static_cast<type>(lhs) | static_cast<type>(rhs));                     \
  }                                                                                                      \
                                                                                                         \
  ZB_CK_INLINE_CXPR enum_class operator&(enum_class lhs, enum_class rhs) noexcept {                      \
    using type = std::underlying_type_t<enum_class>;                                                     \
    return static_cast<enum_class>(static_cast<type>(lhs) & static_cast<type>(rhs));                     \
  }                                                                                                      \
                                                                                                         \
  ZB_INLINE_CXPR enum_class& operator|=(enum_class& lhs, enum_class rhs) noexcept {                      \
    return (lhs = (lhs | rhs));                                                                          \
  }                                                                                                      \
                                                                                                         \
  ZB_INLINE_CXPR enum_class& operator&=(enum_class& lhs, enum_class rhs) noexcept {                      \
    return (lhs = (lhs & rhs));                                                                          \
  }                                                                                                      \
                                                                                                         \
  ZB_CK_INLINE_CXPR enum_class operator~(enum_class lhs) noexcept {                                      \
    using type = std::underlying_type_t<enum_class>;                                                     \
    return static_cast<enum_class>(~type(lhs));                                                          \
  }                                                                                                      \
                                                                                                         \
  ZB_CK_INLINE_CXPR bool operator==(enum_class lhs, std::underlying_type_t<enum_class> value) noexcept { \
    return std::underlying_type_t<enum_class>(lhs) == value;                                             \
  }                                                                                                      \
                                                                                                         \
  ZB_CK_INLINE_CXPR bool operator!=(enum_class lhs, std::underlying_type_t<enum_class> value) noexcept { \
    return std::underlying_type_t<enum_class>(lhs) != value;                                             \
  }                                                                                                      \
  ZB_CK_INLINE_CXPR bool operator==(std::underlying_type_t<enum_class> value, enum_class lhs) noexcept { \
    return std::underlying_type_t<enum_class>(lhs) == value;                                             \
  }                                                                                                      \
                                                                                                         \
  ZB_CK_INLINE_CXPR bool operator!=(std::underlying_type_t<enum_class> value, enum_class lhs) noexcept { \
    return std::underlying_type_t<enum_class>(lhs) != value;                                             \
  }
// NOLINTEND
