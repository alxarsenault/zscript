

#include "zgraphics_common.h"
#include "objects/zfunction_prototype.h"
#include <zbase/sys/path.h>

namespace zs::graphics {

zs::error_result graphic_context_parameter::parse(
    parameter_stream& s, bool output_error, graphics::graphic_context_object*& value) {
  if (!s.is_user_data_with_uid(graphics::s_graphic_context_object_typeid)) {
    s.set_opt_error(output_error, "Invalid graphic context type.");
    return zs::errc::invalid_parameter_type;
  }

  value = &graphic_context_object::get(*s);

  ++s;
  return {};
}

graphic_context_object::~graphic_context_object() {
  if (ctx) {
    CGContextRelease(ctx);
    ctx = nullptr;
  }
}

zs::error_result graphic_context_object::init_bitmap_context(int_t width, int_t height) {
  _is_bitmap_context = true;
  _size = { (float_t)width, (float_t)height };

  CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();

  ctx = CGBitmapContextCreate(nullptr, width, height, 8, width * 4, color_space,
      (uint32_t)kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Little);

  CGColorSpaceRelease(color_space);

  // Vertical flip.
  CGContextConcatCTM(ctx, CGAffineTransformMake(1, 0, 0, -1, 0, height));

  return {};
}

zs::error_result graphic_context_object::set_stroke_color(zb::color color) {
  CGColorRef color_ref
      = CGColorCreateGenericRGB(color.f_red(), color.f_green(), color.f_blue(), color.f_alpha());
  CGContextSetStrokeColorWithColor(ctx, color_ref);
  CGColorRelease(color_ref);
  return {};
}

zs::error_result graphic_context_object::set_fill_color(zb::color color) {
  CGColorRef color_ref
      = CGColorCreateGenericRGB(color.f_red(), color.f_green(), color.f_blue(), color.f_alpha());
  CGContextSetFillColorWithColor(ctx, color_ref);
  CGColorRelease(color_ref);
  return {};
}

zs::error_result graphic_context_object::set_stroke_width(float_t w) {
  CGContextSetLineWidth(ctx, w);
  return {};
}

// zs::error_result graphic_context_object::fill_rect(float_t x, float_t y, float_t w, float_t h) {
//   CGLayerRef  layer_ref =  CGLayerCreateWithContext(ctx, CGSize{w, h}, nullptr);
//   CGContextRef ccc = CGLayerGetContext(layer_ref);
//   zb::color color(0xFF0000FF);
//   CGColorRef color_ref
//       = CGColorCreateGenericRGB(color.f_red(), color.f_green(), color.f_blue(), color.f_alpha());
//   CGContextSetFillColorWithColor(ccc, color_ref);
//   CGColorRelease(color_ref);
//
//
//   CGContextFillRect(ccc, CGRectMake(0, 0, w, h));
//
//   CGContextDrawLayerAtPoint(ctx, CGPoint{x, y}, layer_ref);
//   return {};
// }

zs::error_result graphic_context_object::fill_rect(float_t x, float_t y, float_t w, float_t h) {
  CGContextFillRect(ctx, CGRectMake(x, y, w, h));
  return {};
}

zs::error_result graphic_context_object::stroke_rect(float_t x, float_t y, float_t w, float_t h) {
  CGContextStrokeRect(ctx, CGRectMake(x, y, w, h));
  return {};
}

zs::error_result graphic_context_object::push_state() {
  CGContextSaveGState(ctx);
  return {};
}

zs::error_result graphic_context_object::pop_state() {
  CGContextRestoreGState(ctx);
  return {};
}

zs::error_result graphic_context_object::translate(float_t x, float_t y) {
  CGContextTranslateCTM(ctx, x, y);
  return {};
}

zs::error_result graphic_context_object::scale(float_t x, float_t y) {
  CGContextScaleCTM(ctx, x, y);
  return {};
}

zs::error_result graphic_context_object::rotate(float_t angle) {
  CGContextRotateCTM(ctx, angle);
  return {};
}

zs::error_result graphic_context_object::apply_transform(const zb::transform<float_t>& t) {
  CGContextConcatCTM(ctx, t);
  return {};
}

zs::error_result graphic_context_object::clip_to_rect(float_t x, float_t y, float_t w, float_t h) {
  CGContextClipToRect(ctx, CGRectMake(x, y, w, h));
  return {};
}

zs::error_result graphic_context_object::flush() {
  CGContextFlush(ctx);
  return {};
}

static constexpr const char* get_image_type_uid(image_type img_type) noexcept {
  using enum image_type;
  switch (img_type) {
  case unknown:
  case png:
    return "public.png";

  case jpeg:
    return "public.jpeg";

  case bmp:
    return "com.microsoft.bmp";

  case ico:
    return "com.microsoft.ico";

  case icons:
    return "com.apple.icns";

  case gif:
    return "com.compuserve.gif";

  case tiff:
    return "public.tiff";
  }

  return "public.png";
}

static constexpr image_type get_image_type_from_extension(std::string_view ext) noexcept {
  using enum image_type;

  if (zb::is_one_of(ext, "png", "PNG")) {
    return png;
  }
  else if (zb::is_one_of(ext, "jpg", "jpeg", "JPG", "JPEG")) {
    return jpeg;
  }
  else if (zb::is_one_of(ext, "bmp", "BMP")) {
    return bmp;
  }

  else if (zb::is_one_of(ext, "ico", "ICO")) {
    return ico;
  }
  else if (ext == "icons") {
    return icons;
  }
  else if (zb::is_one_of(ext, "gif", "GIF")) {
    return gif;
  }
  else if (zb::is_one_of(ext, "tiff", "TIFF")) {
    return tiff;
  }
  else {
    return png;
  }
}

zs::error_result graphic_context_object::save_to_image(
    vm_ref vm, std::string_view sv_output_path, image_type img_type) {
  zb::apple::cf::pointer<CGImageRef> img(CGBitmapContextCreateImage(ctx));
  if (!img) {
    return zs::errc::could_not_create_image;
  }

  if (img_type == image_type::unknown) {
    size_t dot_pos = sv_output_path.find_last_of('.');

    if (dot_pos != std::string_view::npos) {
      img_type = get_image_type_from_extension(sv_output_path.substr(dot_pos + 1));
    }
    else {
      img_type = image_type::png;
    }
  }

  zb::apple::cf::string_ref str_img_type(get_image_type_uid(img_type));

  // Check if output path directory exists.
  if (zb::sys::path<zs::string_allocator> output_path_directory(
          zb::sys::path_detail::get_dirname(sv_output_path), vm.get_engine());
      !output_path_directory.exists()) {
    return zs::errc::invalid_directory;
  }

  zb::apple::cf::url_ref output_url(sv_output_path);

  zb::apple::cf::pointer<CGImageDestinationRef> dst(
      CGImageDestinationCreateWithURL(output_url, str_img_type, 1, nullptr));

  if (!dst) {
    return zs::errc::invalid_image_type;
  }

  CGImageDestinationAddImage(dst, img, nullptr);
  if (!CGImageDestinationFinalize(dst)) {
    return zs::errc::could_not_create_image;
  }

  return {};
}

static zs::object create_graphic_context_delegate(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();

  object delegate = zs::_t(eng);
  table_object& tbl = delegate.as_table();
  tbl.emplace(zs::_ss("__typeof"), zs::_s(eng, "graphics_context"));

  tbl.emplace("save_to_image", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    std::string_view output_file_path;
    if (auto err = ps.require<string_parameter>(output_file_path)) {
      return -1;
    }

    image_type img_type = image_type::unknown;
    {
      int_t int_img_type = 0;
      if (auto err = ps.optional<integer_parameter>(int_img_type)) {
        img_type = (image_type)int_img_type;
      }
    }

    if (auto err = ctx->save_to_image(vm, output_file_path, img_type)) {
      switch (err.code) {
      case zs::errc::invalid_directory:
        vm->set_error("Invalid image directory.");
        break;

      case zs::errc::could_not_create_image:
        vm->set_error("Could not create image.");
        break;

      case zs::errc::invalid_image_type:
        vm->set_error("Invalid image type.");
        break;
      }

      return vm.push(err.code);
    }

    return vm.push(zs::errc::success);
  });

