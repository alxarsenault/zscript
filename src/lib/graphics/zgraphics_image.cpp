

#include "zgraphics_common.h"

namespace zs::graphics {

zs::error_result graphic_image_parameter::parse(
    parameter_stream& s, bool output_error, graphics::graphic_image_object*& value) {

  if (!s.is_user_data_with_uid(graphics::s_graphic_image_object_typeid)) {
    s.set_opt_error(output_error, "Invalid graphic image type.");
    return zs::errc::invalid_parameter_type;
  }

  value = &graphic_image_object::get(*s);

  ++s;
  return {};
}

#if __ZBASE_APPLE__
graphic_image_object::graphic_image_object()
    : img_ref(nullptr) {}

graphic_image_object::~graphic_image_object() {
  if (img_ref) {
    CGImageRelease(img_ref);
    img_ref = nullptr;
  }
}

zs::error_result graphic_image_object::load_from_file(std::string_view path) {

  if (img_ref) {
    CGImageRelease(img_ref);
    img_ref = nullptr;
  }

  zb::apple::cf::url_ref url(path);
  zb::apple::cf::pointer<CGImageSourceRef> img_src(CGImageSourceCreateWithURL(url, nullptr));

  if (!img_src) {
    return zs::errc::invalid;
  }

  img_ref = CGImageSourceCreateImageAtIndex(img_src, 0, nullptr);
  return {};
}
#endif // __ZBASE_APPLE__.

static zs::object create_graphic_image_delegate(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();

  object delegate = zs::_t(eng);
  table_object& tbl = delegate.as_table();

  tbl.emplace(zs::_ss("__typeof"), zs::_s(eng, "graphics_image"));

  tbl.emplace("load", [](vm_ref vm) -> int_t {
    if (auto err = graphic_image_object::get(vm[0]).load_from_file(vm[1].get_string_unchecked())) {
      vm.set_error("Could not load image.");
      return -1;
    }

    return 0;
  });

  return delegate;
}

static const zs::object& get_graphic_image_delegate(zs::vm_ref vm) {
  table_object& reg = vm.get_registry_table_object();

  if (auto it = reg.find(zs::_sv(s_graphic_image_delegate_register_uid)); it != reg.end()) {
    return it->second;
  }

  return reg.emplace(zs::_sv(s_graphic_image_delegate_register_uid), create_graphic_image_delegate(vm))
      .first->second;
}

zs::object create_graphic_image(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();

  zs::object obj;
  obj._type = zs::object_type::k_user_data;

  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(graphic_image_object));
  obj._udata = uobj;

  zb_placement_new(uobj->data()) graphic_image_object();

  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    graphic_image_object* t = (graphic_image_object*)ptr;
    t->~graphic_image_object();
  });

  uobj->set_uid(zs::_sv(graphics::s_graphic_image_object_typeid));
  uobj->set_typeid(zs::_sv(graphics::s_graphic_image_object_typeid));

  obj.as_udata().set_delegate(get_graphic_image_delegate(vm));

  return obj;
}

} // namespace zs::graphics.
