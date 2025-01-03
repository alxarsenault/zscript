#pragma once

#include <zscript/common.h>
#include <zscript/base/memory/pointer.h>

namespace zs {

enum class serializer_type { plain, quoted, json, json_compact, plain_compact };

zs::error_result serialize_to_json(zs::engine* eng, std::ostream& stream, const object_base& o, int idt = 0);

struct serializer {
  inline serializer(serializer_type type, const object_base& o, int idt = 0)
      : obj(o)
      , indent(idt)
      , type(type) {}

  inline serializer(serializer_type type, const object_base* o, int idt = 0)
      : obj(*o)
      , indent(idt)
      , type(type) {}

  const object_base& obj;
  int indent = 0;
  serializer_type type;

  friend std::ostream& operator<<(std::ostream& stream, const serializer& s);
};

template <class T, serializer_type SType>
struct streamer_base {
  inline streamer_base(const T& o, int idt = 0)
      : obj(o)
      , indent(idt) {}

  const T& obj;
  int indent;
};

template <serializer_type SType>
struct streamer {
  inline streamer(const object_base& o)
      : obj(o) {}
  inline streamer(const object_base* o)
      : obj(*o) {}

  const object_base& obj;

  inline friend std::ostream& operator<<(std::ostream& stream, const streamer<SType>& s) {
    return stream << streamer_base<object_base, SType>(s.obj);
  }
};

struct native_closure : reference_counted {
  ZS_INLINE native_closure(zs::engine* eng) noexcept
      : reference_counted(eng) {}

  virtual int_t call(zs::vm_ref vm) = 0;

protected:
  virtual ~native_closure() override = default;
};

/// Object.
struct object_base {
  using enum object_type;
  using enum object_flags_t;

  ZBASE_PRAGMA_PUSH()
  ZBASE_CLANG_DIAGNOSTIC(ignored, "-Wnested-anon-types")
  ZBASE_CLANG_DIAGNOSTIC(ignored, "-Wgnu-anonymous-struct")

  /// Data part (8 bytes).
  union {
    uint64_t _lvalue;
    zs::table_object* _table;
    zs::array_object* _array;
    zs::struct_object* _struct;
    zs::struct_instance_object* _struct_instance;
    zs::string_object* _lstring;
    zs::closure_object* _closure;
    zs::native_closure_object* _native_closure;
    zs::user_data_object* _udata;
    zs::reference_counted_object* _ref_counted;
    zs::weak_ref_object* _weak_ref;
    zs::delegable_object* _delegable;
    zs::function_t _nfct;
    alignas(uint64_t) const char* _sview;
    alignas(uint64_t) int_t _int;
    alignas(uint64_t) float_t _float;

    // Raw pointer.
    zs::raw_pointer_t _pointer;

    // This is just to see part of the small string value when debugging.
    alignas(uint64_t) char _sbuffer[8];

    alignas(uint64_t) union {
      alignas(uint64_t) zb::aligned_type_storage<zs::object_map::iterator> table_it;
    } _atom;
  };

  union {

    struct {
      /// Extra part 1 (4 bytes).
      uint32_t _ex1_u32;

      /// Extra part 2 (2 bytes).
      union {
        uint16_t _ex2_u16;

        /// Atom extra.
        struct {
          uint8_t _ex2_delegate_id;
          atom_type _atom_type;
        };
      };

      /// These two are mendatory for all types.
      object_type _type;
      object_flags_t _flags;
    };

    uint64_t _rvalue;
  };

  ZBASE_PRAGMA_POP()

  template <class T>
  inline T*& get_pointer_ref() {
    return reinterpret_cast<T*&>(_pointer);
  }

  static constexpr const uint32_t k_string_mask = zs::constants::k_string_mask;
  static constexpr const uint32_t k_cstring_mask = zs::constants::k_cstring_mask;
  static constexpr const uint32_t k_number_mask = zs::constants::k_number_mask;
  static constexpr const uint32_t k_number_or_bool_mask = zs::constants::k_number_or_bool_mask;
  static constexpr const uint32_t k_function_mask = zs::constants::k_function_mask;

  /// Access underlying type.
  /// It's safer to use these methods to access the object type rather than
  /// using the type directly.

  ZB_CK_INLINE zs::array_object& as_array() const noexcept {
    ZS_ASSERT(is_array(), "Invalid array type.");
    return *_array;
  }

  ZB_CK_INLINE zs::struct_object& as_struct() const noexcept {
    ZS_ASSERT(is_struct(), "Invalid struct type.");
    return *_struct;
  }

  ZB_CK_INLINE zs::struct_instance_object& as_struct_instance() const noexcept {
    ZS_ASSERT(is_struct_instance(), "Invalid struct instance type.");
    return *_struct_instance;
  }

  ZB_CK_INLINE zs::table_object& as_table() const noexcept {
    ZS_ASSERT(is_table(), "Invalid table type.");
    return *_table;
  }

  ZB_CK_INLINE zs::user_data_object& as_udata() const noexcept {
    ZS_ASSERT(is_user_data(), "Invalid user data type.");
    return *_udata;
  }

  ZB_CK_INLINE zs::native_closure_object& as_native_closure() const noexcept {
    ZS_ASSERT(is_native_closure(), "Invalid native closure type.");
    return *_native_closure;
  }

  ZB_CK_INLINE zs::closure_object& as_closure() const noexcept {
    ZS_ASSERT(is_closure(), "Invalid closure type.");
    return *_closure;
  }

  ZB_CK_INLINE zs::string_object& as_string() const noexcept {
    ZS_ASSERT(is_long_string(), "Invalid long string type.");
    return *_lstring;
  }

  ZB_CK_INLINE zs::reference_counted_object& as_ref_counted() const noexcept {
    ZS_ASSERT(is_ref_counted(), "Invalid ref_counted type.");
    return *_ref_counted;
  }

  ZB_CK_INLINE zs::delegable_object& as_delegable() const noexcept {
    ZS_ASSERT(is_delegable(), "Invalid delegate type.");
    return *_delegable;
  }

  //
  // MARK: Type queries
  //

  ZS_CK_INLINE_CXPR object_type get_type() const noexcept;
  ZS_CK_INLINE_CXPR bool is_type(object_type t) const noexcept;

  template <class T, class... Args>
  ZS_CK_INLINE_CXPR bool is_type(T t, Args... args) const noexcept;

  template <class T, class... Args>
  ZS_CK_INLINE_CXPR bool is_not_type(T t, Args... args) const noexcept {
    return !is_type(t, args...);
  }

  ZS_CK_INLINE_CXPR bool has_type_mask(zs::object_type_mask mask) const noexcept;
  ZS_CK_INLINE_CXPR bool has_type_mask(uint32_t mask) const noexcept;

