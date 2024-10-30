#pragma once

#include <zscript/core/common.h>
#include <zscript/core/object_base.h>

namespace zs {

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
  { .vtype = val }, {                                       \
    { { 0 }, { 0 }, object_type::ktype, flags }             \
  }

#define __ZS_OBJECT_INITIALIZER_3(vtype, val, ktype) \
  __ZS_OBJECT_INITIALIZER_4(vtype, val, ktype, object_flags_t::none)

#define ZS_OBJECT_INITIALIZER(...) \
  ZBASE_DEFER(ZBASE_CONCAT(__ZS_OBJECT_INITIALIZER_, ZBASE_NARG(__VA_ARGS__)), __VA_ARGS__)

/// @class object
/// @brief A smart pointer class that handles objects.
///
/// The object class provides functionalities for managing object
/// lifecycles, including creation, manipulation, and destruction.
///
class object : public object_base {
public:
  /// @brief Default constructor for object.
  /// Initializes an empty object (null).
  ZS_INLINE_CXPR object() noexcept;

  /// @brief Copy constructor for object.
  /// @param obj The object to copy from.
  ZS_INLINE object(const object& obj) noexcept;

  ZS_INLINE_CXPR object(object&& obj) noexcept;

  ZS_INLINE_CXPR explicit object(const object_base& obj, bool should_retain) noexcept;

  ZS_INLINE_CXPR explicit object(
      reference_counted_object* ref_obj, object_type otype, bool should_retain) noexcept;

  ZS_INLINE_CXPR object(zs::engine*, const object& obj) noexcept;

  ZS_INLINE_CXPR object(zs::engine*, object&& obj) noexcept;

  //
  // MARK: Null.
  //

  ZS_INLINE_CXPR object(null_t v) noexcept;

  ZS_INLINE_CXPR object(zs::engine*, null_t v) noexcept;

  ZS_CK_INLINE_CXPR static object create_null() noexcept {
    return object(object_base{ ZS_OBJECT_INITIALIZER(_value, 0, k_null) }, false);
  }

  ZS_CK_INLINE static object create_null(zs::engine*) noexcept { return create_null(); }

  //
  // MARK: None.
  //

  ZS_CK_INLINE_CXPR static object create_none() noexcept {
    return object(object_base{ ZS_OBJECT_INITIALIZER(_value, 0, k_none) }, false);
  }

  ZS_CK_INLINE static object create_none(zs::engine*) noexcept { return create_none(); }

  //
  // MARK: Error.
  //
  ZS_CK_INLINE_CXPR object(zs::error_code err) noexcept;
  ZS_CK_INLINE_CXPR object(zs::error_result err) noexcept;

  ZS_CK_INLINE_CXPR static object create_error(zs::error_code err) noexcept { return object(err); }

  ZS_CK_INLINE static object create_error(zs::engine*, zs::error_code err) noexcept { return object(err); }

  ZS_CK_INLINE_CXPR static object create_error(zs::error_result err) noexcept { return object(err); }

  ZS_CK_INLINE static object create_error(zs::engine*, zs::error_result err) noexcept { return object(err); }

  //
  // MARK: Bool.
  //

  template <class BoolType, std::enable_if_t<std::is_same_v<BoolType, bool_t>, int> = 0>
  ZS_INLINE_CXPR object(BoolType v) noexcept;

  //
  // MARK: Integer.
  //

  ZS_INLINE_CXPR object(int_t v) noexcept;
  ZS_INLINE_CXPR object(int32_t v) noexcept;
  ZS_INLINE_CXPR object(uint32_t v) noexcept;
  ZS_INLINE_CXPR object(uint64_t v) noexcept;
  ZS_INLINE_CXPR object(long v) noexcept;
  ZS_INLINE_CXPR object(unsigned long v) noexcept;

  ZS_INLINE_CXPR object(zs::engine*, int_t v) noexcept;
  ZS_INLINE_CXPR object(zs::engine*, int32_t v) noexcept;
  ZS_INLINE_CXPR object(zs::engine*, uint32_t v) noexcept;
  ZS_INLINE_CXPR object(zs::engine*, uint64_t v) noexcept;
  ZS_INLINE_CXPR object(zs::engine*, long v) noexcept;
  ZS_INLINE_CXPR object(zs::engine*, unsigned long v) noexcept;

