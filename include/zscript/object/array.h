#ifndef ZS_SCRIPT_INCLUDE_OBJECTS
#error This file should only be included in object.h
#endif // ZS_SCRIPT_INCLUDE_OBJECTS

namespace zs {
/// Array.
/// Only looks in delegate for non-number keys.
class array_object final : public delegable_object, public zs::vector<object> {
public:
  ZS_OBJECT_CLASS_COMMON;

  using vector_type = zs::vector<object>;

  ZS_CHECK static array_object* create(zs::engine* eng, int_t sz) noexcept;

  virtual ~array_object() override = default;

  ZS_CHECK int_t size() const noexcept { return (int_t)vector_type::size(); }

  zs::error_result get(int_t idx, object& dst) const noexcept;
  zs::error_result get(const object& key, object& dst) const noexcept;

  using vector_type::contains;
  zs::error_result contains(const object& key, object& dst) const noexcept;

  zs::error_result set(int_t idx, const object& obj) noexcept;
  zs::error_result set(int_t idx, object&& obj) noexcept;

  zs::error_result push(const object& obj) noexcept;
  zs::error_result push(object&& obj) noexcept;

  ZS_CHECK object clone() const noexcept override;

  ZS_CHECK bool is_number_array() const noexcept;
  ZS_CHECK bool is_string_array() const noexcept;
  ZS_CHECK bool is_type_array(object_type t) const noexcept;
  ZS_CHECK bool is_type_mask_array(uint32_t tflags) const noexcept;
  ZS_CHECK uint32_t get_array_type_mask() const noexcept;
  ZS_CK_INLINE vector_type& to_vec() noexcept { return *this; }
  ZS_CK_INLINE const vector_type& to_vec() const noexcept { return *this; }

  ZS_CK_INLINE const object* end_ptr() const noexcept { return vector_type::data() + vector_type::size(); }

  ZS_CK_INLINE bool is_ptr_in_range(const object* ptr) const noexcept {
    return ptr >= vector_type::data() and ptr < end_ptr();
  }

  zs::error_result serialize_to_json(zs::engine* eng, std::ostream& stream, int idt = 0);

private:
  array_object(zs::engine* eng) noexcept;
};
} // namespace zs.