  /// Get the object_type mask.
  ZS_CK_INLINE_CXPR uint32_t get_type_mask() const noexcept;

  ZB_CK_INLINE_CXPR exposed_object_type get_exposed_type() const noexcept;
  object get_exposed_type_name() const noexcept;

  ZS_CK_INLINE_CXPR bool is_null() const noexcept;
  ZS_CK_INLINE_CXPR bool is_none() const noexcept;
  ZS_CK_INLINE_CXPR bool is_null_or_none() const noexcept;
  ZS_CK_INLINE_CXPR bool is_integer() const noexcept;
  ZS_CK_INLINE_CXPR bool is_char() const noexcept;
  ZS_CK_INLINE_CXPR bool is_float() const noexcept;
  ZS_CK_INLINE_CXPR bool is_bool() const noexcept;
  ZS_CK_INLINE_CXPR bool is_array() const noexcept;
  ZS_CK_INLINE_CXPR bool is_struct() const noexcept;
  ZS_CK_INLINE_CXPR bool is_struct_instance() const noexcept;
  ZS_CK_INLINE_CXPR bool is_table() const noexcept;
  ZS_CK_INLINE_CXPR bool is_user_data() const noexcept;
  ZS_CK_INLINE bool is_user_data(const user_data_content* content) const noexcept;
  ZS_CK_INLINE bool is_user_data(const object& uid) const noexcept;
  ZS_CK_INLINE_CXPR bool is_closure() const noexcept;
  ZS_CK_INLINE_CXPR bool is_instance() const noexcept;
  ZS_CK_INLINE_CXPR bool is_weak_ref() const noexcept;
  ZS_CK_INLINE_CXPR bool is_long_string() const noexcept;
  ZS_CK_INLINE_CXPR bool is_small_string() const noexcept;
  ZS_CK_INLINE_CXPR bool is_string_view() const noexcept;
  ZS_CK_INLINE_CXPR bool is_native_closure() const noexcept;
  ZS_CK_INLINE_CXPR bool is_native_function() const noexcept;

  ZS_CK_INLINE_CXPR bool is_string() const noexcept;
  ZS_CK_INLINE_CXPR bool is_cstring() const noexcept;
  ZS_CK_INLINE_CXPR bool is_number() const noexcept;
  ZS_CK_INLINE_CXPR bool is_number_or_bool() const noexcept;
  ZS_CK_INLINE_CXPR bool is_function() const noexcept;
  ZS_CK_INLINE_CXPR bool is_ref_counted() const noexcept;
  ZS_CK_INLINE_CXPR bool is_delegable() const noexcept;

  ZS_CK_INLINE_CXPR bool is_atom() const noexcept;
  ZS_CK_INLINE_CXPR bool is_atom(atom_type t) const noexcept;

  template <class... Args>
  ZS_CK_INLINE_CXPR bool is_atom(Args... args) const noexcept;

  ZS_CK_INLINE_CXPR bool is_meta_type() const noexcept;

  ZS_CK_INLINE_CXPR bool is_bool_convertible() const noexcept;
  ZS_CK_INLINE_CXPR bool is_integer_convertible() const noexcept;
  ZS_CK_INLINE_CXPR bool is_float_convertible() const noexcept;

  //
  // MARK: Flags
  //

  template <class Flags>
  ZS_CK_INLINE_CXPR bool has_flags(Flags flags) const noexcept {
    return zb::has_flag(_flags, (object_flags_t)flags);
  }

  template <class Flags>
  ZS_INLINE void set_flags(Flags flags) noexcept {
    zb::set_flag(_flags, (object_flags_t)flags);
  }

  template <class Flags>
  ZS_INLINE void set_flags(Flags flags, bool value) noexcept {
    zb::set_flag(_flags, (object_flags_t)flags, value);
  }

  template <class Flags>
  ZS_INLINE void remove_flags(Flags flags) noexcept {
    zb::remove_flag(_flags, (object_flags_t)flags);
  }

  //
  // MARK: Value getters
  //

  /// @{
  zs::error_result get_bool(bool_t& res) const noexcept;
  zs::error_result get_integer(int_t& res) const noexcept;
  zs::error_result get_float(float_t& res) const noexcept;
  zs::error_result get_string(std::string_view& res) const noexcept;
  /// @}

  //
  // MARK: Value unchecked getters
  //

  ZS_CK_INLINE_CXPR std::string_view get_small_string_unchecked() const noexcept {
    return std::string_view((const char*)&_lvalue);
  }

  ZS_CHECK zb::string_view get_long_string_unchecked() const noexcept;
  ZS_CHECK zb::string_view get_string_view_unchecked() const noexcept;

  ZS_CHECK zb::string_view get_string_unchecked() const noexcept;
  ZS_CHECK const char* get_cstring_unchecked() const noexcept;

  template <object_type ObjType>
  ZS_CHECK inline zb::string_view get_string_unchecked() const noexcept;

  template <object_type ObjType>
  ZS_CHECK inline const char* get_cstring_unchecked() const noexcept;

  /// @{
  ///
  /// Convert to.
  ZS_CHECK zs::error_result convert_to_bool(bool_t& v) const noexcept;
  ZS_CHECK zs::error_result convert_to_integer(int_t& v) const noexcept;
  ZS_CHECK zs::error_result convert_to_float(float_t& v) const noexcept;
  ZS_CHECK zs::error_result convert_to_string(zs::string& s) const;
  ZS_CHECK zs::error_result convert_to_string(std::string& s) const;
  ZS_CHECK zs::error_result convert_to_string_object(zs::engine* eng, zs::object& s) const;

  //
  // MARK: Stream.
  //

  using serializer_type = zs::serializer_type;
  //  enum class serializer_type { plain, quoted, json, json_compact, plain_compact };

private:
  ///
  //  template <class T, serializer_type SType>
  //  struct streamer_base {
  //    inline streamer_base(const T& o, int idt = 0)
  //        : obj(o)
  //        , indent(idt) {}
  //
  //    const T& obj;
  //    int indent;
  //  };

public:
  ZS_CHECK std::string to_json() const;
  ZS_CHECK std::string convert_to_string() const;

  ZS_CHECK zs::error_result to_json(zs::string& output) const;

  std::ostream& stream_to_json(std::ostream& s) const;
  std::ostream& stream_to_json_compact(std::ostream& s) const;
  std::ostream& stream_to_string(std::ostream& ss) const;
  std::ostream& stream(std::ostream& s) const;
  std::ostream& stream(serializer_type stype, std::ostream& s) const;

  using json_streamer = streamer<serializer_type::json>;
  using json_compact_streamer = streamer<serializer_type::json_compact>;
  using quoted_streamer = streamer<serializer_type::plain>;

  template <serializer_type SType>
  inline streamer<SType> stream_streamer() const& noexcept {
    return streamer<SType>{ *this };
  }

