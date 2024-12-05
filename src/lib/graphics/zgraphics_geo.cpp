

#include "zgraphics_common.h"

namespace zs::graphics {

zs::error_result graphic_rect_parameter::parse(
    parameter_stream& s, bool output_error, float_t& x, float_t& y, float_t& w, float_t& h) {
  object xobj, yobj, wobj, hobj;

  if (s->is_struct_instance()) {
    if (auto err = s.vm()->get(*s, zs::_ss("x"), xobj)) {
      s.set_opt_error(output_error, "Could not get 'x'.");
      return zs::errc::not_a_number;
    }

    if (auto err = s.vm()->get(*s, zs::_ss("y"), yobj)) {
      s.set_opt_error(output_error, "Could not get 'y'.");
      return zs::errc::not_a_number;
    }

    if (auto err = s.vm()->get(*s, zs::_ss("width"), wobj)) {
      s.set_opt_error(output_error, "Could not get 'width'.");
      return zs::errc::not_a_number;
    }

    if (auto err = s.vm()->get(*s, zs::_ss("height"), hobj)) {
      s.set_opt_error(output_error, "Could not get 'height'.");
      return zs::errc::not_a_number;
    }

    ++s;
  }

  else if (s.is_array_with_size(4)) {
    xobj = s->as_array()[0];
    yobj = s->as_array()[1];
    wobj = s->as_array()[2];
    hobj = s->as_array()[3];

    ++s;
  }
  else if (s->is_number() and s.size() >= 4) {
    xobj = *s++;
    yobj = *s++;
    wobj = *s++;
    hobj = *s++;
  }
  else {
    return zs::errc::invalid_parameter_count;
  }

  if (!(xobj.is_number() and yobj.is_number() and wobj.is_number() and hobj.is_number())) {
    s.set_opt_error(output_error, "Invalid rect type.");
    return zs::errc::invalid_parameter_type;
  }

  x = xobj.convert_to_float_unchecked();
  y = yobj.convert_to_float_unchecked();
  w = wobj.convert_to_float_unchecked();
  h = hobj.convert_to_float_unchecked();

  return {};
}

zs::error_result graphic_point_parameter::parse(
    parameter_stream& s, bool output_error, float_t& x, float_t& y) {
  object xobj, yobj;

  if (s->is_struct_instance()) {
    if (auto err = s.vm()->get(*s, zs::_ss("x"), xobj)) {
      s.set_opt_error(output_error, "Could not get 'x'.");
      return zs::errc::not_a_number;
    }
    if (auto err = s.vm()->get(*s, zs::_ss("y"), yobj)) {
      s.set_opt_error(output_error, "Could not get 'y'.");
      return zs::errc::not_a_number;
    }

    ++s;
  }

  else if (s.is_array_with_size(2)) {
    xobj = s->as_array()[0];
    yobj = s->as_array()[1];
    ++s;
  }
  else if (s->is_number() and s.size() >= 2) {
    xobj = *(s++);
    yobj = *(s++);
  }
  else {
    return zs::errc::invalid_parameter_count;
  }

  if (!(xobj.is_number() and yobj.is_number())) {
    s.set_opt_error(output_error, "Invalid point type.");
    return zs::errc::invalid_parameter_type;
  }

  x = xobj.convert_to_float_unchecked();
  y = yobj.convert_to_float_unchecked();

  return {};
}

zs::error_result graphic_size_parameter::parse(
    parameter_stream& s, bool output_error, float_t& w, float_t& h) {

  object wobj, hobj;

  if (s->is_struct_instance()) {
    if (auto err = s.vm()->get(*s, zs::_ss("width"), wobj)) {
      s.set_opt_error(output_error, "Could not get 'width'.");
      return zs::errc::not_a_number;
    }

    if (auto err = s.vm()->get(*s, zs::_ss("height"), hobj)) {
      s.set_opt_error(output_error, "Could not get 'height'.");
      return zs::errc::not_a_number;
    }

    ++s;
  }

  else if (s.is_array_with_size(2)) {
    wobj = s->as_array()[0];
    hobj = s->as_array()[1];
    ++s;
  }
  else if (s->is_number() and s.size() >= 2) {
    wobj = *s++;
    hobj = *s++;
  }
  else {
    return zs::errc::invalid_parameter_count;
  }

  if (!(wobj.is_number() and hobj.is_number())) {
    s.set_opt_error(output_error, "Invalid size type.");
    return zs::errc::invalid_parameter_type;
  }

  w = wobj.convert_to_float_unchecked();
  h = hobj.convert_to_float_unchecked();

  return {};
}

void add_point_struct(vm_ref vm, zs::table_object& graphics_module) {
  zs::engine* eng = vm->get_engine();

  zs::object point_struct = zs::object::create_struct(eng);
  zs::struct_object& pobj = point_struct.as_struct();
  pobj.set_name(zs::_ss("graphics_point"));

  pobj.emplace_back(zs::_ss("x"), 0.0);
  pobj.emplace_back(zs::_ss("y"), 0.0);

  pobj.activate_default_constructor();
  pobj.set_constructor([](vm_ref vm) -> int_t {
    vm[0].as_struct_instance().data()[0] = vm[1];
    vm[0].as_struct_instance().data()[1] = vm[2];
    vm.push(vm[0]);
    return 1;
  });

  eng->get_registry_table_object().emplace(zs::_sv(graphics::s_graphic_point_struct_register_uid),
      graphics_module.emplace(zs::_ss("point"), std::move(point_struct)).first->second);
}

void add_size_struct(vm_ref vm, zs::table_object& graphics_module) {
  zs::engine* eng = vm->get_engine();
  zs::object size_struct = zs::object::create_struct(eng);
  zs::struct_object& sobj = size_struct.as_struct();
  sobj.set_name(zs::_ss("graphics_size"));

  sobj.emplace_back(zs::_ss("width"), 0.0);
  sobj.emplace_back(zs::_ss("height"), 0.0);

  sobj.activate_default_constructor();
  sobj.set_constructor(+[](vm_ref vm) -> int_t {
    vm[0].as_struct_instance().data()[0] = vm[1];
    vm[0].as_struct_instance().data()[1] = vm[2];
    return vm.push(vm[0]);
  });

  eng->get_registry_table_object().emplace(zs::_sv(graphics::s_graphic_size_struct_register_uid),
      graphics_module.emplace(zs::_ss("size"), std::move(size_struct)).first->second);
}

void add_rect_struct(vm_ref vm, zs::table_object& graphics_module) {
  zs::engine* eng = vm->get_engine();

  zs::object rect_struct = zs::object::create_struct(eng);
  zs::struct_object& robj = rect_struct.as_struct();
  robj.set_name(zs::_ss("graphics_rect"));

  robj.emplace_back(zs::_ss("x"), 0.0);
  robj.emplace_back(zs::_ss("y"), 0.0);
  robj.emplace_back(zs::_ss("width"), 0.0);
  robj.emplace_back(zs::_ss("height"), 0.0);

  robj.activate_default_constructor();
  robj.set_constructor(+[](vm_ref vm) -> int_t {
    vm[0].as_struct_instance().data()[0] = vm[1];
    vm[0].as_struct_instance().data()[1] = vm[2];
    vm[0].as_struct_instance().data()[2] = vm[3];
    vm[0].as_struct_instance().data()[3] = vm[4];
    return vm.push(vm[0]);
  });

  robj.new_method(
      zs::_ss("position"),
      +[](vm_ref vm) -> int_t {
        object point_struct
            = vm.get_engine()->get_registry_object(graphics::s_graphic_point_struct_register_uid);

        struct_instance_object* strc = point_struct.as_struct().create_instance();
        strc->set_initialized(true);
        strc->data()[0] = vm[0].as_struct_instance().data()[0];
        strc->data()[1] = vm[0].as_struct_instance().data()[1];
        return vm.push(object(strc, false));
      },
      false, true);

  robj.new_method(
      zs::_ss("size"),
      +[](vm_ref vm) -> int_t {
        object size_struct
            = vm.get_engine()->get_registry_object(graphics::s_graphic_size_struct_register_uid);

        struct_instance_object* strc = size_struct.as_struct().create_instance();
        strc->set_initialized(true);
        strc->data()[0] = vm[0].as_struct_instance().data()[2];
        strc->data()[1] = vm[0].as_struct_instance().data()[3];
        return vm.push(object(strc, false));
      },
      false, true);

  eng->get_registry_table_object().emplace(zs::_sv(graphics::s_graphic_rect_struct_register_uid),
      graphics_module.emplace(zs::_ss("rect"), std::move(rect_struct)).first->second);
}
} // namespace zs::graphics.
