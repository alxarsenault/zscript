#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {

struct user_data_content {
  zs::user_data_release_hook_t release_hook = nullptr;
  zs::copy_user_data_to_type_t copy_fct = nullptr;
  zs::to_string_callback_t to_string_cb = nullptr;
  zs::object uid;
  zs::object type_id;
};

class user_data_object final : public zs::delegable_object {
public:
  ZS_OBJECT_CLASS_COMMON;
  
  ZS_CHECK static user_data_object* create(zs::engine* eng, size_t size);
  ZS_CHECK static user_data_object* create(zs::engine* eng, size_t size, const user_data_content* content);

  template <class T, class... Args>
  ZS_CHECK static user_data_object* create(zs::engine* eng, Args&&... args) {
    user_data_object* uobj = create(eng, sizeof(T));

    if (uobj) {
      uobj->construct<T>(std::forward<Args>(args)...);
      uobj->set_type_default_release_hook<T>();
    }

    return uobj;
  }

  virtual ~user_data_object() override;

  ZS_INLINE void set_release_hook(user_data_release_hook_t r) {
    ZS_ASSERT(owns_content());
    _content->release_hook = r;
  }

  template <class T>
  ZS_INLINE void set_type_default_release_hook() {
    ZS_ASSERT(owns_content());
    _content->release_hook = [](zs::engine* eng, zs::raw_pointer_t ptr) { ((T*)ptr)->~T(); };
  }

  ZS_CK_INLINE user_data_release_hook_t get_release_hook() const noexcept { return _content->release_hook; }

  template <class T, class... Args>
  ZS_INLINE T* construct(Args&&... args) {
    return zb_placement_new((void*)data()) T(std::forward<Args>(args)...);
  }

  ZS_CK_INLINE uint8_t* data() noexcept { return _data + (owns_content() ? sizeof(user_data_content) : 0); }

  ZS_CK_INLINE const uint8_t* data() const noexcept {
    return _data + (owns_content() ? sizeof(user_data_content) : 0);
  }

  template <class T>
  ZS_CK_INLINE T* data() noexcept {
    return reinterpret_cast<T*>(data());
  }

  template <class T>
  ZS_CK_INLINE const T* data() const noexcept {
    return reinterpret_cast<const T*>(data());
  }

  template <class T>
  ZS_CK_INLINE T& data_ref() noexcept {
    return *reinterpret_cast<T*>(data());
  }

  template <class T>
  ZS_CK_INLINE const T& data_ref() const noexcept {
    return *reinterpret_cast<const T*>(data());
  }
 

  template<class Object>
  inline void set_uid(Object&& uid) {
    ZS_ASSERT(owns_content());
    _content->uid = std::forward<Object>(uid);
  }

  ZS_CK_INLINE const zs::object& get_uid() const noexcept { return _content->uid; }

  template<class Object>
  inline void set_type_id(Object&& tid) {
    ZS_ASSERT(owns_content());
    _content->type_id = std::forward<Object>(tid);
  }
 

  ZS_CK_INLINE const zs::object& get_type_id() const noexcept { return _content->type_id; }
  ZS_CHECK object clone() const noexcept override;

  zs::error_result copy_to_type(void* obj, size_t data_size, std::string_view tid);

  inline void set_to_string_callback(zs::to_string_callback_t cb) noexcept {
    ZS_ASSERT(owns_content());
    _content->to_string_cb = cb;
  }

  zs::error_result convert_to_string(std::ostream& stream);

  inline bool owns_content() const noexcept { return _content == (user_data_content*)_data; }

  inline const user_data_content* get_content() const noexcept { return _content; }

private:
  inline user_data_object(zs::engine* eng) noexcept
      : delegable_object(eng, zs::object_type::k_user_data) {}


  user_data_content* _content = nullptr;
  uint8_t _data[1];
};
} // namespace zs.
