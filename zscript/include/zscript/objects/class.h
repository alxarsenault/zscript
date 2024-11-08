#include <zscript/objects/object_include_guard.h>

namespace zs {
class class_object final : public delegate_object, private zs::object_unordered_map<object> {
public:
  ZS_OBJECT_CLASS_COMMON;

  using map_type = zs::object_unordered_map<object>;
  using size_type = map_type::size_type;
  using iterator = map_type::iterator;
  using const_iterator = map_type::const_iterator;

  using map_type::begin;
  using map_type::end;
  using map_type::operator[];
  using map_type::contains;
  using map_type::insert;
  using map_type::emplace;
  using map_type::try_emplace;
  using map_type::find;
  using map_type::erase;
  using map_type::clear;
  using map_type::empty;

  ZS_CHECK static class_object* create(zs::engine* eng) noexcept;

  virtual ~class_object() override = default;

  object create_instance();

  ZS_CK_INLINE int_t size() const noexcept { return (int_t)map_type::size(); }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE bool contains(const T& s) const noexcept {
    return map_type::contains(zs::object::create_string(get_engine(), s));
  }

  zs::error_result get(const object& key, object& dst) const noexcept;

  ZS_CHECK zs::object* get(const object& key) noexcept;
  ZS_CHECK const zs::object* get(const object& key) const noexcept;

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE zs::object* get(const T& s) noexcept {
    return get(zs::_s(get_engine(), s));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE const zs::object* get(const T& s) const noexcept {
    return get(zs::_s(get_engine(), s));
  }

  zs::object get_opt(const object& key, const object& opt = object()) const noexcept;

  //
  //
  //

  zs::error_result set(const object& key, const object& obj) noexcept;
  zs::error_result set(const object& key, object&& obj) noexcept;
  zs::error_result set(object&& key, object&& obj) noexcept;
  zs::error_result set(object&& key, const object& obj) noexcept;

  template <class K, class T>
    requires std::is_constructible_v<std::string_view, K>
  ZS_INLINE zs::error_result set(const K& s, T&& val) noexcept {
    return set(zs::_s(get_engine(), s), std::forward<T>(val));
  }

  ZS_CK_INLINE zs::error_result set_no_replace(const object& key, const object& obj) noexcept {
    return set_no_replace_internal(key, obj);
  }

  ZS_CK_INLINE zs::error_result set_no_replace(const object& key, object&& obj) noexcept {
    return set_no_replace_internal(key, std::move(obj));
  }

  ZS_CK_INLINE zs::error_result set_no_replace(object&& key, object&& obj) noexcept {
    return set_no_replace_internal(std::move(key), std::move(obj));
  }

  ZS_CK_INLINE zs::error_result set_no_replace(object&& key, const object& obj) noexcept {
    return set_no_replace_internal(std::move(key), obj);
  }

  template <class K, class T>
    requires std::is_constructible_v<std::string_view, K>
  ZS_CK_INLINE zs::error_result set_no_replace(const K& s, T&& val) noexcept {
    return set_no_replace_internal(zs::_s(get_engine(), s), std::forward<T>(val));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE const_iterator find(const T& s) const noexcept {
    return map_type::find(zs::_s(get_engine(), s));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE iterator find(const T& s) noexcept {
    return map_type::find(zs::_s(get_engine(), s));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE object& operator[](const T& s) noexcept {
    return map_type::operator[](zs::_s(get_engine(), s));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE size_type erase(const T& s) noexcept {
    return map_type::erase(zs::_s(get_engine(), s));
  }

  //  ZS_CHECK table_object* clone() const noexcept;

  ZS_CK_INLINE map_type& get_map() noexcept { return *this; }
  ZS_CK_INLINE const map_type& get_map() const noexcept { return *this; }

private:
  class_object(zs::engine* eng);

  template <class Key, class T>
  ZS_CK_INLINE zs::error_result set_no_replace_internal(Key&& key, T&& obj) noexcept {
    return emplace(std::forward<Key>(key), std::forward<T>(obj)).second ? zs::error_code::already_exists
                                                                        : zs::error_code::success;
  }
};

class class_instance_object final : public delegate_object, private zs::object_unordered_map<object> {
public:
  ZS_OBJECT_CLASS_COMMON;

  using map_type = zs::object_unordered_map<object>;
  using size_type = map_type::size_type;
  using iterator = map_type::iterator;
  using const_iterator = map_type::const_iterator;

  using map_type::begin;
  using map_type::end;
  using map_type::operator[];
  using map_type::contains;
  using map_type::insert;
  using map_type::emplace;
  using map_type::try_emplace;
  using map_type::find;
  using map_type::erase;
  using map_type::clear;
  using map_type::empty;

  ZS_CHECK static class_instance_object* create(zs::engine* eng, zs::object cls) noexcept;

  virtual ~class_instance_object() override = default;

  ZS_CK_INLINE object& get_class() noexcept { return _cls; }
  ZS_CK_INLINE const object& get_class() const noexcept { return _cls; }

  ZS_CK_INLINE int_t size() const noexcept { return (int_t)map_type::size(); }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE bool contains(const T& s) const noexcept {
    return map_type::contains(zs::object::create_string(get_engine(), s));
  }

  zs::error_result get(const object& key, object& dst) const noexcept;

  ZS_CHECK zs::object* get(const object& key) noexcept;
  ZS_CHECK const zs::object* get(const object& key) const noexcept;

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE zs::object* get(const T& s) noexcept {
    return get(zs::_s(get_engine(), s));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE const zs::object* get(const T& s) const noexcept {
    return get(zs::_s(get_engine(), s));
  }

  zs::object get_opt(const object& key, const object& opt = object()) const noexcept;

  //
  //
  //

  zs::error_result set(const object& key, const object& obj) noexcept;
  zs::error_result set(const object& key, object&& obj) noexcept;
  zs::error_result set(object&& key, object&& obj) noexcept;
  zs::error_result set(object&& key, const object& obj) noexcept;

  template <class K, class T>
    requires std::is_constructible_v<std::string_view, K>
  ZS_INLINE zs::error_result set(const K& s, T&& val) noexcept {
    return set(zs::_s(get_engine(), s), std::forward<T>(val));
  }

  ZS_CK_INLINE zs::error_result set_no_replace(const object& key, const object& obj) noexcept {
    return set_no_replace_internal(key, obj);
  }

  ZS_CK_INLINE zs::error_result set_no_replace(const object& key, object&& obj) noexcept {
    return set_no_replace_internal(key, std::move(obj));
  }

  ZS_CK_INLINE zs::error_result set_no_replace(object&& key, object&& obj) noexcept {
    return set_no_replace_internal(std::move(key), std::move(obj));
  }

  ZS_CK_INLINE zs::error_result set_no_replace(object&& key, const object& obj) noexcept {
    return set_no_replace_internal(std::move(key), obj);
  }

  template <class K, class T>
    requires std::is_constructible_v<std::string_view, K>
  ZS_CK_INLINE zs::error_result set_no_replace(const K& s, T&& val) noexcept {
    return set_no_replace_internal(zs::_s(get_engine(), s), std::forward<T>(val));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE const_iterator find(const T& s) const noexcept {
    return map_type::find(zs::_s(get_engine(), s));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE iterator find(const T& s) noexcept {
    return map_type::find(zs::_s(get_engine(), s));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE object& operator[](const T& s) noexcept {
    return map_type::operator[](zs::_s(get_engine(), s));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE size_type erase(const T& s) noexcept {
    return map_type::erase(zs::_s(get_engine(), s));
  }

  //  ZS_CHECK table_object* clone() const noexcept;

  ZS_CK_INLINE map_type& get_map() noexcept { return *this; }
  ZS_CK_INLINE const map_type& get_map() const noexcept { return *this; }

private:
  class_instance_object(zs::engine* eng, zs::object cls);

  object _cls;

  template <class Key, class T>
  ZS_CK_INLINE zs::error_result set_no_replace_internal(Key&& key, T&& obj) noexcept {
    return emplace(std::forward<Key>(key), std::forward<T>(obj)).second ? zs::error_code::already_exists
                                                                        : zs::error_code::success;
  }
};
} // namespace zs.