  //
  // MARK: Float.
  //

  ZS_INLINE_CXPR object(float_t v) noexcept;
  ZS_INLINE_CXPR object(float v) noexcept;

  ZS_INLINE_CXPR object(zs::engine*, float_t v) noexcept;
  ZS_INLINE_CXPR object(zs::engine*, float v) noexcept;

  //
  // MARK: Raw pointer.
  //

  template <class Ptr, std::enable_if_t<std::is_same_v<Ptr, void>, int> = 0>
  ZS_INLINE_CXPR explicit object(Ptr* ptr) noexcept;

  template <class Ptr, std::enable_if_t<std::is_same_v<Ptr, void>, int> = 0>
  ZS_INLINE_CXPR explicit object(zs::engine*, Ptr* ptr) noexcept;

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

  struct small_string_tag {};

  ZS_INLINE object(small_string_tag, std::string_view s) noexcept
      : object_base{ ._value = 0,
        ._reserved_u32 = 0,
        ._reserved_u16 = 0,
        ._type = zs::object_type::k_small_string,
        ._flags = zs::object_flags_t::none } {
    zbase_assert(s.size() <= constants::k_small_string_max_size, "invalid size");
    memcpy(_sbuffer, s.data(), s.size());
  }
 
  template <size_t Size> requires (Size <= constants::k_small_string_max_size)
  ZS_INLINE object(small_string_tag, const char (&s)[Size]) noexcept
      : object_base{ ._value = 0,
        ._reserved_u32 = 0,
        ._reserved_u16 = 0,
        ._type = zs::object_type::k_small_string,
        ._flags = zs::object_flags_t::none } {
    memcpy(_sbuffer, &s[0], Size);
  }

  template <size_t Size> requires (Size <= constants::k_small_string_max_size)
  ZS_CK_INLINE static object create_small_string(const char (&s)[Size]) noexcept {
    static_assert(Size <= constants::k_small_string_max_size, "invalid size");
    return object(small_string_tag{}, s);
  }

  ZS_CK_INLINE static object create_small_string(std::string_view s) noexcept {
    return object(small_string_tag{}, s);
  }

  ZS_CK_INLINE static object create_small_string(zs::engine*, std::string_view s) {
    return object(small_string_tag{}, s);
  }

  //
  // k_string_view.
  //

  struct string_view_tag {};

  ZS_CK_INLINE_CXPR object(string_view_tag, std::string_view s) noexcept
      : object_base{ ._sview = s.data(),
        ._sview_size = (uint32_t)s.size(),
        ._reserved_u16 = 0,
        ._type = zs::object_type::k_string_view,
        ._flags = zs::object_flags_t::none } {

    zbase_assert(s.size() <= (std::numeric_limits<uint32_t>::max)(), "wrong size");
  }

  ZS_CK_INLINE_CXPR static object create_string_view(std::string_view s) {
    return object(string_view_tag{}, s);
  }

  ZS_CK_INLINE_CXPR static object create_string_view(zs::engine*, std::string_view s) {
    return create_string_view(s);
  }

  ZS_CHECK static object create_string(zs::engine* eng, std::string_view s);
  ZS_CHECK static object create_long_string(zs::engine* eng, std::string_view s);
  ZS_CHECK static object create_mutable_string(zs::engine* eng, std::string_view s);

  ZS_CHECK static object create_concat_string(zs::engine* eng, std::string_view s1, std::string_view s2);
  ZS_CHECK static object create_concat_string(
      zs::engine* eng, std::string_view s1, std::string_view s2, std::string_view s3);

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
  ZS_CHECK static object create_table_with_delegate(zs::engine* eng, zs::object delegate);

  ZS_CHECK inline static object create_table(
      zs::engine* eng, std::initializer_list<std::pair<zs::object, zs::object>> list);

  ZS_CHECK inline static object create_table(
      zs::engine* eng, std::span<std::pair<zs::object, zs::object>> list);

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

