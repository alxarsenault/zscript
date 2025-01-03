#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {
/// Array.
class array_object final : public delegable_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  using vector_type = zs::vector<object>;
  using value_type = vector_type::value_type;
  using size_type = vector_type::size_type;
  using iterator = vector_type::iterator;
  using const_iterator = vector_type::const_iterator;

  ZS_CHECK static array_object* create(zs::engine* eng, int_t sz) noexcept;
  ZS_CHECK static object create_object(zs::engine* eng, int_t sz) noexcept;

  ZS_CK_INLINE int_t size() const noexcept { return (int_t)_vec->size(); }
  ZS_CK_INLINE size_t capacity() const noexcept { return _vec->capacity(); }
  ZS_CK_INLINE bool empty() const noexcept { return _vec->empty(); }
  ZS_INLINE void clear() noexcept { _vec->clear(); }
  ZS_INLINE void pop_back() noexcept { _vec->pop_back(); }
  ZS_INLINE void resize(size_t sz) noexcept { _vec->resize(sz); }
  ZS_INLINE void reserve(size_t sz) noexcept { _vec->reserve(sz); }

  ZS_CK_INLINE bool contains(const object& item) const noexcept { return _vec->contains(item); }
  ZS_CK_INLINE bool contains(std::string_view item) const noexcept { return _vec->contains(zs::_sv(item)); }

  ZS_CK_INLINE object& back() noexcept { return _vec->back(); }
  ZS_CK_INLINE const object& back() const noexcept { return _vec->back(); }

  ZS_CK_INLINE object* data() noexcept { return _vec->data(); }
  ZS_CK_INLINE const object* data() const noexcept { return _vec->data(); }

  ZS_CK_INLINE iterator begin() noexcept { return _vec->begin(); }
  ZS_CK_INLINE iterator end() noexcept { return _vec->end(); }
  ZS_CK_INLINE const_iterator begin() const noexcept { return _vec->begin(); }
  ZS_CK_INLINE const_iterator end() const noexcept { return _vec->end(); }
  ZS_CK_INLINE const_iterator cbegin() const noexcept { return _vec->begin(); }
  ZS_CK_INLINE const_iterator cend() const noexcept { return _vec->end(); }

  zs::error_result get(int_t idx, object& dst) const noexcept;
  zs::error_result get(const object& key, object& dst) const noexcept;

  template <class... Args>
  ZS_INLINE auto erase(Args&&... args) {
    return _vec->erase(std::forward<Args>(args)...);
  }

  template <class... Args>
  ZS_INLINE auto insert(Args&&... args) {
    return _vec->insert(std::forward<Args>(args)...);
  }

  zs::error_result contains(const object& key, object& dst) const noexcept;

  ZS_CK_INLINE object& operator[](size_t index) noexcept { return (*_vec)[index]; }

  ZS_CK_INLINE const object& operator[](size_t index) const noexcept { return (*_vec)[index]; }

  template <class T>
  ZS_INLINE void push_back(T&& value) {
    _vec->push_back(std::forward<T>(value));
  }

  template <class... Args>
  ZS_INLINE void emplace_back(Args&&... args) {
    _vec->emplace_back(std::forward<Args>(args)...);
  }

  zs::error_result set(int_t idx, const object& obj) noexcept;
  zs::error_result set(int_t idx, object&& obj) noexcept;

  zs::error_result push(const object& obj) noexcept;
  zs::error_result push(object&& obj) noexcept;

  ZS_CHECK bool is_number_array() const noexcept;
  ZS_CHECK bool is_number_array(bool& has_float) const noexcept;

  ZS_CHECK bool is_string_array() const noexcept;
  ZS_CHECK bool is_type_array(object_type t) const noexcept;
  ZS_CHECK bool is_type_mask_array(uint32_t tflags) const noexcept;
  ZS_CHECK uint32_t get_array_type_mask() const noexcept;

  ZS_CK_INLINE vector_type& to_vec() noexcept { return *_vec; }
  ZS_CK_INLINE const vector_type& to_vec() const noexcept { return *_vec; }

  ZS_CK_INLINE const object* end_ptr() const noexcept { return _vec->data() + _vec->size(); }

  ZS_CK_INLINE bool is_ptr_in_range(const object* ptr) const noexcept {
    return ptr >= _vec->data() and ptr < end_ptr();
  }

  zs::error_result serialize_to_json(zs::engine* eng, std::ostream& stream, int idt = 0);

  ZS_CK_INLINE bool operator==(const array_object& arr) const noexcept { return *_vec == *arr._vec; }
  ZS_CK_INLINE bool operator!=(const array_object& arr) const noexcept { return *_vec != *arr._vec; }
  ZS_CK_INLINE bool operator<(const array_object& arr) const noexcept { return *_vec < *arr._vec; }
  ZS_CK_INLINE bool operator>(const array_object& arr) const noexcept { return *_vec > *arr._vec; }
  ZS_CK_INLINE bool operator<=(const array_object& arr) const noexcept { return *_vec <= *arr._vec; }
  ZS_CK_INLINE bool operator>=(const array_object& arr) const noexcept { return *_vec >= *arr._vec; }

private:
  vector_type* _vec;
  uint8_t _data[1];

  array_object(zs::engine* eng) noexcept;
  ~array_object() noexcept = default;

  static void destroy_callback(zs::engine* eng, reference_counted_object* obj) noexcept;
  static object clone_callback(zs::engine* eng, const reference_counted_object* obj) noexcept;
};
} // namespace zs.
