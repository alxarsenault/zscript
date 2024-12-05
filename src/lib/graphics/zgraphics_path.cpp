

#include "zgraphics_common.h"

namespace zs::graphics {

zs::error_result graphic_path_parameter::parse(
    parameter_stream& s, bool output_error, graphics::graphic_path_object*& value) {
  if (!s.is_user_data_with_uid(graphics::s_graphic_path_object_typeid)) {
    s.set_opt_error(output_error, "Invalid graphic path type.");
    return zs::errc::invalid_parameter_type;
  }

  value = &graphic_path_object::get(*s);

  ++s;
  return {};
}

#if __ZBASE_APPLE__
graphic_path_object::graphic_path_object()
    : path_ref(CGPathCreateMutable()) {}

graphic_path_object::~graphic_path_object() {
  if (path_ref) {
    CGPathRelease(path_ref);
    path_ref = nullptr;
  }
}

zs::error_result graphic_path_object::move_to(float_t x, float_t y) {
  CGPathMoveToPoint(path_ref, nullptr, x, y);
  return {};
}

zs::error_result graphic_path_object::line_to(float_t x, float_t y) {
  CGPathAddLineToPoint(path_ref, nullptr, x, y);
  return {};
}

zs::error_result graphic_path_object::add_line(float_t x1, float_t y1, float_t x2, float_t y2) {
  CGPathMoveToPoint(path_ref, nullptr, x1, y1);
  CGPathAddLineToPoint(path_ref, nullptr, x2, y2);
  return {};
}

zs::error_result graphic_path_object::close_sub_path() {
  CGPathCloseSubpath(path_ref);
  return {};
}

zs::error_result graphic_path_object::add_rect(float_t x, float_t y, float_t w, float_t h) {
  CGPathAddRect(path_ref, nullptr, CGRectMake(x, y, w, h));
  return {};
}

zs::error_result graphic_path_object::quadratic_curve_to(float_t x, float_t y, float_t cpx, float_t cpy) {
  CGPathAddQuadCurveToPoint(path_ref, nullptr, cpx, cpy, x, y);
  return {};
}

zs::error_result graphic_path_object::bezier_curve_to(
    float_t x, float_t y, float_t cp1x, float_t cp1y, float_t cp2x, float_t cp2y) {
  CGPathAddCurveToPoint(path_ref, nullptr, cp1x, cp1y, cp2x, cp2y, x, y);
  return {};
}

zs::error_result graphic_path_object::add_ellipse_in_rect(float_t x, float_t y, float_t w, float_t h) {
  CGPathAddEllipseInRect(path_ref, nullptr, CGRectMake(x, y, w, h));
  return {};
}

zs::error_result graphic_path_object::add_path(const graphic_path_object& p) {
  CGPathAddPath(path_ref, nullptr, p.get_native_path());
  return {};
}

zs::error_result graphic_path_object::add_path(
    const zb::transform<float_t>& t, const graphic_path_object& p) {
  CGAffineTransform afft = t;
  CGPathAddPath(path_ref, &afft, p.get_native_path());
  return {};
}

zs::error_result graphic_path_object::add_rounded_rect(
    float_t x, float_t y, float_t w, float_t h, float_t cw, float_t ch) {
  CGPathAddRoundedRect(path_ref, nullptr, CGRectMake(x, y, w, h), cw, ch);
  return {};
}

zs::error_result graphic_path_object::add_rounded_rect(
    const zb::transform<float_t>& t, float_t x, float_t y, float_t w, float_t h, float_t cw, float_t ch) {
  CGAffineTransform afft = t;
  CGPathAddRoundedRect(path_ref, &afft, CGRectMake(x, y, w, h), cw, ch);
  return {};
}

#endif // __ZBASE_APPLE__.

static zs::object create_graphic_path_delegate(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();

  object delegate = zs::_t(eng);
  table_object& tbl = delegate.as_table();
  tbl.emplace(zs::_ss("__typeof"), zs::_s(eng, "graphics_path"));

  tbl.emplace("move_to", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_path_object* path_obj = nullptr;
    if (auto err = ps.require<graphic_path_parameter>(path_obj)) {
      return -1;
    }

    zb::point<float_t> pos;
    if (auto err = ps.require<graphic_point_parameter>(pos)) {
      return -1;
    }

    path_obj->move_to(pos.x, pos.y);
    return 0;
  });