  ZS_CHECK static object create_array(zs::engine* eng, size_t sz);
  ZS_CHECK inline static object create_array(zs::engine* eng, std::initializer_list<zs::object> list);
  ZS_CHECK inline static object create_array(zs::engine* eng, std::span<zs::object> list);
  ZS_CHECK inline static object create_array(zs::engine* eng, std::span<const zs::object> list);

  template <class Container>
    requires zb::is_contiguous_container_not_string_v<Container>
  ZS_CHECK inline static object create_array(zs::engine* eng, const Container& container);

  //
  //
  //

  ZS_INLINE_CXPR object(native_array_type na_type) noexcept;

  ZS_CK_INLINE_CXPR static object create_native_function(zs::native_cpp_closure_t fct) noexcept;

  //
  // MARK: Native Closure.
  //

  explicit object(native_closure_object* obj, bool should_retain) noexcept;

  ZS_CHECK static object create_native_closure(zs::engine* eng, zs::native_cclosure_t nc);
  ZS_CHECK static object create_native_closure(zs::engine* eng, zs::native_cpp_closure_t nc);
  ZS_CHECK static object create_native_closure(zs::engine* eng, zs::native_closure* nc);

  template <class Fct>
  ZS_CHECK inline static object create_native_closure(zs::engine* eng, Fct&& fct);

  // TODO: Change this name.

  template <class Fct>
  ZS_CHECK inline static object create_native_closure_function(zs::engine* eng, Fct&& fct);

  template <class R, class... Args>
  ZS_CHECK inline static object create_native_closure_function(
      zs::engine* eng, zb::function_pointer<R, Args...> fct);

  template <class ClassType, class R, class... Args>
  ZS_CHECK inline static object create_native_closure_function(
      zs::engine* eng, zb::member_function_pointer<ClassType, R, Args...> fct, const zs::object& uid = {});

  template <class ClassType, class R, class... Args>
  ZS_CHECK inline static object create_native_closure_function(zs::engine* eng,
      zb::const_member_function_pointer<ClassType, R, Args...> fct, const zs::object& uid = {});

  //
  //
  //

  ZS_INLINE_CXPR object(zs::native_cpp_closure_t fct) noexcept;

  ZS_INLINE_CXPR object(zs::closure_t fct) noexcept;

  template <class T>
    requires(!std::is_same_v<T, zs::closure_t>) and std::is_convertible_v<T, zs::closure_t>
  ZS_INLINE_CXPR object(T fct) noexcept;

  template <class T>
    requires std::is_same_v<T, zs::closure_t> or std::is_convertible_v<T, zs::closure_t>
  inline object& operator=(T fct) noexcept {
    *this = object(zs::closure_t(fct));
    return *this;
  }

  //
  // MARK: Create object
  //

  //
  // Create array.
  //

  ZS_CHECK static object create_native_array(zs::engine* eng, zs::native_array_type ntype, size_t sz);

  template <class T>
  ZS_CK_INLINE static object create_native_array(zs::engine* eng, size_t sz);

  //
  // MARK: Closure.
  //

  ZS_CHECK static object create_closure(zs::engine* eng, const object& fct_prototype, const object& root);

  //
  // MARK: User Data.
  //

  ZS_CHECK static object create_user_data(zs::engine* eng, size_t size);

  template <class T, class... Args>
  ZS_CHECK inline static object create_user_data(zs::engine* eng, Args&&... args);

  //
  // MARK: Class.
  //

  ZS_CHECK static object create_class(zs::engine* eng);

  ZS_CHECK static object create_struct(zs::engine* eng);
  ZS_CHECK static object create_struct(zs::engine* eng, size_t sz);

  //
  // MARK: Node.
  //

  ZS_CHECK static object create_node(zs::engine* eng, const object& name);

  ZS_CK_INLINE static object create_node(zs::engine* eng, std::string_view name) noexcept {
    return create_node(eng, create_string(eng, name));
  }

  ZS_CK_INLINE static object create_node(zs::engine* eng, const char* name) noexcept {
    return create_node(eng, create_string(eng, name));
  }

  ZS_CHECK static object create_node(zs::engine* eng, const object& name, const object& value);

  ZS_CK_INLINE static object create_node(
      zs::engine* eng, std::string_view name, const object& value) noexcept {
    return create_node(eng, create_string(eng, name), value);
  }

