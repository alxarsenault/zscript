#pragma once

#include <zscript/core/common.h>
#include <zscript/core/memory.h>

namespace zs {
enum class serializer_type { plain, quoted, json, json_compact, plain_compact };

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

  virtual int_t call(virtual_machine*) = 0;

protected:
  virtual ~native_closure() override = default;
};

class closure_object_t {
public:
  inline zs::object call(zs::vm_ref vm, zb::span<const object> params);
  inline zs::object call(zs::vm_ref vm, std::initializer_list<const object> params);
  closure_t fct;
};

/// Object.
struct object_base {
  using enum object_type;

  ZBASE_PRAGMA_PUSH()
  ZBASE_CLANG_DIAGNOSTIC(ignored, "-Wnested-anon-types")
  ZBASE_CLANG_DIAGNOSTIC(ignored, "-Wgnu-anonymous-struct")

  /// Data part (8 bytes).
  union {
    uint64_t _value;
    zs::table_object* _table;
    zs::class_object* _class;
    zs::class_instance_object* _instance;
    zs::array_object* _array;
    zs::struct_object* _struct;
    zs::struct_instance_object* _struct_instance;
    zs::native_array_object_interface* _native_array_interface;
    zs::string_object* _lstring;
    zs::mutable_string_object* _mstring;
    zs::closure_object* _closure;
    zs::native_closure_object* _native_closure;
    zs::user_data_object* _udata;
    zs::function_prototype_object* _fproto;
    zs::reference_counted_object* _ref_counted;
    zs::weak_ref_object* _weak_ref;
    zs::delegate_object* _odelegate;
    zs::node_object* _node;
    zs::native_cpp_closure_t _nfct;
    zs::closure_t _fct;
    zs::closure_object_t _cfct;
    void* _pointer;
    const char* _sview;
    int_t _int;
    float_t _float;
    alignas(uint64_t) bool_t _bool;
    alignas(uint64_t) uint32_t _color;
    alignas(uint64_t) zs::error_code _error;
    alignas(uint64_t) zs::object* _array_it;
    zb::aligned_type_storage<zs::object_unordered_map<zs::object>::iterator> _table_it;
    alignas(uint64_t) char _sbuffer[8];
  };

  /// Metadata part (8 bytes).
  union {
    struct {
      union {
        /// Extra part 1 (4 bytes).
        uint32_t _reserved_u32;
        uint32_t _sview_size;
        int32_t _reserved_i32;
        float _reserved_f32;
      };

      /// Extra part 2 (2 bytes).
      union {
        uint16_t _reserved_u16;

        /// Native function extra.
        struct {
          bool _nf_is_ref_counted;
          uint8_t _nf_reserved;
        };

        /// Native array extra.
        struct {
          uint8_t _na_reserved;
          native_array_type _na_type;
        };

        /// Extension extra.
        struct {
          uint8_t _ext_reserved;
          extension_type _ext_type;
        };
      };

      /// These two are mendatory for all types.
      object_type _type;
      object_flags_t _flags;
    };

    uint64_t _ext;
  };

  ZBASE_PRAGMA_POP()

  template <class T>
  ZB_CK_INLINE zs::native_array_object<T>*& _native_array() const noexcept {
    return (zs::native_array_object<T>*&)_pointer;
  }

  ZB_CK_INLINE zs::array_object& as_array() const noexcept { return *_array; }
  ZB_CK_INLINE zs::struct_object& as_struct() const noexcept { return *_struct; }
  ZB_CK_INLINE zs::struct_instance_object& as_struct_instance() const noexcept { return *_struct_instance; }
  ZB_CK_INLINE zs::table_object& as_table() const noexcept { return *_table; }
  ZB_CK_INLINE zs::class_object& as_class() const noexcept { return *_class; }
  ZB_CK_INLINE zs::class_instance_object& as_instance() const noexcept { return *_instance; }
  ZB_CK_INLINE zs::user_data_object& as_udata() const noexcept { return *_udata; }
  ZB_CK_INLINE zs::native_closure_object& as_native_closure() const noexcept { return *_native_closure; }
  ZB_CK_INLINE zs::closure_object& as_closure() const noexcept { return *_closure; }
  ZB_CK_INLINE zs::string_object& as_string() const noexcept { return *_lstring; }
  ZB_CK_INLINE zs::mutable_string_object& as_mutable_string() const noexcept { return *_mstring; }
  ZB_CK_INLINE zs::function_prototype_object& as_proto() const noexcept { return *_fproto; }
  ZB_CK_INLINE zs::node_object& as_node() const noexcept { return *_node; }
  ZB_CK_INLINE zs::reference_counted_object& as_ref_counted() const noexcept { return *_ref_counted; }
  ZB_CK_INLINE zs::delegate_object& as_delegate() const noexcept { return *_odelegate; }
  ZB_CK_INLINE zs::native_array_object_interface& as_native_array_interface() const noexcept {
    return *_native_array_interface;
  }

