#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {

struct object_map_content : zs::object_map {
  using base = object_map;
  using base::base;
};

class table_object final : public delegable_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  using map_type = object_map_content;
  using value_type = map_type::value_type;
  using size_type = map_type::size_type;
  using iterator = map_type::iterator;
  using const_iterator = map_type::const_iterator;

  /// @brief Creates a table object.
  ZS_CHECK static table_object* create(zs::engine* eng) noexcept;

  /// @brief Creates a table object.
  ZS_CHECK static object create_object(zs::engine* eng) noexcept;

  //
  // MARK: Size.
  //

  ZS_CK_INLINE int_t size() const noexcept { return (int_t)_map->size(); }
  ZS_CK_INLINE bool empty() const noexcept { return _map->empty(); }

  ZS_INLINE void reserve(size_type n) noexcept { _map->reserve(n); }
  ZS_INLINE void clear() noexcept { _map->clear(); }

  //
  // MARK: Queries.
  //

  template <class K>
  ZS_CK_INLINE bool contains(const K& key) const noexcept {
    return _map->contains(key);
  }

  ZS_CHECK bool contains_all(const table_object& tbl) const noexcept;

  template <class K>
  ZS_CK_INLINE iterator find(const K& key) noexcept {
    return _map->find(key);
  }

  template <class K>
  ZS_CK_INLINE const_iterator find(const K& key) const noexcept {
    return _map->find(key);
  }

  ZS_CK_INLINE iterator begin() noexcept { return _map->begin(); }
  ZS_CK_INLINE iterator end() noexcept { return _map->end(); }
  ZS_CK_INLINE const_iterator begin() const noexcept { return _map->begin(); }
  ZS_CK_INLINE const_iterator end() const noexcept { return _map->end(); }
  ZS_CK_INLINE const_iterator cbegin() const noexcept { return _map->begin(); }
  ZS_CK_INLINE const_iterator cend() const noexcept { return _map->end(); }

  //
  // MARK: Get.
  //

  template <class K>
  ZS_CK_INLINE object& operator[](K&& key) noexcept {
    if constexpr (std::is_constructible_v<std::string_view, K>) {
      return (*_map)[zs::_s(get_engine(), key)];
    }
    else {
      return (*_map)[std::forward<K>(key)];
    }
  }

  zs::error_result get(const object& key, object& dst) const noexcept;

  template <class K>
  ZS_CK_INLINE zs::object* get(const K& key) noexcept {
    auto it = _map->find(key);
    return it == _map->end() ? nullptr : &it->second;
  }

  template <class K>
  ZS_CK_INLINE const zs::object* get(const K& key) const noexcept {
    auto it = _map->find(key);
    return it == _map->end() ? nullptr : &it->second;
  }

  //
  // MARK: Set.
  //

  template <class Key, class Value>
  inline zs::error_result set(Key&& key, Value&& obj) noexcept {

    if constexpr (std::is_constructible_v<std::string_view, Key>) {
      _map->insert_or_assign(zs::_s(get_engine(), key), std::forward<Value>(obj));
    }
    else {
      _map->insert_or_assign(std::forward<Key>(key), std::forward<Value>(obj));
      return {};
    }
  }

  template <class Key, class Value>
  inline zs::error_result set_no_replace(Key&& key, Value&& obj) noexcept {

    if (find(key) != end()) {
      return errc::already_exists;
    }

    if constexpr (std::is_constructible_v<std::string_view, Key>) {
      _map->emplace(zs::_s(get_engine(), std::string_view(key)), std::forward<Value>(obj));
      return {};
    }
    else {
      _map->emplace(std::forward<Key>(key), std::forward<Value>(obj));
      return {};
    }
  }

  template <class Key, class Value>
  inline zs::error_result set_no_create(const Key& key, Value&& obj) noexcept {
    if (auto it = find(key); it != end()) {
      it->second = std::forward<Value>(obj);
      return {};
    }

    return errc::not_found;
  }

  template <class K, class T>
  ZS_INLINE std::pair<iterator, bool> insert_or_assign(K&& key, T&& val) {
    return _map->insert_or_assign(std::forward<K>(key), std::forward<T>(val));
  }

  template <class K, class... _Args>
  inline std::pair<iterator, bool> emplace(K&& key, _Args&&... args) {
    if constexpr (std::is_constructible_v<std::string_view, K>) {
      return _map->emplace(zs::_s(get_engine(), std::string_view(key)), std::forward<_Args>(args)...);
    }
    else {
      return _map->emplace(std::forward<K>(key), std::forward<_Args>(args)...);
    }
  }

  zs::error_result erase(const object& key) noexcept;

  ZS_CK_INLINE map_type& get_map() noexcept { return *_map; }
  ZS_CK_INLINE const map_type& get_map() const noexcept { return *_map; }

  /// @brief Compare the content of two tables.
  ZS_CK_INLINE bool operator==(const table_object& tbl) const noexcept { return *_map == *tbl._map; }

  zs::error_result serialize_to_json(zs::engine* eng, std::ostream& stream, int idt = 0);

private:
  map_type* _map;
  uint8_t _data[1];

  table_object(zs::engine* eng);
  ~table_object() noexcept = default;

  static void destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept;
  static object clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept;
};
} // namespace zs.
