

#include "zgraphics_common.h"

namespace zs::graphics {

zs::error_result graphic_color_parameter::parse(
    zs::parameter_stream& s, bool output_error, zb::color& value) {
  if (s->is_color()) {
    value = zb::color(s->_color);
    ++s;
    return {};
  }

  if (s->is_integer()) {
    value = zb::color((uint32_t)s->_int);
    ++s;
    return {};
  }

  return zs::errc::invalid_parameter_type;
}

int_t create_color_with_component_impl(zs::vm_ref vm, const object& r, const object& g, const object& b) {
  switch (create_type_mask(r, g, b)) {
  case (uint32_t)object_type_mask::k_integer:
    return vm.push(create_color((uint8_t)r._int, (uint8_t)g._int, (uint8_t)b._int));
  case (uint32_t)object_type_mask::k_float:
    return vm.push(create_color(zb::color::float_rgb<float_t>{ r._float, g._float, b._float }));
  case object_base::k_number_mask:
    return vm.push(create_color(zb::color::float_rgb<float_t>{ r.to_float(), g.to_float(), b.to_float() }));
  default:
    return -1;
  }
}

int_t create_color_with_component_impl(
    zs::vm_ref vm, const object& r, const object& g, const object& b, const object& a) {
  switch (create_type_mask(r, g, b, a)) {
  case (uint32_t)object_type_mask::k_integer:
    return vm.push(create_color((uint8_t)r._int, (uint8_t)g._int, (uint8_t)b._int, (uint8_t)a._int));
  case (uint32_t)object_type_mask::k_float:
    return vm.push(create_color(zb::color::float_rgba<float_t>{ r._float, g._float, b._float, a._float }));
  case object_base::k_number_mask:
    return vm.push(create_color(
        zb::color::float_rgba<float_t>{ r.to_float(), g.to_float(), b.to_float(), a.to_float() }));
  default:
    return -1;
  }
}
int_t create_color_with_hsv_component_impl(zs::vm_ref vm, const object& h, const object& s, const object& v) {
  switch (create_type_mask(h, s, v)) {
  case (uint32_t)object_type_mask::k_integer:
    return vm.push(create_hsv_color(h._int, s._int, v._int));
  case (uint32_t)object_type_mask::k_float:
    return vm.push(create_hsv_color(h._float * 255.0, s._float * 255.0, v._float * 255.0));
  case object_base::k_number_mask:
    return vm.push(create_hsv_color(h.to_float() * 255.0, s.to_float() * 255.0, v.to_float() * 255.0));
  default:
    return -1;
  }
}

int_t create_color_with_hsv_component_impl(
    zs::vm_ref vm, const object& h, const object& s, const object& v, const object& a) {
  switch (create_type_mask(h, s, v, a)) {
  case (uint32_t)object_type_mask::k_integer:
    return vm.push(create_hsv_color((uint8_t)h._int, (uint8_t)s._int, (uint8_t)v._int, (uint8_t)a._int));
  case (uint32_t)object_type_mask::k_float:
    return vm.push(create_hsv_color(h._float * 255.0, s._float * 255.0, v._float * 255.0, a._float * 255.0));
  case object_base::k_number_mask:
    return vm.push(create_hsv_color(
        h.to_float() * 255.0, s.to_float() * 255.0, v.to_float() * 255.0, a.to_float() * 255.0));
  default:
    return -1;
  }
}

int_t create_color_impl(zs::vm_ref vm) {
  const int_t nargs = vm.stack_size();

  if (nargs == 1) {
    return vm.push(object::create_color((uint32_t)0));
  }
  else if (nargs == 2) {
    const auto& c0 = vm[1];

    if (c0.is_color()) {
      return vm.push(c0);
    }
    else if (c0.is_integer()) {
      return vm.push(object::create_color((uint32_t)c0._int));
    }
    else if (c0.is_string()) {

      std::string_view col_str = c0.get_string_unchecked();
      zb::color col;
      zb::error_result err = zb::color::from_string(col_str, col);
      if (err) {

        if (col_str == "red") {
          return vm.push(object::create_color((uint32_t)0xFF0000FF));
        }
        else if (col_str == "green") {
          return vm.push(object::create_color((uint32_t)0x00FF00FF));
        }
        else if (col_str == "blue") {
          return vm.push(object::create_color((uint32_t)0x0000FFFF));
        }
        else if (col_str == "black") {
          return vm.push(object::create_color((uint32_t)0x000000FF));
        }
        else if (col_str == "white") {
          return vm.push(object::create_color((uint32_t)0xFFFFFFFF));
        }
        else if (col_str == "transparent") {
          return vm.push(object::create_color((uint32_t)0x00000000));
        }
        return -1;
      }
      return vm.push(create_color(col));
    }

    else if (c0.is_array()) {
      const array_object& arr = c0.as_array();
      if (arr.size() == 3) {
        return create_color_with_component_impl(vm, arr[0], arr[1], arr[2]);
      }
      else if (arr.size() == 4) {
        return create_color_with_component_impl(vm, arr[0], arr[1], arr[2], arr[3]);
      }
      else {
        vm.set_error("Invalid array size in color().\n");
        return -1;
      }
    }
    else {
      vm.set_error("Invalid type in color().\n");
      return -1;
    }
  }
  else if (nargs == 4) {
    return create_color_with_component_impl(vm, vm[1], vm[2], vm[3]);
  }
  else if (nargs == 5) {
    return create_color_with_component_impl(vm, vm[1], vm[2], vm[3], vm[4]);
  }

  vm.set_error("Invalid number of arguments in color().\n");
  return -1;
}

int_t create_color_hsv_impl(zs::vm_ref vm) {
  const int_t nargs = vm.stack_size();

  if (nargs == 2) {
    const auto& c0 = vm[1];

    if (c0.is_array()) {
      const array_object& arr = c0.as_array();
      if (arr.size() == 3) {
        return create_color_with_hsv_component_impl(vm, arr[0], arr[1], arr[2]);
      }
      else if (arr.size() == 4) {
        return create_color_with_hsv_component_impl(vm, arr[0], arr[1], arr[2], arr[3]);
      }
      else {
        vm.set_error("Invalid array size in color().\n");
        return -1;
      }
    }
    else {
      vm.set_error("Invalid type in color().\n");
      return -1;
    }
  }
  else if (nargs == 4) {
    return create_color_with_hsv_component_impl(vm, vm[1], vm[2], vm[3]);
  }
  else if (nargs == 5) {
    return create_color_with_hsv_component_impl(vm, vm[1], vm[2], vm[3], vm[4]);
  }

  vm.set_error("Invalid number of arguments in color().\n");
  return -1;
}
} // namespace zs::graphics.
