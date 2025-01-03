#include <zscript/zscript.h>

namespace zs {

inline constexpr object k_capture_uid = _sv("__capture_object__");
inline constexpr object k_capture_type_id = _sv("capture");

inline constexpr user_data_content k_capture_udata_content
    = { [](zs::engine* eng, zs::raw_pointer_t ptr) noexcept { ((capture*)ptr)->~capture(); },
        [](const zs::object_base& obj, std::ostream& stream) noexcept -> error_result {
          stream << obj.as_udata().data_ref<capture>().get_value();
          return {};
        },
        k_capture_uid, k_capture_type_id };

capture::capture(object* ptr) noexcept
    : _ptr(ptr) {}

capture& capture::as_capture(const object_base& obj) noexcept { return obj.as_udata().data_ref<capture>(); }

bool capture::is_capture(const object_base& obj) noexcept {
  return obj.is_user_data(&k_capture_udata_content);
}

object capture::create(zs::engine* eng, object* ptr) noexcept {
  if (user_data_object* uobj = user_data_object::create(eng, sizeof(capture), &k_capture_udata_content)) {
    zb_placement_new((void*)uobj->data()) capture(ptr);
    uobj->set_no_default_none();
    return object(uobj, false);
  }

  return nullptr;
}

void capture::bake() noexcept {
  if (_is_baked) {
    return;
  }
  _value = *_ptr;
  _ptr = &_value;
  _is_baked = true;
}

} // namespace zs.
