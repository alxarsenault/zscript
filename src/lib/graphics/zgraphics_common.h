// Copyright(c) 2024, Meta-Sonic.
// All rights reserved.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.  See the file COPYING included with
// this distribution for more information.
//
// Alternatively, if you have a valid commercial licence for aulib obtained
// by agreement with the copyright holders, you may redistribute and/or modify
// it under the terms described in that licence.
//
// If you wish to distribute code using aulib under terms other than those of
// the GNU General Public License, you must obtain a valid commercial licence
// before doing so.

#pragma once

#include <zscript.h>
#include "lib/zgraphics.h"

#include "zparameter_stream.h"

#include "zvirtual_machine.h"
#include <zbase/graphics/geometry.h>

#if __ZBASE_APPLE__
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreImage/CoreImage.h>
#include <ImageIO/ImageIO.h>

#include <zbase/sys/apple/objc/objc.h>
#include <zbase/sys/apple/cf/url_ref.h>
#endif // __ZBASE_APPLE__.

namespace zs::graphics {
static constexpr std::string_view s_graphic_transform_struct_register_uid = "__graphic_transform_struct__";
static constexpr std::string_view s_graphic_point_struct_register_uid = "__graphic_point_struct__";
static constexpr std::string_view s_graphic_size_struct_register_uid = "__graphic_size_struct__";
static constexpr std::string_view s_graphic_rect_struct_register_uid = "__graphic_rect_struct__";

static constexpr std::string_view s_graphic_context_object_typeid = "__graphic_context_object__";
static constexpr std::string_view s_graphic_path_object_typeid = "__graphic_path_object__";
static constexpr std::string_view s_graphic_image_object_typeid = "__graphic_image_object__";

static constexpr std::string_view s_graphic_path_delegate_register_uid = "__graphic_path_delegate__";
static constexpr std::string_view s_graphic_image_delegate_register_uid = "__graphic_image_delegate__";
static constexpr std::string_view s_graphic_context_delegate_register_uid = "__graphic_context_delegate__";

zb::transform<float_t> get_transform(const object& trans_struc_inst);
object create_transform(vm_ref vm, const zb::transform<float_t>& t);

struct graphic_context_object;
struct graphic_path_object;
struct graphic_image_object;

zs::object create_graphic_path(zs::vm_ref vm);
zs::object create_graphic_image(zs::vm_ref vm);
zs::object create_graphic_context(zs::vm_ref vm);
int_t create_graphic_context_impl(zs::vm_ref vm);

enum class image_type { unknown, png, jpeg, bmp, ico, icons, gif, tiff };

struct graphic_context_parameter {
  static zs::error_result parse(
      zs::parameter_stream& s, bool output_error, graphics::graphic_context_object*& value);
};

struct graphic_path_parameter {
  static zs::error_result parse(
      zs::parameter_stream& s, bool output_error, graphics::graphic_path_object*& value);
};

struct graphic_image_parameter {
  static zs::error_result parse(
      zs::parameter_stream& s, bool output_error, graphics::graphic_image_object*& value);
};

struct graphic_rect_parameter {
  static zs::error_result parse(
      zs::parameter_stream& s, bool output_error, float_t& x, float_t& y, float_t& w, float_t& h);

  inline static zs::error_result parse(zs::parameter_stream& s, bool output_error, zb::rect<float_t>& value) {
    return graphic_rect_parameter::parse(s, output_error, value.x, value.y, value.width, value.height);
  }
};

struct graphic_point_parameter {

  static zs::error_result parse(zs::parameter_stream& s, bool output_error, float_t& x, float_t& y);

  inline static zs::error_result parse(
      zs::parameter_stream& s, bool output_error, zb::point<float_t>& value) {
    return graphic_point_parameter::parse(s, output_error, value.x, value.y);
  }
};

struct graphic_size_parameter {
  ;

  static zs::error_result parse(zs::parameter_stream& s, bool output_error, float_t& w, float_t& h);

