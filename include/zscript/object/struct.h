#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {
struct struct_item {
  struct_item() = default;
  struct_item(const struct_item&) = default;
  struct_item(struct_item&&) = default;

  inline struct_item(const object& k, const object& v, uint32_t _mask = 0, bool is_private = false,
      bool is_const = false) noexcept
      : key(k)
      , value(v)
      , mask(_mask)
      , is_private(is_private)
      , is_const(is_const) {}

  template <class K, class V>
  inline struct_item(
      K&& k, V&& v, uint32_t _mask = 0, bool is_private = false, bool is_const = false) noexcept
      : key(std::forward<K>(k))
      , value(std::forward<V>(v))
      , mask(_mask)
      , is_private(is_private)
      , is_const(is_const) {}

  struct_item& operator=(const struct_item&) noexcept = default;
  struct_item& operator=(struct_item&&) noexcept = default;

  inline friend std::ostream& operator<<(std::ostream& stream, const struct_item& si) {
    return stream << "{" << si.key.get_string_unchecked() << " : " << si.value << "}";
  }

  object key;
  object value;

  // TODO: Replace with variable_type_info or variable_attribute_t?
  uint32_t mask = 0;
  bool is_private = false;
  bool is_const = false;
};

struct struct_method {
  struct_method() noexcept = default;
  struct_method(const struct_method&) noexcept = default;
  struct_method(struct_method&&) noexcept = default;

  inline struct_method(
      const object& name, const object& closure, bool is_private = false, bool is_const = false) noexcept
      : name(name)
      , closure(closure)
      , is_private(is_private)
      , is_const(is_const) {}

  template <class K, class V>
  inline struct_method(K&& name, V&& closure, bool is_private = false, bool is_const = false) noexcept
      : name(std::forward<K>(name))
      , closure(std::forward<V>(closure))
      , is_private(is_private)
      , is_const(is_const) {}

  struct_method& operator=(const struct_method&) noexcept = default;
  struct_method& operator=(struct_method&&) noexcept = default;

  inline friend std::ostream& operator<<(std::ostream& stream, const struct_method& m) {
    return stream << "{" << m.name.get_string_unchecked() << "}";
  }

  object name;
  object closure;

  // TODO: Replace with variable_attribute_t.
  bool is_static = false;
  bool is_private = false;
  bool is_const = false;
};

class struct_instance_object final : public zs::reference_counted_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  ZS_CHECK static struct_instance_object* create(zs::engine* eng, int_t sz) noexcept;

  virtual ~struct_instance_object() override;

  ZS_CHECK int_t size() const noexcept { return (int_t)_size; }
  ZS_CHECK bool contains(const object& name) const noexcept;
  ZS_CHECK bool contains_member(const object& name) const noexcept;
  ZS_CHECK bool contains_static(const object& name) const noexcept;

  ZS_CK_INLINE zb::span<object> get_span() noexcept { return zb::span<object>(data(), _size); }

  ZS_CK_INLINE zb::span<const object> get_span() const noexcept {
    return zb::span<const object>(data(), _size);
  }

  ZS_CK_INLINE object* data() noexcept { return _data.data(); }
  ZS_CK_INLINE const object* data() const noexcept { return _data.data(); }

  ZS_CHECK zb::span<const struct_item> base_vector() const noexcept;

  ZS_CK_INLINE object& operator[](size_t index) noexcept { return data()[index]; }
  ZS_CK_INLINE const object& operator[](size_t index) const noexcept { return data()[index]; }

  ZS_CHECK object& key(size_t index) noexcept;
  ZS_CHECK const object& key(size_t index) const noexcept;

  ZS_CK_INLINE object& back() noexcept { return data()[size() - 1]; }
  ZS_CK_INLINE const object& back() const noexcept { return data()[size() - 1]; }

  zs::error_result get(const object& name, object& dst, bool can_access_private = false) const noexcept;

  zs::error_result contains(const object& name, object& dst, bool can_access_private = false) const noexcept;

  zs::error_result set(const object& name, const object& obj, bool can_access_private = false) noexcept;

  zs::error_result get_meta_method(const object& name, object& dst) const noexcept;
  const zs::object* get_meta_method(const object& name) const noexcept;

  ZS_CHECK object clone() const noexcept override;
  
  inline zb::span<struct_method> get_methods() noexcept;
  inline zb::span<const struct_method> get_methods() const noexcept;

  inline zb::span<struct_item> get_static_members() noexcept;
  inline zb::span<const struct_item> get_static_members() const noexcept;

  inline void set_initialized(bool val) { _initialized = val; }

  ZS_CK_INLINE zs::object& get_base() noexcept { return _base; }
  ZS_CK_INLINE const zs::object& get_base() const noexcept { return _base; }
  ZS_CK_INLINE const object& get_name() const noexcept;

