

#include "zgraphics_common.h"

namespace zs::graphics {

zb::transform<float_t> get_transform(const object& trans_struc_inst) {
  zb::transform<float_t> t;

  const object* values = trans_struc_inst.as_struct_instance().data();
  t.a = (*values++).convert_to_float_unchecked();
  t.b = (*values++).convert_to_float_unchecked();
  t.c = (*values++).convert_to_float_unchecked();
  t.d = (*values++).convert_to_float_unchecked();
  t.ty = (*values++).convert_to_float_unchecked();
  t.tx = (*values++).convert_to_float_unchecked();
  return t;
}

inline void set_transform(struct_instance_object* trans_struc_inst, const zb::transform<float_t>& t) {
  object* values = trans_struc_inst->data();
  *values++ = t.a;
  *values++ = t.b;
  *values++ = t.c;
  *values++ = t.d;
  *values++ = t.ty;
  *values++ = t.tx;
}

inline const object& set_transform(const object& trans_struc_inst, const zb::transform<float_t>& t) {
  set_transform(trans_struc_inst._struct_instance, t);
  return trans_struc_inst;
}

zs::error_result graphic_transform_parameter::parse(
    zs::parameter_stream& s, bool output_error, zb::transform<float_t>& value) {

  if (!s.is_struct_instance_with_name("graphics_transform")) {
    s.set_opt_error(output_error, "Invalid graphic transform type.");
    return zs::errc::invalid_parameter_type;
  }

  value = get_transform(*s);
  ++s;
  return {};
}

object create_transform(vm_ref vm, const zb::transform<float_t>& t) {
  object transform_struct
      = vm.get_engine()->get_registry_object(graphics::s_graphic_transform_struct_register_uid);

  struct_instance_object* out_strc = transform_struct.as_struct().create_instance();
  out_strc->set_initialized(true);
  set_transform(out_strc, t);
  return object(out_strc, false);
}

void add_transform_struct(vm_ref vm, zs::table_object& graphics_module) {
  zs::engine* eng = vm->get_engine();

  zs::object transform_struct = zs::object::create_struct(eng);
  zs::struct_object& strc = transform_struct.as_struct();
  strc.set_name(zs::_s(eng, "graphics_transform"));

  strc.emplace_back(zs::_ss("a"), 1.0);
  strc.emplace_back(zs::_ss("b"), 0.0);
  strc.emplace_back(zs::_ss("c"), 0.0);
  strc.emplace_back(zs::_ss("d"), 1.0);
  strc.emplace_back(zs::_ss("tx"), 0.0);
  strc.emplace_back(zs::_ss("ty"), 0.0);

  strc.activate_default_constructor();
  strc.set_constructor([](vm_ref vm) -> int_t {
    const object* params = vm->stack().stack_base_pointer();
    const object* strc = params++;
    object* values = strc->as_struct_instance().data();

    int_t nparams = zb::minimum(vm.stack_size() - 1, 6);

    for (int_t i = 0; i < nparams; i++) {
      *values++ = (*params++).convert_to_float_unchecked();
    }

    return vm.push(*strc);
  });

  strc.new_static_method(
      zs::_s(eng, "translation"),
      +[](vm_ref vm) -> int_t {
        return vm.push(create_transform(vm,
            zb::transform<float_t>::translation(
                { vm[1].convert_to_float_unchecked(), vm[2].convert_to_float_unchecked() })));
      },
      false, true);

  strc.new_static_method(
      zs::_s(eng, "scaling"),
      +[](vm_ref vm) -> int_t {
        return vm.push(create_transform(vm,
            zb::transform<float_t>::scale(
                { vm[1].convert_to_float_unchecked(), vm[2].convert_to_float_unchecked() })));
      },
      false, true);

  strc.new_static_method(
      zs::_s(eng, "rotation"),
      +[](vm_ref vm) -> int_t {
        return vm.push(
            create_transform(vm, zb::transform<float_t>::rotation(vm[1].convert_to_float_unchecked())));
      },
      false, true);

  strc.new_method(
      zs::_s(eng, "apply_translation"),
      +[](vm_ref vm) -> int_t {
        const object& strc = vm[0];
        return vm.push(set_transform(strc,
            get_transform(strc).translated(
                { vm[1].convert_to_float_unchecked(), vm[2].convert_to_float_unchecked() })));
      },
      false, false);

  strc.new_method(
      zs::_ss("translate"),
      +[](vm_ref vm) -> int_t {
        const object& strc = vm[0];
        return vm.push(create_transform(vm,
            get_transform(strc).with_translation(
                { vm[1].convert_to_float_unchecked(), vm[2].convert_to_float_unchecked() })));
      },
      false, true);

  strc.new_method(
      zs::_ss("apply_scale"),
      +[](vm_ref vm) -> int_t {
        const object& strc = vm[0];
        return vm.push(set_transform(strc,
            get_transform(strc).scaled(
                { vm[1].convert_to_float_unchecked(), vm[2].convert_to_float_unchecked() })));
      },
      false, false);

  strc.new_method(
      zs::_ss("scale"),
      +[](vm_ref vm) -> int_t {
        const object& strc = vm[0];
        return vm.push(create_transform(vm,
            get_transform(strc).with_scale(
                { vm[1].convert_to_float_unchecked(), vm[2].convert_to_float_unchecked() })));
      },
      false, true);

  strc.new_method(
      zs::_ss("apply_rotation"),
      +[](vm_ref vm) -> int_t {
        const object& strc = vm[0];
        return vm.push(set_transform(strc, get_transform(strc).rotated(vm[1].convert_to_float_unchecked())));
      },
      false, false);

  strc.new_method(
      zs::_ss("rotate"),
      +[](vm_ref vm) -> int_t {
        const object& strc = vm[0];
        return vm.push(
            create_transform(vm, get_transform(strc).with_rotation(vm[1].convert_to_float_unchecked())));
      },
      false, true);

  strc.new_method(
      zs::_ss("apply"),
      +[](vm_ref vm) -> int_t {
        const object& strc = vm[0];
        const object& rhs = vm[1];
        return vm.push(set_transform(strc,

            get_transform(strc) * get_transform(rhs)));
      },
      false, false);

  strc.new_method(
      zs::_ss("concat"),
      +[](vm_ref vm) -> int_t {
        const object& strc = vm[0];
        const object& rhs = vm[1];
        return vm.push(create_transform(vm, get_transform(strc) * get_transform(rhs)));
      },
      false, true);

  strc.new_method(
      zs::_ss("__mul"),
      +[](vm_ref vm) -> int_t {
        const object& strc = vm[0];
        const object& rhs = vm[1];

        if (!rhs.is_struct_instance()) {
        }
        //          if(!rhs.as_struct_instance().get_base().as_struct().ge) {

        //          }

        return vm.push(create_transform(vm, get_transform(strc) * get_transform(rhs)));
      },
      false, true);

  eng->get_registry_table_object().emplace(zs::_sv(graphics::s_graphic_transform_struct_register_uid),
      graphics_module.emplace(zs::_ss("transform"), std::move(transform_struct)).first->second);
}
} // namespace zs::graphics.