  ZS_CK_INLINE static object create_node(zs::engine* eng, const char* name, const object& value) noexcept {
    return create_node(eng, create_string(eng, name), value);
  }

  //
  // MARK: Extension.
  //

  /// Create color.
  ZS_CK_INLINE static object create_color(uint32_t c) noexcept;
  ZS_CK_INLINE static object create_array_iterator(const object& arr) noexcept;
  ZS_CK_INLINE static object create_array_iterator(object* ptr, uint32_t index) noexcept;

  ZS_CK_INLINE static object create_table_iterator(const object& tbl) noexcept;
  ZS_CK_INLINE static object create_table_iterator(zs::object_unordered_map<object>::iterator it) noexcept;

  //
  //
  //

  ZS_INLINE_CXPR ~object() noexcept {
    if (is_ref_counted()) {
      _ref_counted->release();
    }
  }

  //
  //
  //

  object& operator=(const object& obj);
  object& operator=(object&& obj);

  //
  // MARK: Flags.
  //

  inline object& with_flags(object_flags_t flags) noexcept {
    _flags |= flags;
    return *this;
  }

  inline object& with_flags(uint32_t flags) noexcept { return with_flags((object_flags_t)flags); }

  //
  //
  //

  bool is_if_true() const noexcept;
  bool is_false() const noexcept;

  inline explicit operator bool() const noexcept { return !is_false(); }

  //
  // MARK: Native closure
  //

  zs::error_result set_closure_release_hook(zs::native_closure_release_hook_t hook);
  zs::error_result set_closure_user_pointer(zs::raw_pointer_t ptr);
  zs::error_result get_closure_user_pointer(zs::raw_pointer_t& ptr) const;

  //
  // MARK: User Data
  //

  zs::error_result set_user_data_release_hook(zs::user_data_release_hook_t hook);
  ZS_CHECK void* get_user_data_data() const noexcept;

  zs::error_result set_user_data_uid(const zs::object& uid) noexcept;
  zs::error_result set_user_data_uid(zs::object&& uid) noexcept;
  ZS_CHECK zs::error_result get_user_data_uid(zs::object& uid) noexcept;
  ZS_CHECK zs::object get_user_data_uid() const noexcept;
  ZS_CHECK bool has_user_data_uid(const zs::object& uid) const noexcept;

  zs::error_result set_user_data_typeid(const zs::object& tid) noexcept;
  zs::error_result set_user_data_typeid(zs::object&& tid) noexcept;
  ZS_CHECK zs::error_result get_user_data_typeid(zs::object& tid) noexcept;
  ZS_CHECK bool has_user_data_typeid(const zs::object& tid) const noexcept;

  zs::error_result copy_user_data_to_type(void* obj, size_t data_size, std::string_view tid) const noexcept;

  zs::error_result set_user_data_copy_to_type_function(zs::copy_user_data_to_type_t fct) noexcept;

  //
  //
  //

  template <class T>
  inline zs::error_result get_value(T& value) const noexcept;

  template <class T>
  ZS_CHECK inline T get_value() const;

  template <class T>
  ZS_CHECK inline T get_value(const T& opt_value) const;

  template <class T>
  ZS_CHECK inline static object_type get_value_conv_obj_type() noexcept;

  template <class T>
  ZS_CHECK inline constexpr static const char* get_value_conv_obj_name() noexcept;

  //
  //
  //

  object* operator[](size_t idx);
  const object* operator[](size_t idx) const;

  object* operator[](size_t idx1, size_t idx2);
  const object* operator[](size_t idx1, size_t idx2) const;

  object* operator[](size_t idx1, size_t idx2, size_t idx3);
  const object* operator[](size_t idx1, size_t idx2, size_t idx3) const;

  //
  //
  //

  zs::error_result set_delegate(object delegate) noexcept;
  zs::table_object* get_delegate() const noexcept;

  bool has_delegate() const noexcept;

  //
  //
  //

  /// Returns a weak_ref object for reference counted objects.
  /// Returns itself for non reference counted objects and for weak_ref objects.
  ZS_CHECK object get_weak_ref() const noexcept;
  ZS_CHECK object get_weak_ref_value() const noexcept;

  //
  //
  //