  inline quoted_streamer string_stream() const& noexcept { return quoted_streamer{ *this }; }
  inline json_streamer json_stream() const& noexcept { return json_streamer{ *this }; }
  inline json_compact_streamer json_compact_stream() const& noexcept {
    return json_compact_streamer{ *this };
  }

  inline quoted_streamer string_stream() const&& noexcept = delete;
  inline json_streamer json_stream() const&& noexcept = delete;
  inline json_compact_streamer json_compact_stream() const&& noexcept = delete;

  //
  //
  //
  ZS_CHECK bool_t convert_to_bool_unchecked() const noexcept;

  /// Converts a bool, an integer or a float to an integer, other wise returns 0.
  ZS_CHECK int_t convert_to_integer_unchecked() const noexcept;
  ZS_CK_INLINE int_t to_int() const noexcept { return convert_to_integer_unchecked(); }

  ZS_CHECK float_t convert_to_float_unchecked() const noexcept;
  ZS_CK_INLINE float_t to_float() const noexcept { return convert_to_float_unchecked(); }

  ZS_CHECK std::string to_debug_string() const;
  ZS_CHECK zs::error_result to_debug_string(zs::string& s) const;

  ZS_CHECK size_t get_ref_count() const noexcept;

  ZS_CHECK size_t hash() const noexcept;

  //
  // MARK: Table
  //

  ZS_CHECK zs::object_map* get_table_internal_map() const noexcept;

  //
  // MARK: Array
  //

  ZS_CHECK zs::vector<object>* get_array_internal_vector() const noexcept;

  /// Same as operator== but different type of numbers cannot be equal.
  ZS_CHECK bool strict_equal(const object_base& rhs) const noexcept;

  /// @{
  /// Different type of numbers can be equal.
  /// Different type of strings can be equal.
  /// Othewise if the types differs, the object_type is compared.
  /// If both types are the same their value gets compared.
  ZS_CHECK bool operator==(const object_base& rhs) const noexcept;
  ZS_CK_INLINE bool operator!=(const object_base& rhs) const noexcept { return !operator==(rhs); }
  ZS_CHECK bool operator<(const object_base& rhs) const noexcept;
  ZS_CHECK bool operator>(const object_base& rhs) const noexcept;
  ZS_CHECK bool operator<=(const object_base& rhs) const noexcept;
  ZS_CHECK bool operator>=(const object_base& rhs) const noexcept;
  /// @}

  ZS_CHECK bool operator==(std::string_view rhs) const noexcept;
  ZS_CK_INLINE bool operator!=(std::string_view rhs) const noexcept { return !operator==(rhs); }

  template <class BoolType, std::enable_if_t<std::is_same_v<BoolType, bool_t>, int> = 0>
  ZS_CK_INLINE bool operator==(BoolType rhs) const noexcept {
    bool_t v;
    return !convert_to_bool(v) && v == rhs;
  }

  template <class IntType,
      std::enable_if_t<std::is_integral_v<IntType> && !std::is_same_v<IntType, bool_t>, int> = 0>
  ZS_CK_INLINE bool operator==(IntType rhs) const noexcept {
    int_t v;
    return !convert_to_integer(v) && v == (int_t)rhs;
  }

  ZS_CK_INLINE bool operator==(float_t rhs) const noexcept {
    float_t v;
    return !convert_to_float(v) && v == rhs;
  }

