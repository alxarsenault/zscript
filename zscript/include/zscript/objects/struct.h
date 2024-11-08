#include <zscript/objects/object_include_guard.h>

namespace zs {
struct struct_item {
  struct_item() = default;
  struct_item(const struct_item&) = default;
  struct_item(struct_item&&) = default;

  struct_item(const object& k, const object& v, uint32_t _mask = 0, bool _is_const = false)
      : key(k)
      , value(v)
      , mask(_mask)
      , is_const(_is_const) {}

  template <class K, class V>
  struct_item(K&& k, V&& v, uint32_t _mask = 0, bool _is_const = false)
      : key(std::forward<K>(k))
      , value(std::forward<V>(v))
      , mask(_mask)
      , is_const(_is_const) {}

  struct_item& operator=(const struct_item&) = default;
  struct_item& operator=(struct_item&&) = default;

  inline friend std::ostream& operator<<(std::ostream& stream, const struct_item& si) {
    return stream << "{" << si.key.get_string_unchecked() << " : " << si.value << "}";
  }

  object key;
  object value;
  uint32_t mask = 0;
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
  //
  //  ZS_CK_INLINE zb::span<object> get_span() noexcept { return zb::span<object>(_data, _size); }
  //
  //  ZS_CK_INLINE zb::span<const object> get_span() const noexcept {
  //    return zb::span<const object>(_data, _size);
  //  }
  //
  //  ZS_CK_INLINE object* data() noexcept { return _data; }
  //  ZS_CK_INLINE const object* data() const noexcept { return _data; }
  //
  //  ZS_CHECK zb::span<const struct_item> base_vector() const noexcept;
  //
  //  [[nodiscard]] inline object& operator[](size_t index) noexcept { return data()[index]; }
  //
  //  [[nodiscard]] inline const object& operator[](size_t index) const noexcept { return data()[index]; }
  //
  //  [[nodiscard]] object& key(size_t index) noexcept;
  //
  //  [[nodiscard]] const object& key(size_t index) const noexcept;
  //
  //  [[nodiscard]] inline object& back() noexcept { return data()[size() - 1]; }
  //
  //  [[nodiscard]] inline const object& back() const noexcept { return data()[size() - 1]; }

  ZS_CK_INLINE zb::span<object> get_span() noexcept { return zb::span<object>(data(), _size); }

  ZS_CK_INLINE zb::span<const object> get_span() const noexcept {
    return zb::span<const object>(data(), _size);
  }

  ZS_CK_INLINE object* data() noexcept { return _data.data(); }
  ZS_CK_INLINE const object* data() const noexcept { return _data.data(); }

  ZS_CHECK zb::span<const struct_item> base_vector() const noexcept;

  [[nodiscard]] inline object& operator[](size_t index) noexcept { return data()[index]; }

  [[nodiscard]] inline const object& operator[](size_t index) const noexcept { return data()[index]; }

  [[nodiscard]] object& key(size_t index) noexcept;

  [[nodiscard]] const object& key(size_t index) const noexcept;

  [[nodiscard]] inline object& back() noexcept { return data()[size() - 1]; }

  [[nodiscard]] inline const object& back() const noexcept { return data()[size() - 1]; }

  zs::error_result get(const object& name, object& dst) const noexcept;
  zs::error_result set(const object& name, const object& obj) noexcept;
  ZS_CHECK struct_instance_object* clone() const noexcept;

  inline zb::span<struct_item> get_static_members() noexcept;

  inline zb::span<const struct_item> get_static_members() const noexcept;

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
  ZS_CHECK bool contains_static(const object& name) const noexcept;

  zs::error_result get(const object& name, object& dst) const noexcept;

  zs::error_result set(const object& name, const object& obj) noexcept;
  zs::error_result set_static(const object& name, const object& obj) noexcept;
  zs::error_result new_slot(
      const object& name, const object& obj, uint32_t mask, bool is_static, bool is_const) noexcept;

  zs::error_result new_slot(const object& name, uint32_t mask, bool is_static, bool is_const) noexcept;

  ZS_CHECK struct_instance_object* create_instance() const noexcept;
  ZS_CHECK struct_object* clone() const noexcept;

  using static_member_vector = zs::small_vector<struct_item, 2>;

  inline static_member_vector& get_static_members() noexcept { return _statics; }

  inline const static_member_vector& get_static_members() const noexcept { return _statics; }

private:
  struct_object(zs::engine* eng) noexcept;

  static_member_vector _statics;
  zs::object _constructor;
  bool _initialized = false;
};

inline zb::span<struct_item> struct_instance_object::get_static_members() noexcept {
  return _base.as_struct().get_static_members();
}

inline zb::span<const struct_item> struct_instance_object::get_static_members() const noexcept {
  return _base.as_struct().get_static_members();
}
} // namespace zs.