  tbl.emplace("line_to", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_path_object* path_obj = nullptr;
    if (auto err = ps.require<graphic_path_parameter>(path_obj)) {
      return -1;
    }

    zb::point<float_t> pos;
    if (auto err = ps.require<graphic_point_parameter>(pos)) {
      return -1;
    }
    path_obj->line_to(pos.x, pos.y);
    return 0;
  });

  tbl.emplace("add_line", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_path_object* path_obj = nullptr;
    if (auto err = ps.require<graphic_path_parameter>(path_obj)) {
      return -1;
    }

    zb::point<float_t> start_pos;
    if (auto err = ps.require<graphic_point_parameter>(start_pos)) {
      return -1;
    }

    zb::point<float_t> end_pos;
    if (auto err = ps.require<graphic_point_parameter>(end_pos)) {
      return -1;
    }

    path_obj->add_line(start_pos.x, start_pos.y, end_pos.x, end_pos.y);
    return 0;
  });

  tbl.emplace("add_rounded_rect", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_path_object* path_obj = nullptr;
    if (auto err = ps.require<graphic_path_parameter>(path_obj)) {
      return -1;
    }

    zb::transform<float_t> t;
    bool has_transform = !ps.optional<graphic_transform_parameter>(t);

    float_t x, y, w, h;
    if (auto err = ps.require<graphic_rect_parameter>(x, y, w, h)) {
      return -1;
    }

    float_t cw;
    if (auto err = ps.require<number_parameter>(cw)) {
      return -1;
    }

    float_t ch;
    if (auto err = ps.require<number_parameter>(ch)) {
      return -1;
    }

    if (has_transform) {
      path_obj->add_rounded_rect(t, x, y, w, h, cw, ch);
    }
    else {
      path_obj->add_rounded_rect(x, y, w, h, cw, ch);
    }
    return 0;
  });

  tbl.emplace("add_path", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_path_object* path_obj = nullptr;
    if (auto err = ps.require<graphic_path_parameter>(path_obj)) {
      return -1;
    }

    zb::transform<float_t> t;
    bool has_transform = !ps.optional<graphic_transform_parameter>(t);

    graphics::graphic_path_object* in_path_obj = nullptr;
    if (auto err = ps.require<graphic_path_parameter>(in_path_obj)) {
      return -1;
    }

    if (has_transform) {
      path_obj->add_path(t, *in_path_obj);
    }
    else {
      path_obj->add_path(*in_path_obj);
    }
    return 0;
  });

  return delegate;
}

static const zs::object& get_graphic_path_delegate(zs::vm_ref vm) {
  table_object& reg = vm.get_registry_table_object();

  if (auto it = reg.find(zs::_sv(s_graphic_path_delegate_register_uid)); it != reg.end()) {
    return it->second;
  }

  return reg.emplace(zs::_sv(s_graphic_path_delegate_register_uid), create_graphic_path_delegate(vm))
      .first->second;
}

zs::object create_graphic_path(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();

  zs::object obj;
  obj._type = zs::object_type::k_user_data;

  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(graphic_path_object));
  obj._udata = uobj;

  zb_placement_new(uobj->data()) graphic_path_object();

  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    graphic_path_object* t = (graphic_path_object*)ptr;
    t->~graphic_path_object();
  });

  uobj->set_uid(zs::_sv(graphics::s_graphic_path_object_typeid));
  uobj->set_typeid(zs::_sv(graphics::s_graphic_path_object_typeid));

  obj.as_udata().set_delegate(get_graphic_path_delegate(vm));

  return obj;
}

} // namespace zs::graphics.
