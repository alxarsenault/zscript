#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {

class object_internal_data {
public:
};

class table_object final : public delegable_object, public zs::object_unordered_map<object> {
public:
  ZS_OBJECT_CLASS_COMMON;

  using map_type = zs::object_unordered_map<object>;
  using size_type = map_type::size_type;
  using iterator = map_type::iterator;
  using const_iterator = map_type::const_iterator;

  ZS_CHECK static table_object* create(zs::engine* eng) noexcept;

  virtual ~table_object() override = default;

  ZS_CK_INLINE int_t size() const noexcept { return (int_t)map_type::size(); }

  ZS_CK_INLINE bool empty() const noexcept { return map_type::empty(); }

  bool contains(const object& key) const noexcept;

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE bool contains(const T& s) const noexcept {
    return map_type::contains(zs::_sv(std::string_view(s)));
  }

  zs::error_result contains(const object& key, object& dst) const noexcept;

  zs::error_result erase(const object& key) noexcept;

  zs::error_result get(const object& key, object& dst) const noexcept;

  ZS_CHECK zs::object* get(const object& key) noexcept;
  ZS_CHECK const zs::object* get(const object& key) const noexcept;

  template <class... Keys>
  ZS_CK_INLINE zs::object* multi_get(const object& key, const Keys&... keys) noexcept {

    if (auto it = map_type::find(key); it != map_type::end()) {
      return &it->second;
    }

    if constexpr (sizeof...(Keys)) {
      return multi_get(keys...);
    }

    return nullptr;
  }

  template <class... Keys>
  ZS_CK_INLINE const zs::object* multi_get(const object& key, const Keys&... keys) const noexcept {

    if (auto it = map_type::find(key); it != map_type::end()) {
      return &it->second;
    }

    if constexpr (sizeof...(Keys)) {
      return multi_get(keys...);
    }

    return nullptr;
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE zs::object* get(const T& s) noexcept {
    return get(zs::_sv(std::string_view(s)));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE const zs::object* get(const T& s) const noexcept {
    return get(zs::_sv(std::string_view(s)));
  }

  zs::object get_opt(const object& key, const object& opt = object()) const noexcept;

  //
  //
  //

  template <class Key, class Value>
  inline zs::error_result set(Key&& key, Value&& obj) noexcept {
    map_type::insert_or_assign(std::forward<Key>(key), std::forward<Value>(obj));
    return {};
  }

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
    return set_no_replace_internal(zs::_sv(std::string_view(s)), std::forward<T>(val));
  }

  ZS_CK_INLINE zs::error_result set_no_create(const object& key, const object& obj) noexcept {
    return set_no_create_internal(key, obj);
  }

  ZS_CK_INLINE zs::error_result set_no_create(const object& key, object&& obj) noexcept {
    return set_no_create_internal(key, std::move(obj));
  }

  ZS_CK_INLINE zs::error_result set_no_create(object&& key, object&& obj) noexcept {
    return set_no_create_internal(std::move(key), std::move(obj));
  }

  ZS_CK_INLINE zs::error_result set_no_create(object&& key, const object& obj) noexcept {
    return set_no_create_internal(std::move(key), obj);
  }

  template <class K, class T>
    requires std::is_constructible_v<std::string_view, K>
  ZS_CK_INLINE zs::error_result set_no_create(const K& s, T&& val) noexcept {
    return set_no_create_internal(zs::_s(get_engine(), s), std::forward<T>(val));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE const_iterator find(const T& s) const noexcept {
    return map_type::find(std::string_view(s));
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE iterator find(const T& s) noexcept {
    return map_type::find(std::string_view(s));
  }

  template <class T, class... _Args>
    requires std::is_constructible_v<std::string_view, T>
  inline std::pair<iterator, bool> emplace(const T& key, _Args&&... __args) {
    return map_type::emplace(zs::_s(get_engine(), std::string_view(key)), std::forward<_Args>(__args)...);
  }

  template <class... _Args>
  inline std::pair<iterator, bool> emplace(_Args&&... __args) {
    return map_type::emplace(std::forward<_Args>(__args)...);
  }

  ZS_CK_INLINE object& operator[](const object& key) noexcept { return map_type::operator[](key); }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE object& operator[](const T& s) noexcept {
    return map_type::operator[](zs::_s(get_engine(), s));
  }

  template <size_t N>
  ZS_CK_INLINE object& operator[](const char (&s)[N]) noexcept {
    if constexpr (N - 1 <= constants::k_small_string_max_size) {
      return map_type::operator[](zs::_ss(s));
    }
    else {
      return map_type::operator[](zs::_s(get_engine(), std::string_view(s)));
    }
  }

  template <class T>
    requires std::is_constructible_v<std::string_view, T>
  ZS_CK_INLINE size_type erase(const T& s) noexcept {
    return map_type::erase(zs::_s(get_engine(), s));
  }


  ZS_CHECK object clone() const noexcept override;
  
  ZS_CK_INLINE map_type& get_map() noexcept { return *this; }
  ZS_CK_INLINE const map_type& get_map() const noexcept { return *this; }

  zs::error_result serialize_to_json(zs::engine* eng, std::ostream& stream, int idt = 0);

private:
  table_object(zs::engine* eng);

  template <class Key, class T>
  ZS_CK_INLINE zs::error_result set_no_replace_internal(Key&& key, T&& obj) noexcept {
    return map_type::emplace(std::forward<Key>(key), std::forward<T>(obj)).second
        ? zs::error_code::success
        : zs::error_code::already_exists;
  }

  template <class Key, class T>
  ZS_CK_INLINE zs::error_result set_no_create_internal(Key&& key, T&& obj) noexcept {
    if (auto it = map_type::find(std::forward<Key>(key)); it != end()) {
      it->second = std::forward<T>(obj);
      return {};
    }

    return zs::errc::not_found;
  }
};
} // namespace zs.