  ZS_CHECK zs::error_result get_binary_size(size_t& write_size) const;

  template <class VectorType>
  ZS_CHECK zs::error_result to_binary(VectorType& buffer, size_t& sz, uint32_t flags = 0) const;

  ZS_CHECK zs::error_result to_binary(
      write_function_t write_function_t, size_t& write_size, void* data, uint32_t flags = 0) const;

  ZS_CHECK static zs::error_result from_binary(
      zs::engine* eng, std::span<uint8_t> buffer, object& out, size_t& offset);

  //
  //
  //

  void reset() noexcept;

  struct auto_converter {
    const object& obj;

    ZS_INLINE explicit operator bool() const noexcept = delete;

    template <class T>
      requires std::is_integral_v<T>
    ZS_INLINE operator T() const noexcept {
      zbase_assert(obj.is_integer_convertible(), "Not integer convertible object");
      return (T)obj.convert_to_integer_unchecked();
    }

    template <class T>
      requires std::is_floating_point_v<T>
    ZS_INLINE operator T() const noexcept {
      zbase_assert(obj.is_float_convertible(), "Not float convertible object");
      return (T)obj.convert_to_float_unchecked();
    }

    ZS_INLINE operator node_object&() const noexcept {
      zbase_assert(obj.is_node(), "Not a node object");
      return obj.as_node();
    }

    ZS_INLINE operator array_object&() const noexcept {
      zbase_assert(obj.is_array(), "Not an array object");
      return obj.as_array();
    }

    ZS_INLINE operator table_object&() const noexcept {
      zbase_assert(obj.is_table(), "Not a table object");
      return obj.as_table();
    }

    ZS_INLINE operator string_object&() const noexcept {
      zbase_assert(obj.is_long_string(), "Not a long string object");
      return obj.as_string();
    }

    ZS_INLINE operator user_data_object&() const noexcept {
      zbase_assert(obj.is_user_data(), "Not a user data object");
      return obj.as_udata();
    }

    ZS_INLINE operator reference_counted_object&() const noexcept {
      zbase_assert(obj.is_ref_counted(), "Not a reference counted object");
      return obj.as_ref_counted();
    }

    ZS_INLINE operator delegate_object&() const noexcept {
      zbase_assert(obj.is_delegable(), "Not a delegable object");
      return obj.as_delegate();
    }

    ZS_INLINE operator std::string_view() const noexcept {
      zbase_assert(obj.is_string(), "Not a string object");
      return obj.get_string_unchecked();
    }
  };

  inline auto get() const noexcept { return auto_converter{ *this }; }

private:
  struct helper;

  template <class Fct, class ClassType, class R, class... Args>
  ZS_CHECK inline static object create_native_closure_function(zs::engine* eng,
      zb::member_function_pointer_wrapper<Fct, ClassType, R, Args...> fct, const zs::object& uid = {});

  template <typename ClassType, typename ReturnType, class Fct, typename... Args>
  inline static object create_native_closure_function(zs::engine* eng, Fct&& fct, zb::type_list<Args...>);
};

//
//
//
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

class parameter_list : public zb::span<const object> {
public:
  using span = zb::span<const object>;
  using element_type = span::element_type;
  using value_type = span::value_type;
  using size_type = span::size_type;
  using difference_type = span::difference_type;
  using pointer = span::pointer;
  using const_pointer = span::const_pointer;
  using reference = span::reference;
  using const_reference = span::const_reference;
  using iterator = span::iterator;

  static constexpr const size_type npos = -1;

  using span::span;
  using span::operator=;
  using span::data;
  using span::size;
  using span::operator[];

  ZB_INLINE parameter_list(const span& s) noexcept
      : span(s) {}

  ZB_INLINE parameter_list(const std::span<object>& s) noexcept
      : span(s) {}

  ZB_INLINE parameter_list(const zb::span<object>& s) noexcept
      : span(s) {}

  ZB_INLINE parameter_list(const parameter_list&) noexcept = default;
  ZB_INLINE parameter_list(parameter_list&&) noexcept = default;

  ZB_INLINE parameter_list& operator=(const parameter_list&) noexcept = default;
  ZB_INLINE parameter_list& operator=(parameter_list&&) noexcept = default;