  template <class T>
  ZB_CK_INLINE zs::native_array_object<T>& as_native_array() const noexcept {
    constexpr native_array_type native_type = to_native_array_type<T>();
    static_assert(native_type != native_array_type::n_invalid, "invalid type");
    return *((zs::native_array_object<T>*)_pointer);
  }

  template <native_array_type Type>
  ZB_CK_INLINE zs::native_array_object<typename native_array_type_t<Type>::type>&
  as_native_array() const noexcept {
    static_assert(Type != native_array_type::n_invalid, "invalid type");
    return *((zs::native_array_object<typename native_array_type_t<Type>::type>*)_pointer);
  }

  static constexpr const uint32_t k_string_mask = zs::constants::k_string_mask;
  static constexpr const uint32_t k_number_mask = zs::constants::k_number_mask;
  static constexpr const uint32_t k_number_or_bool_mask = zs::constants::k_number_or_bool_mask;
  static constexpr const uint32_t k_function_mask = zs::constants::k_function_mask;

  //
  // MARK: Type queries
  //

  ZS_CK_INLINE_CXPR object_type get_type() const noexcept;
  ZS_CK_INLINE_CXPR bool is_type(object_type t) const noexcept;
  ZS_CK_INLINE_CXPR bool is_null() const noexcept;
  ZS_CK_INLINE_CXPR bool is_none() const noexcept;
  ZS_CK_INLINE_CXPR bool is_integer() const noexcept;
  ZS_CK_INLINE_CXPR bool is_float() const noexcept;
  ZS_CK_INLINE_CXPR bool is_bool() const noexcept;
  ZS_CK_INLINE_CXPR bool is_error() const noexcept;
  ZS_CK_INLINE_CXPR bool is_array() const noexcept;
  ZS_CK_INLINE_CXPR bool is_struct() const noexcept;
  ZS_CK_INLINE_CXPR bool is_struct_instance() const noexcept;
  ZS_CK_INLINE_CXPR bool is_native_array() const noexcept;
  ZS_CK_INLINE_CXPR bool is_table() const noexcept;
  ZS_CK_INLINE_CXPR bool is_user_data() const noexcept;
  ZS_CK_INLINE_CXPR bool is_closure() const noexcept;
  ZS_CK_INLINE_CXPR bool is_class() const noexcept;
  ZS_CK_INLINE_CXPR bool is_instance() const noexcept;
  ZS_CK_INLINE_CXPR bool is_weak_ref() const noexcept;
  ZS_CK_INLINE_CXPR bool is_long_string() const noexcept;
  ZS_CK_INLINE_CXPR bool is_small_string() const noexcept;
  ZS_CK_INLINE_CXPR bool is_string_view() const noexcept;
  ZS_CK_INLINE_CXPR bool is_mutable_string() const noexcept;
  ZS_CK_INLINE_CXPR bool is_native_closure() const noexcept;
  ZS_CK_INLINE_CXPR bool is_native_function() const noexcept;
  ZS_CK_INLINE_CXPR bool is_raw_pointer() const noexcept;
  ZS_CK_INLINE_CXPR bool is_node() const noexcept;
  ZS_CK_INLINE_CXPR bool is_function_prototype() const noexcept;
  ZS_CK_INLINE_CXPR bool is_string() const noexcept;
  ZS_CK_INLINE_CXPR bool is_number() const noexcept;
  ZS_CK_INLINE_CXPR bool is_number_or_bool() const noexcept;
  ZS_CK_INLINE_CXPR bool is_function() const noexcept;
  ZS_CK_INLINE_CXPR bool is_ref_counted() const noexcept;
  ZS_CK_INLINE_CXPR bool is_delegable() const noexcept;
  ZS_CK_INLINE_CXPR bool is_extension() const noexcept;
  ZS_CK_INLINE_CXPR bool is_color() const noexcept;
  ZS_CK_INLINE_CXPR bool is_array_iterator() const noexcept;
  ZS_CK_INLINE_CXPR bool is_table_iterator() const noexcept;
  ZS_CK_INLINE_CXPR bool is_enum() const noexcept;
  ZS_CK_INLINE_CXPR bool has_type_mask(zs::object_type_mask mask) const noexcept;
  ZS_CK_INLINE_CXPR bool has_type_mask(uint32_t mask) const noexcept;
  ZS_CK_INLINE_CXPR bool is_type_info() const noexcept;

