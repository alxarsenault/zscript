#include <zscript/core/objects/object_include_guard.h>

namespace zs {
class user_data_object final : public zs::delegate_object {
public:
  ZS_OBJECT_CLASS_COMMON;

  ZS_CHECK static user_data_object* create(zs::engine* eng, size_t size);

  virtual ~user_data_object() override;

  ZS_INLINE void set_release_hook(user_data_release_hook_t r) { _release_hook = r; }

  ZS_CK_INLINE user_data_release_hook_t get_release_hook() const noexcept { return _release_hook; }

  ZS_CK_INLINE uint8_t* data() noexcept { return _data; }

  ZS_CK_INLINE const uint8_t* data() const noexcept { return _data; }

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

  inline void set_uid(const object& uid) { _uid = uid; }
  inline void set_uid(object&& uid) { _uid = std::move(uid); }

  inline zs::object& get_uid() noexcept { return _uid; }
  inline const zs::object& get_uid() const noexcept { return _uid; }

  inline void set_typeid(const object& tid) { _type_id = tid; }
  inline void set_typeid(object&& tid) { _type_id = std::move(tid); }

  inline zs::object& get_typeid() noexcept { return _type_id; }
  inline const zs::object& get_typeid() const noexcept { return _type_id; }

  zs::error_result copy_to_type(void* obj, size_t data_size, std::string_view tid);

  inline void set_to_string_callback(zs::to_string_callback_t cb) noexcept { _to_string_cb = cb; }

  zs::error_result convert_to_string(std::ostream& stream);

private:
  inline user_data_object(zs::engine* eng) noexcept
      : delegate_object(eng) {}

  user_data_release_hook_t _release_hook = nullptr;
  zs::object _uid;
  zs::object _type_id;
  zs::copy_user_data_to_type_t _copy_fct = nullptr;
  zs::to_string_callback_t _to_string_cb = nullptr;
  uint8_t _data[1];
};
} // namespace zs.