  ZB_CHECK ZB_INLINE reference operator()(difference_type n) const noexcept {
    const size_t sz = size();
    zbase_assert(sz, "call operator[] in an empty vector");
    return span::operator[]((n + sz) % sz);
  }

  ZB_CHECK ZB_INLINE object opt_get(difference_type n) const noexcept {
    const size_t sz = size();
    n = (n + sz) % sz;
    if (n >= sz) {
      return nullptr;
    }
    return span::operator[](n);
  }

  ZB_CHECK ZB_INLINE object opt_get(difference_type n, const object& obj) const noexcept {
    const size_t sz = size();
    n = (n + sz) % sz;
    if (n >= sz) {
      return obj;
    }
    return span::operator[](n);
  }

  ZB_CHECK ZB_INLINE pointer data(size_type index) const noexcept { return data() + index; }

  ZB_CHECK ZB_INLINE reference get_self() const noexcept {
    zbase_assert(!empty(), "No self argument");
    return span::operator[](0);
  }

  ZB_CHECK ZB_INLINE bool has_arguments() const noexcept { return size() > 1; }

  ZB_CHECK ZB_INLINE bool has_meta_arguments() const noexcept {
    return size() > 1 and span::operator[](1).is_meta_argument();
  }

  ZB_CHECK ZB_INLINE bool has_normal_arguments() const noexcept {
    const size_t sz = size();
    return sz > 2 or (sz > 1 and !span::operator[](1).is_meta_argument());
  }

  ZB_CHECK ZB_INLINE reference get_meta_argument_object() const noexcept {
    zbase_assert(has_meta_arguments(), "No meta arguments");
    return span::operator[](1);
  }

  ZB_CHECK ZB_INLINE span get_normal_arguments() const noexcept {
    const size_t sz = size();
    if (sz <= 1) {
      return {};
    }

    if (span::operator[](1).is_meta_argument()) {
      return sz > 2 ? span::subspan(2) : span{};
    }

    return span::subspan(1);
  }

  ZB_CHECK ZB_INLINE span get_meta_arguments() const noexcept;
};

//
// MARK: Create object helpers.
//

/// Create a small_string object.
template <size_t Size>
ZS_CK_INLINE static object _ss(const char (&s)[Size]) noexcept {
  static_assert(Size <= constants::k_small_string_max_size, "invalid size");
  return object::create_small_string(std::string_view(s));
}

/// Create a small_string object.
ZS_CK_INLINE object _ss(std::string_view s) noexcept { return object::create_small_string(s); }

/// Create a string object.
template <class EngineAccess>
ZS_CK_INLINE object _s(EngineAccess&& a_eng, std::string_view s) noexcept {
  return object::create_string(zs::get_engine(a_eng), s);
}

ZS_CK_INLINE_CXPR object _sv(std::string_view s) {
  return object{ { { ._sview = s.data() },
                     { { { ._sview_size = (uint32_t)s.size() }, { 0 }, object_type::k_string_view,
                         object_flags_t::none } } },
    false };
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

template <class T, class... Args>
ZS_CK_INLINE object _u(zs::engine* eng, Args&&... args) {
  return zs::object::create_user_data<T>(eng, std::forward<Args>(args)...);
}

ZS_CK_INLINE object _nf(zs::native_cpp_closure_t fct) noexcept { return object::create_native_function(fct); }

namespace literals {
  inline constexpr zs::object operator""_ss(const char* str, size_t) noexcept { return zs::_ss(str); }
} // namespace literals.

} // namespace zs.

//
// MARK: Objects.
//

#define ZS_SCRIPT_INCLUDE_OBJECTS 1
#include <zscript/core/objects/delegate.h>
#include <zscript/core/objects/weak_ref.h>
#include <zscript/core/objects/table.h>
#include <zscript/core/objects/array.h>
#include <zscript/core/objects/node.h>
#include <zscript/core/objects/class.h>
#include <zscript/core/objects/struct.h>
#include <zscript/core/objects/string.h>
#include <zscript/core/objects/mutable_string.h>
#include <zscript/core/objects/user_data.h>
#include <zscript/core/objects/native_closure.h>
#include <zscript/core/objects/typed_array.h>
#include <zscript/core/objects/closure.h>
#undef ZS_SCRIPT_INCLUDE_OBJECTS

//
//
//

namespace zs {

ZS_CXPR object::object() noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_value, 0, k_null) } {}