  ZS_CK_INLINE_CXPR bool is_meta_argument() const noexcept;

  /// Get the object_type mask.
  ZS_CK_INLINE_CXPR uint32_t get_type_mask() const noexcept;

  template <class T, class... Args>
  ZS_CK_INLINE_CXPR bool is_type(T t, Args... args) const noexcept;

  ZS_CK_INLINE_CXPR bool is_bool_convertible() const noexcept;
  ZS_CK_INLINE_CXPR bool is_integer_convertible() const noexcept;
  ZS_CK_INLINE_CXPR bool is_float_convertible() const noexcept;

  ZS_CK_INLINE_CXPR bool has_flags(object_flags_t flags) const noexcept { return (_flags & flags) == flags; }
  ZS_CK_INLINE_CXPR bool has_flags(uint32_t flags) const noexcept { return has_flags((object_flags_t)flags); }

  inline void set_flags(object_flags_t flags) noexcept { _flags |= flags; }
  inline void set_flags(uint32_t flags) noexcept { set_flags((object_flags_t)flags); }

  inline void remove_flags(object_flags_t flags) noexcept { _flags &= ~flags; }
  inline void remove_flags(uint32_t flags) noexcept { remove_flags((object_flags_t)flags); }

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
    return std::string_view((const char*)&_value);
  }

  ZS_CHECK std::string_view get_long_string_unchecked() const noexcept;
  ZS_CHECK std::string_view get_string_view_unchecked() const noexcept;
  ZS_CHECK std::string_view get_string_unchecked() const noexcept;
  ZS_CHECK std::string_view get_mutable_string_unchecked() const noexcept;

  template <object_type ObjType>
  ZS_CHECK inline std::string_view get_string_unchecked() const noexcept;

  /// @{
  ///
  /// Convert to.
  ZS_CHECK zs::error_result convert_to_bool(bool_t& v) const noexcept;
  ZS_CHECK zs::error_result convert_to_integer(int_t& v) const noexcept;
  ZS_CHECK zs::error_result convert_to_float(float_t& v) const noexcept;
  ZS_CHECK zs::error_result convert_to_string(zs::string& s) const;
  ZS_CHECK zs::error_result convert_to_string(std::string& s) const;

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

  ZS_CHECK zs::object_unordered_map<object>* get_table_internal_map() const noexcept;

  //
  // MARK: Array
  //

  ZS_CHECK zs::vector<object>* get_array_internal_vector() const noexcept;

  //
  // MARK: Type info
  //

  ZS_CK_INLINE_CXPR bool is_null_type_info() const noexcept {
    return is_type_info() && _type == k_integer && _int == (int_t)k_null;
  }

  ZS_CK_INLINE_CXPR bool is_bool_type_info() const noexcept {
    return is_type_info() && _type == k_integer && _int == (int_t)k_bool;
  }

  ZS_CK_INLINE_CXPR bool is_int_type_info() const noexcept {
    return is_type_info() && _type == k_integer && _int == (int_t)k_integer;
  }

  ZS_CK_INLINE_CXPR bool is_float_type_info() const noexcept {
    return is_type_info() && _type == k_integer && _int == (int_t)k_float;
  }

  ZS_CK_INLINE_CXPR bool is_string_type_info() const noexcept {
    return is_type_info() && _type == k_integer && ((_int & (int_t)k_string_mask) != 0);
  }

  ZS_CK_INLINE_CXPR bool is_custom_type_info() const noexcept { return is_type_info() && is_string(); }

  ZS_CK_INLINE_CXPR bool is_multi_custom_type_info() const noexcept { return is_type_info() && is_array(); }

  ZS_CHECK int compare(const object_base& rhs) const noexcept;

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

