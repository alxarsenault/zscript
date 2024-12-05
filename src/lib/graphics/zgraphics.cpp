#include "zgraphics_common.h"

namespace zs::graphics {

void add_transform_struct(vm_ref vm, zs::table_object& graphics_module);

void add_point_struct(vm_ref vm, zs::table_object& graphics_module);
void add_size_struct(vm_ref vm, zs::table_object& graphics_module);
void add_rect_struct(vm_ref vm, zs::table_object& graphics_module);

int_t create_color_hsv_impl(zs::vm_ref vm);
int_t create_color_impl(zs::vm_ref vm);
} // namespace zs::graphics

namespace zs {

zs::object create_graphics_lib(zs::virtual_machine* vm) {
  using namespace zs::literals;

  zs::engine* eng = vm->get_engine();

  zs::object graphics_module = zs::_t(eng);
  zs::table_object& tbl = graphics_module.as_table();

  tbl.emplace("rgb", graphics::create_color_impl);
  tbl.emplace("hsv", graphics::create_color_hsv_impl);
  tbl.emplace("black", object::create_color((uint32_t)0x000000FF));
  tbl.emplace("white", object::create_color((uint32_t)0xFFFFFFFF));
  tbl.emplace("red", object::create_color((uint32_t)0xFF0000FF));
  tbl.emplace("green", object::create_color((uint32_t)0x00FF00FF));
  tbl.emplace("blue", object::create_color((uint32_t)0x0000FFFF));
  tbl.emplace("transparent", object::create_color((uint32_t)0x00000000));
  tbl.emplace("hot_pink", object::create_color((uint32_t)0xFF69B4FF));
  tbl.emplace("deep_pink", object::create_color((uint32_t)0xFF1493FF));

  tbl.emplace("coral", object::create_color((uint32_t)0xFF7F50FF));
  tbl.emplace("tomato", object::create_color((uint32_t)0xFF6347FF));
  tbl.emplace("orangered", object::create_color((uint32_t)0xFF4500FF));
  tbl.emplace("gold", object::create_color((uint32_t)0xFFD700FF));
  tbl.emplace("orange", object::create_color((uint32_t)0xFFA500FF));
  tbl.emplace("dark_orange", object::create_color((uint32_t)0xFF8C00FF));
  tbl.emplace("royal_blue", object::create_color((uint32_t)0x4169E1FF));
  tbl.emplace("navy", object::create_color((uint32_t)0x000080FF));
  tbl.emplace("cyan", object::create_color((uint32_t)0x00FFFFFF));
  tbl.emplace("turquoise", object::create_color((uint32_t)0x40E0D0FF));
  tbl.emplace("lightgrey", object::create_color((uint32_t)0xD3D3D3FF));
  tbl.emplace("grey", object::create_color((uint32_t)0x808080FF));
  tbl.emplace("yellow", object::create_color((uint32_t)0xFFFF00FF));

  tbl.emplace("context", graphics::create_graphic_context_impl);

  tbl.emplace("path", [](vm_ref vm) { return vm.push(graphics::create_graphic_path(vm)); });
  tbl.emplace("image", [](vm_ref vm) { return vm.push(graphics::create_graphic_image(vm)); });

  graphics::add_transform_struct(vm_ref(vm), tbl);
  graphics::add_point_struct(vm_ref(vm), tbl);
  graphics::add_size_struct(vm_ref(vm), tbl);
  graphics::add_rect_struct(vm_ref(vm), tbl);

  zs::object image_type_table = zs::_t(eng);
  zs::table_object& image_type_table_obj = image_type_table.as_table();
  image_type_table_obj.reserve(8);

  image_type_table_obj.emplace(zs::_ss("png"), (int_t)graphics::image_type::png);
  image_type_table_obj.emplace(zs::_ss("jpeg"), (int_t)graphics::image_type::jpeg);
  image_type_table_obj.emplace(zs::_ss("bmp"), (int_t)graphics::image_type::bmp);
  image_type_table_obj.emplace(zs::_ss("ico"), (int_t)graphics::image_type::ico);
  image_type_table_obj.emplace(zs::_ss("icons"), (int_t)graphics::image_type::icons);
  image_type_table_obj.emplace(zs::_ss("gif"), (int_t)graphics::image_type::gif);
  image_type_table_obj.emplace(zs::_ss("tiff"), (int_t)graphics::image_type::tiff);

  vm->make_enum_table(image_type_table);
  tbl.emplace("image_type", image_type_table);

  return graphics_module;
}

} // namespace zs.