object::object(const object& obj) noexcept
    : object_base((const object_base&)obj) {

  //  memcpy(this, &obj, sizeof(object));

  if (is_ref_counted()) {
    _ref_counted->retain();
  }
}

ZS_CXPR object::object(object&& obj) noexcept
    : object_base((const object_base&)obj) {

  //    ::memcpy(this, &obj, sizeof(obj));
  ::memset(&obj, 0, sizeof(obj));
  obj._type = k_null;
}

ZS_CXPR object::object(const object_base& obj, bool should_retain) noexcept
    : object_base(obj) {

  if (should_retain && is_ref_counted()) {
    _ref_counted->retain();
  }
}

ZS_CXPR object::object(reference_counted_object* ref_obj, object_type otype, bool should_retain) noexcept {

  ::memset(this, 0, sizeof(object_base));

  _type = otype;
  _ref_counted = ref_obj;

  if (should_retain && is_ref_counted()) {
    _ref_counted->retain();
  }
}

ZS_CXPR object::object(zs::engine*, const object& obj) noexcept
    : object_base((const object_base&)obj) {

  if (is_ref_counted()) {
    _ref_counted->retain();
  }
}

ZS_CXPR object::object(zs::engine*, object&& obj) noexcept
    : object_base((object_base&)obj) {
  ::memset(&obj, 0, sizeof(obj));
  obj._type = k_null;
}

ZS_CXPR object::object(null_t v) noexcept
    : object() {}

ZS_CXPR object::object(zs::engine*, null_t v) noexcept
    : object() {}

template <class BoolType, std::enable_if_t<std::is_same_v<BoolType, bool_t>, int>>
ZS_CXPR object::object(BoolType v) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_bool, v, k_bool) } {}

ZS_CXPR object::object(int_t v) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_int, v, k_integer) } {}

ZS_CXPR object::object(zs::engine*, int_t v) noexcept
    : object(v) {}

ZS_CXPR object::object(int32_t v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(zs::engine*, int32_t v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(uint32_t v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(zs::engine*, uint32_t v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(uint64_t v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(zs::engine*, uint64_t v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(long v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(zs::engine*, long v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(unsigned long v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(zs::engine*, unsigned long v) noexcept
    : object((int_t)v) {}

ZS_CXPR object::object(float_t v) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_float, v, k_float) } {}

ZS_CXPR object::object(zs::engine*, float_t v) noexcept
    : object(v) {}

ZS_CXPR object::object(float v) noexcept
    : object((float_t)v) {}

ZS_CXPR object::object(zs::engine*, float v) noexcept
    : object((float_t)v) {}

ZS_CXPR object::object(zs::error_code err) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_error, err, k_error) } {

  if (err == zs::error_code::success) {
    *this = nullptr;
  }
}

ZS_CXPR object::object(zs::error_result err) noexcept
    : object(err.code) {}

template <class Ptr, std::enable_if_t<std::is_same_v<Ptr, void>, int>>
ZS_CXPR object::object(Ptr* ptr) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_pointer, ptr, k_raw_pointer) } {}

template <class Ptr, std::enable_if_t<std::is_same_v<Ptr, void>, int>>
ZS_CXPR object::object(zs::engine*, Ptr* ptr) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_pointer, ptr, k_raw_pointer) } {}

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

ZS_CXPR object::object(native_array_type na_type) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_int, (int_t)na_type, k_integer, object_flags_t::array_type) } {}

ZS_CXPR object::object(zs::native_cpp_closure_t fct) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_nfct, fct, k_native_function) } {}

ZS_CXPR object::object(zs::closure_t fct) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_fct, fct, k_native_function2) } {}

template <class T>
  requires(!std::is_same_v<T, zs::closure_t>) and std::is_convertible_v<T, zs::closure_t>
ZS_INLINE_CXPR object::object(T fct) noexcept
    : object_base{ ZS_OBJECT_INITIALIZER(_fct, (zs::closure_t)fct, k_native_function2) } {}