  inline static zs::error_result parse(zs::parameter_stream& s, bool output_error, zb::size<float_t>& value) {
    return graphic_size_parameter::parse(s, output_error, value.width, value.height);
  }
};

struct graphic_color_parameter {
  static zs::error_result parse(zs::parameter_stream& s, bool output_error, zb::color& value);
};

struct graphic_transform_parameter {
  static zs::error_result parse(zs::parameter_stream& s, bool output_error, zb::transform<float_t>& value);
};

struct graphic_path_object {
  inline static graphic_path_object& get(const object& obj) {
    return obj.as_udata().data_ref<graphic_path_object>();
  }

  graphic_path_object();
  ~graphic_path_object();

  zs::error_result move_to(float_t x, float_t y);

  zs::error_result line_to(float_t x, float_t y);

  zs::error_result add_line(float_t x1, float_t y1, float_t x2, float_t y2);

  zs::error_result close_sub_path();

  zs::error_result add_rect(float_t x, float_t y, float_t w, float_t h);

  zs::error_result quadratic_curve_to(float_t x, float_t y, float_t cpx, float_t cpy);

  /// Append a cubic Bézier curve from the current point to `(x,y)' with
  /// control points `(cp1x, cp1y)' and `(cp2x, cp2y)' in `path' and move the
  /// current point to `(x, y)'. If `m' is non-NULL, then transform all points
  /// by `m' first.
  zs::error_result bezier_curve_to(
      float_t x, float_t y, float_t cp1x, float_t cp1y, float_t cp2x, float_t cp2y);

  zs::error_result add_ellipse_in_rect(float_t x, float_t y, float_t w, float_t h);

  zs::error_result add_path(const graphic_path_object& p);
  zs::error_result add_path(const zb::transform<float_t>& t, const graphic_path_object& p);

  /// Add a rounded rectangle to `path'. The rounded rectangle coincides with
  /// the edges of `rect'. Each corner is consists of one-quarter of an ellipse
  /// with axes equal to `cornerWidth' and `cornerHeight'. The rounded
  /// rectangle forms a complete subpath of the path --- that is, it begins
  /// with a "move to" and ends with a "close subpath" --- oriented in the
  /// clockwise direction. If `transform' is non-NULL, then the path elements
  /// representing the rounded rectangle will be transformed by `transform'
  /// before they are added to the path.
  zs::error_result add_rounded_rect(float_t x, float_t y, float_t w, float_t h, float_t cw, float_t ch);

  zs::error_result add_rounded_rect(
      const zb::transform<float_t>& t, float_t x, float_t y, float_t w, float_t h, float_t cw, float_t ch);

#if __ZBASE_APPLE__
  inline CGMutablePathRef get_native_path() const noexcept { return path_ref; }

  CGMutablePathRef path_ref = nullptr;
#endif // __ZBASE_APPLE__.
};

struct graphic_image_object {
  inline static graphic_image_object& get(const object& obj) {
    return obj.as_udata().data_ref<graphic_image_object>();
  }

  graphic_image_object();
  ~graphic_image_object();

  zs::error_result load_from_file(std::string_view path);

#if __ZBASE_APPLE__
  inline explicit graphic_image_object(CGImageRef img)
      : img_ref(img) {}

  inline CGImageRef get_native_image() const noexcept { return img_ref; }

  CGImageRef img_ref = nullptr;
#endif // __ZBASE_APPLE__.
};

struct graphic_context_object {

  inline static graphic_context_object& get(const object& obj) {
    return obj.as_udata().data_ref<graphic_context_object>();
  }
  //  EXTERN CGContextRef __nullable CGBitmapContextCreate(void * __nullable data,
  //      size_t width, size_t height, size_t bitsPerComponent, size_t bytesPerRow,
  //      CGColorSpaceRef cg_nullable space, uint32_t bitmapInfo)
  //      CG_AVAILABLE_STARTING(10.0, 2.0);

  ~graphic_context_object();

  zs::error_result init_bitmap_context(int_t width, int_t height);

  zs::error_result set_stroke_color(zb::color color);
  zs::error_result set_fill_color(zb::color color);

  zs::error_result set_stroke_width(float_t w);
  zs::error_result fill_rect(float_t x, float_t y, float_t w, float_t h);
  zs::error_result stroke_rect(float_t x, float_t y, float_t w, float_t h);