ZS_CXPR object_type object_base::get_type() const noexcept { return _type; }
ZS_CXPR bool object_base::is_type(object_type t) const noexcept { return _type == t; }
ZS_CXPR bool object_base::is_null() const noexcept { return _type == k_null; }
ZS_CXPR bool object_base::is_none() const noexcept { return _type == k_none; }
ZS_CXPR bool object_base::is_integer() const noexcept { return _type == k_integer; }
ZS_CXPR bool object_base::is_float() const noexcept { return _type == k_float; }
ZS_CXPR bool object_base::is_bool() const noexcept { return _type == k_bool; }
ZS_CXPR bool object_base::is_error() const noexcept { return _type == k_error; }
ZS_CXPR bool object_base::is_array() const noexcept { return _type == k_array; }
ZS_CXPR bool object_base::is_struct() const noexcept { return _type == k_struct; }
ZS_CXPR bool object_base::is_struct_instance() const noexcept { return _type == k_struct_instance; }

ZS_CXPR bool object_base::is_native_array() const noexcept {
  return _type == k_native_array and _na_type != native_array_type::n_invalid;
}
ZS_CXPR bool object_base::is_table() const noexcept { return _type == k_table; }
ZS_CXPR bool object_base::is_user_data() const noexcept { return _type == k_user_data; }
ZS_CXPR bool object_base::is_closure() const noexcept { return _type == k_closure; }
ZS_CXPR bool object_base::is_class() const noexcept { return _type == k_class; }
ZS_CXPR bool object_base::is_instance() const noexcept { return _type == k_instance; }
ZS_CXPR bool object_base::is_weak_ref() const noexcept { return _type == k_weak_ref; }
ZS_CXPR bool object_base::is_long_string() const noexcept { return _type == k_long_string; }
ZS_CXPR bool object_base::is_small_string() const noexcept { return _type == k_small_string; }
ZS_CXPR bool object_base::is_string_view() const noexcept { return _type == k_string_view; }

ZS_CXPR bool object_base::is_mutable_string() const noexcept { return _type == k_mutable_string; }

ZS_CXPR bool object_base::is_native_closure() const noexcept { return _type == k_native_closure; }
ZS_CXPR bool object_base::is_native_function() const noexcept { return _type == k_native_function; }
ZS_CXPR bool object_base::is_node() const noexcept { return _type == k_node; }

ZS_CXPR bool object_base::is_raw_pointer() const noexcept { return _type == k_raw_pointer; }

ZS_CXPR bool object_base::is_function_prototype() const noexcept { return _type == k_function_prototype; }

ZS_CXPR bool object_base::is_string() const noexcept {
  return zs::get_object_type_mask(get_type()) & k_string_mask;
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

ZS_CXPR bool object_base::is_ref_counted() const noexcept {
  return _type >= k_long_string or (_type == object_type::k_native_function2 and _nf_is_ref_counted);
}

ZS_CXPR bool object_base::is_delegable() const noexcept { return zs::is_object_type_delegable(get_type()); }

ZS_CXPR bool object_base::is_extension() const noexcept { return _type == k_extension; }

ZS_CXPR bool object_base::is_color() const noexcept {
  return is_extension() and _ext_type == extension_type::kext_color;
}

ZS_CXPR bool object_base::is_array_iterator() const noexcept {
  return is_extension() and _ext_type == extension_type::kext_array_iterator;
}
ZS_CXPR bool object_base::is_table_iterator() const noexcept {
  return is_extension() and _ext_type == extension_type::kext_table_iterator;
}

ZS_CXPR bool object_base::is_enum() const noexcept {
  return is_table() and (_flags & object_flags_t::enum_table) != 0;
}

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

ZS_CXPR bool object_base::is_type_info() const noexcept {
  return (_flags & object_flags_t::type_info) == object_flags_t::type_info;
}

ZS_CXPR bool object_base::is_meta_argument() const noexcept {
  return (_flags & object_flags_t::meta_argument) == object_flags_t::meta_argument;
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

template <object_type ObjType>
ZS_CHECK inline std::string_view object_base::get_string_unchecked() const noexcept {
  if constexpr (ObjType == k_small_string) {
    return get_small_string_unchecked();
  }
  else if constexpr (ObjType == k_string_view) {
    return get_string_view_unchecked();
  }
  else if constexpr (ObjType == k_long_string) {
    return get_long_string_unchecked();
  }
  else if constexpr (ObjType == k_mutable_string) {
    return get_mutable_string_unchecked();
  }
  else {
    zb_static_error("not a string type");
    return {};
  }
}
} // namespace zs.