object object::create_table(zs::engine* eng, std::initializer_list<std::pair<zs::object, zs::object>> list) {

  object table = create_table(eng);
  auto& map = *table.get_table_internal_map();

  for (auto it = list.begin(); it != list.end(); ++it) {
    map[std::move(it->first)] = std::move(it->second);
  }

  return table;
}

object object::create_table(zs::engine* eng, std::span<std::pair<zs::object, zs::object>> list) {

  object table = create_table(eng);
  auto& map = *table.get_table_internal_map();

  for (auto it = list.begin(); it != list.end(); ++it) {
    map[std::move(it->first)] = std::move(it->second);
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

template <class T>
object object::create_native_array(zs::engine* eng, size_t sz) {
  object obj;
  obj._type = object_type::k_native_array;
  obj._na_type = to_native_array_type<T>();
  obj._native_array<T>() = native_array_object<T>::create(eng, sz);
  return obj;
}

//
//
//

template <class T, class... Args>
object object::create_user_data(zs::engine* eng, Args&&... args) {
  object obj = create_user_data(eng, (size_t)sizeof(T));
  zb_placement_new(obj.get_user_data_data()) T(std::forward<Args>(args)...);

  obj.set_user_data_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    T* t = (T*)ptr;
    t->~T();
  });

  obj.set_user_data_typeid(zs::object::create_string(eng, typeid(T).name()));

  // VERY DANGEROUS.
  obj.set_user_data_copy_to_type_function(
      [](void* dst, size_t data_size, std::string_view tid, void* data) -> zs::error_result {
        if (data_size == sizeof(T) and tid == typeid(T).name()) {
          T& dst_ref = *((T*)dst);
          const T& src_ref = *((const T*)data);
          dst_ref = src_ref;
          return {};
        }

        return zs::error_code::invalid_type;
      });

  return obj;
}

object object::create_color(uint32_t c) noexcept {
  object obj;
  obj._type = object_type::k_extension;
  obj._ext_type = extension_type::kext_color;
  obj._color = c;
  return obj;
}

object object::create_array_iterator(const object& arr) noexcept {
  object obj;
  obj._type = object_type::k_extension;
  obj._ext_type = extension_type::kext_array_iterator;
  obj._array_it = arr.as_array().data();
  obj._reserved_u32 = 0;
  return obj;
}

object object::create_array_iterator(object* ptr, uint32_t index) noexcept {
  object obj;
  obj._type = object_type::k_extension;
  obj._ext_type = extension_type::kext_array_iterator;
  obj._array_it = ptr;
  obj._reserved_u32 = index;
  return obj;
}

object object::create_table_iterator(const object& tbl) noexcept {
  object obj;
  obj._type = object_type::k_extension;
  obj._ext_type = extension_type::kext_table_iterator;

  obj._table_it.construct(tbl.as_table().begin());
  //  static_assert(sizeof(it) == 8, "");
  static_assert(std::is_trivially_destructible_v<zs::object_unordered_map<object>::iterator>, "");
  static_assert(std::is_trivially_copyable_v<zs::object_unordered_map<object>::iterator>, "");
  //  ::memcpy(&obj._value, &it, sizeof(it));
  obj._reserved_u32 = 0;
  return obj;
}

object object::create_table_iterator(zs::object_unordered_map<object>::iterator it) noexcept {
  object obj;

  obj._type = object_type::k_extension;
  obj._ext_type = extension_type::kext_table_iterator;

  obj._table_it.construct(it);
  //  static_assert(sizeof(it) == 8, "");
  static_assert(std::is_trivially_destructible_v<zs::object_unordered_map<object>::iterator>, "");
  static_assert(std::is_trivially_copyable_v<zs::object_unordered_map<object>::iterator>, "");
  //  ::memcpy(&obj._value, &it, sizeof(it));
  obj._reserved_u32 = 0;
  return obj;
}

ZB_CHECK ZB_INLINE parameter_list::span parameter_list::get_meta_arguments() const noexcept {
  zbase_assert(has_meta_arguments(), "No meta arguments");
  const object& meta = span::operator[](1);

  if (meta.is_array()) {
    return meta.as_array().to_vec();
  }

  return span(&meta, 1);
}

} // namespace zs.