  tbl.emplace("get_size", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    object size_struct = vm.get_engine()->get_registry_object(graphics::s_graphic_size_struct_register_uid);

    struct_instance_object* strc = size_struct.as_struct().create_instance();
    strc->set_initialized(true);
    strc->data()[0] = ctx->_size.width;
    strc->data()[1] = ctx->_size.height;
    return vm.push(object(strc, false));
  });

  tbl.emplace("is_bitmap_context", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    return vm.push(ctx->_is_bitmap_context);
  });

  tbl.emplace("flush", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    ctx->flush();
    return 0;
  });

  tbl.emplace("push", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    ctx->push_state();
    return 0;
  });

  tbl.emplace("pop", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    ctx->pop_state();
    return 0;
  });

  tbl.emplace("apply_transform", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    zb::transform<float_t> transform;
    if (auto err = ps.require<graphic_transform_parameter>(transform)) {
      return -1;
    }

    ctx->apply_transform(transform);
    return 0;
  });

  tbl.emplace("transform", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    zb::transform<float_t> transform;
    if (auto err = ps.require<graphic_transform_parameter>(transform)) {
      return -1;
    }

    ctx->apply_transform(transform);
    return 0;
  });

  tbl.emplace("translate", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    zb::point<float_t> pos;
    if (auto err = ps.require<graphic_point_parameter>(pos)) {
      return -1;
    }

    ctx->translate(pos.x, pos.y);

    return 0;
  });

  tbl.emplace("scale", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    zb::size<float_t> size;
    if (auto err = ps.require<graphic_size_parameter>(size)) {
      return -1;
    }

    ctx->scale(size.width, size.height);
    return 0;
  });

  tbl.emplace("rotate", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    float_t angle;
    if (auto err = ps.require<number_parameter>(angle)) {
      return -1;
    }

    ctx->rotate(angle);
    return 0;
  });

  tbl.emplace("get_transform", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    zb::transform<float_t> t = ctx->get_transform();
    return vm.push(create_transform(vm, t));
  });

  tbl.emplace("set_stroke_width", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    float_t swidth;
    if (auto err = ps.require<number_parameter>(swidth)) {
      return -1;
    }

    ctx->set_stroke_width(swidth);
    return 0;
  });

  tbl.emplace("set_stroke_color", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    zb::color color;
    if (auto err = ps.require<graphic_color_parameter>(color)) {
      return -1;
    }

    ctx->set_stroke_color(color);
    return 0;
  });

  tbl.emplace("set_fill_color", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    zb::color color;
    if (auto err = ps.require<graphic_color_parameter>(color)) {
      return -1;
    }

    ctx->set_fill_color(color);
    return 0;
  });

  tbl.emplace("clip_to_rect", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    float_t x, y, w, h;
    if (auto err = ps.require<graphic_rect_parameter>(x, y, w, h)) {
      return -1;
    }

    ctx->clip_to_rect(x, y, w, h);
    return 0;
  });

  tbl.emplace("fill_rect", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    float_t x, y, w, h;
    if (auto err = ps.require<graphic_rect_parameter>(x, y, w, h)) {
      return -1;
    }

    ctx->fill_rect(x, y, w, h);
    return 0;
  });

  tbl.emplace("stroke_rect", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    float_t x, y, w, h;
    if (auto err = ps.require<graphic_rect_parameter>(x, y, w, h)) {
      return -1;
    }

    ctx->stroke_rect(x, y, w, h);
    return 0;
  });

  tbl.emplace("stroke_path", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    graphics::graphic_path_object* path_obj = nullptr;
    if (auto err = ps.require<graphic_path_parameter>(path_obj)) {
      return -1;
    }

    ctx->stroke_path(*path_obj);
    return 0;
  });

  tbl.emplace("fill_path", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    graphics::graphic_path_object* path_obj = nullptr;
    if (auto err = ps.require<graphic_path_parameter>(path_obj)) {
      return -1;
    }

    ctx->fill_path(*path_obj);
    return 0;
  });

  tbl.emplace("draw_image", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    graphics::graphic_image_object* img_obj = nullptr;
    if (auto err = ps.require<graphic_image_parameter>(img_obj)) {
      return -1;
    }

    zb::point<float_t> pos;
    if (auto err = ps.require<graphic_point_parameter>(pos)) {
      return -1;
    }

    ctx->draw_image(img_obj, pos);
    return 0;
  });

  tbl.emplace("draw_image_resized", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    graphics::graphic_image_object* img_obj = nullptr;
    if (auto err = ps.require<graphic_image_parameter>(img_obj)) {
      return -1;
    }

    float_t x, y, w, h;
    if (auto err = ps.require<graphic_rect_parameter>(x, y, w, h)) {
      return -1;
    }

    ctx->draw_image_resized(img_obj, x, y, w, h);

    return 0;
  });

  tbl.emplace("draw_image_part", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    graphics::graphic_image_object* img_obj = nullptr;
    if (auto err = ps.require<graphic_image_parameter>(img_obj)) {
      return -1;
    }

    zb::point<float_t> pos;
    if (auto err = ps.require<graphic_point_parameter>(pos)) {
      return -1;
    }

    zb::rect<float_t> irect;
    if (auto err = ps.require<graphic_rect_parameter>(irect)) {
      return -1;
    }

    ctx->draw_image_part(img_obj, pos, irect);

    return 0;
  });

  tbl.emplace("draw_image_part_resized", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    graphics::graphic_image_object* img_obj = nullptr;
    if (auto err = ps.require<graphic_image_parameter>(img_obj)) {
      return -1;
    }

    float_t x, y, w, h;
    if (auto err = ps.require<graphic_rect_parameter>(x, y, w, h)) {
      return -1;
    }

    float_t ix, iy, iw, ih;
    if (auto err = ps.require<graphic_rect_parameter>(ix, iy, iw, ih)) {
      return -1;
    }

    ctx->draw_image_part_resized(img_obj, x, y, w, h, ix, iy, iw, ih);
    return 0;
  });

  tbl.emplace("scoped_draw", [](vm_ref vm) -> int_t {
    zs::parameter_stream ps(vm);

    graphics::graphic_context_object* ctx = nullptr;
    if (auto err = ps.require<graphic_context_parameter>(ctx)) {
      return -1;
    }

    const object& fct = *ps;
    if (!fct.is_function()) {
      return -1;
    }

    ctx->push_state();
    object ret;

    zs::error_result err;
    if (fct.is_closure() and fct.as_closure().get_proto()._parameter_names.size() == 2) {
      err = vm->call(fct, { vm[0], vm[0] }, ret);
    }
    else {
      err = vm->call(fct, vm[0], ret);
    }

    ctx->pop_state();

    if (err) {
      vm->set_error("Could not call graphics.context.scoped()");
      return -1;
    }
    return 0;
  });

  return delegate;
}