  zs::error_result fill_path(const graphic_path_object& path) {
    CGContextAddPath(ctx, path.get_native_path());
    CGContextFillPath(ctx);

    return {};
  }

  inline void flip(float flipHeight) {
    CGContextConcatCTM(ctx, CGAffineTransformMake(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, flipHeight));
  }

  zs::error_result draw_image(const graphic_image_object* img, float_t x, float_t y) {
    float_t w = CGImageGetWidth(img->get_native_image());
    float_t h = CGImageGetHeight(img->get_native_image());

    CGContextTranslateCTM(ctx, static_cast<CGFloat>(x), static_cast<CGFloat>(y));
    flip(h);
    CGContextDrawImage(ctx, { 0.0, 0.0, w, h }, img->get_native_image());
    flip(h);
    CGContextTranslateCTM(ctx, static_cast<CGFloat>(-x), static_cast<CGFloat>(-y));

    return {};
  }

  inline zs::error_result draw_image(const graphic_image_object* img, const zb::point<float_t>& pos) {
    return draw_image(img, pos.x, pos.y);
  }

  zs::error_result draw_image_resized(
      const graphic_image_object* img, float_t x, float_t y, float_t w, float_t h) {

    CGContextTranslateCTM(ctx, static_cast<CGFloat>(x), static_cast<CGFloat>(y));
    flip(h);
    CGContextDrawImage(ctx, { 0.0, 0.0, w, h }, img->get_native_image());
    flip(h);
    CGContextTranslateCTM(ctx, static_cast<CGFloat>(-x), static_cast<CGFloat>(-y));

    return {};
  }

  zs::error_result draw_image_part(
      const graphic_image_object* img, float_t x, float_t y, float_t ix, float_t iy, float_t iw, float_t ih) {

    graphic_image_object img2(CGImageCreateWithImageInRect(img->get_native_image(), { ix, iy, iw, ih }));
    return draw_image(&img2, x, y);
  }

  zs::error_result draw_image_part(
      const graphic_image_object* img, const zb::point<float_t>& pos, const zb::rect<float_t>& irect) {

    graphic_image_object img2(CGImageCreateWithImageInRect(img->get_native_image(), (CGRect)irect));
    return draw_image(&img2, pos.x, pos.y);
  }

  zs::error_result draw_image_part_resized(const graphic_image_object* img, float_t x, float_t y, float_t w,
      float_t h, float_t ix, float_t iy, float_t iw, float_t ih) {

    graphic_image_object img2(CGImageCreateWithImageInRect(img->get_native_image(), { ix, iy, iw, ih }));
    return draw_image_resized(&img2, x, y, w, h);
  }

  zs::error_result stroke_path(const graphic_path_object& path) {
    CGContextAddPath(ctx, path.get_native_path());
    CGContextStrokePath(ctx);
    return {};
  }

  // CTM (current transformation matrix)
  // clip region
  // image interpolation quality
  // line width
  // line join
  // miter limit
  // line cap
  // line dash
  // flatness
  // should anti-alias
  // rendering intent
  // fill color space
  // stroke color space
  // fill color
  // stroke color
  // alpha value
  // font
  // font size
  // character spacing
  // text drawing mode
  // shadow parameters
  // the pattern phase
  // the font smoothing parameter
  // blend mode
  zs::error_result push_state();
  zs::error_result pop_state();

  zs::error_result translate(float_t x, float_t y);
  zs::error_result scale(float_t x, float_t y);
  zs::error_result rotate(float_t angle);
  zs::error_result apply_transform(const zb::transform<float_t>& t);

  zb::transform<float_t> get_transform() { return CGContextGetCTM(ctx); }

  zs::error_result clip_to_rect(float_t x, float_t y, float_t w, float_t h);
  zs::error_result flush();
  zs::error_result save_to_image(vm_ref vm, std::string_view path, image_type img_type);

  CGContextRef ctx = nullptr;
  zb::size<float_t> _size = { 0, 0 };
  bool _is_bitmap_context = false;
};
} // namespace zs::graphics.
