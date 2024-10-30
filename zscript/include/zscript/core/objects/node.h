#include <zscript/core/objects/object_include_guard.h>

namespace zs {
/// Node.
class node_object final : public delegate_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  struct attribute {
    attribute() = default;
    attribute(const attribute&) = default;
    attribute(attribute&&) = default;

    inline attribute(const object& k, const object& val)
        : name(k)
        , value(val) {}

    template <class K, class V>
    inline attribute(K&& k, V&& val)
        : name(std::forward<K>(k))
        , value(std::forward<V>(val)) {}

    attribute& operator=(const attribute&) = default;
    attribute& operator=(attribute&&) = default;

    inline friend std::ostream& operator<<(std::ostream& stream, const attribute& att) {
      return stream << "{" << att.name.get_string_unchecked() << " : " << att.value << "}";
    }

    object name;
    object value;
  };

  using iterator = zs::vector<object>::iterator;
  using const_iterator = zs::vector<object>::const_iterator;

  using attribute_iterator = zs::vector<attribute>::iterator;
  using attribute_const_iterator = zs::vector<attribute>::const_iterator;

  ZS_CHECK static node_object* create(zs::engine* eng, const object& name) noexcept;
  ZS_CHECK static node_object* create(zs::engine* eng, const object& name, const object& value) noexcept;

  virtual ~node_object() override = default;

  ZS_CK_INLINE int_t size() const noexcept { return (int_t)_children.size(); }

  ZS_CK_INLINE object& name() noexcept { return _name; }
  ZS_CK_INLINE const object& name() const noexcept { return _name; }

  ZS_CK_INLINE std::string_view name_str() const noexcept { return _name.get_string_unchecked(); }

  ZS_CK_INLINE object& value() noexcept { return _value; }
  ZS_CK_INLINE const object& value() const noexcept { return _value; }

  zs::error_result get(int_t idx, object& dst) const noexcept;

  zs::error_result set(int_t idx, const object& obj) noexcept;
  zs::error_result set(int_t idx, object&& obj) noexcept;

  zs::error_result add_child(const object& obj) noexcept;
  zs::error_result add_child(object&& obj) noexcept;

  template <class... Args>
  inline zs::error_result emplace_child(Args&&... args) noexcept {
    _children.emplace_back(std::forward(args)...);
    return {};
  }

  zs::error_result add_attribute(const object& key, const object& value) noexcept {
    _attributes.emplace_back(key, value);
    return {};
  }

  zs::error_result add_attribute(std::string_view key, const object& value) noexcept {
    return add_attribute(zs::_s(_engine, key), value);
  }

  zs::error_result add_attribute(const char* key, const object& value) noexcept {
    return add_attribute(zs::_s(_engine, key), value);
  }

  ZS_CK_INLINE bool has_children() const noexcept { return !_children.empty(); }
  ZS_CK_INLINE bool has_attributes() const noexcept { return !_attributes.empty(); }

  ZS_CK_INLINE zs::vector<object>& children() noexcept { return _children; }
  ZS_CK_INLINE const zs::vector<object>& children() const noexcept { return _children; }

  ZS_CK_INLINE zs::vector<attribute>& attributes() noexcept { return _attributes; }
  ZS_CK_INLINE const zs::vector<attribute>& attributes() const noexcept { return _attributes; }

  ZS_CK_INLINE iterator begin() noexcept { return _children.begin(); }
  ZS_CK_INLINE const_iterator begin() const noexcept { return _children.begin(); }

  ZS_CK_INLINE iterator end() noexcept { return _children.end(); }
  ZS_CK_INLINE const_iterator end() const noexcept { return _children.cend(); }

  ZS_CK_INLINE const_iterator cbegin() const noexcept { return _children.cbegin(); }
  ZS_CK_INLINE const_iterator cend() const noexcept { return _children.cend(); }

  ZS_CK_INLINE attribute_iterator attribute_begin() noexcept { return _attributes.begin(); }
  ZS_CK_INLINE attribute_const_iterator attribute_begin() const noexcept { return _attributes.begin(); }

  ZS_CK_INLINE attribute_iterator attribute_end() noexcept { return _attributes.end(); }
  ZS_CK_INLINE attribute_const_iterator attribute_end() const noexcept { return _attributes.cend(); }

  ZB_CHECK ZB_INLINE object& operator[](size_t n) noexcept { return _children[n]; }

  ZB_CHECK ZB_INLINE const object& operator[](size_t n) const noexcept { return _children[n]; }

  ZB_CHECK ZB_INLINE const object* get_attribute(std::string_view att_name) const noexcept {

    for (const auto& att : _attributes) {
      if (att.name == att_name) {
        return &att.value;
      }
    }

    return nullptr;
  }

  ZB_CHECK ZB_INLINE object get_opt_attribute(std::string_view att_name) const noexcept {
    const object* att = get_attribute(att_name);
    return att ? *att : nullptr;
  }

  ZB_CHECK ZB_INLINE object get_opt_attribute(std::string_view att_name, const object& alt) const noexcept {
    const object* att = get_attribute(att_name);
    return att ? *att : alt;
  }

  ZS_CHECK node_object* clone() const noexcept;

  friend std::ostream& operator<<(std::ostream& stream, const node_object& node);

  std::ostream& stream(std::ostream& stream) const;

private:
  object _name;
  object _value;
  zs::vector<object> _children;
  zs::vector<attribute> _attributes;

  node_object(zs::engine* eng) noexcept;
  std::ostream& stream_internal(std::ostream& stream, int indent = 0) const;
};
} // namespace zs.