static const zs::object& get_graphic_context_delegate(zs::vm_ref vm) {
  table_object& reg = vm.get_registry_table_object();

  if (auto it = reg.find(zs::_sv(s_graphic_context_delegate_register_uid)); it != reg.end()) {
    return it->second;
  }

  return reg.emplace(zs::_sv(s_graphic_context_delegate_register_uid), create_graphic_context_delegate(vm))
      .first->second;
}

zs::object create_graphic_context(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();

  zs::object obj;
  obj._type = zs::object_type::k_user_data;

  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(graphic_context_object));
  obj._udata = uobj;

  zb_placement_new(uobj->data()) graphic_context_object();

  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    graphic_context_object* t = (graphic_context_object*)ptr;
    t->~graphic_context_object();
  });

  uobj->set_uid(zs::_sv(graphics::s_graphic_context_object_typeid));
  uobj->set_typeid(zs::_sv(graphics::s_graphic_context_object_typeid));

  obj.as_udata().set_delegate(get_graphic_context_delegate(vm));

  zs::parameter_stream ps(vm);

  ++ps;

  zb::size<float_t> size;
  if (auto err = ps.require<graphic_size_parameter>(size)) {
    return -1;
  }

  if (auto err = graphic_context_object::get(obj).init_bitmap_context(size.width, size.height)) {
    zb::print("ERROR");
    return obj;
  }

  return obj;
}

int_t create_graphic_context_impl(zs::vm_ref vm) {
  zs::engine* eng = vm.get_engine();

  zs::object obj;
  obj._type = zs::object_type::k_user_data;

  zs::user_data_object* uobj = zs::user_data_object::create(eng, sizeof(graphic_context_object));
  obj._udata = uobj;

  zb_placement_new(uobj->data()) graphic_context_object();

  uobj->set_release_hook([](zs::engine* eng, zs::raw_pointer_t ptr) {
    graphic_context_object* t = (graphic_context_object*)ptr;
    t->~graphic_context_object();
  });

  uobj->set_uid(zs::_sv(graphics::s_graphic_context_object_typeid));
  uobj->set_typeid(zs::_sv(graphics::s_graphic_context_object_typeid));

  obj.as_udata().set_delegate(get_graphic_context_delegate(vm));

  zs::parameter_stream ps(vm);

  ++ps;

  zb::size<float_t> size;
  if (auto err = ps.require<graphic_size_parameter>(size)) {
    return -1;
  }

  if (auto err = graphic_context_object::get(obj).init_bitmap_context(size.width, size.height)) {
    zb::print("ERROR");
    return -1;
  }

  return vm.push(obj);
}
} // namespace zs::graphics.