  inline friend std::ostream& operator<<(std::ostream& stream, const zs::object_base& obj);
};

template <class Type, object_base::serializer_type StreamType>
std::ostream& operator<<(std::ostream& stream, const zs::streamer_base<Type, StreamType>& s);

ZB_CK_INLINE_CXPR exposed_object_type object_base::get_exposed_type() const noexcept {

  using enum object_type;
  using enum exposed_object_type;

  switch (_type) {
#define _X(name, str, exposed_name) \
  case ZS_OBJECT_TYPE_PREFIX(name): \
    return ke_##exposed_name;
    ZS_OBJECT_TYPE_ENUM(_X)
#undef _X
  }

  ZS_ERROR("invalid type");
  return exposed_object_type::ke_null;
}

ZS_CXPR object_type object_base::get_type() const noexcept { return _type; }
ZS_CXPR bool object_base::is_type(object_type t) const noexcept { return _type == t; }
ZS_CXPR bool object_base::is_null() const noexcept { return _type == k_null; }

ZS_CXPR bool object_base::is_integer() const noexcept { return _type == k_integer; }

ZS_CXPR bool object_base::is_char() const noexcept { return is_integer() and has_flags(f_char); }

ZS_CXPR bool object_base::is_float() const noexcept { return _type == k_float; }
ZS_CXPR bool object_base::is_bool() const noexcept { return _type == k_bool; }
ZS_CXPR bool object_base::is_array() const noexcept { return _type == k_array; }
ZS_CXPR bool object_base::is_struct() const noexcept { return _type == k_struct; }
ZS_CXPR bool object_base::is_struct_instance() const noexcept { return _type == k_struct_instance; }
ZS_CXPR bool object_base::is_table() const noexcept { return _type == k_table; }
ZS_CXPR bool object_base::is_user_data() const noexcept { return _type == k_user_data; }
ZS_CXPR bool object_base::is_closure() const noexcept { return _type == k_closure; }
ZS_CXPR bool object_base::is_weak_ref() const noexcept { return _type == k_weak_ref; }
ZS_CXPR bool object_base::is_long_string() const noexcept { return _type == k_long_string; }
ZS_CXPR bool object_base::is_small_string() const noexcept { return _type == k_small_string; }
ZS_CXPR bool object_base::is_string_view() const noexcept { return _type == k_string_view; }
ZS_CXPR bool object_base::is_native_closure() const noexcept { return _type == k_native_closure; }
ZS_CXPR bool object_base::is_native_function() const noexcept { return _type == k_native_function; }

ZS_CXPR bool object_base::is_string() const noexcept {
  return zs::get_object_type_mask(get_type()) & k_string_mask;
}

ZS_CXPR bool object_base::is_cstring() const noexcept {
  return zs::get_object_type_mask(get_type()) & k_cstring_mask;
}

ZS_CXPR bool object_base::is_number() const noexcept {
  return zs::get_object_type_mask(get_type()) & k_number_mask;
}

ZS_CXPR bool object_base::is_number_or_bool() const noexcept {
  return zs::get_object_type_mask(get_type()) & k_number_or_bool_mask;
}

ZS_CXPR bool object_base::is_function() const noexcept {
  return zs::get_object_type_mask(get_type()) & k_function_mask;
}

ZS_CXPR bool object_base::is_ref_counted() const noexcept { return _type >= k_long_string; }

ZS_CXPR bool object_base::is_delegable() const noexcept { return zs::is_object_type_delegable(get_type()); }

ZS_CXPR bool object_base::is_atom() const noexcept { return _type == k_atom; }

ZS_CXPR bool object_base::is_none() const noexcept { return _type == k_none; }

ZS_CXPR bool object_base::is_null_or_none() const noexcept { return _type == k_null or _type == k_none; }

ZS_CXPR bool object_base::is_meta_type() const noexcept { return has_type_mask(constants::k_meta_type_mask); }

ZS_CXPR bool object_base::is_bool_convertible() const noexcept {
  return has_type_mask(k_number_or_bool_mask);
}

ZS_CXPR bool object_base::is_integer_convertible() const noexcept {
  return has_type_mask(k_number_or_bool_mask);
}

ZS_CXPR bool object_base::is_float_convertible() const noexcept {
  return has_type_mask(k_number_or_bool_mask);
}

ZS_CXPR bool object_base::has_type_mask(zs::object_type_mask mask) const noexcept {
  return zs::get_object_type_mask(_type) & static_cast<uint32_t>(mask);
}

ZS_CXPR bool object_base::has_type_mask(uint32_t mask) const noexcept {
  return zs::get_object_type_mask(_type) & mask;
}

constexpr uint32_t object_base::get_type_mask() const noexcept { return zs::get_object_type_mask(_type); }

template <class T, class... Args>
ZS_CK_INLINE constexpr bool object_base::is_type(T t, Args... args) const noexcept {
  if constexpr (std::is_same_v<T, zs::object_type_mask>) {
    return has_type_mask(t) || (is_type(args) || ...);
  }
  else if constexpr (std::is_same_v<T, object_type>) {
    return is_type(t) || (is_type(args) || ...);
  }
  else {
    return has_type_mask((uint32_t)t) || (is_type(args) || ...);
  }
}

ZS_CK_INLINE_CXPR bool object_base::is_atom(atom_type t) const noexcept {
  return _type == k_atom and _atom_type == t;
}

template <class... Args>
ZS_CK_INLINE_CXPR bool object_base::is_atom(Args... args) const noexcept {
  return is_atom() and ([&](auto tt) { return tt == _atom_type; }(args) or ...);
}

template <object_type ObjType>
ZS_CHECK inline zb::string_view object_base::get_string_unchecked() const noexcept {
  if constexpr (ObjType == k_small_string) {
    return get_small_string_unchecked();
  }
  else if constexpr (ObjType == k_string_view) {
    return get_string_view_unchecked();
  }
  else if constexpr (ObjType == k_long_string) {
    return get_long_string_unchecked();
  }
  else {
    zb_static_error("not a string type");
    return {};
  }
}

template <object_type ObjType>
ZS_CHECK inline const char* object_base::get_cstring_unchecked() const noexcept {
  if constexpr (ObjType == k_small_string) {
    return get_small_string_unchecked().data();
  }
  else if constexpr (ObjType == k_long_string) {
    return get_long_string_unchecked().data();
  }
  else {
    zb_static_error("not a valid string type");
    return nullptr;
  }
}

ZS_CHECK zs::engine* get_engine(virtual_machine* v, bool) noexcept;

template <class T>
ZS_CK_INLINE zs::engine* get_engine(const T& v) noexcept {
  if constexpr (std::is_same_v<T, engine*>) {
    return v;
  }
  else if constexpr (std::is_same_v<std::remove_cvref_t<T>, engine>) {
    return (zs::engine*)&v;
  }
  else if constexpr (std::is_same_v<T, virtual_machine*>) {
    return zs::get_engine((virtual_machine*)v, true);
  }
  else if constexpr (std::is_same_v<std::remove_cvref_t<T>, virtual_machine>) {
    return zs::get_engine((virtual_machine*)&v, true);
  }
  else if constexpr (std::is_same_v<std::remove_cvref_t<T>, vm_ref>) {
    return v.get_engine();
  }
  else if constexpr (std::is_same_v<std::remove_cvref_t<T>, vm>) {
    return v.get_engine();
  }
  else {
    zb_static_error("wrong");
    return nullptr;
  }
}

#define __ZS_OBJECT_INITIALIZER_4(vtype, val, ktype, flags) \
  { .vtype = val }, ._ex1_u32 = 0, { ._ex2_u16 = 0 }, ._type = ktype, ._flags = flags

#define __ZS_OBJECT_INITIALIZER_3(vtype, val, ktype) __ZS_OBJECT_INITIALIZER_4(vtype, val, ktype, f_none)

#define ZS_OBJECT_INITIALIZER(...) \
  ZBASE_DEFER(ZBASE_CONCAT(__ZS_OBJECT_INITIALIZER_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)

#define ZS_OBJ_BASE_INIT(...) \
  object_base { ZS_OBJECT_INITIALIZER(__VA_ARGS__) }

struct function_parameter_interface;

/// @class object
/// @brief A smart pointer class that handles objects.
///
/// The object class provides functionalities for managing object
/// lifecycles, including creation, manipulation, and destruction.
///
class object : public object_base {

  ZS_INLINE_CXPR object(const object_base& objbase) noexcept
      : object_base(objbase) {}

public:
  ZS_CK_INLINE static const object& invalid_object() noexcept {
    static object invalid_obj;
    return invalid_obj;
  }

  /// @brief Default constructor for object.
  /// Initializes an empty object (null).
  ZS_INLINE_CXPR object() noexcept
      : object_base{ ._lvalue = 0ULL, ._rvalue = 0x1000000000000ULL } {}

  /// @brief Copy constructor for object.
  /// @param obj The object to copy from.
  ZS_INLINE_CXPR object(const object& obj) noexcept;

  ZS_INLINE_CXPR object(object&& obj) noexcept;

  ZS_INLINE_CXPR object(zs::engine*, const object& obj) noexcept
      : object(obj) {}

  ZS_INLINE_CXPR object(zs::engine*, object&& obj) noexcept
      : object(std::move(obj)) {}

  ZS_INLINE_CXPR explicit object(const object_base& obj, bool should_retain) noexcept;
  explicit object(reference_counted_object* ref_obj, bool should_retain) noexcept;

  //
  // MARK: Null.
  //

  ZS_INLINE_CXPR object(null_t v) noexcept
      : object() {}

  ZS_INLINE_CXPR object(zs::engine*, null_t v) noexcept
      : object() {}

  //
  // MARK: None.
  //

  ZS_INLINE_CXPR object(zs::none) noexcept
      : ZS_OBJ_BASE_INIT(_lvalue, 0, k_none) {}

  ZS_INLINE_CXPR object(zs::engine*, zs::none) noexcept
      : ZS_OBJ_BASE_INIT(_lvalue, 0, k_none) {}

  //
  // MARK: Bool.
  //

  template <class BoolType, std::enable_if_t<std::is_same_v<BoolType, bool_t>, int> = 0>
  ZS_INLINE_CXPR object(BoolType v) noexcept
      : ZS_OBJ_BASE_INIT(_int, (int_t)v, k_bool) {}

  //
  // MARK: Integer.
  //

  ZS_INLINE_CXPR object(int_t v) noexcept
      : ZS_OBJ_BASE_INIT(_int, v, k_integer) {}

  ZS_INLINE_CXPR object(zs::engine*, int_t v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(int32_t v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(zs::engine*, int32_t v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(uint32_t v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(zs::engine*, uint32_t v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(uint64_t v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(zs::engine*, uint64_t v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(long v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(zs::engine*, long v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(unsigned long v) noexcept
      : object((int_t)v) {}

  ZS_INLINE_CXPR object(zs::engine*, unsigned long v) noexcept
      : object((int_t)v) {}

  template <class CharT>
  ZS_CK_INLINE static object create_char(CharT c) noexcept {
    return ZS_OBJ_BASE_INIT(_int, (int_t)c, k_integer, f_char);
  }

  //
  // MARK: Float.
  //

  ZS_INLINE_CXPR object(float_t v) noexcept
      : ZS_OBJ_BASE_INIT(_float, v, k_float) {}

  ZS_INLINE_CXPR object(float v) noexcept
      : object((float_t)v) {}

  ZS_INLINE_CXPR object(zs::engine*, float_t v) noexcept
      : object((float_t)v) {}

  ZS_INLINE_CXPR object(zs::engine*, float v) noexcept
      : object((float_t)v) {}

  //
  // MARK: String.
  //

  object(zs::engine* eng, std::string_view v);
  ZS_INLINE object(zs::engine* eng, const char* v);
  ZS_INLINE object(zs::engine* eng, const std::string& v);
  ZS_INLINE object(zs::engine* eng, const zs::string& v);

  template <class CharType, size_t N, std::enable_if_t<std::is_same_v<CharType, char>, int> = 0>
  ZS_INLINE object(zs::engine* eng, const CharType (&v)[N]);

  template <size_t N>
  object(const char (&v)[N]) = delete;
  object(std::string_view v) = delete;
  object(const char* v) = delete;
  object(const std::string& v) = delete;
  object(const zs::string& v) = delete;

  ZS_CHECK static object create_concat_string(zs::engine* eng, std::string_view s1, std::string_view s2);

  //
  // MARK: Raw pointer.
  //

  template <class Ptr, std::enable_if_t<std::is_same_v<Ptr, void>, int> = 0>
  ZS_INLINE_CXPR explicit object(Ptr* ptr) noexcept
      : object_base{ ._pointer = ptr,
        ._ex1_u32 = 0,
        { ._ex2_delegate_id = 0, ._atom_type = atom_type::atom_pointer },
        ._type = k_atom,
        ._flags = f_none } {}

  template <class Ptr, std::enable_if_t<std::is_same_v<Ptr, void>, int> = 0>
  ZS_INLINE_CXPR explicit object(zs::engine*, Ptr* ptr) noexcept
      : object(ptr) {}

  template <class T>
  ZS_CK_INLINE static object create_pointer(T* ptr) noexcept {
    return object((void*)ptr);
  }

  template <class T>
  ZS_CK_INLINE static object create_pointer(zs::engine*, T* ptr) noexcept {
    return object((void*)ptr);
  }

  //
  // MARK: Native functions.
  //

  /// Create a native_function.
  ZS_INLINE_CXPR object(zs::function_t fct) noexcept;

  //
  // MARK: Native Closure.
  //

  ZS_CHECK static object create_native_closure(zs::engine* eng, zs::function_t nc);
  ZS_CHECK static object create_native_closure(zs::engine* eng, zs::native_closure* nc);

  template <class Fct>
  ZS_CHECK inline static object create_native_closure(zs::engine* eng, Fct&& fct);

  //
  // MARK: Closure.
  //

  ZS_CHECK static object create_closure(zs::engine* eng, const object& fct_prototype, const object& root);
  ZS_CHECK static object create_closure(zs::engine* eng, object&& fct_prototype, const object& root);

  //
  // MARK: User Data.
  //

  ZS_CHECK static object create_user_data(zs::engine* eng, size_t size);

  //
  // MARK: Struct.
  //

  ZS_CHECK static object create_struct(zs::engine* eng, size_t sz = 0);

  //
  // MARK: Table.
  //

  ZS_INLINE object(zs::engine* eng, std::initializer_list<std::pair<zs::object, zs::object>> list);

  ZS_INLINE object(zs::engine* eng, std::span<std::pair<zs::object, zs::object>> list);

  template <class MapType>
    requires zb::is_map_type_v<MapType>
  ZS_INLINE object(zs::engine* eng, const MapType& mt);

  template <class MapType>
    requires zb::is_map_type_v<MapType>
  ZS_INLINE object(zs::engine* eng, MapType&& mt);

  ZS_CHECK static object create_table(zs::engine* eng);

  ZS_CHECK static object create_table_with_delegate(zs::engine* eng, const zs::object& delegate);

  ZS_CHECK static object create_table_with_delegate(
      zs::engine* eng, const zs::object& delegate, delegate_flags_t flags);

  ZS_CHECK inline static object create_table(
      zs::engine* eng, std::initializer_list<std::pair<zs::object, zs::object>> list);

  ZS_CHECK inline static object create_table(
      zs::engine* eng, std::span<const std::pair<zs::object, zs::object>> list);

  template <class MapType>
    requires zb::is_map_type_v<MapType>
  ZS_CHECK inline static object create_table(zs::engine* eng, const MapType& mt);

  template <class MapType>
    requires zb::is_map_type_v<MapType>
  ZS_CHECK inline static object create_table(zs::engine* eng, MapType&& mt);

  //
  // MARK: Array.
  //

  template <class Container>
    requires zb::is_contiguous_container_not_string_v<Container>
  ZS_INLINE object(zs::engine* eng, const Container& container);

  ZS_INLINE object(zs::engine* eng, std::initializer_list<zs::object> list);
  ZS_INLINE object(zs::engine* eng, std::span<zs::object> list);
  ZS_INLINE object(zs::engine* eng, std::span<const zs::object> list);

  ZS_CHECK static object create_array(zs::engine* eng, size_t sz = 0);
  ZS_CHECK inline static object create_array(zs::engine* eng, std::initializer_list<zs::object> list);
  ZS_CHECK inline static object create_array(zs::engine* eng, std::span<zs::object> list);
  ZS_CHECK inline static object create_array(zs::engine* eng, std::span<const zs::object> list);

  template <class Container>
    requires zb::is_contiguous_container_not_string_v<Container>
  ZS_CHECK inline static object create_array(zs::engine* eng, const Container& container);

  /// @brief Destructor, release the object if it is reference counted.
  ZS_INLINE_CXPR ~object() noexcept {
    if (is_ref_counted()) {
      _ref_counted->release();
    }
  }

  object& operator=(const object& obj);
  object& operator=(object&& obj);

  //
  // MARK: Flags.
  //

  ZS_INLINE object& with_flags(object_flags_t flags) noexcept {
    zb::set_flag(_flags, flags);
    return *this;
  }

  ZS_INLINE object& with_flags(uint32_t flags) noexcept { return with_flags((object_flags_t)flags); }

  //
  //
  //

  ZS_CHECK bool is_if_true() const noexcept;
  ZS_CHECK bool is_double_question_mark_true() const noexcept;

  ZS_CK_INLINE explicit operator bool() const noexcept { return is_if_true(); }

  //
  // MARK: Delegate.
  //

  object& with_delegate(object delegate) noexcept;

  zs::error_result set_delegate(object delegate) noexcept;

  //
  // MARK: Function parameters.
  //

  ZS_CHECK function_parameter_interface get_parameter_interface() const noexcept;

  //
  // MARK: Weak reference.
  //

  /// Returns a weak_ref object for reference counted objects.
  /// Returns itself for non reference counted objects and for weak_ref objects.
  ZS_CHECK object get_weak_ref() const noexcept;
  ZS_CHECK object get_weak_ref_value() const noexcept;

  //
  // MARK: Reset.
  //

  void reset() noexcept;

  //
  // MARK: Access value.
  //

  template <class T>
  inline zs::error_result get_value(T& value) const noexcept;

private:
  struct check_null_tag {};
  ZS_INLINE_CXPR object(check_null_tag, object_base&& obase, bool _is_null) noexcept;

  ZS_CK_INLINE object_base& base() noexcept { return *this; }

  friend object _ss(std::string_view s) noexcept;
  friend constexpr object _sv(std::string_view s) noexcept;
};

//
// MARK: Variable info.
//

enum class variable_attribute_t : uint8_t { va_none, va_const = 1, va_static = 2, va_private = 4 };

struct variable_type_info {
  using enum variable_attribute_t;

  uint64_t custom_mask = 0;
  uint32_t mask = 0;
  variable_attribute_t flags = va_none;

  ZS_CK_INLINE_CXPR bool has_mask() const noexcept { return mask != 0 or custom_mask != 0; }
  ZS_CK_INLINE_CXPR bool has_custom_mask() const noexcept { return custom_mask != 0; }

  ZS_INLINE_CXPR void set_const(bool value) noexcept { zb::set_flag<va_const>(flags, value); }
  ZS_INLINE_CXPR void set_static(bool value) noexcept { zb::set_flag<va_static>(flags, value); }
  ZS_INLINE_CXPR void set_private(bool value) noexcept { zb::set_flag<va_private>(flags, value); }

  ZS_INLINE_CXPR void set_const() noexcept { zb::set_flag<va_const>(flags); }
  ZS_INLINE_CXPR void set_static() noexcept { zb::set_flag<va_static>(flags); }
  ZS_INLINE_CXPR void set_private() noexcept { zb::set_flag<va_private>(flags); }

  ZS_CK_INLINE_CXPR bool is_const() const noexcept { return zb::has_flag<va_const>(flags); }
  ZS_CK_INLINE_CXPR bool is_static() const noexcept { return zb::has_flag<va_static>(flags); }
  ZS_CK_INLINE_CXPR bool is_private() const noexcept { return zb::has_flag<va_private>(flags); }
};

struct named_variable_type_info : variable_type_info {
  object name;

  inline named_variable_type_info() noexcept = default;
  inline named_variable_type_info(const named_variable_type_info& vinfo) noexcept = default;

  inline named_variable_type_info(const object& name, uint64_t custom_mask = 0, uint32_t mask = 0,
      variable_attribute_t flags = va_none) noexcept
      : variable_type_info{ custom_mask, mask, flags }
      , name(name) {}

  ZS_CK_INLINE bool is_named() const noexcept { return name.is_string(); }

  ZS_CK_INLINE bool is_named(const object& rhs_name) const noexcept {
    return name.is_string() and name.get_string_unchecked() == rhs_name.get_string_unchecked();
  }
};

/// @brief Common function parameter interface for closure and native closure.
struct function_parameter_interface {
  ZS_CK_INLINE bool has_parameter_info() const noexcept { return _parameters_count != 0; }

  /// @brief The the number of parameters.
  ZS_CK_INLINE int_t get_parameters_count() const noexcept { return _parameters_count; }

  ZS_CK_INLINE int_t get_default_parameters_count() const noexcept { return _default_parameters_count; }

  ZS_CK_INLINE int_t get_minimum_required_parameters_count() const noexcept {
    return _parameters_count - _default_parameters_count;
  }

  ZS_CK_INLINE bool is_possible_parameter_count(int_t sz) const noexcept {
    if (!_default_parameters_count) {
      return sz == _parameters_count;
    }

    return sz >= (_parameters_count - _default_parameters_count) and sz <= _parameters_count;
  }

  ZS_CK_INLINE bool has_variadic_parameters() const noexcept { return _has_variadic_parameters; }

  ZS_CK_INLINE std::span<const zs::object> get_default_parameters() const noexcept {
    return _default_parameters;
  }

  ZS_CK_INLINE const zs::object* get_this() const noexcept { return _this_obj; }

  std::span<const zs::object> _default_parameters;
  const zs::object* _this_obj = nullptr;
  int_t _parameters_count = 0;
  int_t _default_parameters_count = 0;
  bool _has_variadic_parameters = false;
};

struct parameter_info {
  inline parameter_info(zs::object _name, bool opt = false, uint32_t _mask = -1)
      : name(_name)
      , mask(_mask)
      , optional(opt) {}

  zs::object name;
  uint32_t mask;
  bool optional = false;
};

struct var_info {
  inline var_info(zs::object _name, uint32_t _mask = -1)
      : name(_name)
      , mask(_mask) {}

  zs::object name;
  uint32_t mask;
};

//
// MARK: Create object helpers.
//

/// Create a small_string object.
ZS_CK_INLINE object _ss(std::string_view s) noexcept {
  ZS_ASSERT(s.size() <= constants::k_small_string_max_size, "invalid small_string size");
  object sobj(object_base{ { ._lvalue = 0 }, ._ex1_u32 = 0, { ._ex2_u16 = 0 },
      ._type = object_type::k_small_string, ._flags = object_flags_t::f_none });
  zb::memcpy(sobj._sbuffer, s.data(), zb::minimum(s.size(), constants::k_small_string_max_size));
  return sobj;
}

/// Create a small_string object.
template <size_t Size>
ZS_CK_INLINE static object _ss(const char (&s)[Size]) noexcept {
  static_assert(Size - 1 <= constants::k_small_string_max_size, "invalid small_string size");
  return zs::_ss(std::string_view(s));
}

/// Create a string object.
template <class EngineAccess>
ZS_CK_INLINE object _s(EngineAccess&& a_eng, std::string_view s) noexcept {
  return object(zs::get_engine(a_eng), s);
}

ZS_CK_INLINE_CXPR object _sv(std::string_view s) noexcept {
  using enum object_type;
  using enum object_flags_t;
  return object_base{ { ._sview = s.data() }, ._ex1_u32 = (uint32_t)s.size(),

    { ._ex2_u16 = 0 }, ._type = k_string_view, ._flags = f_none };
}

/// Create a closure object.
template <class EngineAccess, class FpoObj>
  requires std::is_same_v<object, std::remove_cvref_t<FpoObj>>
ZS_CK_INLINE object _c(EngineAccess&& a_eng, FpoObj&& fct_prototype, const object& root) noexcept {
  return object::create_closure(zs::get_engine(a_eng), std::forward<FpoObj>(fct_prototype), root);
}

/// Create a native_closure object.
template <class T, class EngineAccess>
ZS_CK_INLINE object _nc(EngineAccess&& a_eng, T&& t) {
  return object::create_native_closure(zs::get_engine(a_eng), std::forward<T>(t));
}

template <class EngineAccess>
ZS_CK_INLINE object _t(EngineAccess&& a_eng) noexcept {
  return object::create_table(zs::get_engine(a_eng));
}

template <class EngineAccess>
ZS_CK_INLINE object _t(
    EngineAccess&& a_eng, std::initializer_list<std::pair<zs::object, zs::object>> list) noexcept {
  return object::create_table(zs::get_engine(a_eng), list);
}

template <class EngineAccess>
ZS_CK_INLINE object _a(EngineAccess&& a_eng, size_t sz) noexcept {
  return object::create_array(zs::get_engine(a_eng), sz);
}

template <class EngineAccess>
ZS_CK_INLINE object _a(EngineAccess&& a_eng, std::initializer_list<zs::object> list) noexcept {
  return object::create_array(zs::get_engine(a_eng), list);
}

template <class EngineAccess, class Container>
  requires zb::is_contiguous_container_not_string_v<Container>
ZS_CK_INLINE object _a(EngineAccess&& a_eng, const Container& container) noexcept {
  return object::create_array(zs::get_engine(a_eng), container);
}

template <class... Args>
ZS_CK_INLINE object _o(engine* eng, Args&&... args) noexcept {
  return object(eng, std::forward<Args>(args)...);
}

ZS_CK_INLINE object _u(zs::engine* eng, size_t size) { return zs::object::create_user_data(eng, size); }

// template <class T, class... Args>
// ZS_CK_INLINE object _u(zs::engine* eng, Args&&... args) {
//   return zs::object::create_user_data<T>(eng, std::forward<Args>(args)...);
// }

ZS_CK_INLINE object _nf(zs::function_t fct) noexcept { return object(fct); }

namespace literals {
  inline constexpr zs::object operator""_ss(const char* str, size_t) noexcept { return zs::_ss(str); }
} // namespace literals.

} // namespace zs.

//
// MARK: Objects.
//

#define ZS_SCRIPT_INCLUDE_OBJECTS 1
#include <zscript/detail/objects/delegate.h>
#include <zscript/detail/objects/weak_ref.h>
#include <zscript/detail/objects/table.h>
#include <zscript/detail/objects/array.h>
#include <zscript/detail/objects/struct.h>
#include <zscript/detail/objects/string.h>
#include <zscript/detail/objects/user_data.h>
#include <zscript/detail/objects/closure.h>
#include <zscript/detail/objects/native_closure.h>
#include <zscript/detail/objects/capture.h>
#undef ZS_SCRIPT_INCLUDE_OBJECTS

//
// MARK: Implementation.
//

namespace zs {

bool object_base::is_user_data(const user_data_content* content) const noexcept {
  return is_user_data() and as_udata().get_content() == content;
}

bool object_base::is_user_data(const object& uid) const noexcept {
  return is_user_data() and as_udata().get_uid() == uid;
}

ZS_CXPR object::object(const object& obj) noexcept
    : object_base((const object_base&)obj) {
  if (is_ref_counted()) {
    _ref_counted->retain();
  }
}

ZS_CXPR object::object(object&& obj) noexcept
    : object_base((const object_base&)obj) {
  ::memset(&obj, 0, sizeof(obj));
  obj._type = k_null;
}

ZS_CXPR object::object(const object_base& obj, bool should_retain) noexcept
    : object_base(obj) {

  if (should_retain && is_ref_counted()) {
    _ref_counted->retain();
  }
}

object::object(zs::engine* eng, const char* v)
    : object(eng, std::string_view(v)) {}

object::object(zs::engine* eng, const std::string& v)
    : object(eng, std::string_view(v)) {}

object::object(zs::engine* eng, const zs::string& v)
    : object(eng, std::string_view(v)) {}

template <class CharType, size_t N, std::enable_if_t<std::is_same_v<CharType, char>, int>>
object::object(zs::engine* eng, const CharType (&v)[N])
    : object(eng, std::string_view(v)) {}

object::object(zs::engine* eng, std::initializer_list<std::pair<zs::object, zs::object>> list)
    : object(create_table(eng, list)) {}

object::object(zs::engine* eng, std::span<std::pair<zs::object, zs::object>> list)
    : object(create_table(eng, list)) {}

template <class MapType>
  requires zb::is_map_type_v<MapType>
object::object(zs::engine* eng, const MapType& mt)
    : object(create_table(eng, mt)) {}

template <class MapType>
  requires zb::is_map_type_v<MapType>
object::object(zs::engine* eng, MapType&& mt)
    : object(create_table(eng, std::forward<MapType>(mt))) {}

template <class Container>
  requires zb::is_contiguous_container_not_string_v<Container>
object::object(zs::engine* eng, const Container& container)
    : object(create_array(eng, container)) {}

object::object(zs::engine* eng, std::initializer_list<zs::object> list)
    : object(create_array(eng, list)) {}

object::object(zs::engine* eng, std::span<zs::object> list)
    : object(create_array(eng, list)) {}

object::object(zs::engine* eng, std::span<const zs::object> list)
    : object(create_array(eng, list)) {}

ZS_CXPR object::object(zs::function_t fct) noexcept
    : object(check_null_tag{}, ZS_OBJ_BASE_INIT(_nfct, fct, k_native_function), fct == nullptr) {}

object object::create_table(zs::engine* eng, std::initializer_list<std::pair<zs::object, zs::object>> list) {
  return create_table(eng, std::span<const std::pair<zs::object, zs::object>>(&(*list.begin()), list.size()));
}

object object::create_table(zs::engine* eng, std::span<const std::pair<zs::object, zs::object>> list) {

  object table = create_table(eng);
  auto& map = *table.get_table_internal_map();

  for (auto it = list.begin(); it != list.end(); ++it) {
    map[it->first] = it->second;
  }

  return table;
}

template <class MapType>
  requires zb::is_map_type_v<MapType>
object object::create_table(zs::engine* eng, const MapType& mt) {

  //    using key_type = typename MapType::key_type;
  //    using mapped_type = typename MapType::mapped_type;

  object table = create_table(eng);
  auto& map = *table.get_table_internal_map();

  for (auto it : mt) {
    map[zs::var(eng, it.first)] = zs::var(eng, it.second);
  }

  return table;
}

template <class MapType>
  requires zb::is_map_type_v<MapType>
object object::create_table(zs::engine* eng, MapType&& mt) {

  //    using key_type = typename MapType::key_type;
  //    using mapped_type = typename MapType::mapped_type;

  object table = create_table(eng);
  auto& map = *table.get_table_internal_map();

  for (auto it : mt) {
    map[zs::var(eng, std::move(it.first))] = zs::var(eng, std::move(it.second));
  }

  return table;
}

//
//
//

template <class Container>
  requires zb::is_contiguous_container_not_string_v<Container>
object object::create_array(zs::engine* eng, const Container& container) {
  const size_t sz = container.size();
  object obj = create_array(eng, sz);
  auto& vec = *obj.get_array_internal_vector();

  using containter_value_type = zb::container_value_type_t<Container>;

  if constexpr (std::is_same_v<object, containter_value_type>) {

    for (size_t i = 0; i < sz; i++) {
      vec[i] = container[i];
    }
  }
  else {

    for (size_t i = 0; i < sz; i++) {
      vec[i] = object(eng, container[i]);
    }
  }

  return obj;
}

object object::create_array(zs::engine* eng, std::initializer_list<zs::object> list) {
  const size_t sz = list.size();
  object obj = create_array(eng, sz);
  auto& vec = *obj.get_array_internal_vector();
  size_t i = 0;
  for (auto it = list.begin(); it != list.end(); ++it) {
    vec[i++] = std::move(*it);
  }

  return obj;
}

object object::create_array(zs::engine* eng, std::span<zs::object> list) {
  const size_t sz = list.size();
  object obj = create_array(eng, sz);
  auto& vec = *obj.get_array_internal_vector();
  size_t i = 0;
  for (auto it = list.begin(); it != list.end(); ++it) {
    vec[i++] = std::move(*it);
  }

  return obj;
}

object object::create_array(zs::engine* eng, std::span<const zs::object> list) {
  const size_t sz = list.size();
  object obj = create_array(eng, sz);
  auto& vec = *obj.get_array_internal_vector();
  size_t i = 0;
  for (auto it = list.begin(); it != list.end(); ++it) {
    vec[i++] = *it;
  }

  return obj;
}

ZS_INLINE_CXPR object::object(object::check_null_tag, object_base&& obase, bool _is_null) noexcept
    : object_base(_is_null ? ZS_OBJ_BASE_INIT(_lvalue, 0, k_null) : obase) {}

template <class T>
inline zs::error_result object::get_value(T& value) const noexcept {

  using value_type = std::remove_cvref_t<T>;

  if constexpr (std::is_same_v<T, object>) {
    value = *this;
    return {};
  }
  else if constexpr (std::is_same_v<bool_t, value_type>) {
    return get_bool(value);
  }
  else if constexpr (std::is_same_v<std::nullptr_t, value_type>) {
    return {};
  }
  else if constexpr (std::is_same_v<std::string, T>) {
    std::string_view s;
    if (auto err = get_string(s)) {
      value = convert_to_string();
      return {};
    }

    value = value_type(s);
    return {};
  }
  else if constexpr (std::is_constructible_v<std::string_view, decltype(std::forward<T>(value))>) {
    return get_string(value);
  }
  else if constexpr (std::is_floating_point_v<value_type>) {
    float_t f = 0;

    if (auto err = get_float(f)) {
      return err;
    }

    value = (value_type)f;
    return {};
  }
  else if constexpr (std::is_integral_v<value_type>) {
    int_t i = 0;

    if (auto err = get_integer(i)) {
      return err;
    }

    value = (value_type)i;
    return {};
  }
  else if constexpr (zb::is_contiguous_container_v<value_type>) {
    if (get_type() != k_array) {
      return zs::error_code::invalid_type;
    }
    using containter_value_type = zb::container_value_type_t<value_type>;

    if constexpr (zb::has_push_back_v<value_type>) {

      if constexpr (zb::is_string_view_convertible_v<containter_value_type>) {

        const auto& vec = *get_array_internal_vector();
        for (size_t i = 0; i < vec.size(); i++) {
          containter_value_type val;
          if (auto err = vec[i].get_value<containter_value_type>(val)) {
            return err;
          }
          value.push_back(val);
        }
      }
      else {

        const auto& vec = *get_array_internal_vector();
        for (size_t i = 0; i < vec.size(); i++) {
          containter_value_type val;
          if (auto err = vec[i].get_value<containter_value_type>(val)) {
            return err;
          }
          value.push_back(val);
        }
      }

      return {};
    }
    else if constexpr (zb::is_array_v<value_type>) {

      const auto& vec = *get_array_internal_vector();

      if (zb::array_size_v<value_type> >= vec.size()) {
        for (size_t i = 0; i < vec.size(); i++) {
          containter_value_type val;
          if (auto err = vec[i].get_value<containter_value_type>(val)) {
            return err;
          }

          value[i] = val;
        }

        return {};
      }
      else {
        for (size_t i = 0; i < zb::array_size_v<value_type>; i++) {
          containter_value_type val;
          if (auto err = vec[i].get_value<containter_value_type>(val)) {
            return err;
          }

          value[i] = val;
        }

        return {};
      }
    }
    else {
      zb_static_error("invalid type no push_back");
      return zs::error_code::invalid_type;
    }
    return {};
  }

  else if constexpr (zb::is_map_type_v<value_type>) {
    if (get_type() != k_table) {
      return zs::error_code::invalid_type;
    }

    using key_type = typename value_type::key_type;
    using mapped_type = typename value_type::mapped_type;

    const auto& map = *get_table_internal_map();
    for (auto obj : map) {
      key_type kval;
      if (auto err = obj.first.get_value<key_type>(kval)) {
        return err;
      }

      mapped_type val;
      if (auto err = obj.second.get_value<mapped_type>(val)) {
        return err;
      }

      value[kval] = val;
    }
    return {};
  }

  else {
    //    if (get_type() == k_user_data) {
    //      return copy_user_data_to_type((void*)&value, sizeof(T), typeid(T).name());
    //    }

    return zs::error_code::invalid_type;
  }
}

} // namespace zs.