private:
  friend class struct_object;
  struct_instance_object(zs::engine* eng) noexcept;

  zs::object _base;
  bool _initialized = false;

  size_t _size;
  zb::aligned_type_storage<object> _data;
};

class struct_object final : public zs::reference_counted_object, public zs::vector<struct_item> {
public:
  ZS_OBJECT_CLASS_COMMON;

  using vector_type = zs::vector<struct_item>;

  ZS_CHECK static struct_object* create(zs::engine* eng, int_t sz) noexcept;

  virtual ~struct_object() override = default;

  ZS_CHECK int_t size() const noexcept { return (int_t)vector_type::size(); }
  ZS_CHECK bool contains(const object& name) const noexcept;
  ZS_CHECK bool contains_member(const object& name) const noexcept;
  ZS_CHECK bool contains_method(const object& name) const noexcept;
  ZS_CHECK bool contains_static(const object& name) const noexcept;

  zs::error_result get(const object& name, object& dst) const noexcept;
  zs::error_result contains(const object& name, object& dst) const noexcept;

  zs::error_result set(const object& name, const object& obj) noexcept;
  zs::error_result set_static(const object& name, const object& obj) noexcept;
  zs::error_result new_slot(const object& name, const object& value, uint32_t mask, bool is_static,
      bool is_private, bool is_const) noexcept;

  zs::error_result new_slot(
      const object& name, uint32_t mask, bool is_static, bool is_private, bool is_const) noexcept;

  zs::error_result new_slot(
      const object& name, const object& value, uint32_t mask, zs::variable_attribute_t vflags) noexcept;

  zs::error_result new_slot(const object& name, uint32_t mask, zs::variable_attribute_t vflags) noexcept;

  zs::error_result new_method(
      const object& name, const object& closure, bool is_private, bool is_const) noexcept;

  zs::error_result new_static_method(
      const object& name, const object& closure, bool is_private, bool is_const) noexcept;

  ZS_CHECK struct_instance_object* create_instance() const noexcept;
  ZS_CHECK object clone() const noexcept override;
  
  using method_vector = zs::small_vector<struct_method, 4>;
  ZS_CK_INLINE method_vector& get_methods() noexcept { return _methods; }
  ZS_CK_INLINE const method_vector& get_methods() const noexcept { return _methods; }

  using static_member_vector = zs::small_vector<struct_item, 2>;
  ZS_CK_INLINE static_member_vector& get_static_members() noexcept { return _statics; }
  ZS_CK_INLINE const static_member_vector& get_static_members() const noexcept { return _statics; }

  inline void activate_default_constructor() { _has_default_constructor = true; }

  inline void set_constructor(zs::object&& constructor) { _constructors = std::move(constructor); }
  inline void set_constructor(const zs::object& constructor) { _constructors = constructor; }

  template <class Constructor>
  inline void set_constructor(Constructor&& constructor) {
    _constructors = object(std::forward<Constructor>(constructor));
  }

  inline void set_name(const object& name) { _name = name; }
  void set_doc(const object& doc);
  void set_member_doc(const object& name, const object& doc);

  ZS_CK_INLINE const object& get_name() const noexcept { return _name; }
  ZS_CK_INLINE const object& get_doc() const noexcept { return _doc; }

  ZS_CK_INLINE bool has_default_constructor() const noexcept { return _has_default_constructor; }

  ZS_CK_INLINE bool has_constructors() const noexcept {
    return _constructors.is_function() or _constructors.is_array();
  }

  ZS_CK_INLINE bool has_single_constructor() const noexcept { return _constructors.is_function(); }

  ZS_CK_INLINE bool has_multi_constructors() const noexcept { return _constructors.is_array(); }

  size_t get_constructors_count() const noexcept;

private:
  struct_object(zs::engine* eng) noexcept;

  object _name;
  static_member_vector _statics;
  method_vector _methods;
  zs::object _constructors;
  object _doc;
  bool _has_default_constructor = false;
  bool _initialized = false;
};

inline zb::span<struct_method> struct_instance_object::get_methods() noexcept {
  return _base.as_struct().get_methods();
}

inline zb::span<const struct_method> struct_instance_object::get_methods() const noexcept {
  return _base.as_struct().get_methods();
}

inline zb::span<struct_item> struct_instance_object::get_static_members() noexcept {
  return _base.as_struct().get_static_members();
}

inline zb::span<const struct_item> struct_instance_object::get_static_members() const noexcept {
  return _base.as_struct().get_static_members();
}

ZS_CK_INLINE const object& struct_instance_object::get_name() const noexcept {
  return _base.as_struct().get_name();
}
} // namespace zs.
